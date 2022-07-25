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

static struct rt_spi_device* kb_spi_dev;

uint8_t kb_led_data[KB_CHAIN_LENGTH] = {0};

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

    // Return 0 if nothing has changed
    if (!update)
    {
        return update;
    }

    // Clear out previous data
    memset(&input_events->button_events, 0, sizeof(input_events->button_events));

    // Put event data into input_event struct

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
