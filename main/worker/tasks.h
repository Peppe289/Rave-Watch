#ifndef __WORKER_TASKS_H__
#define __WORKER_TASKS_H__

#include "lvgl.h"
#include "ui.h"
#include "esp_lvgl_port.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "vars.h"

void set_touch_indev(lv_indev_t *indev);
void handler_suspend_tasks(void *e);
void start_background_tasks(void);

#endif