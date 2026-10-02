#include <stdio.h>
#include <stdlib.h>

#include "esp_check.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lvgl_port.h"
#include "nvs_flash.h"
#include "bt.h"
//#include "wifi.h"
#include "st7789v2.h"
#include "brightness.h"
#include "pwr_button.h"
#include "cst816.h"
#include "battery.h"
#include "lvgl.h"
#include "ui.h"
#include "screens.h"

#include <qmi8658c.h>

static const char *TAG = "main";

static bool s_suspend_display = false;

extern esp_lcd_panel_handle_t panel_handle;
extern esp_lcd_panel_io_handle_t io_handle;

static lv_indev_drv_t indev_drv;
static lv_indev_t *touch_indev = NULL;

static TaskHandle_t battery_task_handle = NULL;
static TaskHandle_t eez_flow_task_handle = NULL;

#if LVGL_VERSION_MAJOR >= 9
static lv_display_t *lvgl_disp = NULL;
#else
static lv_disp_t *lvgl_disp = NULL;
#endif

static void touch_read_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
    static uint16_t last_x = 0;
    static uint16_t last_y = 0;

    if (cst816_is_pressed()) {
        cst816_get_coordinates(&last_x, &last_y);
        data->point.x = last_x;
        data->point.y = last_y;
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->point.x = last_x;
        data->point.y = last_y;
        data->state = LV_INDEV_STATE_REL;
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

static void eez_flow_task(void *arg) {
    while (1) {
        if (lvgl_port_lock(0)) {
            ui_tick();
            lvgl_port_unlock();
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

static esp_err_t app_lvgl_init(void) {
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,       /* LVGL task priority */
        .task_stack = 4096,       /* LVGL task stack size */
        .task_affinity = -1,      /* LVGL task pinned to core (-1 is no affinity) */
        .task_max_sleep_ms = 500, /* Maximum sleep in LVGL task */
        .timer_period_ms = 5      /* LVGL timer tick period in ms */
    };
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");

    
    /* Add LCD screen */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = LCD_H_RES * 50 * sizeof(uint16_t),
        .double_buffer = 1,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        /* Rotation values must be same as used in esp_lcd for initial settings of the screen */
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = true,
        }};
    lvgl_disp = lvgl_port_add_disp(&disp_cfg);

    if (!lvgl_disp) {
        return ESP_FAIL;
    }

    lvgl_port_lock(0);
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touch_read_cb;
    touch_indev = lv_indev_drv_register(&indev_drv);
    lvgl_port_unlock();
    return ESP_OK;
}

void freeze_background_tasks(void) {
    if (battery_task_handle != NULL) {
        vTaskSuspend(battery_task_handle);
    }
    if (eez_flow_task_handle != NULL) {
        vTaskSuspend(eez_flow_task_handle);
    }
}

void resume_background_tasks(void) {
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

void action_start_suspend_mode(lv_event_t *e) {
    freeze_background_tasks();
    suspend_display();
    if (lvgl_port_lock(0)) {
        lv_indev_enable(touch_indev, false);
        lvgl_port_unlock();
    }
    xTaskCreatePinnedToCore(resume_display_task, "resume_display_task", 2048, NULL, 5, NULL, 1);   
}

static void cmd_brightness(const char *data) {
    int level = atoi(data);
    if (level < 0) level = 0;
    if (level > 100) level = 100;
    set_brightness_level((uint8_t)level);
    ESP_LOGI(TAG, "BT: brightness = %d", level);
}

static void cmd_suspend(const char *data) {
    (void)data;
    freeze_background_tasks();
    suspend_display();
    if (lvgl_port_lock(0)) {
        lv_indev_enable(touch_indev, false);
        lvgl_port_unlock();
    }
    xTaskCreatePinnedToCore(resume_display_task, "resume_display_task", 2048, NULL, 5, NULL, 1);
}

static void cmd_resume(const char *data) {
    (void)data;
    s_suspend_display = false;
    resume_display();
    resume_background_tasks();
    if (lvgl_port_lock(0)) {
        lv_indev_enable(touch_indev, true);
        lvgl_port_unlock();
    }
}

static void cmd_battery(const char *data) {
    (void)data;
    uint32_t mv = battery_get_voltage_mv();
    ESP_LOGI(TAG, "BT: battery = %.2f V", mv / 1000.0f);
}

static void cmd_log(const char *data) {
    ESP_LOGI(TAG, "BT log: %s", data);
}

static void cmd_reboot(const char *data) {
    (void)data;
    ESP_LOGW(TAG, "BT: reboot richiesto");
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_restart();
}

static void bt_cmds_register(void) {
    bt_cmd_register("brightness", cmd_brightness);
    bt_cmd_register("suspend",    cmd_suspend);
    bt_cmd_register("resume",     cmd_resume);
    bt_cmd_register("battery",    cmd_battery);
    bt_cmd_register("log",        cmd_log);
    bt_cmd_register("reboot",     cmd_reboot);
}

static void storage_nvs_init() {
    esp_err_t ret;

    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

// TODO: implement as well this part
#include <string.h>
#include "vars.h"

char digital_clock[100] = { '0', '9',':','1','2', 0 };

const char *get_var_digital_clock() {
    return digital_clock;
}

void set_var_digital_clock(const char *value) {
    strncpy(digital_clock, value, sizeof(digital_clock) / sizeof(char));
    digital_clock[sizeof(digital_clock) / sizeof(char) - 1] = 0;
}
// ------------------------------------------------------------------------------------

void app_main(void)
{
    board_power_latch();
    st7789vw_init_spi();
    ESP_ERROR_CHECK(i2cdev_init());
    touch_init_i2c();
    st7789vw_set_brightness(get_brightness_level());
    storage_nvs_init();

    ESP_ERROR_CHECK(app_lvgl_init());

    battery_adc_init();
    bluedroid_init_ble_gatt();
    bt_cmds_register();

    if (lvgl_port_lock(0)) {
        ui_init();
        lvgl_port_unlock();
    }
    xTaskCreatePinnedToCore(battery_task, "battery_task", 2048, NULL, 2, &battery_task_handle, 0);
    xTaskCreatePinnedToCore(eez_flow_task, "eez_flow_task", 2048, NULL, 3, &eez_flow_task_handle, 1);
}