#ifndef util
#define util

#include "esp_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_flash.h"

// == function prototypes =======================================

void initialize_non_volatile_flash(void);
void wait_seconds(int seconds);

#endif
