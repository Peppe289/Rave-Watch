#ifndef __WORKER_TASKS_H__
#define __WORKER_TASKS_H__

#include "esp_event.h"
#include "esp_sdp_api.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "ui.h"
#include "esp_lvgl_port.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "vars.h"
#include <sys/_types.h>

typedef struct frtos_task_attr_t {
    TaskFunction_t pxTaskCode;
    char * const pcName;
    const uint32_t usStackDepth;
    void * const pvParameters;
    UBaseType_t uxPriority;
    TaskHandle_t * const pxCreatedTask;
    const BaseType_t xCoreID;
} frtos_task_attr_t;

void set_touch_indev(lv_indev_t *indev);
void handler_suspend_tasks(void *e);
void start_background_tasks(void);

#endif