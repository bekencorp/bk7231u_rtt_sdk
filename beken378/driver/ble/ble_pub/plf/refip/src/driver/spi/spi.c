/**
 ****************************************************************************************
 *
 * @file spi.c
 *
 * @brief spi driver
 *
 * Copyright (C) BeKen 2009-2017
 *
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stddef.h>     // standard definition
#include <stdarg.h>
#include <stdio.h>
#include "spi.h"       // uart definition
#include "BK3435_reg.h"
#include "reg_intc.h"
#include "intc.h"
#include "rwip.h"       // SW interface
#include "uart.h"




/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */
#define GPIO_SPI_CFG            		REG_APB5_GPIOA_CFG
#define GPIO_SPI_SCK_PIN          	4
#define GPIO_SPI_MOSI_PIN        	5
#define GPIO_SPI_MISO_PIN        	6
#define GPIO_SPI_NSS_PIN           	7



#define SPI_CLK_DIVID_SET     (NUMBER_ROUND_UP(SPI_DEFAULT_CLK/2, SPI_DEFAULT_BAUD) - 1)



///SPI Character format
enum SPI_CHARFORMAT
{
 	SPI_CHARFORMAT_8BIT = 0x00UL,
    	SPI_CHARFORMAT_16BIT = 0x01UL
};

enum CKPHA
{
	 CKPHA_CLK1 = 0x00UL,
	 CKPHA_CLK2 = 0x01UL
};

enum CKPOL
{
	 CKPOL_L = 0x00UL,
	 CKPOL_H = 0x01UL
};

enum NSSMD
{
	LINE3_NSS_NULL = 0,
	LINE4_NSS_CHIP = 1,
	LINE4_NSS_M = 2,
	LINE4_NSS_S_E =  3,
};
	


void spi_init(uint32_t baud)
{
	unsigned long freq_div;
	
	if (baud == 0)
	{
		freq_div = SPI_CLK_DIVID_SET;
	}
	else
	{
		freq_div = NUMBER_ROUND_UP(SPI_DEFAULT_CLK/2, baud) - 1;
	}

	 // SPI clock enable
	 REG_AHB0_ICU_SPICLKCON = 0x00;     

	 // Close JTAG to release GPIO to normal function
     	CLOSE_JTAG_MODE();

	 // Enable GPIO P0.4, P0.5, P0.6, P0.7 peripheral function for spi
     	GPIO_SPI_CFG &= ~((0x1<<GPIO_SPI_SCK_PIN)  | (0x1<<GPIO_SPI_MOSI_PIN)
                    | (0x1<<GPIO_SPI_MISO_PIN) | (0x1<<GPIO_SPI_NSS_PIN ));
	

	REG_APB2_SPI_CTRL = (0x01UL << BIT_SPI_CTRL_SPIEN)
					|( 0x1 << BIT_SPI_CTRL_MSTEN)
					| (CKPHA_CLK1 << BIT_SPI_CTRL_CKPHA)
					| (CKPOL_L << BIT_SPI_CTRL_CKPOL)
					| (SPI_CHARFORMAT_8BIT << BIT_SPI_CTRL_BIT_WIDTH)
					| (0x03UL << BIT_SPI_CTRL_NSSMD)
					| (freq_div << BIT_SPI_CTRL_SPI_CKR) 
					| (8 << BIT_SPI_CTRL_SPI_CKR)
					| (0x00UL << BIT_SPI_CTRL_RXINT_EN)
					| (0x00UL << BIT_SPI_CTRL_TXINT_EN)
					| (0x00UL << BIT_SPI_CTRL_RXOVR_EN)
					| (0x00UL << BIT_SPI_CTRL_TXOVR_EN)
					| (0x00UL << BIT_SPI_CTRL_RXINT_MODE)
					| (0x00UL << BIT_SPI_CTRL_TXINT_MODE);
}	


static void spi_tx_start(void)
{
	uint32_t reg ;
	reg = REG_APB2_SPI_CTRL;
	
	reg |= (0x03ul << BIT_SPI_CTRL_NSSMD) ;
	REG_APB2_SPI_CTRL = reg; 	
	
	reg &= ~(0x03ul << BIT_SPI_CTRL_NSSMD) ;
  	reg |= (0x02ul << BIT_SPI_CTRL_NSSMD) ;
	
  	REG_APB2_SPI_CTRL = reg; 
}

static void spi_tx_end(void)
{
	uint32_t reg ;
	uint32_t state;
	reg  = REG_APB2_SPI_CTRL;

	reg |=(0x01UL << BIT_SPI_CTRL_TXINT_EN);	
	
	REG_APB2_SPI_CTRL = reg;

	state = REG_APB2_SPI_STAT;
	
	while(!(state & (0x01 << BIT_SPI_STAT_TXINT)))
	{
		state = REG_APB2_SPI_STAT;
	}		
	reg &=~(0x01UL << BIT_SPI_CTRL_TXINT_EN);	
	
  	REG_APB2_SPI_CTRL = reg;
	
	REG_APB2_SPI_STAT = state;

	reg |= (0x03UL << BIT_SPI_CTRL_NSSMD);	
	
	REG_APB2_SPI_CTRL = reg;
}


static void spi_write_byte(uint8_t data)
{
	while(spi_is_busy());
	
	while(SPI_TX_FIFO_FULL);	
	
	REG_APB2_SPI_DAT = data;
}

static uint8_t spi_read_byte(void)
{
	while(spi_is_busy());
	
	while(SPI_RX_FIFO_EMPTY);	
	
	return (REG_APB2_SPI_DAT & 0xff);
}


void spi_write_read(uint8_t *wbuf, uint32_t w_size, uint8_t *rbuf, uint32_t r_size,uint32_t r_off)
{
	uint32_t max_len;
	
	max_len = (w_size > (r_size + r_off)) ? w_size : (r_size + r_off);

	spi_tx_start();

	while(max_len--)
	{
		if(w_size > 0)
		{
			spi_write_byte(*wbuf);
			wbuf++;
			w_size--;
		}
		else
		{
			spi_write_byte(0x00);
		}
		if(r_off > 0)
		{	
			spi_read_byte();
			r_off--;
		}
		else if(r_size > 0)
		{
			*rbuf = spi_read_byte();
			rbuf++;
			r_size--;
		}else
		{		
			spi_read_byte();
		}
	}
	
	spi_tx_end();
}


uint8_t spi_is_busy(void)
{
  	return REG_APB2_SPI_STAT & (0x01<<BIT_SPI_STAT_SPI_BUSY);
}


/// @} SPI DRIVER
