/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-15     marcus       the first version
 */
#ifndef APPLICATIONS_INPUT_H_
#define APPLICATIONS_INPUT_H_

#include "keyboard.h"


// Frequency that the task loop will run at in Hz
#define INPUT_TASK_FREQ     240

struct InputEvents
{
    struct ButtonEvents button_events;
    int16_t enc_rotation;
};

uint8_t inputTaskInit(void);

extern rt_mq_t input_event_mq;

#endif /* APPLICATIONS_INPUT_H_ */
