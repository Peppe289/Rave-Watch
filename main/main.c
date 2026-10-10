#include <i2cdev.h>
#include <stdint.h>
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
#include <string.h>
#include "vars.h"
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
#include "rtc.h"
#include "handler_lvgl.h"
#include "register_command.h"
#include <qmi8658c.h>
#include "tasks.h"
#include "wifi.h"

static const char *TAG = "main";

static lv_indev_drv_t indev_drv;

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
        .io_handle = st7789vw_get_panel_io_handle(),
        .panel_handle = st7789vw_get_panel_handle(),
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
    set_touch_indev(lv_indev_drv_register(&indev_drv));
    lvgl_port_unlock();
    return ESP_OK;
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

void app_main(void)
{
    board_power_latch();
    st7789vw_init_spi();
    ESP_ERROR_CHECK(i2cdev_init());
    touch_init_i2c();
    st7789vw_set_brightness(get_brightness_level());
    storage_nvs_init();
    rtc_init_custom();
    set_handler_suspend_mode(handler_suspend_tasks);
    wifi_init_sta();

    ESP_ERROR_CHECK(app_lvgl_init());

    battery_adc_init();
    bluedroid_init_ble_gatt();
    struct bt_cmd_entry cmds[] = {
    //    {"cmd", cmd_data},
    };
    bt_cmds_register(cmds, sizeof(cmds) / sizeof(cmds[0]));

    if (lvgl_port_lock(0)) {
        ui_init();
        lvgl_port_unlock();
    }

    start_background_tasks();
}