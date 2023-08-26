#ifndef ONBOARD_LED_H
#define ONBOARD_LED_H

#include <stdbool.h>

struct OnboardLED {
    bool is_on;
};

void toggle_onboard_led();

#endif
