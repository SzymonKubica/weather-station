set(srcs
    "weather_station_main.c"
    "libs/dht/DHT_c.c")
set(includes "libs")
idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ${includes})

