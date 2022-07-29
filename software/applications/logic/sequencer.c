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
#include "gui/gui.h"
#include <string.h>

uint8_t guiUpdateEventSend(void);
void kbLEDDataSend(uint8_t led_data[3], struct PattData* patt);
void seqStepToggle(struct PattData* patt, struct InputEvents* events);
void seqInputQueueGet(struct InputEvents* input_events);
void encBpmAdj(struct InputEvents* input_events);
void encButtonInst(struct InputEvents* input_events);
void bpmSet(uint16_t bpm);
uint8_t beatTimerInit(void);

rt_mq_t patt_state_mq;
rt_event_t gui_update_event;

static struct MachineState state;
static struct PattData pattern[MAX_PATT];

void seqTaskEntry(void* param)
{
    struct InputEvents input_events;

    while (1)
    {
        seqInputQueueGet(&input_events);

        seqStepToggle(pattern, &input_events);
        encBpmAdj(&input_events);
        encButtonInst(&input_events);

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

        guiUpdateEventSend();
    }
}

void seqInputQueueGet(struct InputEvents* input_events)
{
    if (rt_mq_recv(input_event_mq, input_events, sizeof(struct InputEvents), RT_WAITING_FOREVER) != RT_EOK)
    {
        rt_kprintf("failed to read input queue\n");
    }
}

uint8_t guiUpdateEventSend(void)
{
//    if (rt_event_send(gui_update_event, 1) != RT_EOK)
//    {
//        rt_kprintf("failed to send gui update event\n");
//    }
    switch (rt_event_send(gui_update_event, 1))
    {
        case RT_EOK:
//            rt_kprintf("success sending gui update event\n");
            break;

        case RT_ETIMEOUT:
            rt_kprintf("timeout sending gui update event\n");
            break;

        default:
            rt_kprintf("error sending gui update event\n");
            break;
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

void seqSetDefaults(struct MachineState* state, struct PattData* patt)
{
    memset(state, 0, sizeof(state));
    memset(patt, 0, sizeof(patt));

    state->bpm = BPM_DEFAULT;
    state->mode = PATTERN_MODE;

    uint8_t i;
    uint8_t j;
    uint8_t k;
    for (i = 0; i < MAX_PATT; i++);
    {
        patt[i].length = 1;
        for (j = 0; j < INST_AMNT; j++)
        {
            patt->seq[j].length = 1;
            for (k = 0; k < MAX_PAGE; k++)
            {
                patt->seq[j].page[k].length = 16;
            }
        }
    }
}

uint8_t guiQueueSend(struct PattData* patt, struct MachineState* state)
{
    struct PattStatePointers msg;
    msg.patt = patt;
    msg.state = state;

    patt_state_mq = rt_mq_create("patt_state_mq", sizeof(msg), 1, RT_IPC_FLAG_PRIO);
    rt_mq_send(patt_state_mq, &msg, sizeof(msg));
}

uint8_t seqTaskInit(void)
{
    seqSetDefaults(&state, pattern);

    rt_thread_t tid = rt_thread_create("seqTaskEntry", seqTaskEntry, RT_NULL, 4096, 6, 10);

    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    else
    {
        rt_kprintf("failed to start seqTask");
    }

    guiQueueSend(pattern, &state);
    gui_update_event = rt_event_create("gui_update_event", RT_IPC_FLAG_PRIO);

    instSRInit();
    beatTimerInit();
}

void encBpmAdj(struct InputEvents* input_events)
{
    if (input_events->enc_rotation)
    {
        bpmSet(state.bpm + input_events->enc_rotation);
    }
}

void encButtonInst(struct InputEvents* input_events)
{
    if (input_events->button_events.just_pressed.enc_button)
    {
        if (state.seq < 3)
        {
            state.seq ++;
        }
        else
        {
            state.seq = 0;
        }

        rt_kprintf("instrument: %d\n", state.seq);
    }
}

void bpmSet(uint16_t bpm)
{
    uint16_t period = (60 * BEAT_TIM_ARR_SEC) / (bpm * QUARTER_LENGTH);
    state.bpm = bpm;
    rt_kprintf("bpm: %d\n", state.bpm);

    TIM_SetAutoreload(TIM2, period);
}

// TODO: Split beat functions to separate file
// TODO: Modify to support polyrhythms
void beatIncrement(struct MachineState* state)
{
    if (state->step < MAX_STEP-1)
    {
        state->step++;
    }
    else
    {
        state->step = 0;
    }

    guiUpdateEventSend();
}

// Beat timer callback function
__attribute__((interrupt("WCH-Interrupt-fast"))) void TIM2_IRQHandler(void)
{
    // Immediately reset timer interrupt
    TIM2->INTFR = 0;

    rt_interrupt_enter();

    instSROut(pattern, &state);

    beatIncrement(&state);

    rt_interrupt_leave();
}

uint8_t beatTimerInit(void)
{
    // Init timer
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInitTypeDef tim_base_struct = {0};
    tim_base_struct.TIM_Prescaler = BEAT_TIM_PSC;
    tim_base_struct.TIM_ClockDivision = TIM_CKD_DIV1;
    tim_base_struct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &tim_base_struct);

    bpmSet(BPM_DEFAULT);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    // Init interrupt
    NVIC_InitTypeDef itr_struct = {0};
    itr_struct.NVIC_IRQChannel = TIM2_IRQn;
    itr_struct.NVIC_IRQChannelPreemptionPriority = 2;
    itr_struct.NVIC_IRQChannelSubPriority = 0;
    itr_struct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&itr_struct);

    NVIC_EnableIRQ(TIM2_IRQn);

    TIM_Cmd(TIM2, ENABLE);
}

void bpm(int argc, char* argv[])
{
    char* str = argv[1];
    state.bpm = atoi(str);
    uint16_t period = (60 * BEAT_TIM_ARR_SEC) / state.bpm;

    rt_kprintf("set bpm to %d, period is %d\n", state.bpm, period);

    TIM_SetAutoreload(TIM2, period);
}
MSH_CMD_EXPORT(bpm, set bpm);

//void inst(int argc, char* argv[])
//{
//    char* str = argv[1];
//    uint8_t inst = atoi(str);
//    if (inst < 8)
//    {
//        state.seq = inst;
//        kbLEDDataSend(kb_led_data, pattern);
//    }
//    else
//    {
//        rt_kprintf("not an instrument!");
//    }
//}
//MSH_CMD_EXPORT(inst, change instrument);
