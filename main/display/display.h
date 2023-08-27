#ifndef DISPLAY_H
#define DISPLAY_H

#include "../libs/ssd1306/ssd1306.h"
#include "esp_log.h"
#include "forecast.h"
#include "system_message.h"

#define DISPLAY_TAG "SSD1306"

// == function prototypes =======================================

void print_temperature_and_humidity(SSD1306_t *dev, float temperature,
                                    float humidity);

void print_weather_hourly(SSD1306_t *dev, struct ForecastHourly *forecast);
void print_weather_daily(SSD1306_t *dev, struct ForecastDaily *forecast);
void initialise_screen_device(SSD1306_t *dev);

static void send_msg_to_screen(enum DisplayAction message);

enum DisplayMode {
    TEMPERATURE_AND_HUMIDITY,
};

struct Display {
    enum DisplayMode display_mode;
    SSD1306_t *device;
    float temperature;
    float humidity;
    struct ForecastHourly *hourly_forecast;
    struct ForecastDaily *daily_forecast;
    bool is_on;
};

extern const char *display_mode_str[];

#endif
