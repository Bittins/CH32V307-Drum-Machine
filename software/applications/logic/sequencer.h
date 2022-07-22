/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-17     marcus       the first version
 */
#ifndef APPLICATIONS_SEQUENCER_H_
#define APPLICATIONS_SEQUENCER_H_

#include "input/input.h"

#define CUR_SEQ     state->seq
#define CUR_PAGE    state->page
#define CUR_STEP    state->step

#define PATTERN_MODE        1

enum StepType
{
    empty, simple, complex
};

struct StepData
{
    enum StepType type;
    uint8_t tuplet_length;
    uint16_t tuplet_data;
};

struct PageData
{
    struct StepData step[16];
    uint8_t length;
};

struct SeqData
{
    struct PageData page[16];
    uint8_t length;
};

struct MachineState
{
    uint8_t step;
    uint8_t page;
    uint8_t seq;
    uint8_t inst;
    uint8_t bpm;
    uint8_t swing;
    uint8_t mode;
};

uint8_t sequencerTaskInit(void);

#endif /* APPLICATIONS_SEQUENCER_H_ */
