#ifndef __BLUEDROID_BLE_GATT_H__
#define __BLUEDROID_BLE_GATT_H__

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

void bluedroid_init_ble_gatt(void);

typedef void (*bt_cmd_handler_t)(const char *data);
esp_err_t bt_cmd_register(const char *cmd, bt_cmd_handler_t handler);
esp_err_t bt_cmd_unregister(const char *cmd);

#ifdef __cplusplus
}
#endif

#endif
