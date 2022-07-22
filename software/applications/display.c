/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-05     marcus       the first version
 */

#include "display.h"

void display_Entry(void *param)
{
    while (1)
    {
//        u8g2_UpdateDisplay(&u8g2);
//        rt_thread_delay(10);

    }
}

void display_init(void)
{
//    // Initialization
//    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_rtthread_hw_i2c, u8x8_gpio_and_delay_rtthread);
//    u8g2_InitDisplay(&u8g2);
//    u8g2_SetPowerSave(&u8g2, 0);
//
//    /* full buffer example, setup procedure ends in _f */
//    u8g2_ClearBuffer(&u8g2);
//    u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
//    u8g2_DrawStr(&u8g2, 1, 18, "U8g2 on RT-Thread");
//    u8g2_SendBuffer(&u8g2);
//
    // Draw Graphics
//    u8g2_SetFont(&u8g2, u8g2_font_unifont_t_symbols);
//    u8g2_DrawGlyph(&u8g2, 112, 56, 0x2603);
//    u8g2_SendBuffer(&u8g2);

//    rt_thread_t tid = rt_thread_create("display", display_Entry, RT_NULL, 2048, 16, 5);
//
//    if (tid != RT_NULL)
//    {
//        rt_thread_startup(tid);
//    }
}
