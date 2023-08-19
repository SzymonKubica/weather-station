/* NEC remote infrared RMT example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>

#include "driver/periph_ctrl.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "soc/rmt_reg.h"

#include "infrared_nec.h"

#define NEC_TAG "NEC"

/*
 * @brief Build register value of waveform for NEC one data bit
 */
inline void nec_fill_item_level(rmt_item32_t *item, int high_us, int low_us) {
  item->level0 = 1;
  item->duration0 = (high_us) / 10 * RMT_TICK_10_US;
  item->level1 = 0;
  item->duration1 = (low_us) / 10 * RMT_TICK_10_US;
}

/*
 * @brief Generate NEC header value: active 9ms + negative 4.5ms
 */
void nec_fill_item_header(rmt_item32_t *item) {
  nec_fill_item_level(item, NEC_HEADER_HIGH_US, NEC_HEADER_LOW_US);
}

/*
 * @brief Generate NEC data bit 1: positive 0.56ms + negative 1.69ms
 */
void nec_fill_item_bit_one(rmt_item32_t *item) {
  nec_fill_item_level(item, NEC_BIT_ONE_HIGH_US, NEC_BIT_ONE_LOW_US);
}

/*
 * @brief Generate NEC data bit 0: positive 0.56ms + negative 0.56ms
 */
void nec_fill_item_bit_zero(rmt_item32_t *item) {
  nec_fill_item_level(item, NEC_BIT_ZERO_HIGH_US, NEC_BIT_ZERO_LOW_US);
}

/*
 * @brief Generate NEC end signal: positive 0.56ms
 */
void nec_fill_item_end(rmt_item32_t *item) {
  nec_fill_item_level(item, NEC_BIT_END, 0x7fff);
}

/*
 * @brief Check whether duration is around target_us
 */
bool nec_check_in_range(int duration_ticks, int target_us,
                               int margin_us) {
  printf ("Nec item duration: %d\n", NEC_ITEM_DURATION(duration_ticks));
  printf ("Target duration duration: %d +- %d\n", target_us, margin_us);
  if ((NEC_ITEM_DURATION(duration_ticks) < (target_us + margin_us)) &&
      (NEC_ITEM_DURATION(duration_ticks) > (target_us - margin_us))) {
    return true;
  } else {
    return false;
  }
}

/*
 * @brief Check whether this value represents an NEC header
 */
bool nec_header_if(rmt_item32_t *item) {
  printf("Item: level0: %d, level1: %d, duration0: %d, duration1: %d\n", item->level0, item->level1, item->duration0, item->duration1);
  if ((item->level0 == RMT_RX_ACTIVE_LEVEL &&
       item->level1 != RMT_RX_ACTIVE_LEVEL) &&
      nec_check_in_range(item->duration0, NEC_HEADER_HIGH_US, NEC_BIT_MARGIN) &&
      nec_check_in_range(item->duration1, NEC_HEADER_LOW_US, NEC_BIT_MARGIN)) {
    return true;
  }
  return false;
}

/*
 * @brief Check whether this value represents an NEC data bit 1
 */
bool nec_bit_one_if(rmt_item32_t *item) {
  if ((item->level0 == RMT_RX_ACTIVE_LEVEL &&
       item->level1 != RMT_RX_ACTIVE_LEVEL) &&
      nec_check_in_range(item->duration0, NEC_BIT_ONE_HIGH_US,
                         NEC_BIT_MARGIN) &&
      nec_check_in_range(item->duration1, NEC_BIT_ONE_LOW_US, NEC_BIT_MARGIN)) {
    return true;
  }
  return false;
}

/*
 * @brief Check whether this value represents an NEC data bit 0
 */
bool nec_bit_zero_if(rmt_item32_t *item) {
  if ((item->level0 == RMT_RX_ACTIVE_LEVEL &&
       item->level1 != RMT_RX_ACTIVE_LEVEL) &&
      nec_check_in_range(item->duration0, NEC_BIT_ZERO_HIGH_US,
                         NEC_BIT_MARGIN) &&
      nec_check_in_range(item->duration1, NEC_BIT_ZERO_LOW_US,
                         NEC_BIT_MARGIN)) {
    return true;
  }
  return false;
}

/*
 * @brief Parse NEC 32 bit waveform to address and command.
 */
int nec_parse_items(rmt_item32_t *item, int item_num, uint16_t *addr,
                    uint16_t *data) {
  int w_len = item_num;
  printf("w_len: %d\n", w_len);
  if (w_len < NEC_DATA_ITEM_NUM) {
    return -1;
  }
  printf("Data item num fine\n");
  int i = 0, j = 0;
  if (!nec_header_if(item++)) {
    return -1;
  }
  printf("Nec header fine\n");
  uint16_t addr_t = 0;
  for (j = 0; j < 16; j++) {
    if (nec_bit_one_if(item)) {
      addr_t |= (1 << j);
    } else if (nec_bit_zero_if(item)) {
      addr_t |= (0 << j);
    } else {
      return -1;
    }
    item++;
    i++;
  }
  printf("Address fine: %d", addr_t);
  uint16_t data_t = 0;
  for (j = 0; j < 16; j++) {
    if (nec_bit_one_if(item)) {
      data_t |= (1 << j);
    } else if (nec_bit_zero_if(item)) {
      data_t |= (0 << j);
    } else {
      return -1;
    }
    item++;
    i++;
  }
  *addr = addr_t;
  *data = data_t;
  return i;
}

/*
 * @brief Build NEC 32bit waveform.
 */
int nec_build_items(int channel, rmt_item32_t *item, int item_num,
                    uint16_t addr, uint16_t cmd_data) {
  int i = 0, j = 0;
  if (item_num < NEC_DATA_ITEM_NUM) {
    return -1;
  }
  nec_fill_item_header(item++);
  i++;
  for (j = 0; j < 16; j++) {
    if (addr & 0x1) {
      nec_fill_item_bit_one(item);
    } else {
      nec_fill_item_bit_zero(item);
    }
    item++;
    i++;
    addr >>= 1;
  }
  for (j = 0; j < 16; j++) {
    if (cmd_data & 0x1) {
      nec_fill_item_bit_one(item);
    } else {
      nec_fill_item_bit_zero(item);
    }
    item++;
    i++;
    cmd_data >>= 1;
  }
  nec_fill_item_end(item);
  i++;
  return i;
}

/*
 * @brief RMT transmitter initialization
 */
void nec_tx_init() {
  rmt_config_t rmt_tx;
  rmt_tx.channel = RMT_TX_CHANNEL;
  rmt_tx.gpio_num = RMT_TX_GPIO_NUM;
  rmt_tx.mem_block_num = 1;
  rmt_tx.clk_div = RMT_CLK_DIV;
  rmt_tx.tx_config.loop_en = false;
  rmt_tx.tx_config.carrier_duty_percent = 50;
  rmt_tx.tx_config.carrier_freq_hz = 38000;
  rmt_tx.tx_config.carrier_level = 1;
  rmt_tx.tx_config.carrier_en = RMT_TX_CARRIER_EN;
  rmt_tx.tx_config.idle_level = 0;
  rmt_tx.tx_config.idle_output_en = true;
  rmt_tx.rmt_mode = 0;
  rmt_config(&rmt_tx);
  rmt_driver_install(rmt_tx.channel, 0, 0);
}

/*
 * @brief RMT receiver initialization
 */
void nec_rx_init() {
  rmt_config_t rmt_rx;
  rmt_rx.channel = RMT_RX_CHANNEL;
  rmt_rx.gpio_num = RMT_RX_GPIO_NUM;
  rmt_rx.clk_div = RMT_CLK_DIV;
  rmt_rx.mem_block_num = 1;
  rmt_rx.rmt_mode = RMT_MODE_RX;
  rmt_rx.rx_config.filter_en = true;
  rmt_rx.rx_config.filter_ticks_thresh = 100;
  rmt_rx.rx_config.idle_threshold =
      rmt_item32_tIMEOUT_US / 10 * (RMT_TICK_10_US);
  rmt_config(&rmt_rx);
  rmt_driver_install(rmt_rx.channel, 1000, 0);
}

