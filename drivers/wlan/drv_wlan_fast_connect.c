#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>
#include <wlan_dev.h>
#include <wlan_mgnt.h>
#include "drv_wlan.h"
#include "drv_flash.h"
#include "drv_wlan_fast_connect.h"
#include "stdio.h"

extern uint16_t inet_chksum(void *dataptr, uint16_t len);

void flash_data_info_dump(wlan_fast_connect_t *data_info)
{
    char tmp[68];
    int i = 0;

    if (data_info == RT_NULL)
    {
        return;
    }

    memcpy(tmp, data_info->ssid, 32);
    tmp[32] = '\0';
    rt_kprintf("ssid:%s \n", tmp);

    for (i = 0; i < 6; i ++)
    {
        sprintf(&tmp[i * 3], "%02X:",  data_info->bssid[i]);
    }
    tmp[6 * 3 - 1] = '\0';

    rt_kprintf("bssid:%s \n", tmp);
    rt_kprintf("channel:%d \n", data_info->channel);
    rt_kprintf("security:%d \n", data_info->security);
    {
        for (i = 0; i < 32; i ++)
        {
            sprintf(&tmp[i * 2], "%02x",  data_info->psk[i]);
        }
        tmp[64] = '\0';
        rt_kprintf("psk = %s \n", tmp);
    }
}

int fast_connect_dump(void)
{
    struct wlan_fast_connect data_info;

    memset(&data_info, 0, sizeof(struct wlan_fast_connect));
    wlan_fast_connect_info_read(&data_info);
    flash_data_info_dump(&data_info);
}
MSH_CMD_EXPORT(fast_connect_dump, fast_connect_dump);

int wlan_fast_connect_info_write(wlan_fast_connect_t *data_info)
{
    uint32_t crc1, crc2, len;
    uint32_t data[(sizeof(struct wlan_fast_connect) + 4 - 1) / 4 + 1];
    int result = RT_EOK;

    if (data_info == RT_NULL)
    {
        return -RT_EINVAL;
    }

    memset(data, 0, sizeof(data));
    len = sizeof(struct wlan_fast_connect);
    beken_flash_read(FLASH_FAST_DATA_ADDR, data, sizeof(data));

    crc1 = inet_chksum(data_info, sizeof(struct wlan_fast_connect));
    crc2 = data[(sizeof(struct wlan_fast_connect) + 4 - 1 ) / 4];
    //wirte it to flash if different content: SSID, Passphrase, Channel, Security type
    if ((memcmp(data, (uint8_t *) data_info, sizeof(struct wlan_fast_connect)) != 0) || (crc1 != crc2))
    {
        rt_kprintf("write new profile to flash 0x%08X %d byte!\n", FLASH_FAST_DATA_ADDR, len);

        memcpy(data, data_info, sizeof(struct wlan_fast_connect));
        data[(sizeof(struct wlan_fast_connect) + 4 - 1 ) / 4] = crc1;

        beken_flash_erase(FLASH_FAST_DATA_ADDR);
        beken_flash_write(FLASH_FAST_DATA_ADDR, (uint8_t *)data, len + 4);
    }
    else
    {
        rt_kprintf("wlan_fast_connect data not modfiy, skip write to flash!\n");
    }

    flash_data_info_dump(data_info);

    return RT_EOK;
}

int wlan_fast_connect_info_read(wlan_fast_connect_t *data_info)
{
    uint32_t crc1, crc2;
    uint32_t data[(sizeof(struct wlan_fast_connect) + 4 - 1) / 4 + 1];
    int result = RT_EOK;

    if (data_info == RT_NULL)
    {
        return -RT_EINVAL;
    }

    memset(data, 0, sizeof(struct wlan_fast_connect) + 4);
    beken_flash_read(FLASH_FAST_DATA_ADDR, data, sizeof(data));
    crc1 = data[(sizeof(struct wlan_fast_connect) + 4 - 1 ) / 4];
    crc2 = inet_chksum(data, sizeof(struct wlan_fast_connect));

    if ((data[0] != ~0x0)  && (crc1 == crc2)) // 0xFFFFFFFF
    {
        memcpy(data_info, data, sizeof(struct wlan_fast_connect));
        flash_data_info_dump(data_info);
        rt_kprintf("crc1:%X crc2:%X\n", crc1, crc2);
        result = RT_EOK;
    }
    else
    {
        rt_kprintf("fast_connect ap info crc failed, crc1:%X crc2:%X \n", crc1, crc2);
        result = -RT_ERROR;
    }

	// check
	if( (data_info->channel < 1) || (data_info->channel > 13) )
	{
		rt_kprintf("[%s] channel out of range! \n", __FUNCTION__, (int)data_info->channel);
		result = -RT_EINVAL;
	}

    return result;
}

int wlan_fast_connect_info_erase(void)
{
    beken_flash_erase(FLASH_FAST_DATA_ADDR);
}
