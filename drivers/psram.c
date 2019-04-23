/**
 **************************************************************************************
 * @file    psram.c
 * @brief   Driver API PSRAM
 *
 * @author  Aixing.Li
 * @version V1.0.0
 *
 * &copy; 2017 BEKEN Corporation Ltd. All rights reserved.
 **************************************************************************************
 */

#include "psram.h"
#include <rtthread.h>
#include <rthw.h>

#define SPI_REG_BASE    		(0x00802700)
#define SPI_CTRL_REG   			(*(volatile uint32_t*)(SPI_REG_BASE + 0))
#define SPI_STAT_REG   			(*(volatile uint32_t*)(SPI_REG_BASE + 4))
#define SPI_DATA_REG   			(*(volatile uint32_t*)(SPI_REG_BASE + 8))
#define spi_reg        			((volatile SpiRegContext*)(SPI_REG_BASE))

#define SPI_TX_EMPTY_MASK		(0x1)
#define SPI_TX_FULL_MASK		(0x2)
#define SPI_RX_EMPTY_MASK		(0x4)
#define SPI_RX_FULL_MASK		(0x8)
#define SPI_BUSY_MASK			(1 << 15)

#define SPI_PSRAM_CMD_NREAD     (0x03)
#define SPI_PSRAM_CMD_FREAD     (0x0B)
#define SPI_PSRAM_CMD_READ      SPI_PSRAM_CMD_NREAD
#define SPI_PSRAM_CMD_WRITE     (0x02)
#define SPI_PSRAM_MAX_FRAME		(32)
#define SPI_PSRAM_INT_ISOLATION	(1)

#define INT_DISABLE()           int level; \
								level = rt_hw_interrupt_disable();

#define INT_RESTORE()           rt_hw_interrupt_enable(level);
                                
// extern void __enable_irq(void);
// extern void __enable_fiq(void);
// extern int  __disable_irq(void);
// extern int  __disable_fiq(void);

static void spi_cs(uint32_t cs)
{
	uint32_t reg = SPI_CTRL_REG;

	reg &= ~(3 << 16);

	SPI_CTRL_REG = reg | ((2 + (!cs)) << 16);
}

static uint32_t spi_is_busy(void)
{
	return (SPI_STAT_REG & SPI_BUSY_MASK);
}

int32_t spi_psram_init(void)
{
	SPI_CTRL_REG = 0x00C30100;

	return 0;
}

int32_t spi_psram_burst_set(uint32_t burst_size)
{
    uint8_t cmd = 0xC0;

	#if SPI_PSRAM_INT_ISOLATION
	INT_DISABLE();
	#endif

    spi_cs(1);

    SPI_DATA_REG = cmd;

    while(!(SPI_STAT_REG & SPI_TX_EMPTY_MASK));
	while(spi_is_busy());
    while(!(SPI_STAT_REG & SPI_RX_EMPTY_MASK)) cmd = SPI_DATA_REG;

    spi_cs(0);

	#if SPI_PSRAM_INT_ISOLATION
    INT_RESTORE();
    #endif

    return 0;
}

int32_t spi_psram_read_id(uint8_t id[12])
{
	uint8_t  cmd = 0x9F;
	uint32_t i = 0;

	#if SPI_PSRAM_INT_ISOLATION
	INT_DISABLE();
    #endif

    spi_cs(1);

    SPI_DATA_REG = cmd;

	while(!(SPI_STAT_REG & SPI_TX_EMPTY_MASK));
	while(spi_is_busy());
    while(!(SPI_STAT_REG & SPI_RX_EMPTY_MASK)) cmd = SPI_DATA_REG;

	SPI_DATA_REG = 0x00;

	while(1)
	{
		if(!(SPI_STAT_REG & SPI_RX_EMPTY_MASK))
		{
			id[i++] = SPI_DATA_REG;
			if(i >= 11) break;
			SPI_DATA_REG = 0x00;
		}
	}

	while(!(SPI_STAT_REG & SPI_TX_EMPTY_MASK));
	while(spi_is_busy());
    while(!(SPI_STAT_REG & SPI_RX_EMPTY_MASK)) cmd = SPI_DATA_REG;

    spi_cs(0);

	#if SPI_PSRAM_INT_ISOLATION
    INT_RESTORE();
    #endif

    return 0;
}

uint32_t spi_psram_read(uint32_t addr, uint8_t* buffer, uint32_t size)
{
    uint32_t res = 0;
    uint8_t  cmd[4];

    #if SPI_PSRAM_INT_ISOLATION
	INT_DISABLE();
	#endif

    cmd[0] =  SPI_PSRAM_CMD_READ;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >>  8) & 0xFF;
    cmd[3] = (addr & 0xFF);

    spi_cs(1);

	SPI_DATA_REG = cmd[0];
	SPI_DATA_REG = cmd[1];
	SPI_DATA_REG = cmd[2];
	SPI_DATA_REG = cmd[3];

	while(!(SPI_STAT_REG & SPI_TX_EMPTY_MASK));
	while(spi_is_busy());
	while(!(SPI_STAT_REG & SPI_RX_EMPTY_MASK)) *buffer = SPI_DATA_REG;

	SPI_DATA_REG = 0xFF;

	while(1)
	{
		if(!(SPI_STAT_REG & SPI_RX_EMPTY_MASK))
		{
			*buffer++ = SPI_DATA_REG;
			if(++res >= size) break;
			SPI_DATA_REG = 0xFF;
		}
	}

	while(!(SPI_STAT_REG & SPI_TX_EMPTY_MASK));
	while(spi_is_busy());
	while(!(SPI_STAT_REG & SPI_RX_EMPTY_MASK)) size = SPI_DATA_REG;

    spi_cs(0);

	#if SPI_PSRAM_INT_ISOLATION
    INT_RESTORE();
    #endif

    return res;
}

uint32_t spi_psram_write(uint32_t addr, uint8_t* buffer, uint32_t size)
{
    uint32_t res = 0;
    uint8_t  cmd[4];

    #if SPI_PSRAM_INT_ISOLATION
	INT_DISABLE();
	#endif

    cmd[0] =  SPI_PSRAM_CMD_WRITE;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >>  8) & 0xFF;
    cmd[3] = (addr & 0xFF);

    spi_cs(1);

    SPI_DATA_REG = cmd[0];
	SPI_DATA_REG = cmd[1];
	SPI_DATA_REG = cmd[2];
	SPI_DATA_REG = cmd[3];

    while(res < size)
    {
        if(!(SPI_STAT_REG & SPI_TX_FULL_MASK))
        {
            SPI_DATA_REG = *buffer++;
            res++;
        }
    }

    while(!(SPI_STAT_REG & SPI_TX_EMPTY_MASK));
	while(spi_is_busy());
    while(!(SPI_STAT_REG & SPI_RX_EMPTY_MASK)) size = SPI_DATA_REG;

    spi_cs(0);

	#if SPI_PSRAM_INT_ISOLATION
    INT_RESTORE();
    #endif

    return res;
}

#if 1//TEST

#include <stdio.h>
#include <string.h>
#include "typedef.h"
#include "gpio_pub.h"

#define ICU_PERI_CLK_MUX    *((volatile uint32_t*)(0x00802000 + 0 * 4))
#define ICU_PERI_CLK_PWD    *((volatile uint32_t*)(0x00802000 + 2 * 4))
#define PWD_SPI_CLK         (1 <<  6)
#define os_printf           printf

#define BK3000_WDT_CONFIG   *((volatile unsigned long*)(0x00802900))

int32_t os_null_printf(const char *fmt, ...);

void spi_psram_test(void *parameter)
{
	uint8_t id[11];
	uint32_t val;

	memset(id, 0, sizeof(id));

    ICU_PERI_CLK_MUX  = 1;
    ICU_PERI_CLK_PWD &= ~PWD_SPI_CLK;

	val = GFUNC_MODE_SPI;
    gpio_ctrl(CMD_GPIO_ENABLE_SECOND, &val);

	spi_psram_init();
	spi_psram_read_id(id);

	BK3000_WDT_CONFIG = 0x5A0000;
    BK3000_WDT_CONFIG = 0xA50000;

	if(1)
	{
		int i;
		os_printf("\n[PSRAM]: ID = ");
		for(i = 3; i < sizeof(id); i++)
		{
			os_printf("%02X ", id[i]);
		}
		os_printf("\n");
	}

	if(0)
	{
		uint8_t buffer[1024];
		int i;
		os_printf("[PSRAM]: SPRAM test begin\n");
		for(i = 0; i < sizeof(buffer); i++)
		{
			buffer[i] = (uint8_t)i;
		}
		spi_psram_write(0, buffer, sizeof(buffer));
		memset(buffer, 0, sizeof(buffer));
		spi_psram_read(0, buffer, sizeof(buffer));
		for(i = 0; i < sizeof(buffer); i++)
		{
			if(buffer[i] != (uint8_t)i)
			{
				os_printf("[%02d]: %02x - %02x\n", i, (uint8_t)i, buffer[i]);
			}	
		}
		os_printf("[PSRAM]: SPRAM test end\n");
		// while(1);
	}

	if(1)
	{
		#define  BUFFER_SIZE	(120)
		#define  PSRAM_SIZE		(8 * 1024 * 1024)
		uint32_t i, res;
		uint32_t offset = 0;
		uint8_t  buffer1[BUFFER_SIZE];
		uint8_t  buffer2[BUFFER_SIZE];
		os_printf("[PSRAM]: PSRAM R/W test begin\n");
		for(i = 0; i < BUFFER_SIZE; i++) buffer1[i] = i;
		i      = 0;
		res    = 0;
		offset = i * BUFFER_SIZE;
		while(offset < PSRAM_SIZE)
		{
			spi_psram_write(offset, buffer1, BUFFER_SIZE);
			memset(buffer2, 0, BUFFER_SIZE);
			spi_psram_read(offset, buffer2, BUFFER_SIZE);
			if(memcmp(buffer1, buffer2, BUFFER_SIZE) != 0)
			{
				os_printf("\r[PSRAM]: NG @ 0x%08X\n", offset);
				{int j; for(j = 0; j < BUFFER_SIZE; j++) if(buffer1[j] != buffer2[j]) os_printf("               @ %02x - %02x, %d\n", buffer1[j], buffer2[j], j);}
				res++;
			}
			else
			{
				os_printf("\r[PSRAM]: OK @ 0x%08X", offset);
			}
			offset += BUFFER_SIZE;
			i++;
		}
		os_printf("\r[PSRAM]: PSRAM R/W test end\n");
		os_printf("[PSRAM]: PSRAM R/W test %s\n", res ? "NG" : "OK");
	}
}

int spram_samples(int argc, char *argv)
{

    rt_thread_t tid;

    tid = rt_thread_create("spram",
                           spi_psram_test,
                           RT_NULL,
                           1024 * 8,
                           22,
                           10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}

FINSH_FUNCTION_EXPORT_ALIAS(spram_samples, __cmd_spram_samples, spram sample);


#endif
