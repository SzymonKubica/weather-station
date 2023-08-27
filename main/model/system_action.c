#include "system_action.h"

const char *system_action_str[] = {
    [TOGGLE_ONBOARD_LED] = "TOGGLE_ONBOARD_LED",
    [DISPLAY_OFF] = "DISPLAY_OFF",
    [DISPLAY_ON] = "DISPLAY_ON",
    [SHOW_SENSOR_READING] = "SHOW_SENSOR_READING",
    [GET_WEATHER_NOW] = "GET_WEATHER_NOW",
    [GET_WEATHER_TODAY] = "GET_WEATHER_TODAY",
    [GET_WEATHER_TOMORROW] = "GET_WEATHER_TOMORROW",
    [GET_WEATHER_T2] = "GET_WEATHER_T2",
    [REQUEST_UPDATE_WEATHER_DATA] = "REQUEST_UPDATE_WEATHER_DATA"
};

const char *display_action_str[] = {
    [SCREEN_ON] = "SCREEN_ON",
    [SCREEN_OFF] = "SCREEN_OFF",
    [UPDATE_DHT_READING] = "UPDATE_DHT_READING",
    [SHOW_DHT_READING] = "SHOW_DHT_READING",
    [SHOW_WEATHER_NOW] = "SHOW_WEATHER_NOW",
    [SHOW_WEATHER_TODAY] = "SHOW_WEATHER_TODAY",
    [SHOW_WEATHER_TOMORROW] = "SHOW_WEATHER_TOMORROW",
    [SHOW_WEATHER_T2] = "SHOW_WEATHER_T2",
};

const char *forecast_request_str[] = {
  [WEATHER_NOW] = "WEATHER_NOW",
  [WEATHER_TODAY] = "WEATHER_TODAY",
  [WEATHER_TOMORROW] = "WEATHER_TOMORROW",
  [WEATHER_T2] = "WEATHER_T2",
  [UPDATE_WEATHER_DATA] = "UPDATE_WEATHER_DATA",
};
