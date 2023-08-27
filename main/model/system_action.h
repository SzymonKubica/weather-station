#ifndef SYSTEM_ACTION_H
#define SYSTEM_ACTION_H

enum SystemAction {
    TOGGLE_ONBOARD_LED,
    DISPLAY_OFF,
    DISPLAY_ON,
    GET_WEATHER_NOW,
    GET_WEATHER_TODAY,
    GET_WEATHER_TOMORROW,
    GET_WEATHER_T2,
    REQUEST_UPDATE_WEATHER_DATA
};

enum DisplayAction {
    SCREEN_ON,
    SCREEN_OFF,
    SHOW_DHT_READING,
    SHOW_WEATHER_NOW,
    SHOW_WEATHER_TODAY,
    SHOW_WEATHER_TOMORROW,
    SHOW_WEATHER_T2,
};

enum ForecastRequest {
    WEATHER_NOW,
    WEATHER_TODAY,
    WEATHER_TOMORROW,
    WEATHER_T2,
    UPDATE_WEATHER_DATA
};

extern const char *system_action_str[];
extern const char *display_action_str[];
extern const char *forecast_request_str[];

#endif
