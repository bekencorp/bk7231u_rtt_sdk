/*
 * File      : board.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2017, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-5-30      Bernard      the first version
 */

#include <stdlib.h>

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "board.h"
#include "interrupt.h"
#include "driver_pub.h"
#include "drv_uart.h"
#include "include.h"
#include "func_pub.h"
#include "pseudo_random_pub.h"

enum wdg_status {
    WDG_STATUS_STOP,
    WDG_STATUS_REBOOT,
    WDG_STATUS_WATCH,
    WDG_STATUS_MAX,
};

struct wdg_context {
    enum wdg_status wdg_flag;
    int threshold_in_tick;
    int consumed_in_tick;
    rt_tick_t last_fresh_in_tick; /* add time to dealing with power save mode */
};

static struct wdg_context g_wdg_context = { WDG_STATUS_STOP, 0, 0 };

extern void os_clk_init(void);

#if (RT_TICK_PER_SECOND != 1000)
#error "RT_TICK_PER_SECOND of bk7231 can only be configured as 1000!!!"
#endif 

void rt_hw_board_init(void)
{
#ifdef RT_USING_HEAP
    /* init memory system */
    rt_system_heap_init(RT_HW_HEAP_BEGIN, RT_HW_HEAP_END);
#endif
    /* init hardware */
    driver_init();
    /* interrupt init */
    rt_hw_interrupt_init();
    /* init hardware interrupt */
    rt_hw_uart_init();
    /* init system tick */
    os_clk_init();

#ifdef RT_USING_CONSOLE
    /* set console device */
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif /* RT_USING_CONSOLE */

#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

}

#define WDT_DEV_NAME "wdt"
/**
 * reset cpu by dog's time-out
 */
void rt_hw_cpu_reset(void)
{
    struct rt_device *device;
    int time = 1;

    device = rt_device_find(WDT_DEV_NAME);

    if (!device)
    {
        rt_kprintf("Device %s not found \n");
        return ;
    }

    rt_kprintf("reboot system \n");
    bk_reboot();
    
    g_wdg_context.wdg_flag = WDG_STATUS_REBOOT;
}

FINSH_FUNCTION_EXPORT_ALIAS(rt_hw_cpu_reset, __cmd_reboot, reboot system);

/**
 * start dog's time-out
 */
void rt_hw_wdg_start(int argc, char **argv)
{
    struct rt_device *device;
    int time = 10000; /* 10000 * 1ms = 10000ms = 10s */

    if (WDG_STATUS_REBOOT == g_wdg_context.wdg_flag)
    {
        return;
    }

    if (argc > 1)
    {
        time = atoi(argv[1]);
    }

    device = rt_device_find(WDT_DEV_NAME);

    if (!device)
    {
        rt_kprintf("Device %s not found \n");
        return ;
    }

    rt_kprintf("start watch dog\n");
    rt_device_control(device, RT_DEVICE_CTRL_WDT_SET_TIMEOUT, &time);
    rt_device_control(device, RT_DEVICE_CTRL_WDT_START, RT_NULL);

    g_wdg_context.threshold_in_tick = time * 3 / 4;
    g_wdg_context.consumed_in_tick = 0;
    g_wdg_context.last_fresh_in_tick = rt_tick_get();
    rt_kprintf("%s time=%d threshold=%d\n", __FUNCTION__, time, g_wdg_context.threshold_in_tick);

    g_wdg_context.wdg_flag = WDG_STATUS_WATCH;
}

FINSH_FUNCTION_EXPORT_ALIAS(rt_hw_wdg_start, __cmd_wdg_start, wdg_start);

/**
 * refresh dog's time-out
 */
void rt_hw_wdg_refresh(void)
{
    struct rt_device *device;

    device = rt_device_find(WDT_DEV_NAME);

    if (!device)
    {
        rt_kprintf("Device %s not found \n");
        return ;
    }

    if (WDG_STATUS_WATCH == g_wdg_context.wdg_flag)
    {
        rt_kprintf("refresh watch dog\n");
        rt_device_control(device, RT_DEVICE_CTRL_WDT_KEEPALIVE, RT_NULL);
    }
    g_wdg_context.consumed_in_tick = 0;
    g_wdg_context.last_fresh_in_tick = rt_tick_get();
}

FINSH_FUNCTION_EXPORT_ALIAS(rt_hw_wdg_refresh, __cmd_wdg_refresh, wdg_refresh);

/**
 * stop dog's time-out
 */
void rt_hw_wdg_stop(void)
{
    struct rt_device *device;

    if (WDG_STATUS_REBOOT == g_wdg_context.wdg_flag)
    {
        return;
    }

    device = rt_device_find(WDT_DEV_NAME);

    if (!device)
    {
        rt_kprintf("Device %s not found \n");
        return ;
    }

    rt_kprintf("stop watch dog\n");
    rt_device_control(device, RT_DEVICE_CTRL_WDT_STOP, RT_NULL);

    g_wdg_context.wdg_flag = WDG_STATUS_STOP;
    g_wdg_context.threshold_in_tick = 0;
}

FINSH_FUNCTION_EXPORT_ALIAS(rt_hw_wdg_stop, __cmd_wdg_stop, wdg_stop);

void rt_hw_wdg_tick_proc()
{
    if (WDG_STATUS_WATCH != g_wdg_context.wdg_flag)
    {
        return;
    }
    g_wdg_context.consumed_in_tick++;
    /**
      * normal mode: using 120M RTC, 1 tick = 1ms
      * ps mode: using 32K RTC, rt_tick is adapted
      */
    if ((g_wdg_context.consumed_in_tick >= g_wdg_context.threshold_in_tick)
      || ((int)(rt_tick_get() - g_wdg_context.last_fresh_in_tick) >= g_wdg_context.threshold_in_tick))
    {
        rt_hw_wdg_refresh();
    }
}

#ifdef BEKEN_USING_WLAN
static int auto_func_init(void)
{
    unsigned seed;

    func_init();

    seed = prandom_get();
    //rt_kprintf("srand(prandom_get = %d) \n", seed);
    srand(seed);
    return 0;
}
INIT_PREV_EXPORT(auto_func_init);
#endif

extern void cp15_enable_alignfault(void);
static int auto_enable_alignfault(void)
{
    cp15_enable_alignfault();
    return 0;
}
INIT_BOARD_EXPORT(auto_enable_alignfault);
