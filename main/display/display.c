#include "display.h"
#include "ssd1306.h"

void print_weather_hourly(SSD1306_t *dev, struct ForecastHourly *forecast)
{

    int gmt_offset = 1;
    int hour = forecast->time->tm_hour + gmt_offset;

    char time_buffer[26];
    snprintf(time_buffer, 26, "Time:      %2d:%02d", hour,
             forecast->time->tm_min);

    char temperature_buffer[22];
    snprintf(temperature_buffer, 22, "Temp:      %.1fC", forecast->temperature);
    char apparent_temperature_buffer[22];
    snprintf(apparent_temperature_buffer, 22, "Feels:     %.1fC",
             forecast->apparent_temperature);
    char humidity_buffer[22];
    snprintf(humidity_buffer, 22, "Humid:     %.1f%%", forecast->humidity);
    char precipitation_buffer[22];
    snprintf(precipitation_buffer, 22, "Rain:        %2d%%",
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

    // Need to be careful with the correct buffer sizes.
    char date_buffer[32];
    snprintf(date_buffer, 32, "Date: %02d.%02d.%4d", forecast->date->tm_mday,
             forecast->date->tm_mon, forecast->date->tm_year + 1900);
    char sunrise_buffer[28];
    snprintf(sunrise_buffer, 28, "Sunrise:   %2d:%02d",
             forecast->sunrise->tm_hour, forecast->sunrise->tm_min);
    char sunset_buffer[28];
    snprintf(sunset_buffer, 28, "Sunset:     %2d:%02d",
             forecast->sunset->tm_hour, forecast->sunset->tm_min);
    char max_temperature_buffer[22];
    snprintf(max_temperature_buffer, 22, "Max Temp:  %.1fC",
             forecast->max_temperature);
    char min_temperature_buffer[22];
    snprintf(min_temperature_buffer, 22, "Min Temp:  %.1fC",
             forecast->min_temperature);
    char precipitation_buffer[22];
    snprintf(precipitation_buffer, 22, "Rain:        %2d%%",
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
