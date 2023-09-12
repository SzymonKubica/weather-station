// Copyright (c) 2023 Szymon Kubica
// SPDX-License-Identifier: MIT
/*
 * GPIO utility library.
 */
#ifndef gpio_util
#define gpio_util

#define GPIO_OUTPUT_IO_0 CONFIG_GPIO_LED_OUTPUT
#define GPIO_OUTPUT_IO_1 CONFIG_GPIO_OUTPUT_1
#define GPIO_OUTPUT_PIN_SEL                                                    \
    ((1ULL << GPIO_OUTPUT_IO_0) | (1ULL << GPIO_OUTPUT_IO_1))
#define GPIO_INPUT_IO_0 CONFIG_GPIO_INPUT_0
#define GPIO_INPUT_IO_1 CONFIG_GPIO_INPUT_1
#define GPIO_INPUT_PIN_SEL                                                     \
    ((1ULL << GPIO_INPUT_IO_0) | (1ULL << GPIO_INPUT_IO_1))

// == function prototypes =======================================

void configure_gpio_outputs(void);
void configure_gpio_inputs(void);

#endif
