#ifndef __BRIGHTNESS_H__
#define __BRIGHTNESS_H__

#include "stdint.h"
uint8_t get_brightness_level();
void set_brightness_level(uint8_t level);
void suspend_display();
void resume_display();

#endif /* __BRIGHTNESS_H__ */