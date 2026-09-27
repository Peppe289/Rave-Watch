#include "esp_bt.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "esp_gatts_api.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "screens.h"
#include "images.h"
#include "widgets/lv_img.h"

#include "bt.h"

#define GATTS_TABLE_TAG "BLE_GATT"
#define BT_CMD_TAG      "BT_CMD"

#define PROFILE_NUM 1
#define PROFILE_APP_IDX 0
#define ESP_APP_ID 0x55
#define DEVICE_NAME "RaveWatch"
#define SVC_INST_ID 0

#define GATTS_MAX_CHAR_LEN 512
#define CHAR_DECLARATION_SIZE (sizeof(uint8_t))

enum {
    IDX_SVC,
    IDX_CHAR_STR,
    IDX_CHAR_VAL_STR,
    HRS_IDX_NB,
};

uint16_t gatt_handle_table[HRS_IDX_NB];

static const uint16_t GATTS_SERVICE_UUID = 0x00FF;
static const uint16_t GATTS_CHAR_UUID_STR = 0xFF01;

#define BT_CMD_MAX_ENTRIES 16
#define BT_CMD_NAME_MAX    16

struct bt_cmd_entry {
    char name[BT_CMD_NAME_MAX];
    bt_cmd_handler_t handler;
};

static struct bt_cmd_entry s_cmd_table[BT_CMD_MAX_ENTRIES];
static portMUX_TYPE s_cmd_mux = portMUX_INITIALIZER_UNLOCKED;

#define BT_CMD_QUEUE_LEN 8

typedef struct {
    char buf[GATTS_MAX_CHAR_LEN];
} bt_cmd_msg_t;

static QueueHandle_t s_cmd_queue = NULL;

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

static bool s_bt_enabled = true;
static bool s_bt_toggling = false;

static void bt_stack_disable(void) {
    uint8_t inst = 0;
    esp_ble_gap_ext_adv_stop(1, &inst);
    vTaskDelay(pdMS_TO_TICKS(100));

    if (esp_bluedroid_disable() != ESP_OK) {
        ESP_LOGE(GATTS_TABLE_TAG, "esp_bluedroid_disable() failled");
    }
    if (esp_bt_controller_disable() != ESP_OK) {
        ESP_LOGE(GATTS_TABLE_TAG, "esp_bt_controller_disable() failled");
    }
    ESP_LOGI(GATTS_TABLE_TAG, "Bluetooth disabled");
}

static void bt_stack_enable(void) {
    if (esp_bt_controller_enable(ESP_BT_MODE_BLE) != ESP_OK) {
        ESP_LOGE(GATTS_TABLE_TAG, "esp_bt_controller_enable() failled");
        return;
    }
    if (esp_bluedroid_enable() != ESP_OK) {
        ESP_LOGE(GATTS_TABLE_TAG, "esp_bluedroid_enable() failled");
        return;
    }

    esp_ble_gatts_register_callback(gatts_event_handler);
    esp_ble_gap_register_callback(gap_event_handler);
    esp_ble_gatts_app_register(ESP_APP_ID);
    esp_ble_gatt_set_local_mtu(512);

    ESP_LOGI(GATTS_TABLE_TAG, "Bluetooth restarted");
}

static void bt_toggle_task(void *arg) {
    bool enable = *(bool *)arg;
    free(arg);

    if (enable) {
        bt_stack_enable();
    } else {
        bt_stack_disable();
    }
    s_bt_toggling = false;
    vTaskDelete(NULL);
}

void action_toggle_bt(lv_event_t *e) {
    (void)e;
    if (s_bt_toggling) return;

    s_bt_enabled = !s_bt_enabled;
    s_bt_toggling = true;

    if (objects.img_bt != NULL) {
        lv_img_set_src(objects.img_bt,
                       s_bt_enabled ? &img_bluetooth_bit_map
                                    : &img_bluetooth_disabled_bit_map);
    }

    ESP_LOGI(GATTS_TABLE_TAG, "Toggle BT -> %s", s_bt_enabled ? "ON" : "OFF");

    bool *param = malloc(sizeof(bool));
    if (param == NULL) {
        s_bt_toggling = false;
        return;
    }
    *param = s_bt_enabled;
    xTaskCreatePinnedToCore(bt_toggle_task, "bt_toggle", 4096, param, 3, NULL, 0);
}

static void bt_cmd_dispatch_task(void *arg) {
    bt_cmd_msg_t msg;
    for (;;) {
        if (xQueueReceive(s_cmd_queue, &msg, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        char *sep = strchr(msg.buf, ';');
        char cmd[BT_CMD_NAME_MAX] = {0};
        const char *data = "";

        if (sep != NULL) {
            size_t cmd_len = (size_t)(sep - msg.buf);
            if (cmd_len >= sizeof(cmd)) cmd_len = sizeof(cmd) - 1;
            memcpy(cmd, msg.buf, cmd_len);
            data = sep + 1;
        } else {
            size_t cmd_len = strlen(msg.buf);
            if (cmd_len >= sizeof(cmd)) cmd_len = sizeof(cmd) - 1;
            memcpy(cmd, msg.buf, cmd_len);
        }

        ESP_LOGI(BT_CMD_TAG, "Command: '%s' | Data: '%s'", cmd, data);

        bt_cmd_handler_t handler = NULL;
        {
            taskENTER_CRITICAL(&s_cmd_mux);
            for (int i = 0; i < BT_CMD_MAX_ENTRIES; i++) {
                if (s_cmd_table[i].handler != NULL &&
                    strncmp(s_cmd_table[i].name, cmd, BT_CMD_NAME_MAX) == 0) {
                    handler = s_cmd_table[i].handler;
                    break;
                }
            }
            taskEXIT_CRITICAL(&s_cmd_mux);
        }

        if (handler != NULL) {
            handler(data);
        } else {
            ESP_LOGW(BT_CMD_TAG, "Unknow command: '%s'", cmd);
        }
    }
}

esp_err_t bt_cmd_register(const char *cmd, bt_cmd_handler_t handler) {
    if (cmd == NULL || handler == NULL ||
        strlen(cmd) == 0 || strlen(cmd) >= BT_CMD_NAME_MAX) {
        return ESP_ERR_INVALID_ARG;
    }

    taskENTER_CRITICAL(&s_cmd_mux);
    for (int i = 0; i < BT_CMD_MAX_ENTRIES; i++) {
        if (s_cmd_table[i].handler == NULL ||
            strncmp(s_cmd_table[i].name, cmd, BT_CMD_NAME_MAX) == 0) {
            strncpy(s_cmd_table[i].name, cmd, BT_CMD_NAME_MAX - 1);
            s_cmd_table[i].name[BT_CMD_NAME_MAX - 1] = '\0';
            s_cmd_table[i].handler = handler;
            taskEXIT_CRITICAL(&s_cmd_mux);
            return ESP_OK;
        }
    }
    taskEXIT_CRITICAL(&s_cmd_mux);
    return ESP_ERR_NOT_FOUND;
}

esp_err_t bt_cmd_unregister(const char *cmd) {
    if (cmd == NULL) return ESP_ERR_INVALID_ARG;

    taskENTER_CRITICAL(&s_cmd_mux);
    for (int i = 0; i < BT_CMD_MAX_ENTRIES; i++) {
        if (s_cmd_table[i].handler != NULL &&
            strncmp(s_cmd_table[i].name, cmd, BT_CMD_NAME_MAX) == 0) {
            s_cmd_table[i].handler = NULL;
            s_cmd_table[i].name[0] = '\0';
            taskEXIT_CRITICAL(&s_cmd_mux);
            return ESP_OK;
        }
    }
    taskEXIT_CRITICAL(&s_cmd_mux);
    return ESP_ERR_NOT_FOUND;
}

static void bt_cmd_enqueue(const uint8_t *value, uint16_t len) {
    if (s_cmd_queue == NULL) return;

    static bt_cmd_msg_t msg;
    if (len >= GATTS_MAX_CHAR_LEN) len = GATTS_MAX_CHAR_LEN - 1;
    memcpy(msg.buf, value, len);
    msg.buf[len] = '\0';

    if (xQueueSend(s_cmd_queue, &msg, 0) != pdTRUE) {
        ESP_LOGW(BT_CMD_TAG, "Coda comandi piena, messaggio scartato");
    }
}

static esp_ble_gap_ext_adv_params_t ext_adv_params = {
    .type = ESP_BLE_GAP_SET_EXT_ADV_PROP_CONNECTABLE,
    .interval_min = 0x20,
    .interval_max = 0x40,
    .channel_map = ADV_CHNL_ALL,
    .filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    .primary_phy = ESP_BLE_GAP_PHY_1M,
    .max_skip = 0,
    .secondary_phy = ESP_BLE_GAP_PHY_2M,
    .sid = 0,
    .scan_req_notif = false,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .tx_power = EXT_ADV_TX_PWR_NO_PREFERENCE,
};

static uint8_t ext_adv_raw_data[] = {
    0x02, ESP_BLE_AD_TYPE_FLAG, (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
    0x0A, ESP_BLE_AD_TYPE_NAME_CMPL, 'R', 'a', 'v', 'e', 'W', 'a', 't', 'c', 'h',
    0x03, ESP_BLE_AD_TYPE_16SRV_CMPL, 0xFF, 0x00
};

struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
};

static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

static struct gatts_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_APP_IDX] = {
        .gatts_cb = gatts_profile_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,
    },
};

static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static const uint8_t char_prop_read_write = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ;
static uint8_t char_value[GATTS_MAX_CHAR_LEN] = {0};

static const esp_gatts_attr_db_t gatt_db[HRS_IDX_NB] = {
    [IDX_SVC] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
         sizeof(uint16_t), sizeof(GATTS_SERVICE_UUID), (uint8_t *)&GATTS_SERVICE_UUID}
    },
    [IDX_CHAR_STR] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
         CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write}
    },
    [IDX_CHAR_VAL_STR] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_STR, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
         GATTS_MAX_CHAR_LEN, GATTS_MAX_CHAR_LEN, (uint8_t *)char_value}
    },
};

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
        case ESP_GAP_BLE_EXT_ADV_SET_PARAMS_COMPLETE_EVT:
            esp_ble_gap_config_ext_adv_data_raw(0, sizeof(ext_adv_raw_data), ext_adv_raw_data);
            break;

        case ESP_GAP_BLE_EXT_ADV_DATA_SET_COMPLETE_EVT: {
            esp_ble_gap_ext_adv_t ext_adv[1] = {
                [0] = { .instance = 0, .duration = 0, .max_events = 0 }
            };
            esp_ble_gap_ext_adv_start(1, ext_adv);
            break;
        }
        case ESP_GAP_BLE_EXT_ADV_START_COMPLETE_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "Advertising BLE 5.0 started!");
            break;
        default:
            break;
    }
}

static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    switch (event) {
        case ESP_GATTS_REG_EVT:
            esp_ble_gap_set_device_name(DEVICE_NAME);
            esp_ble_gap_ext_adv_set_params(0, &ext_adv_params);

            esp_ble_gatts_create_attr_tab(gatt_db, gatts_if, HRS_IDX_NB, SVC_INST_ID);
            break;

        case ESP_GATTS_WRITE_EVT:
            if (!param->write.is_prep) {
                ESP_LOGI(GATTS_TABLE_TAG, "%d bytes", param->write.len);
                bt_cmd_enqueue(param->write.value, param->write.len);
            }
            break;

        case ESP_GATTS_CONNECT_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "Device connected (conn_id = %d)", param->connect.conn_id);
            break;

        case ESP_GATTS_DISCONNECT_EVT:
            if (!s_bt_enabled) break;
            ESP_LOGI(GATTS_TABLE_TAG, "Device disconnected, restart advertising...");
            /* Riavvia l'istanza 0 */
            esp_ble_gap_ext_adv_t ext_adv[1] = {
                [0] = { .instance = 0, .duration = 0, .max_events = 0 }
            };
            esp_ble_gap_ext_adv_start(1, ext_adv);
            break;

        case ESP_GATTS_CREAT_ATTR_TAB_EVT:
            if (param->add_attr_tab.status == ESP_GATT_OK) {
                memcpy(gatt_handle_table, param->add_attr_tab.handles, sizeof(gatt_handle_table));
                esp_ble_gatts_start_service(gatt_handle_table[IDX_SVC]);
            }
            break;

        default:
            break;
    }
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    if (event == ESP_GATTS_REG_EVT && param->reg.status == ESP_GATT_OK) {
        gl_profile_tab[PROFILE_APP_IDX].gatts_if = gatts_if;
    }
    for (int idx = 0; idx < PROFILE_NUM; idx++) {
        if (gatts_if == ESP_GATT_IF_NONE || gatts_if == gl_profile_tab[idx].gatts_if) {
            if (gl_profile_tab[idx].gatts_cb) gl_profile_tab[idx].gatts_cb(event, gatts_if, param);
        }
    }
}

void bluedroid_init_ble_gatt(void) {
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));

    esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bluedroid_init_with_cfg(&bluedroid_cfg));
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    ESP_ERROR_CHECK(esp_ble_gatts_register_callback(gatts_event_handler));
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(gap_event_handler));
    ESP_ERROR_CHECK(esp_ble_gatts_app_register(ESP_APP_ID));

    ESP_ERROR_CHECK(esp_ble_gatt_set_local_mtu(512));

    s_cmd_queue = xQueueCreate(BT_CMD_QUEUE_LEN, sizeof(bt_cmd_msg_t));
    xTaskCreatePinnedToCore(bt_cmd_dispatch_task, "bt_cmd", 6144, NULL, 5, NULL, 1);
}
