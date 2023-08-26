set(component_srcs "wifi.c")

idf_component_register(SRCS "${component_srcs}"
                       PRIV_REQUIRES driver esp_event esp_wifi nvs_flash
                       INCLUDE_DIRS ".")
