#include "display.h"
#include "ssd1306.h"

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

void initialise_display(SSD1306_t *dev)
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
