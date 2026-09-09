#include "driver/i2c.h"
#include "driver/gpio.h"
#include "cst816.h"

static uint16_t last_x = 0;
static uint16_t last_y = 0;
static bool last_pressed = false;

static esp_err_t cst816_read_raw(uint8_t *data, size_t len) {
    uint8_t reg = 0x01;
    return i2c_master_write_read_device(TOUCH_I2C_NUM, CST816_I2C_ADDR, 
                                        &reg, 1, data, len, pdMS_TO_TICKS(50));
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

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = PIN_TOUCH_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = PIN_TOUCH_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };
    ESP_ERROR_CHECK(i2c_param_config(TOUCH_I2C_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(TOUCH_I2C_NUM, conf.mode, 0, 0, 0));
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