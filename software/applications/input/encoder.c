/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-17     marcus       the first version
 */

#include "encoder.h"

uint8_t encUpdate(struct InputEvents* input_events)
{
    uint8_t update = 0;
    return update;
}

uint8_t encInit(void)
{
    // Enable internal Advanced Peripheral Bus clock for timer
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // Create and initialize timer
    TIM_TimeBaseInitTypeDef tim_base_struct = {0};
    TIM_TimeBaseStructInit(&tim_base_struct);
    TIM_TimeBaseInit(TIM3, &tim_base_struct);

    TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
}
