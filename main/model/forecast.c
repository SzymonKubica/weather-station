#include "forecast.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int parse_hourly_forecast(char *time, char *temperature,
                        char *apparent_temperature, char *humidity,
                        char *precip_probability,
                        struct ForecastHourly *forecast)
{
    strptime(time, "%Y-%0m-%0dT%0H:%0M", &forecast->time);
    forecast->temperature = atof(temperature);
    forecast->apparent_temperature = atof(apparent_temperature);
    forecast->humidity = atof(humidity);
    forecast->precip_probability = atoi(precip_probability);
    return PARSE_SUCCESS;
}

int parse_daily_forecast(char *date, char *sunrise, char *sunset,
                       char *min_temperature, char *max_temperature,
                       char *precip_probability, struct ForecastDaily *forecast)
{

    strptime(date, "%Y-%0m-%0d", &forecast->date);
    strptime(sunset, "%Y-%0m-%0dT%0H:%0M", &forecast->sunset);
    strptime(sunrise, "%Y-%0m-%0dT%0H:%0M", &forecast->sunrise);
    forecast->min_temperature = atof(min_temperature);
    forecast->max_temperature = atof(max_temperature);
    forecast->precip_probability = atof(precip_probability);
    return PARSE_SUCCESS;
}


char * forecast_hourly_to_str(struct ForecastHourly *forecast) {
    return NULL;
}
char * forecast_daily_to_str(struct ForecastDaily *forecast) {
    return NULL;
}

void print_hourly_forecast(struct ForecastHourly *forecast) {
    printf("Forecast for %s\n", asctime(&forecast->time));
    printf("Temperature: %.2f\n", forecast->temperature);
    printf("Apparent temperature: %.2f\n", forecast->apparent_temperature);
    printf("Humidity: %.2f\n", forecast->humidity);
    printf("Precipitation probability: %d\n", forecast->precip_probability);
}
void print_daily_forecast(struct ForecastDaily *forecast) {
}
