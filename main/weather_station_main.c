#include <inttypes.h>
#include <stdio.h>

#include "esp_chip_info.h"
#include "esp_event.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "hal/gpio_types.h"
#include "sdkconfig.h"

#include "driver/gpio.h"
#include "nvs_flash.h"

#include "libs/DHT.h"

static const char *DHT_TAG = "DHT";
static const char *BLINKER_TAG = "LED_BLINKER";

#define GPIO_OUTPUT_IO_0 CONFIG_GPIO_LED_OUTPUT
#define GPIO_OUTPUT_IO_1 CONFIG_GPIO_OUTPUT_1
#define GPIO_OUTPUT_PIN_SEL                                                    \
  ((1ULL << GPIO_OUTPUT_IO_0) | (1ULL << GPIO_OUTPUT_IO_1))
#define GPIO_INPUT_IO_0 CONFIG_GPIO_INPUT_0
#define GPIO_INPUT_IO_1 CONFIG_GPIO_INPUT_1
#define GPIO_INPUT_PIN_SEL                                                     \
  ((1ULL << GPIO_INPUT_IO_0) | (1ULL << GPIO_INPUT_IO_1))
#define ESP_INTR_FLAG_DEFAULT 0

static QueueHandle_t gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void *arg) {
  uint32_t gpio_num = (uint32_t)arg;
  xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task_example(void *arg) {
  uint32_t io_num;
  for (;;) {
    if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
      printf("GPIO[%" PRIu32 "] intr, val: %d\n", io_num,
             gpio_get_level(io_num));
    }
  }
}

void DHT_task(void *pvParameter) {
  setDHTgpio(GPIO_NUM_4);
  ESP_LOGI(DHT_TAG, "Starting DHT Task\n\n");

  while (true) {
    ESP_LOGI(DHT_TAG, "=== Reading DHT ===\n");
    int ret = readDHT();

    errorHandler(ret);

    ESP_LOGI(DHT_TAG, "Hum: %.1f Tmp: %.1f\n", getHumidity(), getTemperature());

    // -- wait at least 2 sec before reading again ------------
    // The interval of whole process must be beyond 2 seconds !!
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

void led_blinker_task(void *pvParameter) {
  ESP_LOGI(BLINKER_TAG, "Starting Led Blinker Task\n\n");
  int i = 0;
  while (true) {
    i++;
    gpio_set_level(GPIO_OUTPUT_IO_0, i % 2);
    char *level = malloc(4 * sizeof(char));
    level = (i % 2 == 1) ? "HIGH" : "LOW";

    ESP_LOGI(BLINKER_TAG, "Setting the led pin %d %s\n", GPIO_OUTPUT_IO_0,
             level);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void configure_gpio_outputs() {
  // zero-initialize the config structure.
  gpio_config_t io_conf = {};
  // disable interrupt
  io_conf.intr_type = GPIO_INTR_DISABLE;
  // set as output mode
  io_conf.mode = GPIO_MODE_OUTPUT;
  // bit mask of the pins that you want to set,e.g.GPIO18/19
  io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
  // disable pull-down mode
  io_conf.pull_down_en = 0;
  // disable pull-up mode
  io_conf.pull_up_en = 0;
  // configure GPIO with the given settings
  gpio_config(&io_conf);
}

void configure_gpio_inputs() {
  // zero-initialize the config structure.
  gpio_config_t io_conf = {};
  // interrupt of rising edge
  io_conf.intr_type = GPIO_INTR_POSEDGE;
  // bit mask of the pins, use GPIO4/5 here
  io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
  // set as input mode
  io_conf.mode = GPIO_MODE_INPUT;
  // enable pull-up mode
  io_conf.pull_up_en = 1;
  gpio_config(&io_conf);
}

void initialize_non_volatile_flash(void) {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
}

void app_main(void) {
  configure_gpio_outputs();
  initialize_non_volatile_flash();
  esp_log_level_set("*", ESP_LOG_INFO);

  printf("Welcome to the weather station!\n");

  xTaskCreate(&DHT_task, "DHT_task", 2048, NULL, 5, NULL);
  xTaskCreate(&led_blinker_task, "led_blinker_task", 2048, NULL, 5, NULL);

  fflush(stdout);
}
