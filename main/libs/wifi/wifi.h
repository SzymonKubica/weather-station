#ifndef WIFI_H
#define WIFI_H

#include <stdint.h>

void wifi_init_sta(uint8_t *wifi_ssid, uint8_t *wifi_pass);
void wifi_init_sta_default(void);

#endif
