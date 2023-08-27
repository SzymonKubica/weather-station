#include "driver/rmt_types_legacy.h"
#include "esp_log.h"
#include "freertos/ringbuf.h"
#include "infrared_nec.h"
#include "system_action.h"
#include "system_message.h"

#define NEC_TAG "NEC"

static void get_nec_ring_buffer(RingbufHandle_t *rb);

void ir_remote_task(void *pvParameter)
{
    if (!ir_remote_input_queue) {
        ESP_LOGE(NEC_TAG, "Failed to create IR Remote Input Queue");
    }
    RingbufHandle_t rb = NULL;
    get_nec_ring_buffer(&rb);

    struct SystemMessage *message = &system_message;

    while (rb) {
        size_t rx_size = 0;
        // try to receive data from ringbuffer.
        // RMT driver will push all the data it receives to its ringbuffer.
        // We just need to parse the value and return the spaces of ringbuffer.
        rmt_item32_t *item =
            (rmt_item32_t *)xRingbufferReceive(rb, &rx_size, 1000);
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
                    ESP_LOGI(NEC_TAG, "RMT RCV --- addr: 0x%04x cmd: 0x%04x",
                             rmt_addr, rmt_cmd);

                    enum RemoteButton registered_button = map_from_int(rmt_cmd);
                    switch (registered_button) {
                    case BUTTON_EQ:
                        system_message.system_action = TOGGLE_ONBOARD_LED;
                        break;
                    case BUTTON_CHANNEL_MINUS:
                        system_message.system_action = DISPLAY_OFF;
                        break;
                    case BUTTON_CHANNEL_PLUS:
                        system_message.system_action = DISPLAY_ON;
                        break;
                    case BUTTON_0:
                        system_message.system_action = SHOW_SENSOR_READING;
                        break;
                    case BUTTON_1:
                        system_message.system_action = GET_WEATHER_NOW;
                        break;
                    case BUTTON_2:
                        system_message.system_action = GET_WEATHER_TODAY;
                        break;
                    case BUTTON_3:
                        system_message.system_action = GET_WEATHER_TOMORROW;
                        break;
                    case BUTTON_4:
                        system_message.system_action = GET_WEATHER_T2;
                        break;
                    case BUTTON_PLAY_PAUSE:
                        system_message.system_action = UPDATE_WEATHER_DATA;
                        break;
                    default:
                        break;
                    }
                    xQueueSend(system_msg_queue, (void *)&message,
                               (TickType_t)0);
                    ESP_LOGI(NEC_TAG, "Button press registered: %s\n",
                             button_str[map_from_int(rmt_cmd)]);

                } else {
                    break;
                }
            }
            // after parsing the data, return spaces to ringbuffer.
            vRingbufferReturnItem(rb, (void *)item);
        }
    }
}

static void get_nec_ring_buffer(RingbufHandle_t *rb)
{
    int channel = RMT_RX_CHANNEL;
    nec_rx_init();
    rmt_get_ringbuf_handle(channel, rb);
    rmt_rx_start(channel, 1);
}
