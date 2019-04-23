#include "sys_rtos.h"
#include "rtos_pub.h"
#include "error.h"

#include "include.h"
#include "mem_pub.h"
#include "uart_pub.h"
#include "str_pub.h"
#include "sys_ctrl_pub.h"
#include "gpio_pub.h"
#include "usb_pub.h"
#include "net_param_pub.h"
#include "fake_clock_pub.h"

#if (CFG_SOC_NAME == SOC_BK7221U)
UINT32 charge_elect = 530;
UINT32 charge_func_init = 0;
UINT32 last_second;

UINT32 usb_charge_oper_val(UINT32 elect)
{
    if(elect < 450)
        elect = 450;
    else if(elect > 750)
        elect = 750;

    return (elect - 450) / 20;
}

void usb_charge_start()
{
    CHARGE_OPER_ST chrg;
    chrg.type = 1;
    if(!get_info_item(CHARGE_CONFIG_ITEM, (UINT8 *)chrg.cal, NULL, NULL))
    {
    }
    else
    {
        os_printf("load charge cal %x %x %x\r\n", chrg.cal[0], chrg.cal[1], chrg.cal[2]);
    }
    chrg.oper = usb_charge_oper_val(charge_elect);
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_USB_CHARGE_START, (void *)&chrg);
}

void usb_charge_stop()
{
    UINT32 charge_type = 1;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_USB_CHARGE_STOP, &charge_type);
}

void usb_charge_check_cb(void)
{
    UINT32 tmp;

    if(charge_func_init && usb_is_pluged())
    {
        tmp = fclk_get_second();
        if(tmp > last_second)
        {   /*check per second,if charge full*/
            if(charger_is_full())
            {
                usb_charge_stop();
                os_printf("charger_is_full\r\n");
            }
            else
            {
            }

            last_second = tmp;
        }
    }

}

void usb_plug_func_handler(void *usr_data, UINT32 event)
{
    UINT32 charge_type = 1;

    switch(event)
    {
    case USB_PLUG_IN_EVENT:
        usb_charge_start();
        break;

    case USB_PLUG_OUT_EVENT:
        usb_charge_stop();
        break;

    case USB_PLUG_NO_EVENT:

        break;

    default:
        break;
    }

}

void usb_plug_func_open(void)
{
    DD_HANDLE usb_plug_hdl;
    UINT32 status;
    USB_PLUG_INOUT_ST user_plug;

    user_plug.handler = usb_plug_func_handler;
    user_plug.usr_data = 0;

    usb_plug_hdl = ddev_open(USB_PLUG_DEV_NAME, &status, (UINT32)&user_plug);
    if(DD_HANDLE_UNVALID == usb_plug_hdl)
    {
        return;
    }
    charge_func_init = 1;
}

void usb_charger_calibration(UINT32 type)
{
    CHARGE_OPER_ST chrg;
    chrg.oper = 1;

    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_USB_CHARGE_CAL, &chrg);
    chrg.oper = 2;
    //sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_USB_CHARGE_CAL, &chrg);
    chrg.oper = 3;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_USB_CHARGE_CAL, &chrg);
    chrg.oper = 4;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_USB_CHARGE_CAL, &chrg);
    if(chrg.cal[0] && chrg.cal[1] && chrg.cal[2])
    {
        os_printf("save charge cal %x %x %x\r\n", chrg.cal[0], chrg.cal[1], chrg.cal[2]);
        save_info_item(CHARGE_CONFIG_ITEM, chrg.cal, NULL, NULL);
    }
}
#endif


