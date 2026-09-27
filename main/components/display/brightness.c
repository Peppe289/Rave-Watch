#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "st7789v2.h"
#include "brightness.h"
#include "lvgl.h"

#define DEFAULT_BRIGHTNESS_LEVEL 10

static int brightness_level = DEFAULT_BRIGHTNESS_LEVEL;

void set_brightness_level(uint8_t level) {
    if (level > 100) level = 100;
    brightness_level = level;
    st7789vw_set_brightness(level);
}

void action_change_brightness_cb(lv_event_t * e) {
    set_brightness_level((uint8_t)lv_slider_get_value(
                            (lv_obj_t *)lv_event_get_target(e)));
}

void suspend_display() {
    st7789vw_set_brightness(0);
}

void resume_display() {
    set_brightness_level(brightness_level);
}

uint8_t get_brightness_level() {
    return brightness_level;
}


