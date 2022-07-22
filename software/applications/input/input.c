/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-15     marcus       the first version
 */
#include "input.h"

void inputQueueSend(struct InputEvents* input_events);

rt_mq_t input_event_mq;

void inputTaskEntry(void* param)
{
    const rt_tick_t delay = rt_tick_from_millisecond(1000 / INPUT_TASK_FREQ);
    rt_tick_t tick = rt_tick_get();

    static struct InputEvents input_events;

    uint8_t button_data[KB_CHAIN_LENGTH] = {0};
    uint8_t led_data[KB_CHAIN_LENGTH] = {0};


    while (1)
    {
        kbLEDDataGet(led_data);
        kbTransferData(led_data, button_data);

        // If there are any updates to the keyboard or encoder, then send event queue
        if (kbParseButtons(button_data, &input_events) || encUpdate(&input_events))
        {
            inputQueueSend(&input_events);
        }

//        rt_kprintf("e");
        rt_thread_delay_until(&tick, delay);
    }
}

void inputQueueSend(struct InputEvents* input_events)
{
//    rt_kprintf("event!\n");
    if (rt_mq_send(input_event_mq, input_events, sizeof(struct InputEvents)) != RT_EOK)
    {
        rt_kprintf("failed to send input message\n");
    }
}

uint8_t inputTaskInit(void)
{
    kbInit();
    encInit();

    input_event_mq = rt_mq_create("input_event_mq", sizeof(struct InputEvents), 8, RT_IPC_FLAG_PRIO);

    rt_thread_t tid = rt_thread_create("inputTaskEntry", inputTaskEntry, RT_NULL, 4096, 7, 10);

    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    else
    {
        rt_kprintf("failed to start inputTask");
    }
}
