#include "driver/gpio.h"
#include "esp_err.h"
#include "hal/gpio_types.h"
#include "i2cdev.h"
#include <stdint.h>
#include "rtc.h"
#include "esp_netif_sntp.h"
#include "esp_sntp.h"
#include "esp_log.h"

static const char *TAG = "RTC";
static SemaphoreHandle_t s_rtc_sync_sem = NULL;

static i2c_dev_t s_rtc = {
    .port = RTC_I2C_NUM,
    .addr = RTC_I2C_ADDR,
    .addr_bit_len = I2C_ADDR_BIT_LEN_7,
    .cfg = {
        .sda_io_num = PIN_RTC_SDA,
        .scl_io_num = PIN_RTC_SCL,
        .sda_pullup_en = 1,
        .scl_pullup_en = 1,
        .master = {
            .clk_speed = 100000,
        },
    },
};

static void IRAM_ATTR intr_handler(void *arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(s_rtc_sync_sem, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

SemaphoreHandle_t rtc_get_semaphore() {
    return s_rtc_sync_sem;
}

esp_err_t rtc_enable_1hz_interrupt() {
    esp_err_t err;

    uint8_t ctrl2 = 0x00; 
    err = i2c_dev_write_reg(&s_rtc, 0x01, &ctrl2, 1);
    if (err != ESP_OK) return err;

    uint8_t timer_val = 0x01; 
    err = i2c_dev_write_reg(&s_rtc, 0x10, &timer_val, 1);
    if (err != ESP_OK) return err;
   
    uint8_t timer_mode = 0x17;
    err = i2c_dev_write_reg(&s_rtc, 0x11, &timer_mode, 1);
    if (err != ESP_OK) return err;

    return ESP_OK;
}

void rtc_init_custom() {
    gpio_config_t int_conf = {
        .pin_bit_mask = (1ULL << RTC_INT_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&int_conf);
    ESP_ERROR_CHECK(i2c_dev_create_mutex(&s_rtc));
    ESP_ERROR_CHECK(rtc_enable_1hz_interrupt());
    s_rtc_sync_sem = xSemaphoreCreateBinary();
    gpio_install_isr_service(0);
    gpio_isr_handler_add(RTC_INT_GPIO, intr_handler, NULL);
}

static uint8_t decToBcd(int val)
{
	return (uint8_t)((val / 10 * 16) + (val % 10));
}

void rtc_sync_ntp() {
    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;

    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_netif_sntp_init(&config);
    
    while(1) {
        if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(100000)) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to update system time within 10s timeout");
        }

        time(&now);
        setenv("TZ", "CET-1", 1);
        tzset();

        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGI(TAG, "The current legally recognized date/time in italy is: %s", strftime_buf);
        rtc_date_time_t rtc_time = {
            .day = timeinfo.tm_mday,
            .month = timeinfo.tm_mon,
            .year = timeinfo.tm_year,
            .hours = timeinfo.tm_hour,
            .minutes = timeinfo.tm_min,
            .seconds = timeinfo.tm_sec,
            .dayweek = timeinfo.tm_wday
        };
        uint8_t buf[7] = {
					  decToBcd(rtc_time.seconds),
					  decToBcd(rtc_time.minutes),
					  decToBcd(rtc_time.hours),
					  decToBcd(rtc_time.day),
					  decToBcd(rtc_time.dayweek),
					  decToBcd(rtc_time.month),
					  decToBcd(rtc_time.year - YEAR_OFFSET)
        };
        ESP_ERROR_CHECK(rtc_set_date_time(buf, sizeof(buf)));
        vTaskDelay(pdMS_TO_TICKS(6000));
    }
}

esp_err_t rtc_get_seconds_time(uint8_t *data, size_t len) {
    return i2c_dev_read_reg(&s_rtc, 0x04, data, len);
}

esp_err_t rtc_get_minutes_time(uint8_t *data, size_t len) {
    return i2c_dev_read_reg(&s_rtc, 0x05, data, len);
}

esp_err_t rtc_get_hours_time(uint8_t *data, size_t len) {
    return i2c_dev_read_reg(&s_rtc, 0x06, data, len);
}

esp_err_t rtc_set_date_time(uint8_t *data, size_t len) {
    return i2c_dev_write_reg(&s_rtc, PCF85063A_RTC_SECOND_ADDR, data, len);
}