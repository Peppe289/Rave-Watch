#include "pwr_button.h"
#include "driver/gpio.h"

#define BOARD_SYS_EN_PIN   GPIO_NUM_41
#define BOARD_SYS_OUT_PIN  GPIO_NUM_40

void board_power_latch(void) {
    gpio_config_t en_conf = {
        .pin_bit_mask = (1ULL << BOARD_SYS_EN_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&en_conf);

    gpio_set_level(BOARD_SYS_EN_PIN, 1);

    gpio_config_t btn_conf = {
        .pin_bit_mask = (1ULL << BOARD_SYS_OUT_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&btn_conf);
}

void board_power_off(void) {
    gpio_set_level(BOARD_SYS_EN_PIN, 0);
}
