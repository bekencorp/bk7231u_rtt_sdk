#include <rtthread.h>
#include <rthw.h>

#ifdef BEKEN_USING_SPI_FLASH

#include "spi_flash.h"
#include "spi_flash_sfud.h"
#include "drv_spi.h"

int rt_hw_spi_flash_with_sfud_init(void)
{
    if (RT_NULL == rt_sfud_flash_probe("spi_flash", "spi01"))
    {
        rt_kprintf("[spi_flash]:The spi flash probe failed !!!\n");
        return RT_ERROR;
    };

	return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_hw_spi_flash_with_sfud_init);
#endif