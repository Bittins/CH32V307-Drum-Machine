/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/06/06
 * Description        : Main program body.
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/
#include "ch32v30x.h"
#include <rtthread.h>
#include <rthw.h>
#include "drivers/pin.h"
#include <board.h>
#include <gui/display.h>
#include <rtdevice.h>
#include "drv_spi.h"

#include "input/input.h"
#include "logic/sequencer.h"


/* Global typedef */

/* Global define */


#define BEAT_TIM_DEV_NAME       "timer0"
#define BEAT_TIM_PSC            2400-1      // Timer prescaler value; CLK / PSC = ARR per 1 second
#define BEAT_TIM_ARR_SEC        60000       // The value the autoreload register would need to be for a 1 second interval

#define LED0_PIN                0           // PA0


/* Global Variable */
//u8g2_t u8g2;
rt_uint16_t tempo = 160;
rt_uint8_t inst_current = 0;

//struct instData
//{
//    rt_uint8_t id;
//    const char* name;
//    struct stepData sequence[8];
//};
//
//struct instData instrument[INST_AMOUNT] = {0};

/* Global Mailbox */


/* Global Events */
rt_event_t inst_trig_event;

/* Function Declarations */

//void stepHandler_init(void);
//void beatTimer_init(void);
//void display_init(void);
//void instSR_init(void);

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    rt_kprintf("MCU: CH32V307\n");
    rt_kprintf("SysClk: %dHz\n", SystemCoreClock);
    rt_kprintf("www.wch.cn\n");

    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(LED0_PIN, PIN_LOW);

    inputTaskInit();
    seqTaskInit();
    guiTaskInit();

    LED1_BLINK_INIT();

    // LED blinky light thing
    GPIO_ResetBits(GPIOA, GPIO_Pin_0);
    while (1)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_0);
        rt_thread_mdelay(500);
        GPIO_ResetBits(GPIOA, GPIO_Pin_0);
        rt_thread_mdelay(500);
    }
}











