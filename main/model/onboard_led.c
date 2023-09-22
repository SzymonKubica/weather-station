// Copyright (c) 2023 Szymon Kubica
// SPDX-License-Identifier: MIT
#include "onboard_led.h"
#include "driver/gpio.h"
#include "gpio_util.h"

static struct OnboardLED onboard_led;

#define LED_OFF 1

/*
 * It seems like the onboard LED is wired in a way that one of its legs
 * is connected to a pin which is always on, and the other is connected to
 * GPIO_OUTPUT_IO_0. Because of this, LED turns on when we set that pin to low
 * (as the current starts flowing) whereas it turn off when we set that pin
 * high. Because of this, if we want to set a given state of the led, we need
 * to set the value of that pin to the opposite of the boolean value of
 * led->is_on.
 *
 */

bool get_led_signal(bool is_led_on) { return !is_led_on; }

void disable_led_by_default() { gpio_set_level(GPIO_OUTPUT_IO_0, LED_OFF); }

void toggle_onboard_led()
{
    struct OnboardLED *led = &onboard_led;
    led->is_on = !led->is_on;
    gpio_set_level(GPIO_OUTPUT_IO_0, get_led_signal(led->is_on));
}
