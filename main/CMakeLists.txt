set(srcs
    "weather_station_main.c"
    "libs/dht/DHT_c.c"
    "display/display.c"
    "gpio/gpio_util.c"
    "util/util.c"
    "util/logging.c"
    )
set(includes "libs"
  "gpio" "display" "util")
idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ${includes})

