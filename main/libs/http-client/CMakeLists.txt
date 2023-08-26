set(component_srcs "http_get.c")

idf_component_register(SRCS "${component_srcs}"
                       PRIV_REQUIRES esp_event esp_netif esp_err
                       INCLUDE_DIRS ".")
