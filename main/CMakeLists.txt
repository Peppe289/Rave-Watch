file(GLOB_RECURSE EXTRA_SRCS 
    "components/ui/*.c"
    "components/ui/*.cpp"
    "components/display/*.c"
    "components/touch/*.c"
    "components/power/*.c"
    "components/bt/*.c"
    "components/rtc/*.c"
    "components/network/*.c"
    "worker/*.c"
)
file(GLOB MAIN_SRCS "main.c" "*.c")

idf_component_register(SRCS ${MAIN_SRCS} ${EXTRA_SRCS}
                       INCLUDE_DIRS "." "worker" "components/network" "components/bt" "components/ui" "components/touch" "components/display" "components/power" "components/rtc/"
                       PRIV_REQUIRES bt nvs_flash spi_flash esp_lcd esp_timer driver spiffs lvgl esp_driver_ledc esp_adc esp_wifi esp_netif lwip)

set_source_files_properties(components/ui/eez-flow.cpp PROPERTIES
    COMPILE_FLAGS "-std=gnu++17"
)