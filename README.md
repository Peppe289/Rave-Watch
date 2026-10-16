# Rave-Watch

## 📐 Architecture and Tech Stack

| Aspect | Technology |
| ------- | ----------- |
| Target / Device | **ESP32-S3** (xtensa) |
| IDF Framework | **ESP-IDF 6.0** |
| Graphical UI | **LVGL 8.3.11** |
| LVGL Port | **esp_lvgl_port ^2.3.0** |
| Display Driver | **ST7789v2** (`esp_lcd`) |
| Network | ESP-WiFi / ESP-Netif (component `network`) |
| BT | Bluedroid / GATT (`components/bt`) |
| Sensors | QMI8658c, CST816 (touch) |
| Measurements | ADC (ESP-ADC) for battery voltage |
| I2C Network | i2cdev (master/slave driver) |
| UI Framework | **EEZ-ui** (custom flow-based) |
| Build CI | **GitHub Actions** (`builder.yml`) |

---

## 📋 Functional Requirements

Here are the functional requirements that Rave-Watch covers, verified against the current implementation:

1. **Real-time TFT display** on ESP32-S3 with ST7789v2 driver (240 × 280 screen, RGB colors, double buffer and DMA).
2. **Digital clock from RTC** updated in the background (time + seconds).
3. **Dynamic battery state display** (full / 50% / low) based on the measured voltage via ADC.
4. **Battery voltage visualizer** via the `power` component.
5. **Brightness control** via slider and hardcoded value, sent to the display.
6. **Touch input** (CST816) handled as indev pointer LVGL.
7. **UI navigation** between 3 screens (Main / SettingsPage / DrawerPage).
8. **Bluetooth BT** with on/off toggle and remote handler.
9. **Bluetooth RSSI** show rssi power of connected device.
10. **WiFi support** (network component, station init available for CI builds).
11. **Suspend/resume mode** with background task blocking and screen suspension.
12. **I2C management** for devices (RTC) via master/slave driver.
13. **RTC** for reading time (reading via I2C, 1 Hz interrupt).
14. **Build and CI** via ESP-IDF with GitHub Actions workflow.
15. **Performance optimization** of build/sdkconfig.

