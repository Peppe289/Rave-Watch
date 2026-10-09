#include "lvgl.h"
#include <string.h>
#include "vars.h"

#include "brightness.h"

void (*handler_suspend_mode)(void *e) = NULL;

// -------------- Digital Clock VARS --------------

char digital_clock_seconds[100] = { 0 };
char digital_clock[100] = { 0 };

const char *get_var_digital_clock() {
    return digital_clock;
}

void set_var_digital_clock(const char *value) {
    strncpy(digital_clock, value, sizeof(digital_clock) / sizeof(char));
    digital_clock[sizeof(digital_clock) / sizeof(char) - 1] = 0;
}

const char *get_var_digital_clock_seconds() {
    return digital_clock_seconds;
}

void set_var_digital_clock_seconds(const char *value) {
    strncpy(digital_clock_seconds, value, sizeof(digital_clock_seconds) / sizeof(char));
    digital_clock_seconds[sizeof(digital_clock_seconds) / sizeof(char) - 1] = 0;
}

// -------------- Brightness VARS --------------

void action_change_brightness_cb(lv_event_t * e) {
    set_brightness_level((uint8_t)lv_slider_get_value(
                            (lv_obj_t *)lv_event_get_target(e)));
}

// -------------- Stats VARS --------------

/// This function isn't from LVGL, but is used to set the handler for suspend mode.
void set_handler_suspend_mode(void (*handler)(void *e)) {
    handler_suspend_mode = handler;
}

void action_start_suspend_mode(lv_event_t *e) {
    if (handler_suspend_mode) {
        handler_suspend_mode(e);
    }
}