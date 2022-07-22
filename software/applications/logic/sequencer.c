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
#include <string.h>

void kbLEDDataSend(uint8_t led_data[3], struct SeqData* seq, struct MachineState* state);
void seqStepToggle(struct SeqData* seq, struct InputEvents* events, struct MachineState* state);
void seqInputQueueGet(struct InputEvents* input_events);

void seqTaskEntry(void* param)
{
    struct SeqData sequence[1];
    struct InputEvents input_events;
    static struct MachineState state;

    memset(sequence, 0, sizeof(struct SeqData));

    while (1)
    {
        seqInputQueueGet(&input_events);

        seqStepToggle(sequence, &input_events, &state);

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

        kbLEDDataSend(kb_led_data, sequence, &state);          // TODO: Move function to UI task
    }
}

void kbLEDDataSend(uint8_t* led_data, struct SeqData* seq, struct MachineState* state)
{
    // [0] = function row L to R, [1] = 1 2 3 4 5 6 7 8, [2] = 9 10 11 12 13 14 15 16
    memset(led_data, 0, (sizeof(uint8_t) * KB_CHAIN_LENGTH));

    uint8_t i;
    uint8_t j;
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 8; j++)
        {
            led_data[i + 1] |= seq[CUR_SEQ].page[CUR_PAGE].step[j + (i * 8)].type << j;
        }
    }
}

void seqStepToggle(struct SeqData* seq, struct InputEvents* events, struct MachineState* state)
{
    uint8_t i;
    for (i = 0; i < 16; i++)
    {
        seq[CUR_SEQ].page[CUR_PAGE].step[i].type ^= (events->button_events.just_pressed.main_row >> i) & 1;
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
    rt_thread_t tid = rt_thread_create("sequencerTaskEntry", seqTaskEntry, RT_NULL, 4096, 6, 10);

    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    else
    {
        rt_kprintf("failed to start sequencerTask");
    }
}

//void stepHandler_Entry(void *param)
//{
//
//    rt_uint8_t kbButtonPressed[KBMODULE_CHAIN_LENGTH] = { 0 };
//
//    /* Infinite loop */
//    for (;;)
//    {
//        /* Do annoying pointer stuff for the mailbox */
//        rt_uint8_t* button_ptr;
//        if (rt_mb_recv(kbButton_MB, (rt_ubase_t*) &button_ptr, RT_WAITING_FOREVER) == RT_EOK)
//        {
//            rt_uint8_t i;
//            for (i = 0; i < KBMODULE_CHAIN_LENGTH; i++)
//            {
//                kbButtonPressed[i] = button_ptr[i];
//            }
//            rt_free(button_ptr);
//
//            rt_uint8_t kbLEDData[KBMODULE_CHAIN_LENGTH] = { 0 };
//            for (i = 0; i < KBMODULE_CHAIN_LENGTH; i++)
//            {
//                rt_uint8_t j;
//                for (j = 0; j < 8; j++)
//                {
//                    // TODO Update to allow other data types such as tuplets, also allow to change which instrument is being updated
//                    instrument[inst_current].sequence[7 - j].type ^= kbButtonPressed[i] & 1;
//                    kbButtonPressed[i] >>= 1;
//                    kbLEDData[i] |= (instrument[inst_current].sequence[7 - j].type) << j;
//                }
//            }
//
//            /* Dumb pointer stuff for mailbox*/
//            rt_uint8_t* led_ptr;
//            led_ptr = (rt_uint8_t *) rt_malloc(sizeof(rt_uint8_t) * KBMODULE_CHAIN_LENGTH);
//            for (i = 0; i < KBMODULE_CHAIN_LENGTH; i++)
//            {
//                led_ptr[i] = kbLEDData[i];
//            }
//            // Send data to the queue
//            if (rt_mb_send(kbLED_MB, (rt_uint32_t) led_ptr) != RT_EOK)
//            {
//                rt_kprintf("LED MB failed to send\n");
//            }
//        }
//        rt_thread_delay(1);
//    }
//}
//
//void stepHandler_init(void)
//{
//    rt_thread_t tid = rt_thread_create("stepHandler", stepHandler_Entry, RT_NULL, 2048, 16, 5);
//
//    if (tid != RT_NULL)
//    {
//        rt_thread_startup(tid);
//    }
//}
