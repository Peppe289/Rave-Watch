#ifndef __CST816_H__
#define __CST816_H__

#include <stdbool.h>
#include <stdint.h>

#define TOUCH_I2C_NUM       I2C_NUM_0
#define PIN_TOUCH_SDA       11
#define PIN_TOUCH_SCL       10
#define PIN_TOUCH_RST       13
#define PIN_TOUCH_INT       14

#define CST816_I2C_ADDR     0x15

void touch_init_i2c(void);
bool cst816_is_pressed(void);
void cst816_get_coordinates(uint16_t *x, uint16_t *y);

#endif // __CST816_H__