set(srcs
    "weather_station_main.c"
    "libs/dht/DHT_c.c"
    "libs/json/cJSON.c"
    "libs/http-client/http_client.c"
    "display/display.c"
    "gpio/gpio_util.c"
    "util/util.c"
    "util/logging.c"
    "model/system_action.c"
    "model/onboard_led.c"
    "model/system_message.c"
    "model/forecast.c"
    "tasks/dht_task.c"
    "tasks/display_task.c"
    "tasks/ir_remote_task.c"
    "tasks/weather_forecast_task.c"
    )
set(includes "libs"
  "gpio" "display" "util" "model" "tasks")
idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ${includes})

