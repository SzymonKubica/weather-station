#ifndef SYSTEM_ACTION_H
#define SYSTEM_ACTION_H

enum SystemAction { TOGGLE_ONBOARD_LED, DISPLAY_REQUEST, FORECAST_REQUEST };

enum DisplayAction {
    SCREEN_ON,
    SCREEN_OFF,
    UPDATE_DHT_READING,
    SHOW_DHT_READING,
    SHOW_WEATHER_DAILY,
    SHOW_WEATHER_HOURLY
};

enum ForecastRequestType { WEATHER_HOURLY, WEATHER_DAILY, UPDATE_WEATHER_DATA };

extern const char *system_action_str[];
extern const char *display_action_str[];
extern const char *forecast_request_str[];

#endif
