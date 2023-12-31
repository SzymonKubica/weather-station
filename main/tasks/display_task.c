// Copyright (c) 2023 Szymon Kubica
// SPDX-License-Identifier: MIT
#include "../libs/ssd1306/font8x8_basic.h"
#include "../libs/ssd1306/ssd1306.h"

#include "display.h"
#include "system_action.h"
#include "system_message.h"

/*
 * Display task is responsible for displaying information on the
 * OLED display. It listens to the display message queue and reacts
 * to requests that appear on that queue.
 */

static SSD1306_t dev;

static void initialize_display(struct Display *display);
static void screen_on(struct Display *display);
static void screen_off(struct Display *display);
static void show_temperature(struct Display *display);
static void show_weather_hourly(struct Display *display);
static void show_weather_daily(struct Display *display);
static void update_display_mode(struct Display *display,
                                enum DisplayMode new_setting);

void display_task(void *pvParameter)
{
    struct Display display;
    initialize_display(&display);

    struct DisplayMessage *received_message;

    while (true) {
        if (xQueueReceive(display_msg_queue, &(received_message),
                          (TickType_t)5)) {

            enum DisplayAction action = received_message->requested_action;
            ESP_LOGI(DISPLAY_TAG, "Display action requested: %s",
                     display_action_str[action]);
            switch (action) {
            case SCREEN_ON:
                screen_on(&display);
                break;
            case SCREEN_OFF:
                screen_off(&display);
                break;
            case UPDATE_DHT_READING:
                display.temperature = received_message->temperature;
                display.humidity = received_message->humidity;
                show_temperature(&display);
                break;
            case SHOW_DHT_READING:
                update_display_mode(&display, TEMPERATURE_AND_HUMIDITY);
                show_temperature(&display);
                break;
            case SHOW_WEATHER_HOURLY:
                update_display_mode(&display, SHOWING_WEATHER_HOURLY);
                display.hourly_forecast = received_message->hourly_forecast;
                show_weather_hourly(&display);
                break;
            case SHOW_WEATHER_DAILY:
                update_display_mode(&display, SHOWING_WEATHER_DAILY);
                display.daily_forecast = received_message->daily_forecast;
                show_weather_daily(&display);
                break;
            }
        }
    }
}

static void initialize_display(struct Display *display)
{
    ESP_LOGI(DISPLAY_TAG, "Initialising the OLED display...\n\n");
    initialise_screen_device(&dev);
    display->display_mode = TEMPERATURE_AND_HUMIDITY;
    display->device = &dev;
    display->is_on = true;
    display->temperature = 0.0;
    display->humidity = 0.0;
}

static void update_display_mode(struct Display *display,
                                enum DisplayMode new_setting)
{
    if (display->display_mode == new_setting) {
        return;
    }
    display->display_mode = new_setting;
    ssd1306_clear_screen(display->device, false);
}

static void screen_on(struct Display *display)
{
    display->is_on = true;
    display->display_mode = TEMPERATURE_AND_HUMIDITY;
    show_temperature(display);
}

static void screen_off(struct Display *display)
{
    ssd1306_clear_screen(display->device, false);
    display->is_on = false;
}

static void show_temperature(struct Display *display)
{
    if (display->is_on && display->display_mode == TEMPERATURE_AND_HUMIDITY) {
        print_temperature_and_humidity(display->device, display->temperature,
                                       display->humidity);
    }
}

static void show_weather_hourly(struct Display *display)
{
    if (display->is_on && display->display_mode == SHOWING_WEATHER_HOURLY) {
        print_weather_hourly(display->device, display->hourly_forecast);
    }
}

static void show_weather_daily(struct Display *display)
{
    if (display->is_on && display->display_mode == SHOWING_WEATHER_DAILY) {
        print_weather_daily(display->device, display->daily_forecast);
    }
}
