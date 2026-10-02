#ifndef __VOLTAGE_H__
#define __VOLTAGE_H__

#include <stdint.h>

typedef enum battery_state_enum {
    BATTERY_LOW = 1,
    BATTERY_MEDIUM = 2,
    BATTERY_HIGH = 3
} battery_state_enum;

#define BATTERY_ADC_UNIT       ADC_UNIT_1
#define BATTERY_ADC_CHANNEL    ADC_CHANNEL_0

#define BATTERY_VOLTAGE_DIVIDER_RATIO 3.07f

void battery_adc_init(void);
uint32_t battery_get_voltage_mv(void);

void battery_set_ui_state(battery_state_enum state);

#endif // __VOLTAGE_H__