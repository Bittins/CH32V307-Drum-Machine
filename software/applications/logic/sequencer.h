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

#define INST_AMNT           8
#define MAX_PATT            1
#define MAX_PAGE            16
#define MAX_STEP            16

#define BEAT_TIM_PSC        2400-1      // Timer prescaler value; CLK / PSC = ARR per 1 second
#define BEAT_TIM_ARR_SEC    60000       // The value the autoreload register would need to be for a 1 second interval

#define BPM_DEFAULT         120
#define QUARTER_LENGTH      4           // How many beats in a quarter note

enum InstName
{
    kick, snare, ch, tom
};

struct InstData
{
    enum InstName inst;
    uint8_t sr_pos;
    uint8_t button;         // The corresponding button to trigger that instrument
};

enum StepType
{
    empty,
    simple,
    complex
};

struct StepData
{
    enum StepType type;
    uint8_t tuplet_length;
    uint16_t tuplet_data;
};

struct PageData
{
    struct StepData step[MAX_STEP];
    uint8_t length;
};

struct SeqData
{
    struct PageData page[MAX_PAGE];
    uint8_t length;
    struct InstData inst;
};

struct PattData
{
    struct SeqData seq[INST_AMNT];
    uint8_t length;
};

struct MachineState
{
    struct PattData patt_data[MAX_PATT];
    enum InstName inst;
    uint8_t step;
    uint8_t page;
    uint8_t seq;
    uint8_t patt;
    uint16_t bpm;
    uint8_t swing;
    uint8_t mode;
    uint8_t play;
};

uint8_t seqTaskInit(void);

extern rt_mq_t state_data_mq;
extern rt_event_t gui_update_event;

#endif /* APPLICATIONS_SEQUENCER_H_ */
