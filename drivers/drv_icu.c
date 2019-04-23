#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "typedef.h"
#include "drv_icu.h"
#include "icu.h"

struct _icu_dev
{
    struct rt_device parent;
};

static struct _icu_dev icu_dev;

static rt_err_t rt_icu_control(rt_device_t dev, int cmd, void *args)
{
    return icu_ctrl(cmd, args);
}

int rt_hw_icu_init(void)
{
    struct rt_device *device;

    rt_memset(&icu_dev, 0, sizeof(icu_dev));
    device = &(icu_dev.parent);
    /* Setting the drive type */
    device->type        = RT_Device_Class_Miscellaneous;
    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;
    /* Setting the driver interface */
    device->init        = RT_NULL;
    device->open        = RT_NULL;
    device->close       = RT_NULL;
    device->read        = RT_NULL;
    device->write       = RT_NULL;
    device->control     = rt_icu_control;
    /* Setting the user data */
    device->user_data   = RT_NULL;

    /* register device */
    return rt_device_register(device, ICU_DEV_NAME, RT_DEVICE_FLAG_DEACTIVATE);
}
INIT_DEVICE_EXPORT(rt_hw_icu_init);

int rt_hw_icu_exit(void)
{
    /* unregister device */
    rt_device_unregister(&(icu_dev.parent));
}


