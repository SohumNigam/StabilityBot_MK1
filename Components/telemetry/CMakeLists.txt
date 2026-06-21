idf_component_register(
    SRCS "telemetry.c"
    INCLUDE_DIRS "include"
    REQUIRES
        esp_http_server
        esp_wifi
        esp_netif
)