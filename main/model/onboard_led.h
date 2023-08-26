#ifndef MODEL_H
#define MODEL_H

#include <stdbool.h>

struct OnboardLED {
    bool is_on;
} onboard_led;

extern void toggle_onboard_led();

#endif
