#include "display.h"
#include "ssd1306.h"

void print_weather_hourly(SSD1306_t *dev, struct ForecastHourly *forecast)
{
    ssd1306_display_text(dev, 0, "Weather Hourly", 16, false);
    char temperature_buffer[22];
    snprintf(temperature_buffer, 22, "           %.1fC", forecast->temperature);
    char humidity_buffer[22];
    snprintf(humidity_buffer, 22, "           %.1f%%", forecast->humidity);
    char precipitation_buffer[22];
    snprintf(precipitation_buffer, 22, "             %2d%%", forecast->precip_probability);
    ssd1306_display_text(dev, 1, "Temperature:", 13, false);
    ssd1306_display_text(dev, 2, temperature_buffer, 22, false);
    ssd1306_display_text(dev, 3, "Humidity:", 9, false);
    ssd1306_display_text(dev, 4, humidity_buffer, 22, false);
    ssd1306_display_text(dev, 5, "Precipitation:", 15, false);
    ssd1306_display_text(dev, 6, precipitation_buffer, 22, false);
}
void print_weather_daily(SSD1306_t *dev, struct ForecastDaily *forecast)
{
    ssd1306_display_text(dev, 0, "Weather daily", 16, false);
    char max_temperature_buffer[22];
    snprintf(max_temperature_buffer, 22, "           %.1fC", forecast->max_temperature);
    char min_temperature_buffer[22];
    snprintf(min_temperature_buffer, 22, "           %.1fC", forecast->min_temperature);
    char precipitation_buffer[22];
    snprintf(precipitation_buffer, 22, "             %2d%%", forecast->max_precip_probability);
    ssd1306_display_text(dev, 1, "Max Temperature:", 17, false);
    ssd1306_display_text(dev, 2, max_temperature_buffer, 22, false);
    ssd1306_display_text(dev, 3, "Min Temperature:", 17, false);
    ssd1306_display_text(dev, 4, min_temperature_buffer, 22, false);
    ssd1306_display_text(dev, 5, "Precipitation:", 15, false);
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
    [SHOWING_WEATHER_NOW] = "SHOWING_WEATHER_NOW",
    [SHOWING_WEATHER_TODAY] = "SHOWING_WEATHER_TODAY",
    [SHOWING_WEATHER_TOMORROW] = "SHOWING_WEATHER_TOMORROW",
    [SHOWING_WEATHER_T2] = "SHOWING_WEATHER_T2",
};
