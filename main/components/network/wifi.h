#ifndef __WIFI_CONN_H__
#define __WIFI_CONN_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct wifi_external_event_handler_t {
  void (*on_wifi_start)(void);
  void (*on_wifi_disconnect)(void);
  void (*on_got_ip)(void);
} wifi_external_event_handler_t;

void wifi_init_sta(void);
void wifi_register_external_handler(wifi_external_event_handler_t *handler);

void wifi_stop();
void wifi_start();

#endif // __WIFI_CONN_H__