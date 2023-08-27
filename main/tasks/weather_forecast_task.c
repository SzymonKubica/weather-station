#include "weather_forecast_task.h"
#include <stdlib.h>
#include <string.h>

#include "../libs/http-client/http_client.h"
#include "../model/forecast.h"
#include "esp_log.h"

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

#define FORECAST_DAYS 3
struct ForecastHourly *forecasts[24 * FORECAST_DAYS];
struct ForecastDaily *forecasts_daily[FORECAST_DAYS];

#define TAG "FORECAST_TASK"

void assemble_request(struct Request *request);
void update_weather_data();
void weather_forecast_task(void *pvParameter)
{
    ESP_LOGI(TAG, "Getting weather data...");
    update_weather_data();

    print_hourly_forecast(forecasts[15]);
    print_daily_forecast(forecasts_daily[2]);
}

void update_weather_data()
{
    ESP_LOGI(TAG, "Sending HTTP request to get weather data...");
    struct Request *request;
    assemble_request(request);
    cJSON *json;
    send_http_request(request, &json);

    ESP_LOGI(TAG, "Extracting forecast data...");
    extract_hourly_forecast(json, FORECAST_DAYS, forecasts);
    extract_daily_forecast(json, FORECAST_DAYS, forecasts_daily);

    ESP_LOGI(TAG, "Weather data extracted successfully");
}

void assemble_request(struct Request *request)
{
    request = malloc(sizeof(struct Request));
    request->web_server = calloc(strlen(WEB_SERVER), sizeof(char));
    request->web_port = calloc(strlen(WEB_PORT), sizeof(char));
    request->web_path = calloc(strlen(WEB_PATH), sizeof(char));
    request->body = calloc(strlen(REQUEST), sizeof(char));
    request->max_attempts = 3;

    strncpy(request->web_server, WEB_SERVER, strlen(WEB_SERVER) + 1);
    strncpy(request->web_port, WEB_PORT, strlen(WEB_PORT) + 1);
    strncpy(request->web_path, WEB_PATH, strlen(WEB_PATH) + 1);
    strncpy(request->body, REQUEST, strlen(REQUEST) + 1);
}
