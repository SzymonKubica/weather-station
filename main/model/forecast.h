#ifndef FORECAST_H
#define FORECAST_H

#include "json/cJSON.h"
#include <time.h>

#define PARSE_SUCCESS 0
#define PARSE_FAILURE -1

struct ForecastHourly {
    struct tm *time;
    float temperature;
    float apparent_temperature;
    float humidity;
    int precip_probability;
};

struct ForecastDaily {
    struct tm *date;
    struct tm *sunrise;
    struct tm *sunset;
    float min_temperature;
    float max_temperature;
    int max_precip_probability;
};

int parse_hourly_forecast(char *time, char *temperature,
                          char *apparent_temperature, char *humidity,
                          char *precip_probability,
                          struct ForecastHourly *forecast);
int parse_daily_forecast(char *date, char *sunrise, char *sunset,
                         char *min_temperature, char *max_temperature,
                         char *precip_probability,
                         struct ForecastDaily *forecast);

void extract_hourly_forecast(cJSON *response, int forecast_days,
                        struct ForecastHourly *forecasts[]);
void extract_daily_forecast(cJSON *response, int forecast_days,
                       struct ForecastDaily *forecasts[]);

void print_hourly_forecast(struct ForecastHourly *forecast);
void print_daily_forecast(struct ForecastDaily *forecast);

void malloc_forecast_hourly(struct ForecastHourly *forecast);
void free_forecast_hourly(struct ForecastHourly *forecast);

void malloc_forecast_daily(struct ForecastDaily *forecast);
void free_forecast_daily(struct ForecastDaily *forecast);

#endif
