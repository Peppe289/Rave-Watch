#ifndef __ST7789V2_H__
#define __ST7789V2_H__
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"

#define PIN_SCL  6
#define PIN_SDA  7
#define PIN_RES  8
#define PIN_DC   4
#define PIN_CS   5
#define PIN_BL   15

#define LCD_H_RES 240
#define LCD_V_RES 280

#define LCD_BL_PWM_TIMER LEDC_TIMER_0
#define LCD_BL_PWM_CH    LEDC_CHANNEL_0

esp_err_t st7789vw_init_spi(void);
void st7789vw_draw_bitmap(int x_start, int y_start, int x_end, int y_end, const void *color_data);
void st7789vw_draw_full_bitmap(const void *color_data);
void st7789vw_set_brightness(uint8_t percentage);

#endif // __ST7789V2_H__