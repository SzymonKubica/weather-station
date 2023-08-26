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

#include "driver/gpio.h"
#include "hal/gpio_types.h"
#include "sdkconfig.h"

// External library imports
#include "libs/dht/DHT.h"
#include "libs/infrared-receiver/infrared_nec.h"
#include "libs/ssd1306/font8x8_basic.h"
#include "libs/ssd1306/ssd1306.h"

// Project module imports
#include "display/display.h"
#include "gpio/gpio_util.h"
#include "model/system_action.h"
#include "model/system_message.h"
#include "model/onboard_led.h"
#include "util/util.h"

#define DHT_TAG "DHT"
#define LED_TAG "ONBOARD_LED"
#define SYSTEM_TAG "SYSTEM"
#define NEC_TAG "NEC"

TaskHandle_t task_0_handle = NULL;
TaskHandle_t task_1_handle = NULL;
TaskHandle_t task_2_handle = NULL;
TaskHandle_t task_3_handle = NULL;

static QueueHandle_t ir_remote_input_queue = NULL;
static QueueHandle_t system_message_queue = NULL;
static QueueHandle_t display_message_queue = NULL;

static void system_task(void *pvParameter);
static void dht_task(void *pvParameter);
static void display_task(void *pvParameter);
static void ir_remote_task(void *pvParameter);

static void disable_led_by_default();

void app_main(void)
{
    configure_gpio_outputs();
    initialize_non_volatile_flash();
    esp_log_level_set("*", ESP_LOG_INFO);

    disable_led_by_default();

    ir_remote_input_queue = xQueueCreate(10, sizeof(struct IRRemoteMessage *));
    system_message_queue = xQueueCreate(10, sizeof(struct SystemMessage *));
    display_message_queue = xQueueCreate(10, sizeof(struct DisplayMessage *));

    xTaskCreate(&system_task, "system", 2048, NULL, 10, &task_0_handle);
    xTaskCreate(&dht_task, "dht-22", 2048, NULL, 5, &task_1_handle);
    xTaskCreate(&ir_remote_task, "nec_rx", 4096, NULL, 10, &task_2_handle);
    xTaskCreate(&display_task, "display", 4096, NULL, 5, &task_3_handle);

    fflush(stdout);
}

static void system_task(void *pvParameter)
{
    struct SystemMessage *received_message;

    struct DisplayMessage *message = &display_message;

    bool led_on = true;

    while (true) {
        if (xQueueReceive(system_message_queue, &(received_message),
                          (TickType_t)5)) {
            ESP_LOGI(SYSTEM_TAG, "Received an incoming system message: %s",
                     system_action_names[received_message->system_action]);
            switch (received_message->system_action) {
            case TOGGLE_ONBOARD_LED:
                toggle_onboard_led();
                break;
            case DISPLAY_OFF:
                message->requested_action = SCREEN_OFF;
                xQueueSend(display_message_queue, (void *)&message,
                           (TickType_t)0);
                break;
            case DISPLAY_ON:
                message->requested_action = SCREEN_ON;
                xQueueSend(display_message_queue, (void *)&message,
                           (TickType_t)0);
            }
        }
    }
}

static void disable_led_by_default() { gpio_set_level(GPIO_OUTPUT_IO_0, 1); }

static void display_task(void *pvParameter)
{
    ESP_LOGI(DISPLAY_TAG, "Initialising the OLED display...\n\n");
    SSD1306_t dev;
    initialise_display(&dev);

    bool display_on = true;
    float temperature = 0.0;
    float humidity = 0.0;
    struct DisplayMessage *received_message;

    while (true) {
        if (xQueueReceive(display_message_queue, &(received_message),
                          (TickType_t)5)) {

            switch (received_message->requested_action) {
            case SCREEN_ON:
                display_on = true;
                print_temperature_and_humidity(&dev, temperature, humidity);
                break;
            case SCREEN_OFF:
                ssd1306_clear_screen(&dev, false);
                display_on = false;
                break;
            case SHOW_DHT_READING:
                if (display_on) {
                    temperature = received_message->temperature;
                    humidity = received_message->humidity;
                    print_temperature_and_humidity(&dev, temperature, humidity);
                }
                break;
            }
        }
    }
}

static void dht_task(void *pvParameter)
{

    ESP_LOGI(DHT_TAG, "Initialising the DHT Sensor...\n\n");
    set_dht_gpio(GPIO_NUM_4);

    struct DisplayMessage *message = &display_message;

    while (true) {
        ESP_LOGI(DHT_TAG, "=== Reading DHT ===");
        int ret = read_dht();
        handle_errors(ret);

        float humidity = get_humidity();
        float temperature = get_temperature();
        ESP_LOGI(DHT_TAG, "Hum: %.1f Tmp: %.1f\n", humidity, temperature);

        message->requested_action = SHOW_DHT_READING;
        message->temperature = temperature;
        message->humidity = humidity;
        xQueueSend(display_message_queue, (void *)&message, (TickType_t)0);

        wait_seconds(3);
    }
}

static void get_nec_ring_buffer(RingbufHandle_t *rb);

static void ir_remote_task(void *pvParameter)
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
                    default:
                        break;
                    }
                    xQueueSend(system_message_queue, (void *)&message,
                               (TickType_t)0);
                    ESP_LOGI(NEC_TAG, "Button press registered: %s\n",
                             button_names[map_from_int(rmt_cmd)]);

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
