// Copyright (c) 2023 Szymon Kubica
// SPDX-License-Identifier: MIT
#ifndef ONBOARD_LED_H
#define ONBOARD_LED_H

#include <stdbool.h>

struct OnboardLED {
    bool is_on;
};

void toggle_onboard_led();
void disable_led_by_default();

#endif
