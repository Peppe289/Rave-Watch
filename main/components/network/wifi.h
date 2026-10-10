#ifndef __WIFI_CONN_H__
#define __WIFI_CONN_H__

#include <stdbool.h>
#include <stdint.h>

void wifi_init_sta(void);
void wifi_register_system_handler(void (*wifi_state_handler)(uint32_t bit,
                                                             bool add_bit),
                                  uint32_t (*system_get_state)(void));

void wifi_stop();
void wifi_start();

#endif // __WIFI_CONN_H__