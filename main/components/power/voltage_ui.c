#include <string.h>
#include "vars.h"

char battery_voltage[100] = { 0 };

const char *get_var_battery_voltage() {
    return battery_voltage;
}

void set_var_battery_voltage(const char *value) {
    strncpy(battery_voltage, value, sizeof(battery_voltage) / sizeof(char));
    battery_voltage[sizeof(battery_voltage) / sizeof(char) - 1] = 0;
}