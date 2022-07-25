/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-05     marcus       the first version
 */

#include "sequencer.h"
#include "inst_sr.h"
#include <string.h>

void kbLEDDataSend(uint8_t led_data[3], struct PattData* patt);
void seqStepToggle(struct PattData* patt, struct InputEvents* events);
void seqInputQueueGet(struct InputEvents* input_events);
uint8_t beatTimerInit(void);

static struct MachineState state;
static struct PattData pattern[1];

void seqTaskEntry(void* param)
{
    struct InputEvents input_events;

    while (1)
    {
        seqInputQueueGet(&input_events);

        seqStepToggle(pattern, &input_events);

//        switch (state.mode)
//        {
//            case PATTERN_MODE:
//
//                sequencerStepToggle(&sequence, &input_events, &state);
//
//                break;
//
//            default:
//
//                break;
//        }

        kbLEDDataSend(kb_led_data, pattern);          // TODO: Move function to UI task
    }
}

void kbLEDDataSend(uint8_t* led_data, struct PattData* patt)
{
    // [0] = function row L to R, [1] = 1 2 3 4 5 6 7 8, [2] = 9 10 11 12 13 14 15 16
    memset(led_data, 0, (sizeof(uint8_t) * KB_CHAIN_LENGTH));

    uint8_t cur_patt = state.patt;
    uint8_t cur_inst = state.seq;
    uint8_t cur_page = state.page;

    uint8_t i;
    uint8_t j;
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 8; j++)
        {
            led_data[i + 1] |= patt[cur_patt].seq[cur_inst].page[cur_page].step[j + (i * 8)].type << j;
        }
    }
}

void seqStepToggle(struct PattData* patt, struct InputEvents* events)
{
    uint8_t cur_patt = state.patt;
    uint8_t cur_inst = state.seq;
    uint8_t cur_page = state.page;

    uint8_t i;
    for (i = 0; i < 16; i++)
    {
        patt[cur_patt].seq[cur_inst].page[cur_page].step[i].type ^= (events->button_events.just_pressed.main_row >> i) & 1;
    }
}

void seqInputQueueGet(struct InputEvents* input_events)
{
    if (rt_mq_recv(input_event_mq, input_events, sizeof(struct InputEvents), RT_WAITING_FOREVER) != RT_EOK)
    {
        rt_kprintf("failed to read input queue\n");
    }
}

uint8_t sequencerTaskInit(void)
{
    memset(&state, 0, sizeof(state));
    memset(pattern, 0, sizeof(pattern));

    rt_thread_t tid = rt_thread_create("sequencerTaskEntry", seqTaskEntry, RT_NULL, 4096, 6, 10);

    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    else
    {
        rt_kprintf("failed to start sequencerTask");
    }

    instSRInit();
    beatTimerInit();
}

uint8_t beatTimerInit(void)
{
    state.bpm = 120;
    uint16_t period = (60 * BEAT_TIM_ARR_SEC) / state.bpm;

    // Create and initialize timer
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseInitTypeDef tim_base_struct = {0};
    tim_base_struct.TIM_Period = period;
    tim_base_struct.TIM_Prescaler = BEAT_TIM_PSC;
    tim_base_struct.TIM_ClockDivision = TIM_CKD_DIV1;
    tim_base_struct.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM2, &tim_base_struct);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    // Create and initialize interrupt
    NVIC_InitTypeDef itr_struct = {0};
    itr_struct.NVIC_IRQChannel = TIM2_IRQn;
    itr_struct.NVIC_IRQChannelPreemptionPriority = 2;
    itr_struct.NVIC_IRQChannelSubPriority = 0;
    itr_struct.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&itr_struct);
    NVIC_EnableIRQ(TIM2_IRQn);

    TIM_Cmd(TIM2, ENABLE);
}

// Beat timer callback function
__attribute__((interrupt("WCH-Interrupt-fast"))) void TIM2_IRQHandler(void)
{
    // Immediately reset timer
    TIM2->INTFR = 0;

    rt_interrupt_enter();

    instSROut(pattern, &state);

    if (state.step < MAX_STEP-1)
    {
        state.step++;
    }
    else
    {
        state.step = 0;
    }

    rt_interrupt_leave();
}
