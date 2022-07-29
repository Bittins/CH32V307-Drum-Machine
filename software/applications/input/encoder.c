/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-17     marcus       the first version
 */

#include "ch32v30x.h"
#include <rtthread.h>
#include <rthw.h>
#include "drivers/pin.h"
#include <board.h>
#include <rtdevice.h>
#include "encoder.h"

uint8_t encUpdate(struct InputEvents* input_events)
{
    uint8_t update = 0;

    int16_t enc_rot = ENC_VAL(TIM4);
    static int16_t enc_rot_prev;

    uint8_t enc_button = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4);
    static uint8_t enc_button_prev;

    // Update encoder rotation
    if (enc_rot != enc_rot_prev)
    {
        input_events->enc_rotation = enc_rot - enc_rot_prev;
        enc_rot_prev = enc_rot;

        update = 1;
    }
    else
    {
        input_events->enc_rotation = 0;
    }

    // Update encoder button
    if (enc_button != enc_button_prev)
    {
        input_events->button_events.is_held.enc_button          = enc_button;
        input_events->button_events.just_pressed.enc_button     = enc_button_prev ^ 1;
        input_events->button_events.just_released.enc_button    = enc_button_prev;

        enc_button_prev = enc_button;

        update = 1;
    }
    else
    {
        input_events->button_events.just_pressed.enc_button     = 0;
        input_events->button_events.just_released.enc_button    = 0;
    }

    return update;
}

uint8_t encInit(void)
{
    /*
     * Encoder Pins:
     *   enc_A  = PB7
     *   enc_B  = PB6
     *   button = PB4
     */

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    // Init GPIO enc_A
    GPIO_InitTypeDef gpio_init_struct = {0};
    gpio_init_struct.GPIO_Pin = GPIO_Pin_7;
    gpio_init_struct.GPIO_Mode = GPIO_Mode_IPD;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio_init_struct);

    // Init GPIO enc_B
    gpio_init_struct.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOB, &gpio_init_struct);

    // Init GPIO button
    gpio_init_struct.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init(GPIOB, &gpio_init_struct);

    // Init encoder timer
    TIM_TimeBaseInitTypeDef tim_base_struct = {0};
    TIM_TimeBaseStructInit(&tim_base_struct);
    TIM_TimeBaseInit(TIM4, &tim_base_struct);

    TIM_ICInitTypeDef tim_ic_struct = {0};
    TIM_ICStructInit(&tim_ic_struct);
    tim_ic_struct.TIM_Channel = TIM_Channel_1;
    tim_ic_struct.TIM_ICFilter = 0xF;
    TIM_ICInit(TIM4, &tim_ic_struct);

    TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_Cmd(TIM4, ENABLE);
}
