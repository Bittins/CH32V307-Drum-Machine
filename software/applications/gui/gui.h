/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-27     marcus       the first version
 */
#ifndef APPLICATIONS_GUI_GUI_H_
#define APPLICATIONS_GUI_GUI_H_

#include "leds.h"

struct GUIData
{
    struct LEDData led_struct[KB_CHAIN_LENGTH * 8];
};

uint8_t guiTaskInit(void);

#endif /* APPLICATIONS_GUI_GUI_H_ */
