#ifndef __VOLTAGE_H__
#define __VOLTAGE_H__

#include <stdint.h>
void battery_adc_init(void);
uint32_t battery_get_voltage_mv(void);

const char *get_var_battery_voltage();
void set_var_battery_voltage(const char *value);

#endif // __VOLTAGE_H__