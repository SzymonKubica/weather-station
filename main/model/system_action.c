#include "system_action.h"

const char *system_action_str[] = {
    [TOGGLE_ONBOARD_LED] = "TOGGLE_ONBOARD_LED",
    [DISPLAY_REQUEST] = "DISPLAY_REQUEST",
    [FORECAST_REQUEST] = "FORECAST_REQUEST",
};

const char *display_action_str[] = {
    [SCREEN_ON] = "SCREEN_ON",
    [SCREEN_OFF] = "SCREEN_OFF",
    [UPDATE_DHT_READING] = "UPDATE_DHT_READING",
    [SHOW_DHT_READING] = "SHOW_DHT_READING",
    [SHOW_WEATHER_DAILY] = "SHOW_WEATHER_DAILY",
    [SHOW_WEATHER_HOURLY] = "SHOW_WEATHER_HOURLY",
};

const char *forecast_request_str[] = {
    [WEATHER_HOURLY] = "WEATHER_HOURLY",
    [WEATHER_DAILY] = "WEATHER_DAILY",
    [UPDATE_WEATHER_DATA] = "UPDATE_WEATHER_DATA",
};
