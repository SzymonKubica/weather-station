#ifndef INFRARED_NEC_H_
#define INFRARED_NEC_H_

#include "driver/rmt.h"

// CHOOSE SELF TEST OR NORMAL TEST
#define RMT_RX_SELF_TEST 0

/******************************************************/
/*****                SELF TEST:                  *****/
/*Connect RMT_TX_GPIO_NUM with RMT_RX_GPIO_NUM        */
/*TX task will send NEC data with carrier disabled    */
/*RX task will print NEC data it receives.            */
/******************************************************/
#if RMT_RX_SELF_TEST
#define RMT_RX_ACTIVE_LEVEL                                                    \
    1                       /*!< Data bit is active high for self test mode    \
                             */
#define RMT_TX_CARRIER_EN 0 /*!< Disable carrier for self test mode  */
#else
// Test with infrared LED, we have to enable carrier for transmitter
// When testing via IR led, the receiver waveform is usually active-low.
#define RMT_RX_ACTIVE_LEVEL                                                    \
    0 /*!< If we connect with a IR receiver, the data is active low */
#define RMT_TX_CARRIER_EN                                                      \
    1 /*!< Enable carrier for IR transmitter test with IR led */
#endif

#define RMT_TX_CHANNEL 1   /*!< RMT channel for transmitter */
#define RMT_TX_GPIO_NUM 18 /*!< GPIO number for transmitter signal */
#define RMT_RX_CHANNEL 0   /*!< RMT channel for receiver */
#define RMT_RX_GPIO_NUM 36 /*!< GPIO number for receiver */
#define RMT_CLK_DIV 100    /*!< RMT counter clock divider */
#define RMT_TICK_10_US                                                         \
    (80000000 / RMT_CLK_DIV /                                                  \
     100000) /*!< RMT counter value for 10 us.(Source clock is APB clock) */

/* My remote seems to send the header of length 9350 */
#define NEC_HEADER_HIGH_US 9350 /*!< NEC protocol header: positive 9ms */
#define NEC_HEADER_LOW_US 4500  /*!< NEC protocol header: negative 4.5ms*/
#define NEC_BIT_ONE_HIGH_US                                                    \
    560 /*!< NEC protocol data bit 1: positive 0.56ms                          \
         */
#define NEC_BIT_ONE_LOW_US                                                     \
    (2250 - NEC_BIT_ONE_HIGH_US) /*!< NEC protocol data bit 1: negative 1.69ms \
                                  */
#define NEC_BIT_ZERO_HIGH_US                                                   \
    560 /*!< NEC protocol data bit 0: positive 0.56ms */
#define NEC_BIT_ZERO_LOW_US                                                    \
    (1120 -                                                                    \
     NEC_BIT_ZERO_HIGH_US) /*!< NEC protocol data bit 0: negative 0.56ms       \
                            */
#define NEC_BIT_END 560    /*!< NEC protocol end: positive 0.56ms */
#define NEC_BIT_MARGIN 400 /*!< NEC parse margin time */

#define NEC_ITEM_DURATION(d)                                                   \
    ((d & 0x7fff) * 10 /                                                       \
     RMT_TICK_10_US) /*!< Parse duration time from memory register value */
#define NEC_DATA_ITEM_NUM                                                      \
    34 /*!< NEC code item number: header + 32bit data + end */
#define RMT_TX_DATA_NUM 100        /*!< NEC tx test data number */
#define rmt_item32_tIMEOUT_US 9500 /*!< RMT receiver timeout value(us) */

void nec_fill_item_level(rmt_item32_t *item, int high_us, int low_us);

void nec_fill_item_header(rmt_item32_t *item);

void nec_fill_item_bit_one(rmt_item32_t *item);

void nec_fill_item_bit_zero(rmt_item32_t *item);

void nec_fill_item_end(rmt_item32_t *item);

bool nec_check_in_range(int duration_ticks, int target_us, int margin_us);

bool nec_header_if(rmt_item32_t *item);

bool nec_bit_one_if(rmt_item32_t *item);

bool nec_bit_zero_if(rmt_item32_t *item);

int nec_parse_items(rmt_item32_t *item, int item_num, uint16_t *addr,
                    uint16_t *data);

int nec_build_items(int channel, rmt_item32_t *item, int item_num,
                    uint16_t addr, uint16_t cmd_data);

void nec_tx_init();

void nec_rx_init();

enum RemoteButton {
    BUTTON_0,
    BUTTON_1,
    BUTTON_2,
    BUTTON_3,
    BUTTON_4,
    BUTTON_5,
    BUTTON_6,
    BUTTON_7,
    BUTTON_8,
    BUTTON_9,
    BUTTON_100_PLUS,
    BUTTON_200_PLUS,
    BUTTON_PLUS,
    BUTTON_MINUS,
    BUTTON_FORWARD,
    BUTTON_BACKWARD,
    BUTTON_PLAY_PAUSE,
    BUTTON_CHANNEL,
    BUTTON_CHANNEL_MINUS,
    BUTTON_CHANNEL_PLUS,
    BUTTON_EQ,
    ERROR,
};

extern enum RemoteButton map_from_int(int data_value);
extern const char *button_names[];

#endif
