#include "onboard_led.h"
#include "driver/gpio.h"
#include "gpio_util.h"

static struct OnboardLED onboard_led;

void toggle_onboard_led()
{
    struct OnboardLED *led = &onboard_led;
    if (led->is_on) {
        gpio_set_level(GPIO_OUTPUT_IO_0, 1);
        led->is_on = false;
    } else {
        gpio_set_level(GPIO_OUTPUT_IO_0, 0);
        led->is_on = true;
    }
}
