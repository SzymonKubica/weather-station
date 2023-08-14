#ifndef display
#define display

#include "../libs/ssd1306/ssd1306.h"
#include "esp_log.h"

#define DISPLAY_TAG "SSD1306"

// == function prototypes =======================================

void print_temperature_and_humidity(SSD1306_t *dev, float temperature,
                                    float humidity);

void initialise_display(SSD1306_t *dev);

#endif
