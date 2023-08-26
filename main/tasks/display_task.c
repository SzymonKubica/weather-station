#include "libs/ssd1306/font8x8_basic.h"
#include "libs/ssd1306/ssd1306.h"

#include "display.h"
#include "system_action.h"
#include "system_message.h"

static void screen_on(struct Display *display);
static void screen_off(struct Display *display);
static void show_temperature(struct Display *display);

void display_task(void *pvParameter)
{
    ESP_LOGI(DISPLAY_TAG, "Initialising the OLED display...\n\n");
    SSD1306_t dev;
    initialise_screen_device(&dev);

    struct Display display = {TEMPERATURE_AND_HUMIDITY, &dev, 0.0, 0.0, true};
    struct DisplayMessage *received_message;

    while (true) {
        if (xQueueReceive(display_msg_queue, &(received_message),
                          (TickType_t)5)) {

            switch (received_message->requested_action) {
            case SCREEN_ON:
                screen_on(&display);
                break;
            case SCREEN_OFF:
                screen_off(&display);
                break;
            case SHOW_DHT_READING:
                display.temperature = received_message->temperature;
                display.humidity = received_message->humidity;
                show_temperature(&display);
                break;
            }
        }
    }
}

static void screen_on(struct Display *display)
{
    display->is_on = true;
    show_temperature(display);
}

static void screen_off(struct Display *display)
{
    ssd1306_clear_screen(display->device, false);
    display->is_on = false;
}

static void show_temperature(struct Display *display)
{
    if (display->is_on) {
        print_temperature_and_humidity(display->device, display->temperature,
                                       display->humidity);
    }
}
