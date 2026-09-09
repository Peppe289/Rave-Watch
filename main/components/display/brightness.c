#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "st7789v2.h"
#include "brightness.h"
#include "lvgl.h"

#define DEFAULT_BRIGHTNESS_LEVEL 10

static int brightness_level = DEFAULT_BRIGHTNESS_LEVEL;

void action_change_brightness_cb(lv_event_t * e) {
    brightness_level = (uint8_t)lv_slider_get_value(
                            (lv_obj_t *)lv_event_get_target(e));
    st7789vw_set_brightness(brightness_level);
}

void suspend_display() {
    st7789vw_set_brightness(0);
}

void resume_display() {
    st7789vw_set_brightness(brightness_level);
}

uint8_t get_brightness_level() {
    return brightness_level;
}


