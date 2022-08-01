/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-22     marcus       the first version
 */
#ifndef APPLICATIONS_INST_SR_H_
#define APPLICATIONS_INST_SR_H_

#define INST_SR_SPI_DEV_NAME    "spi20"
#define INST_SR_SPI_BUS         "spi2"
#define INST_SR_PULSE_PIN       17          // PB1
#define INST_SR_PULSE_LENGTH    5000        // Length of the pulse in microseconds

#define LED1_PIN                1           // PA1

void instSROut(struct MachineState* state);
uint8_t instSRInit(void);

#endif /* APPLICATIONS_INST_SR_H_ */
