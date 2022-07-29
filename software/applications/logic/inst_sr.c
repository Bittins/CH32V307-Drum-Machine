/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-22     marcus       the first version
 */
#include "ch32v30x.h"
#include <rtthread.h>
#include <rthw.h>
#include "drivers/pin.h"
#include <board.h>
#include <rtdevice.h>
#include "drv_spi.h"
#include "sequencer.h"
#include "inst_sr.h"

void instSROut(struct PattData* pattern, struct MachineState *state)
{
    uint16_t sr_out = 0;

    uint8_t cur_patt = state->patt;
//    uint8_t cur_seq = state->seq;
    uint8_t cur_page = state->page;
    uint8_t cur_step = state->step;

    // TODO: Implement complex steps
    uint8_t inst;
    for (inst = 0; inst < INST_AMNT; inst++)
    {
        sr_out |= pattern[cur_patt].seq[inst].page[cur_page].step[cur_step].type << inst;
    }

    // Send out data through SPI
    if(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != RESET)
    {
        // Set NSS low
        GPIO_ResetBits(GPIOB, GPIO_Pin_12);

        SPI_I2S_SendData(SPI2, sr_out);

        // Wait for data to finish transmitting
        while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);

        // Set NSS high
        GPIO_SetBits(GPIOB, GPIO_Pin_12);
    }

    // Start pulse timer
    TIM_Cmd(TIM3, ENABLE);
}

uint8_t instSRInit(void)
{
    /*
     * SPI2 PINS:
     *   SPI_NSS =   PB12 (RCLK)
     *   SPI_SCK =   PB13 (SRCLK)
     *   SPI_MISO =  PB14 (NOT USED)
     *   SPI_MOSI =  PB15 (SER)
     *
     * Pulse Pin = PB0
     */

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // Init GPIO NSS
    GPIO_InitTypeDef gpio_init_struct = {0};
    gpio_init_struct.GPIO_Pin = GPIO_Pin_12;
    gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio_init_struct);

    GPIO_SetBits(GPIOB, GPIO_Pin_12);

    // Init GPIO CLK
    gpio_init_struct.GPIO_Pin = GPIO_Pin_13;
    gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio_init_struct);

    // Init GPIO MOSI
    gpio_init_struct.GPIO_Pin = GPIO_Pin_15;
    gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio_init_struct);

    // Init SPI
    SPI_InitTypeDef spi_init_struct = {0};
    spi_init_struct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi_init_struct.SPI_Mode = SPI_Mode_Master;
    spi_init_struct.SPI_NSS = SPI_NSS_Soft;
    spi_init_struct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
    SPI_Init(SPI2, &spi_init_struct);

    SPI_Cmd(SPI2, ENABLE);

    // Init pulse GPIO
    gpio_init_struct.GPIO_Pin = GPIO_Pin_0;
    gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio_init_struct);

    GPIO_SetBits(GPIOB, GPIO_Pin_0);

    // Init pulse timer
    TIM_TimeBaseInitTypeDef tim_base_struct = {0};
    tim_base_struct.TIM_Period = INST_SR_PULSE_LENGTH;
    tim_base_struct.TIM_Prescaler = 144-1;
    TIM_TimeBaseInit(TIM3, &tim_base_struct);

    TIM_OCInitTypeDef tim_output_struct = {0};
    tim_output_struct.TIM_OCMode = TIM_OCMode_PWM1;
    tim_output_struct.TIM_OutputState = TIM_OutputState_Enable;
    tim_output_struct.TIM_Pulse = 1;
    tim_output_struct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC3Init(TIM3, &tim_output_struct);

    // Configure one shot pulse timer
    TIM_SelectOnePulseMode(TIM3, TIM_OPMode_Single);

//    TIM_Cmd(TIM3, ENABLE);
}

