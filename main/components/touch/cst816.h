#ifndef __CST816_H__
#define __CST816_H__

#include <stdbool.h>
#include <stdint.h>

void touch_init_i2c(void);
bool cst816_is_pressed(void);
void cst816_get_coordinates(uint16_t *x, uint16_t *y);

#endif // __CST816_H__