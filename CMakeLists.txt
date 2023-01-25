idf_component_register(
    SRCS "gpi.c"
    INCLUDE_DIRS include
    REQUIRES driver
    PRIV_REQUIRES log_utils esp_event esp_wifi
)
