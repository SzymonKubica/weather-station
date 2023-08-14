// Standard library imports
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// esp32 library imports
#include "esp_chip_info.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"

#include "hal/gpio_types.h"
#include "sdkconfig.h"

#include "driver/gpio.h"

// External library imports
#include "libs/dht/DHT.h"
#include "libs/ssd1306/font8x8_basic.h"
#include "libs/ssd1306/ssd1306.h"

// Project module imports
#include "gpio/gpio_util.h"
#include "display/display.h"
#include "util/util.h"

#define DHT_TAG "DHT"
#define BLINKER_TAG "LED_BLINKER"

static QueueHandle_t gpio_evt_queue = NULL;

void led_blinker_task(void *pvParameter) {
  ESP_LOGI(BLINKER_TAG, "Starting Led Blinker Task\n\n");
  int i = 0;
  while (true) {
    i = (i + 1) % 2;
    gpio_set_level(GPIO_OUTPUT_IO_0, i % 2);
    char *level;
    level = (i % 2 == 1) ? "HIGH" : "LOW";

    ESP_LOGI(BLINKER_TAG, "Setting the led pin %d %s\n", GPIO_OUTPUT_IO_0,
             level);
    wait_seconds(1);
  }
}


void temperature_monitor_task(void *pvParameter) {
  ESP_LOGI(DISPLAY_TAG, "Initialising the OLED display...\n\n");
  SSD1306_t dev;
  initialise_display(&dev);

  ESP_LOGI(DHT_TAG, "Initialising the DHT Sensor...\n\n");
  set_dht_gpio(GPIO_NUM_4);


  while (true) {
    ESP_LOGI(DHT_TAG, "=== Reading DHT ===\n");
    int ret = read_dht();

    handle_errors(ret);

    float humidity = get_humidity();
    float temperature = get_temperature();

    ESP_LOGI(DHT_TAG, "Hum: %.1f Tmp: %.1f\n", humidity, temperature);
    print_temperature_and_humidity(&dev, temperature, humidity);

    // -- wait at least 2 sec before reading again ------------
    // The interval of whole process must be beyond 2 seconds !!
    wait_seconds(2);
  }
}

void app_main(void) {
  configure_gpio_outputs();
  initialize_non_volatile_flash();
  esp_log_level_set("*", ESP_LOG_INFO);

  printf("Welcome to the weather station!\n");

  // Make sure the LED is off.
  gpio_set_level(GPIO_OUTPUT_IO_0, 1);

  xTaskCreate(&temperature_monitor_task, "temperature_monitor", 4096, NULL, 5,
              NULL);

  fflush(stdout);
}
