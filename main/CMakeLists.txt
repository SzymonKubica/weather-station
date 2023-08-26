set(srcs
    "weather_station_main.c"
    "libs/dht/DHT_c.c"
    "libs/http-client/http_get.c"
    "display/display.c"
    "gpio/gpio_util.c"
    "util/util.c"
    "util/logging.c"
    "model/system_action.c"
    "model/onboard_led.c"
    "model/system_message.c"
    "tasks/dht_task.c"
    "tasks/display_task.c"
    "tasks/ir_remote_task.c"
    )
set(includes "libs"
  "gpio" "display" "util" "model" "tasks")
idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ${includes})

