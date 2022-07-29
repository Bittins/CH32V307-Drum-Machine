/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-27     marcus       the first version
 */
#ifndef APPLICATIONS_GUI_LEDS_H_
#define APPLICATIONS_GUI_LEDS_H_

#include "input/keyboard.h"

#define LED_BRIGHT_LEVELS   4

struct LEDData
{
    uint8_t brightness;
    uint8_t blink_length;
};

void ledBrightnessHandler(uint8_t led_data[], struct LEDData* led_struct);

#endif /* APPLICATIONS_GUI_LEDS_H_ */
