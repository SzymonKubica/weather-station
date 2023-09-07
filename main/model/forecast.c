#include "forecast.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void extract_hourly_forecast(cJSON *response, int forecast_days,
                             struct ForecastHourly *forecasts[])
{
    cJSON *hourly = cJSON_GetObjectItemCaseSensitive(response, "hourly");
    cJSON *times = cJSON_GetObjectItemCaseSensitive(hourly, "time");
    cJSON *temperatures =
        cJSON_GetObjectItemCaseSensitive(hourly, "temperature_2m");
    cJSON *apparent_temperatures =
        cJSON_GetObjectItemCaseSensitive(hourly, "apparent_temperature");
    cJSON *humidities =
        cJSON_GetObjectItemCaseSensitive(hourly, "relativehumidity_2m");
    cJSON *precip_probabilities =
        cJSON_GetObjectItemCaseSensitive(hourly, "precipitation_probability");

    for (int i = 0; i < 24 * forecast_days; i++) {
        forecasts[i] =
            (struct ForecastHourly *)malloc(sizeof(struct ForecastHourly));

        malloc_forecast_hourly(forecasts[i]);

        char *time = cJSON_Print(cJSON_GetArrayItem(times, i));
        char *temperature = cJSON_Print(cJSON_GetArrayItem(temperatures, i));
        char *apparent_temperature =
            cJSON_Print(cJSON_GetArrayItem(apparent_temperatures, i));
        char *humidity = cJSON_Print(cJSON_GetArrayItem(humidities, i));
        char *precip_probability =
            cJSON_Print(cJSON_GetArrayItem(precip_probabilities, i));

        parse_hourly_forecast(time, temperature, apparent_temperature, humidity,
                              precip_probability, forecasts[i]);
    }
}
void extract_daily_forecast(cJSON *response, int forecast_days,
                            struct ForecastDaily *forecasts[])
{
    cJSON *hourly = cJSON_GetObjectItemCaseSensitive(response, "daily");
    cJSON *times = cJSON_GetObjectItemCaseSensitive(hourly, "time");
    cJSON *min_temperatures =
        cJSON_GetObjectItemCaseSensitive(hourly, "temperature_2m_min");
    cJSON *max_temperatures =
        cJSON_GetObjectItemCaseSensitive(hourly, "temperature_2m_max");
    cJSON *sunrise_times = cJSON_GetObjectItemCaseSensitive(hourly, "sunrise");
    cJSON *sunset_times = cJSON_GetObjectItemCaseSensitive(hourly, "sunset");
    cJSON *max_precip_probabilities = cJSON_GetObjectItemCaseSensitive(
        hourly, "precipitation_probability_max");

    for (int i = 0; i < forecast_days; i++) {
        forecasts[i] =
            (struct ForecastDaily *)malloc(sizeof(struct ForecastDaily));

        malloc_forecast_daily(forecasts[i]);

        char *time = cJSON_Print(cJSON_GetArrayItem(times, i));
        char *sunrise_time = cJSON_Print(cJSON_GetArrayItem(sunrise_times, i));
        char *sunset_time = cJSON_Print(cJSON_GetArrayItem(sunset_times, i));
        char *min_temperature =
            cJSON_Print(cJSON_GetArrayItem(min_temperatures, i));
        char *max_temperature =
            cJSON_Print(cJSON_GetArrayItem(max_temperatures, i));
        char *max_precip_probability =
            cJSON_Print(cJSON_GetArrayItem(max_precip_probabilities, i));

        parse_daily_forecast(time, sunrise_time, sunset_time, min_temperature,
                             max_temperature, max_precip_probability,
                             forecasts[i]);
    }
}

int parse_hourly_forecast(char *time, char *temperature,
                          char *apparent_temperature, char *humidity,
                          char *precip_probability,
                          struct ForecastHourly *forecast)
{
    strptime(time, "\"%Y-%m-%dT%H:%M\"", forecast->time);
    forecast->temperature = atof(temperature);
    forecast->apparent_temperature = atof(apparent_temperature);
    forecast->humidity = atof(humidity);
    forecast->precip_probability = atoi(precip_probability);
    return PARSE_SUCCESS;
}

int parse_daily_forecast(char *date, char *sunrise, char *sunset,
                         char *min_temperature, char *max_temperature,
                         char *max_precip_probability,
                         struct ForecastDaily *forecast)
{
    strptime(date, "\"%Y-%m-%d\"", forecast->date);
    strptime(sunset, "\"%Y-%m-%dT%H:%M\"", forecast->sunset);
    strptime(sunrise, "\"%Y-%m-%dT%H:%M\"", forecast->sunrise);
    forecast->min_temperature = atof(min_temperature);
    forecast->max_temperature = atof(max_temperature);
    forecast->max_precip_probability = atoi(max_precip_probability);
    return PARSE_SUCCESS;
}

void print_hourly_forecast(struct ForecastHourly *forecast)
{
    char time[20];
    strftime(time, 20, "%Y-%m-%dT%H:%M", forecast->time);
    printf("Forecast for %s\n", time);
    printf("Temperature: %.2f\n", forecast->temperature);
    printf("Apparent temperature: %.2f\n", forecast->apparent_temperature);
    printf("Humidity: %.2f\n", forecast->humidity);
    printf("Precipitation probability: %d\n", forecast->precip_probability);
}
void print_daily_forecast(struct ForecastDaily *forecast)
{
    char date[20];
    strftime(date, 20, "%Y-%m-%d", forecast->date);
    char sunrise[20];
    strftime(sunrise, 20, "%Y-%m-%dT%H:%M", forecast->sunrise);
    char sunset[20];
    strftime(sunset, 20, "%Y-%m-%dT%H:%M", forecast->sunset);

    printf("Forecast for %s\n", date);
    printf("Sunrise: %s\n", sunrise);
    printf("Sunset: %s\n", sunset);
    printf("Min Temperature: %.2f\n", forecast->min_temperature);
    printf("Max Temperature: %.2f\n", forecast->max_temperature);
    printf("Max Precipitation probability: %d\n",
           forecast->max_precip_probability);
}

void malloc_forecast_hourly(struct ForecastHourly *forecast)
{
    forecast->time = (struct tm *)malloc(sizeof(struct tm));
}

void free_forecast_hourly(struct ForecastHourly *forecast)
{
    free(forecast->time);
}

void malloc_forecast_daily(struct ForecastDaily *forecast)
{
    forecast->date = (struct tm *)malloc(sizeof(struct tm));
    forecast->sunrise = (struct tm *)malloc(sizeof(struct tm));
    forecast->sunset = (struct tm *)malloc(sizeof(struct tm));
}

void free_forecast_daily(struct ForecastDaily *forecast)
{
    free(forecast->date);
    free(forecast->sunrise);
    free(forecast->sunset);
}


static void send_forecast_request(struct ForecastRequest *request)
{
    xQueueSend(weather_forecast_msg_queue, (void *)&request, (TickType_t)0);
}

