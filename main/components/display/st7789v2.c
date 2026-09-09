#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"

#include "st7789v2.h"

#define LCD_BL_PWM_TIMER LEDC_TIMER_0
#define LCD_BL_PWM_CH    LEDC_CHANNEL_0

esp_lcd_panel_handle_t panel_handle = NULL;
esp_lcd_panel_io_handle_t io_handle = NULL; // Esportato per registrare il callback

void st7789vw_draw_bitmap(int x_start, int y_start, int x_end, int y_end,
                          const void *color_data) {
  esp_lcd_panel_draw_bitmap(panel_handle, x_start, y_start, x_end, y_end,
                            color_data);
}

void st7789vw_draw_full_bitmap(const void *color_data) {
  st7789vw_draw_bitmap(0, 0, LCD_H_RES, LCD_V_RES, color_data);
}

void st7789vw_set_brightness(uint8_t percentage) {
  if (percentage > 100) percentage = 100;
  uint32_t duty = (percentage * 255) / 100;
  ledc_set_duty(LEDC_LOW_SPEED_MODE, LCD_BL_PWM_CH, duty);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, LCD_BL_PWM_CH);
}

void st7789vw_init_spi(void) {
  // 1. PWM Retroilluminazione
  ledc_timer_config_t ledc_timer = {
      .speed_mode       = LEDC_LOW_SPEED_MODE,
      .timer_num        = LCD_BL_PWM_TIMER,
      .duty_resolution  = LEDC_TIMER_8_BIT,
      .freq_hz          = 5000,
      .clk_cfg          = LEDC_AUTO_CLK,
  };
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

  ledc_channel_config_t ledc_channel = {
      .speed_mode     = LEDC_LOW_SPEED_MODE,
      .channel        = LCD_BL_PWM_CH,
      .timer_sel      = LCD_BL_PWM_TIMER,
      .intr_type      = LEDC_INTR_DISABLE,
      .gpio_num       = PIN_BL,
      .duty           = 255,
      .hpoint         = 0,
  };
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

  // 2. Bus SPI
  spi_bus_config_t buscfg = {
      .sclk_io_num = PIN_SCL,
      .mosi_io_num = PIN_SDA,
      .miso_io_num = -1,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = LCD_H_RES * 40 * sizeof(uint16_t),
  };
  ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

  // 3. IO Config
  esp_lcd_panel_io_spi_config_t io_config = {
      .dc_gpio_num = PIN_DC,
      .cs_gpio_num = PIN_CS,
      .pclk_hz = 80 * 1000 * 1000,
      .spi_mode = 0,
      .trans_queue_depth = 10,
      .lcd_cmd_bits = 8,
      .lcd_param_bits = 8,
  };
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST,
                                           &io_config, &io_handle));

  // 4. Panel Config
  esp_lcd_panel_dev_config_t panel_config = {
      .reset_gpio_num = PIN_RES,
      .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
      .bits_per_pixel = 16,
  };
  ESP_ERROR_CHECK(
      esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
  
  // Gap standard per pannello 1.69" 240x280 centrato
  esp_lcd_panel_set_gap(panel_handle, 0, 20);

  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
}