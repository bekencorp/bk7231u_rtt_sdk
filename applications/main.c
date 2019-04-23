/*
 * File      : main.c
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

#include "rtthread.h"

#include "include.h"
#include "driver_pub.h"
#include "func_pub.h"
#include "app.h"
#include "ate_app.h"
#include "shell.h"
static int wlan_app_init(void);

int main(int argc, char **argv)
{
    wlan_app_init();
    return 0;
}

extern void rt_hw_wdg_start(int argc, char **argv);
void user_app_start(void)
{
    rt_hw_wdg_start(0, NULL);
}

#ifdef BEKEN_USING_WLAN

extern void ate_app_init(void);
extern void ate_start(void);

static int wlan_app_init(void)
{
	/* init ate mode check. */
	ate_app_init();

	if (get_ate_mode_state())
	{
		rt_kprintf("\r\n\r\nEnter automatic test mode...\r\n\r\n");

		finsh_set_echo(0);
		finsh_set_prompt("#");

		ate_start();
	}
	else
	{
		rt_kprintf("Enter normal mode...\r\n\r\n");
		app_start();

		user_app_start();
	}

	return 0;
}

#endif
