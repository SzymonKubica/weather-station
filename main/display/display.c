// Copyright (c) 2023 Szymon Kubica
// SPDX-License-Identifier: MIT
#include "display.h"
#include "ssd1306.h"

void print_weather_hourly(SSD1306_t *dev, struct ForecastHourly *forecast)
{

    const char *time_format_str = "Time:      %2d:%02d";
    const char *temp_format_str = "Temp:      %.1fC";
    const char *feel_format_str = "Feels:     %.1fC";
    const char *humi_format_str = "Humid:     %.1f%%";
    const char *rain_format_str = "Rain:        %2d%%";

    char time_buffer[26];
    char temperature_buffer[22];
    char apparent_temperature_buffer[22];
    char humidity_buffer[22];
    char precipitation_buffer[22];

    int gmt_offset = 1;
    int hour = forecast->time->tm_hour + gmt_offset;

    snprintf(time_buffer, 26, time_format_str, hour, forecast->time->tm_min);
    snprintf(temperature_buffer, 22, temp_format_str, forecast->temperature);
    snprintf(apparent_temperature_buffer, 22, feel_format_str,
             forecast->apparent_temperature);
    snprintf(humidity_buffer, 22, humi_format_str, forecast->humidity);
    snprintf(precipitation_buffer, 22, rain_format_str,
             forecast->precip_probability);

    ssd1306_display_text(dev, 0, "Weather Hourly", 16, false);
    ssd1306_display_text(dev, 1, time_buffer, 22, false);
    ssd1306_display_text(dev, 3, temperature_buffer, 22, false);
    ssd1306_display_text(dev, 4, apparent_temperature_buffer, 22, false);
    ssd1306_display_text(dev, 5, humidity_buffer, 22, false);
    ssd1306_display_text(dev, 6, precipitation_buffer, 22, false);
}
void print_weather_daily(SSD1306_t *dev, struct ForecastDaily *forecast)
{
    const char *date_format_str = "Date: %02d.%02d.%4d";
    const char *sunrise_format_str = "Sunrise:   %2d:%02d";
    const char *sunset_format_str = "Sunset:    %2d:%02d";
    const char *max_temp_format_str = "Max Temp:  %.1fC";
    const char *min_temp_format_str = "Min Temp:  %.1fC";
    const char *rain_format_str = "Rain:        %2d%%";

    // Need to be careful with the correct buffer sizes.
    char date_buffer[34];
    char sunrise_buffer[28];
    char sunset_buffer[28];
    char max_temperature_buffer[22];
    char min_temperature_buffer[22];
    char precipitation_buffer[22];

    snprintf(date_buffer, 34, date_format_str, forecast->date->tm_mday,
             (forecast->date->tm_mon + 1), forecast->date->tm_year + 1900);
    snprintf(sunrise_buffer, 28, sunrise_format_str, forecast->sunrise->tm_hour,
             forecast->sunrise->tm_min);
    snprintf(sunset_buffer, 28, sunset_format_str, forecast->sunset->tm_hour,
             forecast->sunset->tm_min);
    snprintf(max_temperature_buffer, 22, max_temp_format_str,
             forecast->max_temperature);
    snprintf(min_temperature_buffer, 22, min_temp_format_str,
             forecast->min_temperature);
    snprintf(precipitation_buffer, 22, rain_format_str,
             forecast->max_precip_probability);

    ssd1306_display_text(dev, 0, "Weather daily", 16, false);
    ssd1306_display_text(dev, 1, date_buffer, 22, false);
    ssd1306_display_text(dev, 2, sunrise_buffer, 22, false);
    ssd1306_display_text(dev, 3, sunset_buffer, 22, false);
    ssd1306_display_text(dev, 4, max_temperature_buffer, 22, false);
    ssd1306_display_text(dev, 5, min_temperature_buffer, 22, false);
    ssd1306_display_text(dev, 6, precipitation_buffer, 22, false);
}

void print_temperature_and_humidity(SSD1306_t *dev, float temperature,
                                    float humidity)
{

    char temperature_buffer[22];
    snprintf(temperature_buffer, 22, "           %.1fC", temperature);
    char humidity_buffer[22];
    snprintf(humidity_buffer, 22, "           %.1f%%", humidity);

    ssd1306_display_text(dev, 1, "Weather Station", 16, false);
    ssd1306_display_text(dev, 3, "Temperature:", 13, false);
    ssd1306_display_text(dev, 4, temperature_buffer, 22, false);
    ssd1306_display_text(dev, 5, "Humidity:", 9, false);
    ssd1306_display_text(dev, 6, humidity_buffer, 22, false);
}

void initialise_screen_device(SSD1306_t *dev)
{
    ESP_LOGI(DISPLAY_TAG, "INTERFACE is i2c");
    ESP_LOGI(DISPLAY_TAG, "CONFIG_SDA_GPIO=%d", CONFIG_SDA_GPIO);
    ESP_LOGI(DISPLAY_TAG, "CONFIG_SCL_GPIO=%d", CONFIG_SCL_GPIO);
    ESP_LOGI(DISPLAY_TAG, "CONFIG_RESET_GPIO=%d", CONFIG_RESET_GPIO);
    ESP_LOGI(DISPLAY_TAG, "Panel is 128x64");

    i2c_master_init(dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
    ssd1306_init(dev, 128, 64);
    ssd1306_clear_screen(dev, false);
}

void send_msg_to_screen(enum DisplayAction action)
{
    struct DisplayMessage *message = &display_message;
    message->requested_action = action;
    xQueueSend(display_msg_queue, (void *)&message, (TickType_t)0);
}

const char *display_mode_str[] = {
    [TEMPERATURE_AND_HUMIDITY] = "TEMPERATURE_AND_HUMIDITY",
    [SHOWING_WEATHER_DAILY] = "SHOWING_WEATHER_DAILY",
    [SHOWING_WEATHER_HOURLY] = "SHOWING_WEATHER_HOURLY",
};
