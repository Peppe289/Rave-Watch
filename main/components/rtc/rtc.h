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


#define YEAR_OFFSET     (1970)

#define PCF85063A_RTC_SECOND_ADDR   (0x04)
#define PCF85063A_RTC_MINUTE_ADDR   (0x05)
#define PCF85063A_RTC_HOUR_ADDR		(0x06)
#define PCF85063A_RTC_DAY_ADDR		(0x07)
#define PCF85063A_RTC_WDAY_ADDR		(0x08)
#define PCF85063A_RTC_MONTH_ADDR    (0x09)
#define PCF85063A_RTC_YEAR_ADDR		(0x0A)

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t dayweek;
    uint8_t month;
    uint8_t year;
} rtc_date_time_t;

esp_err_t rtc_get_seconds_time(uint8_t *data, size_t len);
esp_err_t rtc_get_minutes_time(uint8_t *data, size_t len);
esp_err_t rtc_get_hours_time(uint8_t *data, size_t len);
esp_err_t rtc_set_date_time(const uint8_t *data, const size_t len);
void rtc_init_custom();
void rtc_sync_ntp();
SemaphoreHandle_t rtc_get_semaphore();

#endif