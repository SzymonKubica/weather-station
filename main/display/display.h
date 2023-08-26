#ifndef DISPLAY_H
#ifndef DISPLAY_H

#include "../libs/ssd1306/ssd1306.h"
#include "esp_log.h"

#define DISPLAY_TAG "SSD1306"

// == function prototypes =======================================

void print_temperature_and_humidity(SSD1306_t *dev, float temperature,
                                    float humidity);

void initialise_display(SSD1306_t *dev);

enum DisplayMode {
    TEMPERATURE_AND_HUMIDITY,
};

struct DisplayState {
    enum DisplayMode display_mode;
    float temperature;
    float humidity;
    bool is_on;
};

extern const char *display_mode_str[];

#endif
