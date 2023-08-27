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


    xTaskCreate(&system_task, "system", 2048, NULL, 10, &task_0_handle);
    xTaskCreate(&dht_task, "dht-22", 2048, NULL, 5, &task_1_handle);
    xTaskCreate(&ir_remote_task, "nec_rx", 4096, NULL, 10, &task_2_handle);
    xTaskCreate(&display_task, "display", 4096, NULL, 5, &task_3_handle);

    fflush(stdout);
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
