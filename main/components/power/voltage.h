#ifndef __VOLTAGE_H__
#define __VOLTAGE_H__

#include <stdint.h>

#define BATTERY_ADC_UNIT       ADC_UNIT_1
#define BATTERY_ADC_CHANNEL    ADC_CHANNEL_0

#define BATTERY_VOLTAGE_DIVIDER_RATIO 3.07f

void battery_adc_init(void);
uint32_t battery_get_voltage_mv(void);

const char *get_var_battery_voltage();
void set_var_battery_voltage(const char *value);

#endif // __VOLTAGE_H__