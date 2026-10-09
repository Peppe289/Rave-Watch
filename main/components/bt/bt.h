#ifndef __BLUEDROID_BLE_GATT_H__
#define __BLUEDROID_BLE_GATT_H__

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BT_CMD_MAX_ENTRIES 16
#define BT_CMD_NAME_MAX    16

void bluedroid_init_ble_gatt(void);

typedef void (*bt_cmd_handler_t)(const char *data);
esp_err_t bt_cmd_register(const char *cmd, bt_cmd_handler_t handler);
esp_err_t bt_cmd_unregister(const char *cmd);

typedef struct bt_cmd_entry {
    char name[BT_CMD_NAME_MAX];
    bt_cmd_handler_t handler;
} bt_cmd_entry_t;

#ifdef __cplusplus
}
#endif

#endif
