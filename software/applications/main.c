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
#include <rtdevice.h>
#include "drv_spi.h"

#include "display.h"
#include "input/input.h"
#include "logic/sequencer.h"


/* Global typedef */

/* Global define */
#define INST_AMOUNT             1           // Number of instruments
#define INST_SR_SPI_DEV_NAME    "spi20"
#define INST_SR_SPI_BUS         "spi2"
#define INST_SR_PULSE_PIN       17          // PB1
#define INST_SR_PULSE_LENGTH    5           // Length of the pulse in milliseconds

#define BEAT_TIM_DEV_NAME       "timer0"
#define BEAT_TIM_PSC            2400-1      // Timer prescaler value; CLK / PSC = ARR per 1 second
#define BEAT_TIM_ARR_SEC        60000       // The value the autoreload register would need to be for a 1 second interval

#define LED0_PIN                0           // PA0
#define LED1_PIN                1           // PA1

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

    inst_trig_event = rt_event_create("inst_trig_event", RT_IPC_FLAG_PRIO);

    inputTaskInit();
    sequencerTaskInit();
//    beatTimer_init();
//    instSR_init();

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





//__attribute__((interrupt("WCH-Interrupt-fast"))) void TIM2_IRQHandler(void)
//{
//    rt_interrupt_enter();
//    rt_event_send(inst_trig_event, 1);
//    TIM2->INTFR = 0;
//    rt_interrupt_leave();
//}
//
//void beatTimer_init(void)
//{
//    rt_uint16_t period = (60 * BEAT_TIM_ARR_SEC) / tempo;
//
//    // Enable internal Advanced Peripheral Bus clock for timer
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
//
//    // Create and initialize timer
//    TIM_TimeBaseInitTypeDef tim_base_struct = { 0 };
//    tim_base_struct.TIM_Period = period;
//    tim_base_struct.TIM_Prescaler = BEAT_TIM_PSC;
//    tim_base_struct.TIM_ClockDivision = TIM_CKD_DIV1;
//    tim_base_struct.TIM_CounterMode = TIM_CounterMode_Up;
//
//    TIM_TimeBaseInit(TIM2, &tim_base_struct);
//    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
//
//    // Create and initialize interrupt
//    NVIC_InitTypeDef NVIC_InitStructure = { 0 };
//    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//
//    NVIC_Init(&NVIC_InitStructure);
//    NVIC_EnableIRQ(TIM2_IRQn);
//
//    TIM_Cmd(TIM2, ENABLE);
//}
//
//
//
//struct instSR_param
//{
//    struct rt_spi_device* instSR_spi_dev;
////    struct instData* instrument;
//};
//
//void instSR_Entry(void* param)
//{
//    struct instSR_param* entry_param = (struct instSR_param*) param;
//    struct rt_spi_device* instSR_spi_dev = (struct rt_spi_device*) entry_param->instSR_spi_dev;
////    struct instData instrument = *entry_param->instrument;
//
//    rt_uint8_t i;
//
//    while (1)
//    {
//        rt_kprintf("\n");
//
//        for (i = 0; i < 8; i++)
//        {
//            // Wait for event from timer
//            rt_event_recv(inst_trig_event, 1, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, RT_NULL);
//
//            // Arrange sequence data into what needs to be shifted out this beat
//            rt_uint8_t sr_data = 0;
//            rt_uint8_t j;
//            for (j = 0; j < INST_AMOUNT; j++)
//            {
//                sr_data |= (instrument[j].sequence[i].type) << j;   // TODO Update to allow other data types such as tuplets
//            }
//
//            // Send out data through SPI
//            if (rt_spi_transfer(instSR_spi_dev, &sr_data, RT_NULL, 1) == 0)
//            {
//                rt_kprintf("inst transfer failed D:\n");
//            }
//
//            // Wait for data to finish transmitting
//            while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
//
//            // Trigger pulse pin
//            // TODO: Change to perhaps a one shot timer thing
//            rt_pin_write(INST_SR_PULSE_PIN, PIN_LOW);
//            rt_pin_write(LED1_PIN, PIN_LOW);
//            rt_thread_mdelay(INST_SR_PULSE_LENGTH);
//            rt_pin_write(INST_SR_PULSE_PIN, PIN_HIGH);
//            rt_pin_write(LED1_PIN, PIN_HIGH);
//
//            if (sr_data == 0)
//            {
//                rt_kprintf("-");
//            }
//            else
//            {
//                rt_kprintf("!");
//            }
//        }
//    }
//}
//
//void instSR_init(void)
//{
//    /*
//     * SPI2 PINS:
//     *  SPI_NSS =   PB12 (RCLK)
//     *  SPI_SCK =   PB13 (SRCLK)
//     *  SPI_MISO =  PB14 (NOT USED)
//     *  SPI_MOSI =  PB15 (SER)
//     */
//
//    /* Init SPI */
//    // Attach SPI device to bus
//    if (rt_hw_spi_device_attach(INST_SR_SPI_BUS, INST_SR_SPI_DEV_NAME, GPIOB, GPIO_Pin_12) != RT_EOK)
//    {
//        rt_kprintf("failed to attach %s device :(\n", INST_SR_SPI_DEV_NAME);
//    }
//
//    // Find SPI device
//    struct rt_spi_device *instSR_spi_dev;
//    instSR_spi_dev = (struct rt_spi_device *) rt_device_find(INST_SR_SPI_DEV_NAME);
//
//    if (instSR_spi_dev == RT_NULL)
//    {
//        rt_kprintf("failed to find %s device :/\n", INST_SR_SPI_DEV_NAME);
//    }
//
//    // Configure SPI device
//    struct rt_spi_configuration cfg;
//    cfg.data_width = 8;
//    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
//    cfg.max_hz = 115200;        // TODO: Figure out a reliable maximum speed
//
//    if (rt_spi_configure(instSR_spi_dev, &cfg) != RT_EOK)
//    {
//        rt_kprintf("%s failed to configure D:\n", INST_SR_SPI_DEV_NAME);
//    }
//
//    // Configure pulse pin
//    rt_pin_mode(INST_SR_PULSE_PIN, PIN_MODE_OUTPUT);
//    rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);
//    rt_pin_write(INST_SR_PULSE_PIN, PIN_HIGH);
//    rt_pin_write(LED1_PIN, PIN_HIGH);
//
////    struct instData instrument[INST_AMOUNT] = {0};
//
//    rt_uint8_t i;
//    for (i = 0; i < INST_AMOUNT; i++)
//    {
//        instrument[i].id = i;
//        // Initialize sequence to 0
////        memset(&instrument[i].sequence, 0, sizeof(instrument->sequence));
//    }
//
//    static struct instSR_param param;
//    param.instSR_spi_dev = instSR_spi_dev;
////    param.instrument = (struct instData*)instrument;
//
//    rt_thread_t tid = rt_thread_create("instSR", instSR_Entry, &param, 2048, 3, 5);
//
//    if (tid != RT_NULL)
//    {
//        rt_thread_startup(tid);
//    }
//    else
//    {
//        rt_kprintf("failed to start instSR");
//    }
//}

