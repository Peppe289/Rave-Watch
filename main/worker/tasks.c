#include "tasks.h"
#include "esp_event_base.h"
#include "esp_log.h"
#include "rtc.h"
#include "handler_lvgl.h"
#include "cst816.h"
#include "brightness.h"
#include "battery.h"
#include "wifi.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_event.h"
#include <stdio.h>

static TaskHandle_t clock_update_task_handle = NULL;
static TaskHandle_t battery_task_handle = NULL;
static TaskHandle_t eez_flow_task_handle = NULL;
static lv_indev_t *touch_indev = NULL;
static bool s_suspend_display = false;

static void clock_update_task(void *arg) {
    while (1) {
        if (xSemaphoreTake(rtc_get_semaphore(), pdMS_TO_TICKS(1200)) == pdTRUE) {
            uint8_t data[1] = {0};
            uint8_t seconds = 0, minutes = 0, hours = 0;

            if (rtc_get_seconds_time(data, sizeof(data)) == ESP_OK) {
                uint8_t bcd_sec = data[0] & 0x7F;
                seconds = ((bcd_sec >> 4) * 10) + (bcd_sec & 0x0F);
            }

            if (rtc_get_minutes_time(data, sizeof(data)) == ESP_OK) {
                uint8_t bcd_min = data[0] & 0x7F;
                minutes = ((bcd_min >> 4) * 10) + (bcd_min & 0x0F);
            }

            if (rtc_get_hours_time(data, sizeof(data)) == ESP_OK) {
                uint8_t bcd_hr = data[0] & 0x3F;
                hours = ((bcd_hr >> 4) * 10) + (bcd_hr & 0x0F);
            }

            char clock_str[9];
            snprintf(clock_str, sizeof(clock_str), "%02u\n%02u", hours, minutes);
            set_var_digital_clock(clock_str);
            snprintf(clock_str, sizeof(clock_str), ":%02u", seconds);
            set_var_digital_clock_seconds(clock_str);
        }
    }
}

static void eez_flow_task(void *arg) {
    while (1) {
        if (lvgl_port_lock(0)) {
            ui_tick();
            lvgl_port_unlock();
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

static void freeze_background_tasks(void) {
    if (clock_update_task_handle != NULL) {
        vTaskSuspend(clock_update_task_handle);
    }
    if (battery_task_handle != NULL) {
        vTaskSuspend(battery_task_handle);
    }
    if (eez_flow_task_handle != NULL) {
        vTaskSuspend(eez_flow_task_handle);
    }
    wifi_stop();
}

static void resume_background_tasks(void) {
    if (clock_update_task_handle != NULL) {
        vTaskResume(clock_update_task_handle);
    }
    if (battery_task_handle != NULL) {
        vTaskResume(battery_task_handle);
    }
    if (eez_flow_task_handle != NULL) {
        vTaskResume(eez_flow_task_handle);
    }
    wifi_start();
}

static void resume_display_task(void *arg) {
    vTaskDelay(pdMS_TO_TICKS(500));
    while (1) {
        if (s_suspend_display) {
            if (cst816_is_pressed()) {
                resume_display();
                s_suspend_display = false;
                resume_background_tasks();
                if (lvgl_port_lock(0)) {
                    lv_indev_enable(touch_indev, true);
                    lvgl_port_unlock();
                }
                vTaskDelete(NULL);
            }
        } else s_suspend_display = true;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void battery_task(void *arg) {
    static uint32_t voltage_mv;
    while (1) {
        voltage_mv = battery_get_voltage_mv();
        if (voltage_mv >= 40 * 100) battery_set_ui_state(3);
        else if (voltage_mv < 40 * 100 && voltage_mv >= 35 * 100) battery_set_ui_state(2);
        else if (voltage_mv <= 34 * 100) battery_set_ui_state(1);

        if (lvgl_port_lock(0))
            lvgl_port_unlock();

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

const static frtos_task_attr_t s_task_attr[] = {
    [0] = {
        .pxTaskCode = clock_update_task,
        .pcName = "clock_update_task",
        .usStackDepth = 2048,
        .pvParameters = NULL,
        .uxPriority = 2,
        .pxCreatedTask = &clock_update_task_handle,
        .xCoreID = 0,
    },
    [1] = {
        .pxTaskCode = battery_task,
        .pcName = "battery_task",
        .usStackDepth = 2048,
        .pvParameters = NULL,
        .uxPriority = 2,
        .pxCreatedTask = &battery_task_handle,
        .xCoreID = 0,
    },
    [2] = {
        .pxTaskCode = eez_flow_task,
        .pcName = "eez_flow_task",
        .usStackDepth = 2048,
        .pvParameters = NULL,
        .uxPriority = 3,
        .pxCreatedTask = &eez_flow_task_handle,
        .xCoreID = 1,
    }
};

const static ssize_t task_attr_size = sizeof(s_task_attr) / sizeof(frtos_task_attr_t);

void start_background_tasks(void) {
    ssize_t n_task;

    for (n_task = 0; n_task < task_attr_size; n_task++) {
        xTaskCreatePinnedToCore(s_task_attr[n_task].pxTaskCode, 
            s_task_attr[n_task].pcName,
            s_task_attr[n_task].usStackDepth, 
            s_task_attr[n_task].pvParameters,
            s_task_attr[n_task].uxPriority,
            s_task_attr[n_task].pxCreatedTask,
            s_task_attr[n_task].xCoreID);
    }
}

void set_touch_indev(lv_indev_t *indev) {
    touch_indev = indev;
}

void handler_suspend_tasks(void *e) {
    freeze_background_tasks();
    suspend_display();
    if (lvgl_port_lock(0)) {
        lv_indev_enable(touch_indev, false);
        lvgl_port_unlock();
    }
    xTaskCreatePinnedToCore(resume_display_task, "resume_display_task", 2048, NULL, 5, NULL, 1);   
}