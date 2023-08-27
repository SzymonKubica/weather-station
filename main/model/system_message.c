#include "system_message.h"

QueueHandle_t ir_remote_input_queue = NULL;
QueueHandle_t system_msg_queue = NULL;
QueueHandle_t display_msg_queue = NULL;
QueueHandle_t weather_forecast_msg_queue = NULL;

struct IRRemoteMessage ir_remote_message;
struct DisplayMessage display_message;
struct SystemMessage system_message;
struct WeatherForecastRequestMessage forecast_request_message;
