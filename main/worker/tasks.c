#include "tasks.h"
#include "rtc.h"
#include "handler_lvgl.h"
#include "cst816.h"
#include "brightness.h"
#include "battery.h"

static TaskHandle_t battery_task_handle = NULL;
static TaskHandle_t eez_flow_task_handle = NULL;
static lv_indev_t *touch_indev = NULL;
static bool s_suspend_display = false;

void set_touch_indev(lv_indev_t *indev) {
    touch_indev = indev;
}

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
    if (battery_task_handle != NULL) {
        vTaskSuspend(battery_task_handle);
    }
    if (eez_flow_task_handle != NULL) {
        vTaskSuspend(eez_flow_task_handle);
    }
}

static void resume_background_tasks(void) {
    if (battery_task_handle != NULL) {
        vTaskResume(battery_task_handle);
    }
    if (eez_flow_task_handle != NULL) {
        vTaskResume(eez_flow_task_handle);
    }
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

void handler_suspend_tasks(void *e) {
    freeze_background_tasks();
    suspend_display();
    if (lvgl_port_lock(0)) {
        lv_indev_enable(touch_indev, false);
        lvgl_port_unlock();
    }
    xTaskCreatePinnedToCore(resume_display_task, "resume_display_task", 2048, NULL, 5, NULL, 1);   
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


void start_background_tasks(void) {
    xTaskCreatePinnedToCore(clock_update_task, "clock_update_task", 2048, NULL, 2, &battery_task_handle, 0);
    xTaskCreatePinnedToCore(battery_task, "battery_task", 2048, NULL, 2, &battery_task_handle, 0);
    xTaskCreatePinnedToCore(eez_flow_task, "eez_flow_task", 2048, NULL, 3, &eez_flow_task_handle, 1);
}