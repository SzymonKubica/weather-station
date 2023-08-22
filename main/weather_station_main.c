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
#include "libs/infrared-receiver/infrared_nec.h"
#include "libs/ssd1306/font8x8_basic.h"
#include "libs/ssd1306/ssd1306.h"

// Project module imports
#include "display/display.h"
#include "gpio/gpio_util.h"
#include "util/util.h"

#define DHT_TAG "DHT"
#define BLINKER_TAG "LED_BLINKER"
#define NEC_TAG "NEC"

TaskHandle_t task_0_handle = NULL;
TaskHandle_t task_1_handle = NULL;
TaskHandle_t task_2_handle = NULL;

static QueueHandle_t ir_remote_input_queue = NULL;

static void led_blinker_task(void *pvParameter);
static void temperature_monitor_task(void *pvParameter);
static void rmt_nex_rx_continuous_task();
static void disable_led_by_default();

struct QueueMessage {
  enum RemoteButton pressed_button;
} queueMessage;

void app_main(void) {
  configure_gpio_outputs();
  initialize_non_volatile_flash();
  esp_log_level_set("*", ESP_LOG_INFO);

  disable_led_by_default();

  ir_remote_input_queue = xQueueCreate(10, sizeof(struct QueueMessage *));

  xTaskCreate(&led_blinker_task, "led", 2048, NULL, 5, &task_1_handle);
  xTaskCreate(&temperature_monitor_task, "dht-22", 4096, NULL, 5,
              &task_0_handle);
  xTaskCreatePinnedToCore(&rmt_nex_rx_continuous_task, "rmt_nec_rx_task", 4096,
                          NULL, 10, &task_2_handle, 0);

  fflush(stdout);
}

static void disable_led_by_default() { gpio_set_level(GPIO_OUTPUT_IO_0, 1); }

static void led_blinker_task(void *pvParameter) {
  ESP_LOGI(BLINKER_TAG, "Starting Led Blinker Task\n\n");
  int i = 1; // Led is off by default
  struct QueueMessage *received_message;
  while (true) {
    if (xQueueReceive(ir_remote_input_queue, &(received_message), (TickType_t)5) &&
        received_message->pressed_button == BUTTON_EQ) {
      i = (i + 1) % 2;
      gpio_set_level(GPIO_OUTPUT_IO_0, i % 2);
      char *level = (i == 0) ? "HIGH" : "LOW";

      ESP_LOGI(BLINKER_TAG, "Setting the led pin %d %s\n", GPIO_OUTPUT_IO_0,
               level);
    }
  }
}

static void temperature_monitor_task(void *pvParameter) {
  ESP_LOGI(DISPLAY_TAG, "Initialising the OLED display...\n\n");
  SSD1306_t dev;
  initialise_display(&dev);

  ESP_LOGI(DHT_TAG, "Initialising the DHT Sensor...\n\n");
  set_dht_gpio(GPIO_NUM_4);

  while (true) {
    ESP_LOGI(DHT_TAG, "=== Reading DHT ===");
    int ret = read_dht();
    handle_errors(ret);

    float humidity = get_humidity();
    float temperature = get_temperature();

    ESP_LOGI(DHT_TAG, "Hum: %.1f Tmp: %.1f\n", humidity, temperature);
    print_temperature_and_humidity(&dev, temperature, humidity);

    wait_seconds(3);
  }
}

static void get_nec_ring_buffer(RingbufHandle_t *rb);

static void rmt_nex_rx_continuous_task() {
  if (!ir_remote_input_queue) {
    ESP_LOGE(NEC_TAG, "Failed to create IR Remote Input Queue");
  }

  RingbufHandle_t rb = NULL;
  get_nec_ring_buffer(&rb);

  struct QueueMessage *message = & queueMessage;
  while (rb) {
    size_t rx_size = 0;
    // try to receive data from ringbuffer.
    // RMT driver will push all the data it receives to its ringbuffer.
    // We just need to parse the value and return the spaces of ringbuffer.
    rmt_item32_t *item = (rmt_item32_t *)xRingbufferReceive(rb, &rx_size, 1000);
    if (item) {
      uint16_t rmt_addr;
      uint16_t rmt_cmd;
      int offset = 0;
      while (1) {
        // parse data value from ringbuffer.
        int res = nec_parse_items(item + offset, rx_size / 4 - offset,
                                  &rmt_addr, &rmt_cmd);
        if (res > 0) {
          offset += res + 1;
          ESP_LOGI(NEC_TAG, "RMT RCV --- addr: 0x%04x cmd: 0x%04x", rmt_addr,
                   rmt_cmd);

          message->pressed_button = mapFromInt(rmt_cmd);
          xQueueSend(ir_remote_input_queue, (void *)&message, (TickType_t)0);
          ESP_LOGI(NEC_TAG, "Button press registered: %s\n",
                   button_names[mapFromInt(rmt_cmd)]);

        } else {
          break;
        }
      }
      // after parsing the data, return spaces to ringbuffer.
      vRingbufferReturnItem(rb, (void *)item);
    }
  }
}

static void get_nec_ring_buffer(RingbufHandle_t *rb) {
  int channel = RMT_RX_CHANNEL;
  nec_rx_init();
  rmt_get_ringbuf_handle(channel, rb);
  rmt_rx_start(channel, 1);
}
