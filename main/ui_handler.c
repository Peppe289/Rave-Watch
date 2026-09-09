#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "st7789v2.h"
#include "lvgl.h"

void action_change_brightness_cb(lv_event_t * e) {
    st7789vw_set_brightness(
        (uint8_t)lv_slider_get_value(
            (lv_obj_t *)lv_event_get_target(e)));
}
