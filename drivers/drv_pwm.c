#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "typedef.h"
#include "drv_pwm.h"
#include "pwm.h"


#ifdef BEKEN_USING_PWM

struct _pwm_dev
{
    struct rt_device parent;
};

static struct _pwm_dev pwm_dev;

static rt_err_t rt_pwm_control(rt_device_t dev, int cmd, void *args)
{
    return pwm_ctrl(cmd, args);
}

int rt_hw_pwm_init(void)
{
    struct rt_device *device;

    rt_memset(&pwm_dev, 0, sizeof(pwm_dev));
    device = &(pwm_dev.parent);
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
    device->control     = rt_pwm_control;
    /* Setting the user data */
    device->user_data   = RT_NULL;

    /* register device */
    return rt_device_register(device, PWM_DEV_NAME, RT_DEVICE_FLAG_DEACTIVATE);
}
INIT_DEVICE_EXPORT(rt_hw_pwm_init);

int rt_hw_pwm_exit(void)
{
    /* unregister device */
    rt_device_unregister(&(pwm_dev.parent));
}

#endif
