/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-17     marcus       the first version
 */
#ifndef APPLICATIONS_ENCODER_H_
#define APPLICATIONS_ENCODER_H_

#include "ch32v30x.h"
#include <rtthread.h>
#include <rthw.h>
#include "drivers/pin.h"
#include <board.h>
#include <rtdevice.h>
#include "keyboard.h"

// Get timer value, offset by 2 and divide by four, quadrature encoder quirk
//#define ENC_VAL(tim_handle)     (((int16_t) __HAL_TIM_GET_COUNTER(tim_handle) + 2) >> 2)

uint8_t encUpdate(struct InputEvents* input_events);
uint8_t encInit(void);

#endif /* APPLICATIONS_ENCODER_H_ */
