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
#include "libs/infrared-receiver/infrared_nec.h"

// Project module imports
#include "display/display.h"
#include "gpio/gpio_util.h"
#include "util/util.h"
#include "util/logging.h"

#define DHT_TAG "DHT"
#define BLINKER_TAG "LED_BLINKER"
#define NEC_TAG "NEC"

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
    wait_seconds(3);
  }
}

/**
 * @brief RMT receiver demo, this task will print each received NEC data.
 *
 */
static void rmt_nex_rx_continuous_task() {
  int channel = RMT_RX_CHANNEL;
  nec_rx_init();
  RingbufHandle_t rb = NULL;
  // get RMT RX ringbuffer
  rmt_get_ringbuf_handle(channel, &rb);
  rmt_rx_start(channel, 1);
  while (rb) {
    size_t rx_size = 0;
    // try to receive data from ringbuffer.
    // RMT driver will push all the data it receives to its ringbuffer.
    // We just need to parse the value and return the spaces of ringbuffer.
    rmt_item32_t *item = (rmt_item32_t *)xRingbufferReceive(rb, &rx_size, 1000);
    printf("Item received\n");
    if (item) {
      uint16_t rmt_addr;
      uint16_t rmt_cmd;
      int offset = 0;
      while (1) {
        // parse data value from ringbuffer.
        int res = nec_parse_items(item + offset, rx_size / 4 - offset,
                                  &rmt_addr, &rmt_cmd);
        printf("Response: %d", res);
        if (res > 0) {
          offset += res + 1;
          ESP_LOGI(NEC_TAG, "RMT RCV --- addr: 0x%04x cmd: 0x%04x", rmt_addr,
                   rmt_cmd);
        } else {
          break;
        }
      }
      // after parsing the data, return spaces to ringbuffer.
      vRingbufferReturnItem(rb, (void *)item);
    }
  }
}
static void rmt_nex_rx_receive_and_stop_after_timeout_task() {
  int channel = RMT_RX_CHANNEL;
  nec_rx_init();
  RingbufHandle_t rb = NULL;
  // get RMT RX ringbuffer
  rmt_get_ringbuf_handle(channel, &rb);
  rmt_rx_start(channel, 1);
  while (rb) {
    size_t rx_size = 0;
    // try to receive data from ringbuffer.
    // RMT driver will push all the data it receives to its ringbuffer.
    // We just need to parse the value and return the spaces of ringbuffer.
    rmt_item32_t *item = (rmt_item32_t *)xRingbufferReceive(rb, &rx_size, 1000);
    printf("Item received\n");
    if (item) {
      uint16_t rmt_addr;
      uint16_t rmt_cmd;
      int offset = 0;
      while (1) {
        // parse data value from ringbuffer.
        int res = nec_parse_items(item + offset, rx_size / 4 - offset,
                                  &rmt_addr, &rmt_cmd);
        printf("Response: %d", res);
        if (res > 0) {
          offset += res + 1;
          ESP_LOGI(NEC_TAG, "RMT RCV --- addr: 0x%04x cmd: 0x%04x", rmt_addr,
                   rmt_cmd);
        } else {
          break;
        }
      }
      // after parsing the data, return spaces to ringbuffer.
      vRingbufferReturnItem(rb, (void *)item);
    } else {
      break;
    }
  }
  printf("Buffere receive call timed out, terminating the task.\n");
  vTaskDelete(NULL);
}

/**
 * @brief RMT transmitter demo, this task will periodically send NEC data. (100
 * * 32 bits each time.)
 *
 */
static void rmt_example_nec_tx_task() {
  vTaskDelay(10);
  nec_tx_init();
  esp_log_level_set(NEC_TAG, ESP_LOG_INFO);
  int channel = RMT_TX_CHANNEL;
  uint16_t cmd = 0x0;
  uint16_t addr = 0x11;
  int nec_tx_num = RMT_TX_DATA_NUM;
  for (;;) {
    ESP_LOGI(NEC_TAG, "RMT TX DATA");
    size_t size = (sizeof(rmt_item32_t) * NEC_DATA_ITEM_NUM * nec_tx_num);
    // each item represent a cycle of waveform.
    rmt_item32_t *item = (rmt_item32_t *)malloc(size);
    int item_num = NEC_DATA_ITEM_NUM * nec_tx_num;
    memset((void *)item, 0, size);
    int i, offset = 0;
    while (1) {
      // To build a series of waveforms.
      i = nec_build_items(channel, item + offset, item_num - offset,
                          ((~addr) << 8) | addr, ((~cmd) << 8) | cmd);
      if (i < 0) {
        break;
      }
      cmd++;
      addr++;
      offset += i;
    }
    // To send data according to the waveform items.
    rmt_write_items(channel, item, item_num, true);
    // Wait until sending is done.
    rmt_wait_tx_done(channel, portMAX_DELAY);
    // before we free the data, make sure sending is already done.
    free(item);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}


void app_main(void) {
  configure_gpio_outputs();
  initialize_non_volatile_flash();
  esp_log_level_set("*", ESP_LOG_INFO);

  printf("Welcome to the weather station!\n");
  LOG_ERROR("Welcome to the weather station!");
  LOG_INFO("Welcome to the weather station!");
  LOG_DEBUG("Welcome to the weather station!");


  // Make sure the LED is off.
  gpio_set_level(GPIO_OUTPUT_IO_0, 1);

  xTaskCreate(&temperature_monitor_task, "temperature_monitor", 4096, NULL, 5,
              NULL);

  xTaskCreate(&rmt_nex_rx_receive_and_stop_after_timeout_task, "rmt_nec_rx_task", 4096, NULL, 10, NULL);

  fflush(stdout);
}
