#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <stdio.h>
#include <string.h>

#include "drv_spi.h"

#ifdef BEKEN_USING_SPI

#define DBG_ENABLE
#define DBG_SECTION_NAME  "[SPI]:"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

#define SPI_REG_BASE            (0x00802700)
#define SPI_CTRL_REG            (*(volatile uint32_t*)(SPI_REG_BASE + 0))
#define SPI_STAT_REG            (*(volatile uint32_t*)(SPI_REG_BASE + 4))
#define SPI_DATA_REG            (*(volatile uint32_t*)(SPI_REG_BASE + 8))
#define spi_reg                 ((volatile SpiRegContext*)(SPI_REG_BASE))

#define SPI_TX_EMPTY_MASK       (0x1)
#define SPI_TX_FULL_MASK        (0x2)
#define SPI_RX_EMPTY_MASK       (0x4)
#define SPI_RX_FULL_MASK        (0x8)
#define SPI_BUSY_MASK           (1 << 15)

#define SPI_PSRAM_CMD_NREAD     (0x03)
#define SPI_PSRAM_CMD_FREAD     (0x0B)
#define SPI_PSRAM_CMD_READ      SPI_PSRAM_CMD_NREAD
#define SPI_PSRAM_CMD_WRITE     (0x02)
#define SPI_PSRAM_MAX_FRAME     (32)
#define SPI_PSRAM_INT_ISOLATION (0)

#define ICU_PERI_CLK_MUX    *((volatile uint32_t*)(0x00802000 + 0 * 4))
#define ICU_PERI_CLK_PWD    *((volatile uint32_t*)(0x00802000 + 2 * 4))
#define PWD_SPI_CLK         (1 <<  6)

static void bk_spi_init(void)
{
    uint32_t val;

    ICU_PERI_CLK_MUX  = 1;
    val = GFUNC_MODE_SPI;
    gpio_ctrl(CMD_GPIO_ENABLE_SECOND, &val);
    SPI_CTRL_REG = 0x00C30100;
}

static void bk_spi_cs(uint32_t cs)
{
    uint32_t reg = SPI_CTRL_REG;

    reg &= ~(3 << 16);

    SPI_CTRL_REG = reg | ((2 + (!cs)) << 16);
}

#define SPI_PERI_CLK        (26 * 1000 * 1000)
static int bk_spi_ckr_set(uint32_t max_hz)
{
    int div = 0;

    /* hz = spi_clk / 2 /  (div + 1))  ==> div = spi_clk / (2 * hz) - 1 */
    if (max_hz >= SPI_PERI_CLK / 2)
    {
        div = 0;
    }
    else
    {
        div = SPI_PERI_CLK / (2 * max_hz) - 1;
    }

    if (div >= 255)
    {
        div = 255;
    }
    spi_ctrl(CMD_SPI_SET_CKR, (void *)&div);
    dbg_log(DBG_LOG, "div = %d \n", div);
    dbg_log(DBG_LOG, "target frequency = %d, actual frequency = %d \n", max_hz, SPI_PERI_CLK / 2 / (div + 1));
}

static uint32_t bk_spi_is_busy(void)
{
    return (SPI_STAT_REG & SPI_BUSY_MASK);
}

/* RT-Thread SPI_Bus Device */

struct bk_spi_dev
{
    struct rt_spi_bus *spi_bus;
};

static struct bk_spi_dev *spi_dev;

extern UINT32 spi_ctrl(UINT32 cmd, void *param);
rt_err_t _spi_configure(struct rt_spi_device *dev, struct rt_spi_configuration *cfg)
{
    int result = RT_EOK;
    uint32_t param;

    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    dbg_log(DBG_LOG, "data_width = %d \n", cfg->data_width);

    /* data_width */
    if (cfg->data_width == 8)
    {
        param = 0;
        spi_ctrl(CMD_SPI_SET_BITWIDTH, (void *)&param);
    }
    else if (cfg->data_width == 16)
    {
        param = 1;
        spi_ctrl(CMD_SPI_SET_BITWIDTH, (void *)&param);
    }
    else
    {
        return -RT_EIO;
    }

    /* baudrate */
    dbg_log(DBG_LOG, "max_hz = %d \n", cfg->max_hz);
    bk_spi_ckr_set(cfg->max_hz);

    dbg_log(DBG_LOG, "mode = 0x%08x \n", cfg->mode);

    /* CPOL */
    if (cfg->mode & RT_SPI_CPOL)
    {
        param = 1;
        spi_ctrl(CMD_SPI_SET_CKPOL, (void *)&param);
    }
    else
    {
        param = 0;
        spi_ctrl(CMD_SPI_SET_CKPOL, (void *)&param);
    }

    /* CPHA */
    if (cfg->mode & RT_SPI_CPHA)
    {
        param = 1;
        spi_ctrl(CMD_SPI_SET_CKPHA, (void *)&param);
    }
    else
    {
        param = 0;
        spi_ctrl(CMD_SPI_SET_CKPHA, (void *)&param);
    }

    /* Master or Slave */
    if (cfg->mode & RT_SPI_SLAVE)
    {
        param = 0;
        spi_ctrl(CMD_SPI_SET_MSTEN, (void *)&param);
    }
    else
    {
        param = 1;
        spi_ctrl(CMD_SPI_SET_MSTEN, (void *)&param);
    }
    dbg_log(DBG_INFO, "[CTRL]:0x%08x \n", SPI_CTRL_REG);

    return RT_EOK;
}

rt_uint32_t _spi_xfer(struct rt_spi_device *dev, struct rt_spi_message *msg)
{
    rt_uint32_t size;
    struct rt_spi_bus *spi_bus = RT_NULL;
    struct rt_spi_configuration *cfg = RT_NULL;

    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(msg != RT_NULL);

    size = msg->length;
    spi_bus = dev->bus;
    cfg = &dev->config;

    /* take CS */
    if (msg->cs_take)
    {
        bk_spi_cs(1);
    }

    if (cfg->data_width == 8)
    {
        rt_uint8_t data;
        rt_uint8_t *recv_ptr = RT_NULL;
        const rt_uint8_t *send_ptr = RT_NULL;

        data = 0xFF;
        recv_ptr = msg->recv_buf;
        send_ptr = msg->send_buf;

        while (size --)
        {
            while ((SPI_STAT_REG & SPI_TX_FULL_MASK));
            if (send_ptr)
            {
                SPI_DATA_REG = *send_ptr++;
            }
            else
            {
                SPI_DATA_REG = 0xFF;
            }

            if (recv_ptr)
            {
                while ((SPI_STAT_REG & SPI_RX_EMPTY_MASK));
                *recv_ptr++ = SPI_DATA_REG;
            }
            else
            {
                while ((SPI_STAT_REG & SPI_RX_EMPTY_MASK));
                data = SPI_DATA_REG;
            }
        }
    }

    /* release CS */
    if (msg->cs_release)
    {
        bk_spi_cs(0);
    }

    return msg->length;
}

static struct rt_spi_ops spi_ops =
{
    .configure = _spi_configure,
    .xfer = _spi_xfer
};

int rt_hw_spi_bus_register(char *name)
{
    int result = RT_EOK;
    struct rt_spi_bus *spi_bus = RT_NULL;

    if (spi_dev)
    {
        return RT_EOK;
    }

    /* Initialize the SPI peripherals */
    bk_spi_init();

    spi_dev = rt_malloc(sizeof(struct bk_spi_dev));
    if (!spi_dev)
    {
        rt_kprintf("[spi]:malloc memory for spi_dev failed\n");
        result = -RT_ENOMEM;
        goto _exit;
    }
    memset(spi_dev, 0, sizeof(struct bk_spi_dev));

    spi_bus = rt_malloc(sizeof(struct rt_spi_bus));
    if (!spi_bus)
    {
        rt_kprintf("[spi]:malloc memory for spi_bus failed\n");
        result = -RT_ENOMEM;
        goto _exit;
    }
    memset(spi_bus, 0, sizeof(struct rt_spi_bus));

    spi_bus->parent.user_data = spi_dev;
    rt_spi_bus_register(spi_bus, name, &spi_ops);

    return result;

_exit:
    if (spi_dev)
    {
        rt_free(spi_dev);
        spi_dev = RT_NULL;
    }

    if (spi_bus)
    {
        rt_free(spi_bus);
        spi_bus = RT_NULL;
    }

    return result;
}

static struct rt_spi_device *spi_device = RT_NULL;
int rt_hw_spi_device_init(void)
{
    int result = RT_EOK;

    if (spi_device)
    {
        return RT_EOK;
    }
    spi_device = rt_malloc(sizeof(struct rt_spi_device));
    if (!spi_device)
    {
        rt_kprintf("[spi]:malloc memory for spi_device failed\n");
        result = -RT_ENOMEM;
    }
    memset(spi_device, 0, sizeof(struct rt_spi_device));

    /* register spi bus */
    result = rt_hw_spi_bus_register("spi0");
    if (result != RT_EOK)
    {
        rt_kprintf("[spi]:register spi bus error : %d !!!\n", result);
        goto _exit;
    }

    /* attach cs */
    result = rt_spi_bus_attach_device(spi_device, "spi01", "spi0", NULL);
    if (result != RT_EOK)
    {
        rt_kprintf("[spi]:attach spi bus error : %d !!!\n", result);
        goto _exit;
    }
    return RT_EOK;

_exit:
    if (spi_device)
    {
        rt_free(spi_device);
        spi_device = RT_NULL;
    }

    return result;
}
INIT_PREV_EXPORT(rt_hw_spi_device_init);

#endif
