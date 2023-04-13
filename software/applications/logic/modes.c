/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-08-01     marcus       the first version
 */

#include <rtdef.h>
#include "ch32v30x.h"
#include <rtthread.h>
#include <rthw.h>
#include "drivers/pin.h"
#include <board.h>
#include <rtdevice.h>
#include "drv_spi.h"
#include "modes.h"

void modePattern(struct MachineState* state, struct InputEvents* input);
void modeLive(struct MachineState* state, struct InputEvents* input);

void seqStepToggle(struct PageData* page, struct InputEvents* input);

struct MachineMode modes[] =
{
        {.mode = pattern_mode, .func = modePattern},
        {.mode = live_mode, .func = modeLive}
};

void modePattern(struct MachineState* state, struct InputEvents* input)
{
    seqStepToggle(&state->patt_data[state->patt].seq[state->seq].page[state->page], input);
}

void seqStepToggle(struct PageData* page, struct InputEvents* input)
{
    uint8_t i;
    for (i = 0; i < page->length; i++)
    {
        page->step[i].type ^= (input->button_events.just_pressed.main_row >> i) & 1;
    }
}

/*****************************************************************************/
void modeLive(struct MachineState* state, struct InputEvents* input)
{

}
