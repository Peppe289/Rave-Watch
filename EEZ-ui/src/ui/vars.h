#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations

// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_NONE
};

// Native global variables

extern const char *get_var_digital_clock();
extern void set_var_digital_clock(const char *value);
extern const char *get_var_digital_clock_seconds();
extern void set_var_digital_clock_seconds(const char *value);
extern int32_t get_var_rssi_value();
extern void set_var_rssi_value(int32_t value);
extern bool get_var_rssi_enabled();
extern void set_var_rssi_enabled(bool value);
extern bool get_var_bt_connected();
extern void set_var_bt_connected(bool value);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/