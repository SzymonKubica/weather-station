#ifndef SYSTEM_MESSAGE_H
#define SYSTEM_MESSAGE_H

#include "../../libs/infrared-receiver/infrared_nec.h"
#include "forecast.h"
#include "system_action.h"

struct IRRemoteMessage {
    enum RemoteButton pressed_button;
};

struct DisplayMessage {
    enum DisplayAction requested_action;
    float temperature;
    float humidity;
    struct ForecastHourly *hourly_forecast;
    struct ForecastDaily *daily_forecast;
};

struct SystemMessage {
    enum SystemAction system_action;
    void *message_payload;
};

struct ForecastRequest {
    enum ForecastRequestType request_type;
    // When the message requests a hourly/daily forecast
    // the offset controlls which day/hour starting from
    // now is to be displayed
    int requested_offset;
};

extern QueueHandle_t ir_remote_input_queue;
extern QueueHandle_t system_msg_queue;
extern QueueHandle_t display_msg_queue;
extern QueueHandle_t display_msg_queue;
extern QueueHandle_t weather_forecast_msg_queue;

extern struct IRRemoteMessage ir_remote_message;
extern struct DisplayMessage display_message;
extern struct SystemMessage system_message;
extern struct ForecastRequest forecast_request_message;

#endif
