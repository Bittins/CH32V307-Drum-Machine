/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-27     marcus       the first version
 */

#include <rtdef.h>
#include "ch32v30x.h"
#include <rtthread.h>
#include <rthw.h>
#include "drivers/pin.h"
#include <board.h>
#include <rtdevice.h>
#include "drv_spi.h"
#include <string.h>
#include "gui.h"
#include "input/keyboard.h"
#include "logic/sequencer.h"

uint8_t guiUpdateEventGet(void);
void guiUpdate(struct PattData* patt, struct MachineState* state);
uint8_t guiKBEventGet(void);
uint8_t guiQueueGet(struct PattData** patt, struct MachineState** state);

static struct PattData* pattern;
static struct MachineState* state;
static struct GUIData gui_data;

void guiTaskEntry(void* param)
{
    while (1)
    {
        if (guiUpdateEventGet())
        {
            guiUpdate(pattern, state);
        }

        if (guiKBEventGet())
        {
            ledBrightnessHandler(kb_led_data, gui_data.led_struct);
        }
//        rt_thread_delay(5);
    }
}

// Wait until keyboard has transfered its data
uint8_t guiKBEventGet(void)
{
    uint8_t status = 0;

    switch (rt_event_recv(kb_update_event, 1, RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR, 0, RT_NULL))
    {
        case RT_EOK:
//            rt_kprintf("success getting kb update event\n");
            status = 1;
            break;

        case -RT_ETIMEOUT:
//            rt_kprintf("timeout getting kb update event\n");
            status = 0;
            break;

        default:
            rt_kprintf("error getting kb update event\n");
            break;
    }

    return status;
}

uint8_t guiUpdateEventGet(void)
{
    uint8_t status = 0;

    switch (rt_event_recv(gui_update_event, 1, RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR, 0, RT_NULL))
    {
        case RT_EOK:
//            rt_kprintf("success getting gui update event\n");
            status = 1;
            break;

        case -RT_ETIMEOUT:
//            rt_kprintf("timeout getting gui update event\n");
            status = 0;
            break;

        default:
            rt_kprintf("error getting gui update event\n");
            break;
    }

    return status;
}

void guiUpdate(struct PattData* patt, struct MachineState* state)
{
    uint8_t cur_patt = state->patt;
    uint8_t cur_seq  = state->seq;
    uint8_t cur_page = state->page;
    uint8_t cur_step = state->step;

    uint8_t i;
    // Set main row of 16 to brightness 3 if it has a beat
    for (i = 0; i < 16; i++)
    {
        if (patt[cur_patt].seq[cur_seq].page[cur_page].step[i].type)
        {
            // Offset by 8 since data starts with 8 function buttons
            gui_data.led_struct[i + 8].brightness = 1;
//            rt_kprintf("1");
        }
        else
        {
            gui_data.led_struct[i + 8].brightness = 0;
//            rt_kprintf("0");
        }
    }
//    rt_kprintf("\n");
    // Set current beat to brightness 4
    gui_data.led_struct[cur_step + 8].brightness = 4;
}

uint8_t guiQueueGet(struct PattData** patt, struct MachineState** state)
{
    struct PattStatePointers msg;

    RT_ASSERT(rt_mq_recv(patt_state_mq, &msg, sizeof(msg), 0) == RT_EOK);

    *patt  = msg.patt;
    *state = msg.state;

    rt_mq_delete(patt_state_mq);
}

uint8_t guiTaskInit(void)
{
    guiQueueGet(&pattern, &state);

    memset(&gui_data, 0, sizeof(gui_data));

    rt_thread_t tid = rt_thread_create("guiTaskEntry", guiTaskEntry, RT_NULL, 2048, 7, 10);

    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    else
    {
        rt_kprintf("failed to start guiTask\n");
    }
}
