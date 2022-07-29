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

void kbGUIEventSend(void);

rt_event_t kb_update_event;
uint8_t kb_led_data[KB_CHAIN_LENGTH] = {0};
static struct rt_spi_device* kb_spi_dev;

// TODO: Modify for future LED data (PWM brightness)
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

    // LED:    [0] = function row L to R, [1] = 1 2 3 4 5 6 7 8, [2] = 9 10 11 12 13 14 15 16
    // Button: [0] == 16 15 14 13 12 11 10 9, [1] == 8 7 6 5 4 3 2 1, [2] == function buttons R to L
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

    kbGUIEventSend();
}

void kbGUIEventSend(void)
{
    switch (rt_event_send(kb_update_event, 1))
    {
        case RT_EOK:
//            rt_kprintf("success sending kb update event\n");
            break;

        case RT_ETIMEOUT:
            rt_kprintf("timeout sending kb update event\n");
            break;

        default:
            rt_kprintf("error sending kb update event\n");
            break;
    }
}

uint8_t kbParseButtons(uint8_t* button_data, struct ButtonEvents* button_events)
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

    // Return 0 if nothing has changed
    if (!update)
    {
        button_events->just_pressed.main_row   = 0;
        button_events->just_released.main_row  = 0;

        return update;
    }

    // Clear out previous data
    memset(button_events, 0, sizeof(*button_events));

    // Put event data into input_event struct

    // [0] == 16 15 14 13 12 11 10 9, [1] == 8 7 6 5 4 3 2 1, [2] == function buttons R to L
    // Designed the boards dumb and backwards so i have to manipulate the data dumb and backwards
    for (i = 0; i < 2; i++)
    {
        button_events->is_held.main_row        |= (((uint16_t)is_held[i]) << ((1 - i) * 8));
        button_events->just_pressed.main_row   |= (((uint16_t)just_pressed[i]) << ((1 - i) * 8));
        button_events->just_released.main_row  |= (((uint16_t)just_released[i]) << ((1 - i) * 8));
    }

    button_events->is_held.function        = is_held[2];
    button_events->just_pressed.function   = just_pressed[2];
    button_events->just_released.function  = just_released[2];

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

    kb_update_event = rt_event_create("kb_update_event", RT_IPC_FLAG_PRIO);
}
