/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdint.h>
#include <string.h>
#include "credential.h"
#include "wifi.h"

static const char *TAG = "WIFI";

static esp_netif_t *netif = NULL;
static bool suspend_wifi = false;
static esp_event_handler_instance_t instance_any_id;
static esp_event_handler_instance_t instance_got_ip;

static wifi_external_event_handler_t s_wifi_external_event_handler = {
    .on_wifi_start = NULL,
    .on_wifi_disconnect = NULL,
    .on_got_ip = NULL,
};

void wifi_register_external_handler(wifi_external_event_handler_t *handler) {
  s_wifi_external_event_handler.on_wifi_start = handler->on_wifi_start;
  s_wifi_external_event_handler.on_wifi_disconnect = handler->on_wifi_disconnect;
  s_wifi_external_event_handler.on_got_ip = handler->on_got_ip;
}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    if (!suspend_wifi)
      esp_wifi_connect();
    if (s_wifi_external_event_handler.on_wifi_start) {
      s_wifi_external_event_handler.on_wifi_start();
    }
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    if (!suspend_wifi)
      esp_wifi_connect();
    if (s_wifi_external_event_handler.on_wifi_disconnect) {
      s_wifi_external_event_handler.on_wifi_disconnect();
    }
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "Got IP: " IPSTR "\n", IP2STR(&event->ip_info.ip));
    if (s_wifi_external_event_handler.on_got_ip) {
      s_wifi_external_event_handler.on_got_ip();
    }
  }
}

//void wifi_deinit() {
//  ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
//      WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
//  ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
//      IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
//  ESP_ERROR_CHECK(esp_wifi_disconnect());
//  ESP_ERROR_CHECK(esp_wifi_stop());
//  ESP_ERROR_CHECK(esp_wifi_deinit());
//}

void wifi_stop() {
  if (suspend_wifi) {
    return;
  }
  suspend_wifi = true;
  ESP_ERROR_CHECK(esp_wifi_disconnect());
  ESP_ERROR_CHECK(esp_wifi_stop());
}

void wifi_start() {
  if (!suspend_wifi) {
    return;
  }
  suspend_wifi = false;
  ESP_ERROR_CHECK(esp_wifi_start());
}

#ifndef WIFI_SSID
# warning "You don't put WIFI ssid. This build is only for CI"
#define WIFI_SSID "CI WIFI"
#endif

#ifndef WIFI_PASSWD
# warning "You don't put wifi passwd. This build is only for CI"
#define WIFI_PASSWD "testing purpose only"
#endif

void wifi_init_sta() {
  suspend_wifi = false;
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  if (netif == NULL) {
    netif = esp_netif_create_default_wifi_sta();
  }

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

  wifi_config_t wifi_config = {
      .sta.threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK,
  };

  strncpy((char *)wifi_config.sta.ssid, WIFI_SSID,
          sizeof(wifi_config.sta.ssid) - 1);
  wifi_config.sta.ssid[sizeof(wifi_config.sta.ssid) - 1] = '\0';
  strncpy((char *)wifi_config.sta.password, WIFI_PASSWD,
          sizeof(wifi_config.sta.password) - 1);
  wifi_config.sta.password[sizeof(wifi_config.sta.password) - 1] = '\0';

  ESP_LOGI(TAG, "Try to connect %s WIFI", wifi_config.sta.ssid);

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
}
