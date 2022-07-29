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

#include "input.h"

// Get timer value, offset by 2 and divide by four, quadrature encoder quirk
#define ENC_VAL(TIMx)     (((int16_t) TIM_GetCounter(TIMx) + 2) >> 2)

uint8_t encUpdate(struct InputEvents* input_events);
uint8_t encInit(void);

#endif /* APPLICATIONS_ENCODER_H_ */
