#include "lvgl.h"
#include <string.h>
#include "vars.h"

#include "brightness.h"
#include "esp_log.h"

void (*handler_suspend_mode)(void *e) = NULL;

// -------------- Digital Clock VARS --------------

char digital_clock_seconds[100] = { 0 };
char digital_clock[100] = { 0 };
int32_t rssi_value = 0;
bool rssi_enabled = false;
bool bt_connected = false;
char bt_status[100] = { 0 };

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

// ------------ RSSI Bar ------------

int32_t get_var_rssi_value() {
    return rssi_value;
}

void set_var_rssi_value(int32_t value) {
    rssi_value = ((float)value + 127) / (127 + 20) * 100;
}

// ------------ RSSI toggle ------------

bool get_var_rssi_enabled() {
    return rssi_enabled;
}

void set_var_rssi_enabled(bool value) {
    rssi_enabled = value;
}

// ------------ BT enabled ref ------------
 
bool get_var_bt_connected() {
    return bt_connected;
}

void set_var_bt_connected(bool value) {
    bt_connected = value;
}

// ------------ BT Status -----------------

const char *get_var_bt_status() {
    return bt_status;
}

void set_var_bt_status(const char *value) {
    strncpy(bt_status, value, sizeof(bt_status) / sizeof(char));
    bt_status[sizeof(bt_status) / sizeof(char) - 1] = 0;
}
