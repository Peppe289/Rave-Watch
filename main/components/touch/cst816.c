#include "driver/gpio.h"
#include "i2cdev.h"
#include "cst816.h"

static i2c_dev_t s_cst816 = {
    .port = TOUCH_I2C_NUM,
    .addr = CST816_I2C_ADDR,
    .addr_bit_len = I2C_ADDR_BIT_LEN_7,
    .cfg = {
        .sda_io_num = PIN_TOUCH_SDA,
        .scl_io_num = PIN_TOUCH_SCL,
        .sda_pullup_en = 1,
        .scl_pullup_en = 1,
        .master = {
            .clk_speed = 400000,
        },
    },
};

static uint16_t last_x = 0;
static uint16_t last_y = 0;
static bool last_pressed = false;

static esp_err_t cst816_read_raw(uint8_t *data, size_t len) {
    return i2c_dev_read_reg(&s_cst816, 0x01, data, len);
}

void touch_init_i2c(void) {
    if (PIN_TOUCH_RST >= 0) {
        gpio_config_t rst_conf = {
            .pin_bit_mask = (1ULL << PIN_TOUCH_RST),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&rst_conf);
        gpio_set_level(PIN_TOUCH_RST, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(PIN_TOUCH_RST, 1);
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    ESP_ERROR_CHECK(i2c_dev_create_mutex(&s_cst816));
}

bool cst816_is_pressed(void) {
    uint8_t buf[6] = {0};

    // Read 6 byte start from 0x01:
    // buf[0] -> 0x01: Gesture ID
    // buf[1] -> 0x02: Finger Num (0 = nessuno, >0 = toccato)
    // buf[2] -> 0x03: X pos [11:8]
    // buf[3] -> 0x04: X pos [7:0]
    // buf[4] -> 0x05: Y pos [11:8]
    // buf[5] -> 0x06: Y pos [7:0]
    if (cst816_read_raw(buf, sizeof(buf)) == ESP_OK) {
        uint8_t fingers = buf[1] & 0x0F;
        if (fingers > 0) {
            last_x = ((buf[2] & 0x0F) << 8) | buf[3];
            last_y = ((buf[4] & 0x0F) << 8) | buf[5];
            last_pressed = true;
            return true;
        }
    }

    last_pressed = false;
    return false;
}

void cst816_get_coordinates(uint16_t *x, uint16_t *y) {
    if (x) *x = last_x;
    if (y) *y = last_y;
}
