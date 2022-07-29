/*
#include <gui/leds.h>
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-27     marcus       the first version
 */

#include <rtdef.h>
#include "ch32v30x.h"
#include <rtthread.h>
#include <rthw.h>
#include "drivers/pin.h"
#include <board.h>
#include <rtdevice.h>
#include "drv_spi.h"
#include <string.h>
#include "leds.h"
#include "input/keyboard.h"

void ledBrightnessHandler(uint8_t led_data[], struct LEDData* led_struct)
{
    static uint8_t counter;

    // limit counter to a value between 0 and LED_BRIGHT_LEVELS
    counter %= LED_BRIGHT_LEVELS;

    uint8_t i;
    uint8_t j;
    for (i = 0; i < KB_CHAIN_LENGTH; i++)
    {
        led_data[i] = 0;
        for (j = 0; j < 8; j++)
        {
            if (counter < led_struct[j + (i * 8)].brightness)
            {
                led_data[i] |= 1 << j;
            }
        }
    }

    counter++;
}

