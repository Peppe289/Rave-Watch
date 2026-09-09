#ifndef __BRIGHTNESS_H__
#define __BRIGHTNESS_H__

#include "stdint.h"
uint8_t get_brightness_level();
void suspend_display();
void resume_display();

#endif /* __BRIGHTNESS_H__ */