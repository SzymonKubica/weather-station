#ifndef SYSTEM_MESSAGE_H
#define SYSTEM_MESSAGE_H

#include "../../libs/infrared-receiver/infrared_nec.h"
#include "system_action.h"

struct IRRemoteMessage {
    enum RemoteButton pressed_button;
};

struct DisplayMessage {
    enum DisplayAction requested_action;
    float temperature;
    float humidity;
};

struct SystemMessage {
    enum SystemAction system_action;
};

struct ForecastMessage {
    enum ForecastRequest forecast_request;
};

extern QueueHandle_t ir_remote_input_queue;
extern QueueHandle_t system_msg_queue;
extern QueueHandle_t display_msg_queue;
extern QueueHandle_t display_msg_queue;
extern QueueHandle_t weather_forecast_msg_queue;

extern struct IRRemoteMessage ir_remote_message;
extern struct DisplayMessage display_message;
extern struct SystemMessage system_message;
extern struct WeatherForecastRequestMessage forecast_request_message;

#endif
