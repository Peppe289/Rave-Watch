#ifndef __RTC_H__
#define __RTC_H__

#include "driver/gpio.h"
#include "esp_err.h"
#include "i2cdev.h"
#include <stdint.h>

#define RTC_I2C_NUM     I2C_NUM_0
#define PIN_RTC_SDA     11
#define PIN_RTC_SCL     10

// interrupt RTC_INT=GPIO39
// i2c address 0x51
#define RTC_I2C_ADDR    0x51
#define RTC_INT_GPIO    39

esp_err_t rtc_get_seconds_time(uint8_t *data, size_t len);
esp_err_t rtc_get_minutes_time(uint8_t *data, size_t len);
esp_err_t rtc_get_hours_time(uint8_t *data, size_t len);
void rtc_init_custom();
SemaphoreHandle_t rtc_get_semaphore();

#endif