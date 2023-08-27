// Standard library imports
#include "json/cJSON.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// esp32 library imports
#include "esp_chip_info.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"

#include "driver/gpio.h"
#include "hal/gpio_types.h"
#include "sdkconfig.h"

// External library imports
#include "libs/dht/DHT.h"
#include "libs/http-client/http_client.h"
#include "libs/infrared-receiver/infrared_nec.h"
#include "libs/wifi/wifi.h"

// Project module imports
#include "display/display.h"
#include "gpio/gpio_util.h"
#include "model/forecast.h"
#include "model/onboard_led.h"
#include "model/system_action.h"
#include "model/system_message.h"
#include "util/util.h"

// Weather station tasks
#include "tasks/dht_task.h"
#include "tasks/display_task.h"
#include "tasks/ir_remote_task.h"

#define SYSTEM_TAG "SYSTEM"

TaskHandle_t task_0_handle = NULL;
TaskHandle_t task_1_handle = NULL;
TaskHandle_t task_2_handle = NULL;
TaskHandle_t task_3_handle = NULL;

static void system_task(void *pvParameter);

#define SERVER "api.open-meteo.com"
#define PORT "80"
#define PATH                                                                   \
    "https://api.open-meteo.com/v1/"                                           \
    "forecast?latitude=51.5085&longitude=-0.1257&hourly=temperature_2m,"       \
    "relativehumidity_2m,apparent_temperature,precipitation_probability&"      \
    "daily=temperature_2m_max,temperature_2m_min,sunrise,sunset,"              \
    "precipitation_probability_max&timezone=Europe%2FLondon&forecast_days=3&"  \
    "models=best_match"

char *WEB_SERVER = SERVER;
char *WEB_PORT = PORT;
char *WEB_PATH = PATH;

char *REQUEST = "GET " PATH " HTTP/1.0\r\n"
                "Host: " SERVER " \r\n"
                "User-Agent: esp-idf/1.0 esp32\r\n"
                "\r\n";

void explore_json(cJSON *json);
void app_main(void)
{
    configure_gpio_outputs();
    initialize_non_volatile_flash();
    esp_log_level_set("*", ESP_LOG_INFO);

    disable_led_by_default();
    wifi_init_sta_default();

    ir_remote_input_queue = xQueueCreate(10, sizeof(struct IRRemoteMessage *));
    system_msg_queue = xQueueCreate(10, sizeof(struct SystemMessage *));
    display_msg_queue = xQueueCreate(10, sizeof(struct DisplayMessage *));

    struct Request *request = malloc(sizeof(struct Request));
    request->web_server = calloc(strlen(WEB_SERVER), sizeof(char));
    request->web_port = calloc(strlen(WEB_PORT), sizeof(char));
    request->web_path = calloc(strlen(WEB_PATH), sizeof(char));
    request->body = calloc(strlen(REQUEST), sizeof(char));
    request->max_attempts = 3;

    strncpy(request->web_server, WEB_SERVER, strlen(WEB_SERVER) + 1);
    strncpy(request->web_port, WEB_PORT, strlen(WEB_PORT) + 1);
    strncpy(request->web_path, WEB_PATH, strlen(WEB_PATH) + 1);
    strncpy(request->body, REQUEST, strlen(REQUEST) + 1);

    cJSON *json;

    send_http_request(request, &json);

    explore_json(json);

    xTaskCreate(&system_task, "system", 2048, NULL, 10, &task_0_handle);
    xTaskCreate(&dht_task, "dht-22", 2048, NULL, 5, &task_1_handle);
    xTaskCreate(&ir_remote_task, "nec_rx", 4096, NULL, 10, &task_2_handle);
    xTaskCreate(&display_task, "display", 4096, NULL, 5, &task_3_handle);

    fflush(stdout);
}

void extract_strings(cJSON *array, char *strings[])
{
    int index = 0;
    cJSON *item;

    cJSON_ArrayForEach(item, array)
    {
        strings[index] = cJSON_Print(item);
        index++;
    }
}
void explore_json(cJSON *json)
{
    printf("%s\n", json->child->string);
    printf("%s\n", json->child->next->string);
    cJSON *hourly = cJSON_GetObjectItemCaseSensitive(json, "hourly");
    cJSON *times = cJSON_GetObjectItemCaseSensitive(hourly, "time");
    cJSON *temperatures =
        cJSON_GetObjectItemCaseSensitive(hourly, "temperature_2m");
    cJSON *apparent_temperatures =
        cJSON_GetObjectItemCaseSensitive(hourly, "apparent_temperature");
    cJSON *humidities =
        cJSON_GetObjectItemCaseSensitive(hourly, "relativehumidity_2m");
    cJSON *precip_probabilities =
        cJSON_GetObjectItemCaseSensitive(hourly, "precipitation_probability");

    printf("%d\n", cJSON_IsArray(times));
    printf("%d\n", cJSON_IsArray(temperatures));
    printf("%d\n", cJSON_IsArray(apparent_temperatures));
    printf("%d\n", cJSON_IsArray(humidities));
    printf("%d\n", cJSON_IsArray(precip_probabilities));

    printf("%s\n", cJSON_Print(temperatures));
    cJSON *temperature = NULL;

    // Forecast for 3 days -> 3 * 24 = 72 data points.

    char *time_strings[72];
    char *temperature_strings[72];
    char *apparent_temperature_strings[72];
    char *humidity_strings[72];
    char *precip_probability_strings[72];

    extract_strings(times, time_strings);
    extract_strings(temperatures, temperature_strings);
    extract_strings(apparent_temperatures, apparent_temperature_strings);
    extract_strings(humidities, humidity_strings);
    extract_strings(precip_probabilities, precip_probability_strings);

    struct ForecastHourly forecasts[72];
    for (int i = 0; i < 72; i++) {
        parse_hourly_forecast(time_strings[i], temperature_strings[i],
                              apparent_temperature_strings[i],
                              humidity_strings[i],
                              precip_probability_strings[i], &forecasts[i]);
    }
    print_hourly_forecast(&forecasts[15]);
}

static void send_msg_to_screen(enum DisplayAction message);

static void system_task(void *pvParameter)
{
    struct SystemMessage *received_message;

    while (true) {
        if (xQueueReceive(system_msg_queue, &(received_message),
                          (TickType_t)5)) {
            enum SystemAction action = received_message->system_action;
            ESP_LOGI(SYSTEM_TAG, "Received an incoming system message: %s",
                     system_action_str[action]);
            switch (action) {
            case TOGGLE_ONBOARD_LED:
                toggle_onboard_led();
                break;
            case DISPLAY_OFF:
                send_msg_to_screen(SCREEN_OFF);
                break;
            case DISPLAY_ON:
                send_msg_to_screen(SCREEN_ON);
            }
        }
    }
}

static void send_msg_to_screen(enum DisplayAction action)
{
    struct DisplayMessage *message = &display_message;
    message->requested_action = action;
    xQueueSend(display_msg_queue, (void *)&message, (TickType_t)0);
}
