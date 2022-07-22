/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-05     marcus       the first version
 */

#include <string.h>
#include "keyboard.h"

static struct rt_spi_device *kb_spi_dev;

uint8_t kb_led_data[KB_CHAIN_LENGTH] = {0};

void kbLEDDataGet(uint8_t* led_data)
{
    memcpy(led_data, kb_led_data, sizeof(uint8_t) * KB_CHAIN_LENGTH);
}

void kbTransferData(uint8_t* led_data, uint8_t* button_data)
{
    /*
     * Send latch pulse
     * Goes to button SR SH/!LD and LED SR RCLK    ‾‾‾‾|_|‾‾‾‾
     */
    rt_pin_write(KB_LATCH_PIN, PIN_LOW);
    rt_pin_write(KB_LATCH_PIN, PIN_HIGH);

    uint8_t button_data_temp[KB_CHAIN_LENGTH] = {0};

    // Download KB button data and upload LED data
    if (rt_spi_transfer(kb_spi_dev, led_data, button_data_temp, KB_CHAIN_LENGTH) == 0)
    {
        rt_kprintf("kb transfer failed D:\n");
    }

    // Button data requires logical inversion
    uint8_t i;
    for (i = 0; i < KB_CHAIN_LENGTH; i++)
    {
        button_data[i] = ~button_data_temp[i];
    }
}

uint8_t kbParseButtons(uint8_t* button_data, struct InputEvents* input_events)
{
    uint8_t update = 0;
    static uint8_t button_data_prev[KB_CHAIN_LENGTH];

    uint8_t is_held[KB_CHAIN_LENGTH]          = {0};
    uint8_t just_pressed[KB_CHAIN_LENGTH]     = {0};
    uint8_t just_released[KB_CHAIN_LENGTH]    = {0};

    // Go through the button data and mask to split it into events
    uint8_t i;
    for (i = 0; i < 2; i++)
    {
        if (button_data[i] != button_data_prev[i])
        {
            is_held[i]          = button_data[i];
            // Set pressed to what is pressed now AND what was NOT (& ~) pressed last time
            just_pressed[i]     = button_data[i] & ~button_data_prev[i];
            // Set released to what is NOT pressed now AND what was pressed last time
            just_released[i]    = ~button_data[i] & button_data_prev[i];

            button_data_prev[i] = button_data[i];

            update = 1;
        }
    }

    if (!update)
    {
        return update;
    }

    memset(&input_events->button_events, 0, sizeof(input_events->button_events));

    // [0] == 16 15 14 13 12 11 10 9, [1] == 8 7 6 5 4 3 2 1, [2] == function buttons R to L
    // Designed the boards dumb and backwards so i have to manipulate the data dumb and backwards
    for (i = 0; i < 2; i++)
    {
        input_events->button_events.is_held.main_row        |= (((uint16_t)is_held[i]) << ((1 - i) * 8));
        input_events->button_events.just_pressed.main_row   |= (((uint16_t)just_pressed[i]) << ((1 - i) * 8));
        input_events->button_events.just_released.main_row  |= (((uint16_t)just_released[i]) << ((1 - i) * 8));
    }


    input_events->button_events.is_held.function        = is_held[2];
    input_events->button_events.just_pressed.function   = just_pressed[2];
    input_events->button_events.just_released.function  = just_released[2];

    return update;
}


uint8_t kbInit(void)
{
    /*
     * SPI1 PINS:
     *  SPI_NSS =   PA4 (NOT USED)
     *  SPI_SCK =   PA5 (CLK)
     *  SPI_MISO =  PA6 (BUTTONOUT)
     *  SPI_MOSI =  PA7 (LEDIN)
     */

    // Attach SPI device to bus
    if (rt_hw_spi_device_attach(KB_SPI_BUS, KB_SPI_DEV_NAME, GPIOA, GPIO_Pin_4) != RT_EOK)
    {
        rt_kprintf("failed to attach %s device :(\n", KB_SPI_DEV_NAME);
    }

    // Find SPI device
    kb_spi_dev = (struct rt_spi_device *) rt_device_find(KB_SPI_DEV_NAME);
    if (kb_spi_dev == RT_NULL)
    {
        rt_kprintf("failed to find %s device :/\n", KB_SPI_DEV_NAME);
    }

    // Configure SPI device
    struct rt_spi_configuration cfg;
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB | RT_SPI_NO_CS;
    cfg.max_hz = 115200;        // TODO: Figure out a reliable maximum speed

    if (rt_spi_configure(kb_spi_dev, &cfg) != RT_EOK)
    {
        rt_kprintf("failed to configure %s device D:\n", KB_SPI_DEV_NAME);
    }

    // Configure latch pin & LED0 pin
    rt_pin_mode(KB_LATCH_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(KB_LATCH_PIN, PIN_HIGH);
}

//void kbHandler_Entry(void *param)
//{
//    struct kbHandler_param* entry_param = (struct kbHandler_param*) param;
//    struct rt_spi_device* kbModule_spi_dev = entry_param->kbModule_spi_dev;
//    if (kbModule_spi_dev == RT_NULL)
//    {
//        rt_kprintf("failed to pass kbmodule_spi_dev to kbhandler_entry\n");
//    }
//
//    // MS time delay between tasks
//    const rt_uint8_t delay = rt_tick_from_millisecond(10);
//    rt_uint32_t tick = rt_tick_get();
//
//    /*
//     * Keyboard button and LED data are stored in an array, each element = one keyboard module
//     *  A good reference: https://www.mathcs.emory.edu/%7Echeung/Courses/255/Syllabus/1-C-intro/bit-array.html
//     */
//    rt_uint8_t kbButtonData[KBMODULE_CHAIN_LENGTH] = { 0 }, kbButtonDataPrev[KBMODULE_CHAIN_LENGTH] = { 0 };
//    rt_uint8_t kbLEDData[KBMODULE_CHAIN_LENGTH] = { 0 };
//
//    /* Infinite loop */
//    for (;;)
//    {
//        /* Handle KB module */
//        rt_uint8_t* led_ptr;
//        rt_mb_recv(kbLED_MB, (rt_ubase_t *) &led_ptr, 0);
//        rt_uint8_t i;
//        for (i = 0; i < KBMODULE_CHAIN_LENGTH; i++)
//        {
//            kbLEDData[i] = led_ptr[i];
//        }
//        rt_free(led_ptr);
//
//        /*
//         * Send latch pulse
//         * Goes to button SR SH/!LD and LED SR RCLK    ‾‾‾‾|_|‾‾‾‾
//         */
//        rt_pin_write(KBMODULE_LATCH_PIN, PIN_LOW);
//        //rt_thread_delay(1);
//        rt_pin_write(KBMODULE_LATCH_PIN, PIN_HIGH);
//
//        // Download KB button data and upload LED data
//        if (rt_spi_transfer(kbModule_spi_dev, kbLEDData, kbButtonData, KBMODULE_CHAIN_LENGTH) == 0)
//        {
//            rt_kprintf("transfer failed D:\n");
//        }
//
//        // Figure out what buttons have been pressed
//        rt_uint8_t kbButtonPressed[KBMODULE_CHAIN_LENGTH] = { 0 };
//        for (i = 0; i < KBMODULE_CHAIN_LENGTH; i++)
//        {
//            // Invert bits since logic on buttons is pressed == high
//            kbButtonData[i] = ~kbButtonData[i];
//            // set pressed value to what was pressed now and what was not (& ~) pressed last time
//            kbButtonPressed[i] = kbButtonData[i] & ~kbButtonDataPrev[i];
//            kbButtonDataPrev[i] = kbButtonData[i];
//        }
//
//        /* Dumb pointer stuff for mailbox*/
//        rt_uint8_t* button_ptr;
//        button_ptr = (rt_uint8_t *) rt_malloc(sizeof(rt_uint8_t) * KBMODULE_CHAIN_LENGTH);
//        for (i = 0; i < KBMODULE_CHAIN_LENGTH; i++)
//        {
//            button_ptr[i] = kbButtonPressed[i];
//        }
//
//        // Send data to the queue
//        if (rt_mb_send(kbButton_MB, (rt_uint32_t) button_ptr) != RT_EOK)
//        {
//            rt_kprintf("Button MB failed to send\n");
//        }
//
//        /* Delay task */
//        rt_thread_delay_until(&tick, delay);
//    }
//}
//
//void kbHandler_init(void)
//{
//    /*
//     * SPI1 PINS:
//     *  SPI_NSS =   PA4 (NOT USED)
//     *  SPI_SCK =   PA5 (CLK)
//     *  SPI_MISO =  PA6 (BUTTONOUT)
//     *  SPI_MOSI =  PA7 (LEDIN)
//     */
//
//    /* Init SPI */
//    // Attach SPI device to bus
//    if (rt_hw_spi_device_attach(KBMODULE_SPI_BUS, KBMODULE_SPI_DEV_NAME, GPIOA, GPIO_Pin_4) != RT_EOK)
//    {
//        rt_kprintf("failed to attach %s device :(\n", KBMODULE_SPI_DEV_NAME);
//    }
//
//    // Find SPI device
//    struct rt_spi_device *kbModule_spi_dev;
//    kbModule_spi_dev = (struct rt_spi_device *) rt_device_find(KBMODULE_SPI_DEV_NAME);
//
//    if (kbModule_spi_dev == RT_NULL)
//    {
//        rt_kprintf("failed to find %s device :/\n", KBMODULE_SPI_DEV_NAME);
//    }
//
//    // Configure SPI device
//    struct rt_spi_configuration cfg;
//    cfg.data_width = 8;
//    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_LSB | RT_SPI_NO_CS;
//    cfg.max_hz = 115200;        // TODO: Figure out a reliable maximum speed
//
//    if (rt_spi_configure(kbModule_spi_dev, &cfg) != RT_EOK)
//    {
//        rt_kprintf("%s failed to configure D:\n", KBMODULE_SPI_DEV_NAME);
//    }
//
//    // Configure latch pin & LED0 pin
//    rt_pin_mode(KBMODULE_LATCH_PIN, PIN_MODE_OUTPUT);
//    rt_pin_write(KBMODULE_LATCH_PIN, PIN_HIGH);
//
//    // Create and startup thread
//    static struct kbHandler_param param;
//    param.kbModule_spi_dev = kbModule_spi_dev;
//    rt_thread_t tid = rt_thread_create("kbHandler", kbHandler_Entry, &param, 4096, 15, 5);
//
//    if (tid != RT_NULL)
//    {
//        rt_thread_startup(tid);
//    }
//}
