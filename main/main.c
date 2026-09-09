#include <stdio.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
//#include "esp_chip_info.h"
//#include "esp_flash.h"
//#include "esp_system.h"
//#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"

#include "st7789v2.h"
#include "pwr_button.h"
#include "cst816.h"
#include "voltage.h"
#include "lvgl.h"
#include "ui.h"

#define LCD_H_RES 240
#define LCD_V_RES 280
#define LVGL_BUF_LINES 140
#define DISP_BUF_SIZE (LCD_H_RES * LVGL_BUF_LINES)

extern esp_lcd_panel_handle_t panel_handle;
extern esp_lcd_panel_io_handle_t io_handle;

static lv_disp_drv_t disp_drv;
static lv_disp_draw_buf_t disp_buf;
static lv_color_t *buf1 = NULL;
static lv_color_t *buf2 = NULL;

static SemaphoreHandle_t lvgl_mutex = NULL;

static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, 
                                    esp_lcd_panel_io_event_data_t *edata, 
                                    void *user_ctx) {
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map) {
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map);
}

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

static void lvgl_handler_task(void *arg) {
    while (1) {
        lv_tick_inc(10);

        if (xSemaphoreTake(lvgl_mutex, portMAX_DELAY) == pdTRUE) {
            lv_timer_handler();
            ui_tick();
            xSemaphoreGive(lvgl_mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void battery_task(void *arg) {
    char voltage_str[16];

    while (1) {
        uint32_t voltage_mv = battery_get_voltage_mv();
        snprintf(voltage_str, sizeof(voltage_str), "%.2f V", voltage_mv / 1000.0f);

        if (xSemaphoreTake(lvgl_mutex, portMAX_DELAY) == pdTRUE) {
            set_var_battery_voltage(voltage_str);
            xSemaphoreGive(lvgl_mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void)
{
    board_power_latch();
    st7789vw_init_spi();
    touch_init_i2c();
    st7789vw_set_brightness(10);

    lvgl_mutex = xSemaphoreCreateMutex();
    assert(lvgl_mutex != NULL);

    lv_init();
    battery_adc_init();

    buf1 = heap_caps_aligned_alloc(4, DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    buf2 = heap_caps_aligned_alloc(4, DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    assert(buf1 != NULL && buf2 != NULL);

    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, DISP_BUF_SIZE);

    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_H_RES;
    disp_drv.ver_res = LCD_V_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touch_read_cb;
    lv_indev_drv_register(&indev_drv);

    const esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = notify_lvgl_flush_ready,
    };
    ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(io_handle, &cbs, &disp_drv));

    ui_init();

    xTaskCreatePinnedToCore(battery_task, "battery_task", 2048, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(lvgl_handler_task, "lvgl_handler", 8192, NULL, 5, NULL, 1);
}