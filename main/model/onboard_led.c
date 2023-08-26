#include "onboard_led.h"
#include "driver/gpio.h"
#include "gpio_util.h"

void toggle_onboard_led()
{
    if (onboard_led.is_on) {
        gpio_set_level(GPIO_OUTPUT_IO_0, 1);
        onboard_led.is_on = false;
    } else {
        gpio_set_level(GPIO_OUTPUT_IO_0, 0);
        onboard_led.is_on = true;
    }
}
