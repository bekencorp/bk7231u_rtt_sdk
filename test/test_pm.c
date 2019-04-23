#include <rthw.h>
#include <rtthread.h>

#include <stdint.h>
#include <stdlib.h>

extern int beken_pm_level;

extern int bk_wlan_dtim_rf_ps_mode_enable(void );
extern int bk_wlan_dtim_rf_ps_mode_need_disable(void);

extern int bk_wlan_mcu_ps_mode_enable(void);
extern int bk_wlan_mcu_ps_mode_disable(void);

extern int bk_wlan_dtim_with_normal_open(void);
extern int bk_wlan_dtim_with_normal_close(void);

extern void power_save_set_linger_time(uint32_t data_wakeup_time);

static int pm_level(int argc, char **argv)
{
	uint32_t level;

    if(argc != 2)
    {
        rt_kprintf("input argc is err!\n");
		return -1;
    }

	level = atoi(argv[1]);
	if(level > 99)
	{
		level = 99;
	}

	rt_kprintf("power_save_level: %d\n", level);
	//power_save_set_linger_time(level);
	//beken_pm_level = level;
    {
        struct rt_wlan_device *sta_device = (struct rt_wlan_device *)rt_device_find(WIFI_DEVICE_STA_NAME);
        if (NULL != sta_device)
        {
            rt_wlan_enter_powersave(sta_device, level);
            rt_kprintf("rt_wlan_enter_powersave switch to %d\n", level);
        }
    }

	return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>

MSH_CMD_EXPORT(pm_level, pm_level 1);

#if CFG_USE_STA_PS
FINSH_FUNCTION_EXPORT_ALIAS(bk_wlan_dtim_rf_ps_mode_enable, __cmd_rf_ps, bk_wlan_dtim_rf_ps_mode_enable);
#endif
//FINSH_FUNCTION_EXPORT_ALIAS(bk_wlan_dtim_rf_ps_mode_need_disable, __cmd_rf_ps_disable, bk_wlan_dtim_rf_ps_mode_need_disable);

#if CFG_USE_MCU_PS
FINSH_FUNCTION_EXPORT_ALIAS(bk_wlan_mcu_ps_mode_enable, __cmd_mcu_ps, bk_wlan_mcu_ps_mode_enable);
FINSH_FUNCTION_EXPORT_ALIAS(bk_wlan_mcu_ps_mode_disable, __cmd_mcu_ps_disable, bk_wlan_mcu_ps_mode_disable);
#endif

//FINSH_FUNCTION_EXPORT_ALIAS(bk_wlan_dtim_with_normal_open, __cmd_dtim_normal_open, bk_wlan_dtim_with_normal_open);
//FINSH_FUNCTION_EXPORT_ALIAS(bk_wlan_dtim_with_normal_close, __cmd_dtim_normal_close, bk_wlan_dtim_with_normal_close);

#endif /* RT_USING_FINSH */

