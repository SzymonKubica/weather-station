#include <stdbool.h>

#include "dht/DHT.h"
#include "esp_log.h"
#include "hal/gpio_types.h"
#include "system_message.h"
#include "util.h"

#define DHT_TAG "DHT"

void dht_task(void *pvParameter)
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
        xQueueSend(display_msg_queue, (void *)&message, (TickType_t)0);

        wait_seconds(3);
    }
}
