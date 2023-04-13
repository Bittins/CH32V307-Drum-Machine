/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-08-01     marcus       the first version
 */
#ifndef APPLICATIONS_LOGIC_MODES_H_
#define APPLICATIONS_LOGIC_MODES_H_

#include "sequencer.h"

enum Modes
{
    pattern_mode,
    live_mode
};

struct MachineMode
{
    enum Modes mode;
    void (*func)(struct MachineState*, struct InputEvents*);
};

extern struct MachineMode modes[];

#endif /* APPLICATIONS_LOGIC_MODES_H_ */
