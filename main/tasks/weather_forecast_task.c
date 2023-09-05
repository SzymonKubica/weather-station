#include "weather_forecast_task.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../libs/http-client/http_client.h"
#include "../model/forecast.h"
#include "../util/date_time.h"
#include "esp_log.h"
#include "system_action.h"
#include "system_message.h"

#define SERVER "api.open-meteo.com"
#define PORT "80"
#define PATH                                                                   \
    "https://api.open-meteo.com/v1/"                                           \
    "forecast?latitude=51.5085&longitude=-0.1257&hourly=temperature_2m,"       \
    "relativehumidity_2m,apparent_temperature,precipitation_probability&"      \
    "daily=temperature_2m_max,temperature_2m_min,sunrise,sunset,"              \
    "precipitation_probability_max&timezone=Europe%2FLondon&forecast_days=7&"  \
    "models=best_match"

char *WEB_SERVER = SERVER;
char *WEB_PORT = PORT;
char *WEB_PATH = PATH;

char *REQUEST = "GET " PATH " HTTP/1.0\r\n"
                "Host: " SERVER " \r\n"
                "User-Agent: esp-idf/1.0 esp32\r\n"
                "\r\n";

#define FORECAST_DAYS 7
struct ForecastHourly *forecasts[24 * FORECAST_DAYS];
struct ForecastDaily *forecasts_daily[FORECAST_DAYS];

#define TAG "FORECAST_TASK"

void assemble_request(struct Request *request);
void update_weather_data();
void send_weather_daily_update(struct ForecastDaily *forecast);
void send_weather_hourly_update(struct ForecastHourly *forecast);

void weather_forecast_task(void *pvParameter)
{
    ESP_LOGI(TAG, "Getting weather data...");
    update_weather_data();

    struct ForecastRequest *received_message;
    while (true) {
        if (xQueueReceive(weather_forecast_msg_queue, &(received_message),
                          (TickType_t)5)) {

            int offset;
            switch (received_message->request_type) {
            case WEATHER_HOURLY:
                update_time();
                int number_of_datapoints = 24 * FORECAST_DAYS;
                int hour = system_time.date_time_utc->tm_hour;
                offset = (hour + received_message->requested_offset) %
                         number_of_datapoints;
                print_hourly_forecast(forecasts[offset]);
                send_weather_hourly_update(forecasts[offset]);
                break;
            case WEATHER_DAILY:
                offset = (received_message->requested_offset) % FORECAST_DAYS;
                print_daily_forecast(forecasts_daily[offset]);
                send_weather_daily_update(forecasts_daily[offset]);
                break;
            case UPDATE_WEATHER_DATA:
                update_weather_data();
                break;
            }
        }
    }
}

void free_old_data_if_present(
    struct ForecastHourly *forecasts[24 * FORECAST_DAYS],
    struct ForecastDaily *forecasts_daily[FORECAST_DAYS]);

void update_weather_data()
{
    free_old_data_if_present(forecasts, forecasts_daily);
    ESP_LOGI(TAG, "Sending HTTP request to get weather data...");
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
    int status = send_http_request(request, &json);

    if (status == REQUEST_FAILED) {
        return;
    }

    free(request->web_server);
    free(request->web_port);
    free(request->web_path);
    free(request->body);
    free(request);

    ESP_LOGI(TAG, "Extracting forecast data...");
    extract_hourly_forecast(json, FORECAST_DAYS, forecasts);
    extract_daily_forecast(json, FORECAST_DAYS, forecasts_daily);

    ESP_LOGI(TAG, "Weather data extracted successfully");
}

void free_old_data_if_present(
    struct ForecastHourly *forecasts[24 * FORECAST_DAYS],
    struct ForecastDaily *forecasts_daily[FORECAST_DAYS])
{
    for (int i = 0; i < 24 * FORECAST_DAYS; i++) {
        if (forecasts[i] != NULL) {
            free(forecasts[i]);
        }
    }

    for (int i = 0; i < FORECAST_DAYS; i++) {
        if (forecasts_daily[i] != NULL) {
            free(forecasts_daily[i]);
        }
    }
}

void send_weather_daily_update(struct ForecastDaily *forecast)
{
    struct DisplayMessage *message = &display_message;
    message->requested_action = SHOW_WEATHER_DAILY;
    message->daily_forecast = forecast;
    xQueueSend(display_msg_queue, (void *)&message, (TickType_t)0);
}
void send_weather_hourly_update(struct ForecastHourly *forecast)
{
    struct DisplayMessage *message = &display_message;
    message->requested_action = SHOW_WEATHER_HOURLY;
    message->hourly_forecast = forecast;
    xQueueSend(display_msg_queue, (void *)&message, (TickType_t)0);
}
