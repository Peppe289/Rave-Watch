#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"

#include "st7789v2.h"
#include "cst816.h"
#include "lvgl.h"
#include "ui.h"

void action_change_brightness_cb(lv_event_t * e) {
    // Recupera l'oggetto slider che ha scatenato l'evento
    lv_obj_t * slider = lv_event_get_target(e);

    // Legge il valore corrente (da 10 a 100)
    int32_t val = lv_slider_get_value(slider);

    ESP_LOGI("BRIGHTNESS", "Nuova luminosità: %ld%%", (long)val);

    // Applica il nuovo duty cycle al canale PWM LEDC
    st7789vw_set_brightness((uint8_t)val);
}

void action_callback_function(void * e) {
    ESP_LOGI("UI", "Action callback triggered");
}
