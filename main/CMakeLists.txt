file(GLOB_RECURSE EXTRA_SRCS 
    "components/ui/*.c"
    "components/ui/*.cpp"
    "components/display/*.c"
    "components/touch/*.c"
    "components/power/*.c"
    "components/bt/*.c"
)
file(GLOB MAIN_SRCS "*.c")

idf_component_register(SRCS ${MAIN_SRCS} ${EXTRA_SRCS}
                       INCLUDE_DIRS "." "components/bt" "components/ui" "components/touch" "components/display" "components/power"
                       PRIV_REQUIRES bt nvs_flash spi_flash esp_lcd esp_timer driver spiffs lvgl esp_driver_ledc esp_adc esp_wifi esp_netif)

set_source_files_properties(components/ui/eez-flow.cpp PROPERTIES
    COMPILE_FLAGS "-std=gnu++17"
)