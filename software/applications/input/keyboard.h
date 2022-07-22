/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-14     marcus       the first version
 */
#ifndef APPLICATIONS_KEYBOARD_H_
#define APPLICATIONS_KEYBOARD_H_

#include <rtdef.h>
#include "ch32v30x.h"
#include <rtthread.h>
#include <rthw.h>
#include "drivers/pin.h"
#include <board.h>
#include <rtdevice.h>
#include "drv_spi.h"

#define KB_SPI_DEV_NAME     "spi10"
#define KB_SPI_BUS          "spi1"
#define KB_CHAIN_LENGTH     3
#define KB_LATCH_PIN        3           // PA3  MUST NOT BE THE SAME AS THE SPI NSS PIN

struct ButtonData
{
    uint16_t main_row;
    uint8_t function;
};

struct ButtonEvents
{
    struct ButtonData is_held;
    struct ButtonData just_pressed;
    struct ButtonData just_released;
};

enum FunctionKeys
{
    shift,
    write_record,
    page,
    tuplet,
    mute_solo,
    accent_flam,
    mode,
    play_pause
};

struct InputEvents
{
    struct ButtonEvents button_events;
    uint8_t enc_button;
    int enc_rotation;
};

extern uint8_t kb_led_data[KB_CHAIN_LENGTH];

void kbLEDDataGet(uint8_t* led_data);
void kbTransferData(uint8_t* led_data, uint8_t* button_data);
uint8_t kbParseButtons(uint8_t* button_data, struct InputEvents* input_events);
uint8_t kbInit(void);

#endif /* APPLICATIONS_KEYBOARD_H_ */
