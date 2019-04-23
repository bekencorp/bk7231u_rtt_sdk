
/**
****************************************************************************************
*
* @file ir.c
*
* @brief icu initialization and specific functions
*
* Copyright (C) Beken 2009-2016
*
* $Rev: $
*
****************************************************************************************
*/

/**
****************************************************************************************
* @addtogroup IR
* @ingroup IR
* @brief IR
*
* This is the driver block for ir
* @{
****************************************************************************************
*/

#include <stdint.h>        // standard integer definition
#include <string.h>        // string manipulation
#include <stddef.h>     // standard definition
#include "BK3435_reg.h"
#include "gpio.h"
#include "uart.h"
#include "ir.h"
#include "lld_evt.h"



#define	IR_SEND_LOW			REG_APB9_IR_CFG &= ~(0x01 << IR_DATA_OUT_BIT);
#define	IR_SEND_HIGH			REG_APB9_IR_CFG |= (0x01 << IR_DATA_OUT_BIT);

#define	IR_ZAIBO_AVER4

uint32_t	ir_zaibo, ir_rec_overtimer, ir_return_count;
uint32_t	ir_rec_buf[200];

extern		volatile uint32_t XVR_ANALOG_REG_BAK[16];


static	uint32_t	timer_high , timer_low;
static	uint32_t	timer_high_over_start , timer_low_over_start;
//static	uint32_t	timer_high_decode_start , timer_low_decode_start;

static	uint32_t	timer_high_phase , timer_low_phase;
static	uint8_t		f_start, f_state;

static		void		ir_read_timer(void);
static		void		ir_delay_ms(int num);
static		void	 	ir_delay_us(int num);
static		void 	ir_delay_1us(void);
static	uint8_t		zaibo_check(uint8_t	us_overtimer , uint32_t *start_timer_high, uint32_t *start_timer_low, uint32_t *zaibo);
static	uint32_t	cal_timer_us(uint32_t timer1_high, uint32_t timer1_low, uint32_t timer2_high, uint32_t timer2_low);
static	uint8_t		state_check(uint8_t	*state , uint32_t *start_timer_high, uint32_t *start_timer_low, uint32_t *phase_timer, uint32_t overtimer);
static	uint8_t		nozaibo_state_check(uint8_t	*state , uint32_t *start_timer_high, uint32_t *start_timer_low, uint32_t *phase_timer, uint32_t overtimer ,uint32_t cash);
static	uint8_t		ir_read_io(void);
//static	void			ir_delay(void);
static	void 		ir_send_data(uint8_t x);
//static	void	ir_send_test(uint32_t		zaibo, uint32_t		modulation_timer, uint32_t  no_modulation_timer);
static	void 		ir_sensitivity_reg6_set(uint8_t sen);


void ir_init(void)
{
	//UART_PRINTF("ir_init \r\n");
	gpio_config(GPIOA_2,FLOAT,PULL_NONE);

	//UART_PRINTF("REG_APB9_IR_CFG = 0x%08x \r\n",REG_APB9_IR_CFG);
	REG_APB9_IR_CFG |= ~(0x01 << 1);
	REG_APB9_IR_CFG &= ~(0x01 << 0);
	REG_APB9_IR_CFG =0x00;
	#if (IR_RECEIVE)
	//REG_APB9_IR_CFG |=(0x01 << 1);
	#endif


	
	//UART_PRINTF("REG_APB9_IR_CFG = 0x%08x \r\n",REG_APB9_IR_CFG);
}

void ir_sensitivity_regb_set(uint8_t sen)
{
	uint32_t	temp;

	temp =XVR_ANALOG_REG_BAK[0x0b];
	temp  &= ~(0x03 << IR_SEN_BIT);
	temp  |= ((uint32_t)sen << IR_SEN_BIT);
	XVR_REG0B =	temp;

	if(sen ==0)			ir_sensitivity_reg6_set(0);
	if(sen ==1)			ir_sensitivity_reg6_set(1);
	if(sen ==2)			ir_sensitivity_reg6_set(2);
	if(sen ==3)			ir_sensitivity_reg6_set(3);
}
void ir_sensitivity_reg6_set(uint8_t sen)
{
	uint32_t	temp;

	temp =XVR_ANALOG_REG_BAK[0x06];
	temp  &= ~(0x07 << 8);
	temp  |= ((uint32_t)sen << 8);
	XVR_REG06 =	temp;
	
}

void ir_send_power(uint8_t power)
{
	uint32_t	temp;
	
	if(power >=2)		power =2;

	temp =XVR_ANALOG_REG_BAK[0x0b];
	temp  &= ~(0x03 << 15);
	temp  |= ((uint32_t)power << 15);
	XVR_REG0B =	temp;
	
}



void ir_demo_test(void)
{
	uint8_t 	temp;

//	UART_PRINTF("ir_init \r\n");
	//ir_init();
	ir_power(1);
	ir_mode(1);
	ir_send_power(3);
	ir_sensitivity_regb_set(0);

	for(;;)
	{


		#if 0
			ir_send_test(0,0,0);
		#else
			temp =ir_receive(5000, &ir_zaibo , ir_rec_buf , 100, &ir_return_count);
			if(temp ==IR_REC_ERR)
			{
				//UART_PRINTF("IR_REC_ERR\r\n");

			}
			else if(temp ==IR_OVERTIMER)
			{
				//UART_PRINTF("over timer\r\n");
				//UART_PRINTF("zaibo %08x \r\n",ir_zaibo);
				//UART_PRINTF("ir_return_count %08x \r\n",ir_return_count);
				
				for(temp =0 ; temp <ir_return_count ; temp++)
				{
					//UART_PRINTF("ir_rec_buf[%d] %08x \r\n", temp ,ir_rec_buf[temp]);
				}
			}
			else
			{
				//UART_PRINTF("IR_REC_ok\r\n");
				
				//UART_PRINTF("zaibo %08x \r\n",ir_zaibo);
				//UART_PRINTF("ir_return_count %08x \r\n",ir_return_count);
				

				#if 1
				for(temp =0 ; temp <ir_return_count ; temp++)
				{
					//UART_PRINTF("ir_rec_buf[%d] %08x \r\n", temp ,ir_rec_buf[temp]);
				}
				#endif

				
				#if 1
					ir_delay_ms(2000);
					//UART_PRINTF("IR_send_start\r\n");	
					ir_send(ir_zaibo , ir_rec_buf , ir_return_count);

					//UART_PRINTF("IR_send_ok\r\n");					
				#endif
				
			}
		#endif
	}		
	
}


void ir_send_data(uint8_t x)
{
	if(x == 1)
	{
		REG_APB9_IR_CFG |= (0x01 << IR_DATA_OUT_BIT);
	}else
	{
		REG_APB9_IR_CFG &= ~(0x01 << IR_DATA_OUT_BIT);
	}


}

void		ir_mode(uint8_t	mode)
{
	if(mode ==0)	
	{
		//receive
		REG_APB9_IR_CFG |= (0x01 << 1);

	}
	else
	{
		//send
		REG_APB9_IR_CFG &= ~(0x01 << IR_DATA_OUT_BIT);
		REG_APB9_IR_CFG &= ~(0x01 << 1);


	}
}

void		ir_power(uint8_t	 power)
{
	if(power ==0)
	{
		//power down
		REG_APB9_IR_CFG |= (0x01 << 0);
	}
	else
	{
		//power on
		REG_APB9_IR_CFG &= ~(0x01 << 0);		
	}

}


void		ir_modulation_send(uint32_t	zaibo, uint32_t  send_timer)
{
	uint32_t	timer_high_temp , timer_low_temp, middle_high , middle_low, end_high , end_low;
	uint32_t	half_zaibo ,sub1;

	

	//ir_mode(1);				//send mode
	IR_SEND_LOW;

	ir_read_timer( );
	timer_high_temp =timer_high;		timer_low_temp =timer_low;			//
	middle_high =timer_high;				middle_low =timer_low;			//
	
	half_zaibo =zaibo >>1;


	while(1)
	{
		#if 0
			ir_send_data(1);
			ir_delay_us(half_zaibo);
			ir_send_data(0);
			ir_delay_us(half_zaibo-2);
		#else
			ir_send_data(1);
			while(1)
			{
				ir_read_timer( );
				end_high =timer_high;		end_low =timer_low;			
				sub1 =cal_timer_us(middle_high, middle_low, end_high, end_low);
				if(sub1 >half_zaibo)
				{
					//middle_high =end_high; middle_low =end_low;
					break;
				}					
			}
			ir_send_data(0);
			while(1)
			{
				ir_read_timer( );
				end_high =timer_high;		end_low =timer_low;			//
				sub1 =cal_timer_us(middle_high, middle_low, end_high, end_low);
				//if(sub1 >half_zaibo-2)
				if(sub1 >zaibo-2)
				{
					middle_high =end_high; middle_low =end_low;				
					break;
				}					
			}
		#endif

		ir_read_timer( );
		end_high =timer_high;		end_low =timer_low;			//
		sub1 =cal_timer_us(timer_high_temp, timer_low_temp, end_high, end_low);
		if(sub1 >send_timer)
		{
			break;
		}	
	}		

	
}


void		ir_no_modulation_send(uint32_t  send_timer , uint32_t  send_level)
{
	uint32_t	timer_high_temp , timer_low_temp, end_high , end_low;
	uint32_t	sub1;



	//ir_mode(1);				//send mode
	ir_send_data(send_level);

	ir_read_timer( );
	timer_high_temp =timer_high;		timer_low_temp =timer_low;			//
	
	while(1)
	{
		ir_read_timer( );
		end_high =timer_high;		end_low =timer_low;			//
		sub1 =cal_timer_us(timer_high_temp, timer_low_temp, end_high, end_low);
		if(sub1 >send_timer)
		{		
			break;
		}	
	}		


}


/*
void		ir_send_test(uint32_t		zaibo, uint32_t		modulation_timer, uint32_t  no_modulation_timer)
{
	#if 0
		//ir_modulation_send(zaibo, modulation_timer);
		//ir_no_modulation_send(no_modulation_timer);

			ir_delay();
		
		
	#else
	//gpio_config(GPIOA_3,OUTPUT, PULL_LOW);
	while(1)
	{
		#if 0
			ir_rec_buf[0] =0x00002396; 
			ir_rec_buf[1] =0x0000118b; 
			ir_rec_buf[2] =0x00000270; 
			ir_rec_buf[3] =0x0000020f; 
			ir_rec_buf[4] =0x00000270; 
			ir_rec_buf[5] =0x00000210; 
			ir_rec_buf[6] =0x0000026f; 
			ir_rec_buf[7] =0x00000211; 
			ir_rec_buf[8] =0x0000026f; 
			ir_rec_buf[9] =0x0000020f; 
			ir_rec_buf[10] =0x00000270; 
			ir_rec_buf[11] =0x00000211; 
			ir_rec_buf[12] =0x00000271; 
			ir_rec_buf[13] =0x0000020e; 
			ir_rec_buf[14] =0x00000270; 
			ir_rec_buf[15] =0x00000680; 
			ir_rec_buf[16] =0x0000026f; 
			ir_rec_buf[17] =0x00000210; 
			ir_rec_buf[18] =0x0000026f; 
			ir_rec_buf[19] =0x00000680; 
			ir_rec_buf[20] =0x00000270; 
			ir_rec_buf[21] =0x00000680; 
			ir_rec_buf[22] =0x00000270; 
			ir_rec_buf[23] =0x0000067d; 
			ir_rec_buf[24] =0x00000270; 
			ir_rec_buf[25] =0x00000681; 
			ir_rec_buf[26] =0x0000026f; 
			ir_rec_buf[27] =0x0000067f; 
			ir_rec_buf[28] =0x00000270; 
			ir_rec_buf[29] =0x00000680; 
			ir_rec_buf[30] =0x0000026f; 
			ir_rec_buf[31] =0x0000020f; 
			ir_rec_buf[32] =0x00000270; 
			ir_rec_buf[33] =0x0000067f; 
			ir_rec_buf[34] =0x00000270; 
			ir_rec_buf[35] =0x00000210; 
			ir_rec_buf[36] =0x0000026f; 
			ir_rec_buf[37] =0x00000680; 
			ir_rec_buf[38] =0x0000026f; 
			ir_rec_buf[39] =0x00000211; 
			ir_rec_buf[40] =0x00000270; 
			ir_rec_buf[41] =0x00000210; 
			ir_rec_buf[42] =0x00000270; 
			ir_rec_buf[43] =0x0000067f; 
			ir_rec_buf[44] =0x00000270; 
			ir_rec_buf[45] =0x0000020e; 
			ir_rec_buf[46] =0x00000270; 
			ir_rec_buf[47] =0x0000020f; 
			ir_rec_buf[48] =0x00000273; 
			ir_rec_buf[49] =0x0000020f; 
			ir_rec_buf[50] =0x0000026d; 
			ir_rec_buf[51] =0x00000680; 
			ir_rec_buf[52] =0x00000270; 
			ir_rec_buf[53] =0x00000211; 
			ir_rec_buf[54] =0x00000270; 
			ir_rec_buf[55] =0x0000067f; 
			ir_rec_buf[56] =0x0000026f; 
			ir_rec_buf[57] =0x0000067f; 
			ir_rec_buf[58] =0x00000271; 
			ir_rec_buf[59] =0x0000020d; 
			ir_rec_buf[60] =0x00000271; 
			ir_rec_buf[61] =0x0000067e; 
			ir_rec_buf[62] =0x00000273; 
			ir_rec_buf[63] =0x0000067e; 
			ir_rec_buf[64] =0x00000270; 
			ir_rec_buf[65] =0x0000067f; 
			ir_rec_buf[66] =0x00000270; 
			ir_send(26, ir_rec_buf, 67);
		#else
			#if 1
			ir_rec_buf[0] =0x00000010; 
			ir_rec_buf[1] =0x000021ca; 
			ir_rec_buf[2] =0x00000010; 
			ir_rec_buf[3] =0x0000167d; 
			ir_rec_buf[4] =0x00000010; 
			ir_rec_buf[5] =0x0000167c; 
			ir_rec_buf[6] =0x00000010; 
			ir_rec_buf[7] =0x00001680; 
			ir_rec_buf[8] =0x00000010; 
			ir_rec_buf[9] =0x0000167d; 
			ir_rec_buf[10] =0x00000010; 
			ir_rec_buf[11] =0x0000167e; 
			ir_rec_buf[12] =0x00000010; 
			ir_rec_buf[13] =0x0000167f; 
			ir_rec_buf[14] =0x00000010; 
			ir_rec_buf[15] =0x0000167d; 
			ir_rec_buf[16] =0x00000010; 
			ir_rec_buf[17] =0x0000167c; 
			ir_rec_buf[18] =0x00000010; 
			ir_rec_buf[19] =0x0000167d; 
			ir_rec_buf[20] =0x00000010; 
			ir_rec_buf[21] =0x0000167d; 
			ir_rec_buf[22] =0x00000010; 
			ir_rec_buf[23] =0x000103c0; 
			ir_rec_buf[24] =0x00000010; 
			ir_rec_buf[25] =0x0000167d; 
			ir_rec_buf[26] =0x00000010; 
			ir_rec_buf[27] =0x0000167e; 
			ir_rec_buf[28] =0x00000010; 
			ir_rec_buf[29] =0x0000167e; 
			ir_rec_buf[30] =0x00000010; 
			ir_rec_buf[31] =0x0000167f; 
			ir_rec_buf[32] =0x00000010; 
			ir_rec_buf[33] =0x0000167e; 
			ir_rec_buf[34] =0x00000010; 
			ir_rec_buf[35] =0x0000167f; 
			ir_rec_buf[36] =0x00000010; 
			ir_rec_buf[37] =0x0000167d; 
			ir_rec_buf[38] =0x00000010; 
			ir_rec_buf[39] =0x0000167f; 
			ir_rec_buf[40] =0x00000010; 
			ir_rec_buf[41] =0x00001680; 
			ir_rec_buf[42] =0x00000010; 
			ir_rec_buf[43] =0x0000167f; 
			ir_rec_buf[44] =0x00000010; 
			ir_rec_buf[45] =0x0000167e; 
			ir_rec_buf[46] =0x00000010; 
			ir_rec_buf[47] =0x000103c1; 
			ir_rec_buf[48] =0x00000010; 
			ir_rec_buf[49] =0x0000167f; 
			ir_rec_buf[50] =0x00000010; 
			ir_rec_buf[51] =0x0000167d; 
			ir_rec_buf[52] =0x00000010; 
			ir_rec_buf[53] =0x0000167f; 
			ir_rec_buf[54] =0x00000010; 
			ir_rec_buf[55] =0x0000167e; 
			ir_rec_buf[56] =0x00000010; 
			ir_rec_buf[57] =0x0000167e; 
			ir_rec_buf[58] =0x00000010; 
			ir_rec_buf[59] =0x0000167f; 
			ir_rec_buf[60] =0x00000010; 
			ir_rec_buf[61] =0x0000167d; 
			ir_rec_buf[62] =0x00000010; 
			ir_rec_buf[63] =0x00001680; 
			ir_rec_buf[64] =0x00000010; 
			ir_rec_buf[65] =0x0000167f; 
			ir_rec_buf[66] =0x00000010; 
			#else

			ir_rec_buf[0] =0x00000016; 
			ir_rec_buf[1] =0x00001690; 
			ir_rec_buf[2] =0x00000015; 
			ir_rec_buf[3] =0x0000167d; 
			ir_rec_buf[4] =0x00000017; 
			ir_rec_buf[5] =0x0000167c; 
			ir_rec_buf[6] =0x00000015; 
			ir_rec_buf[7] =0x00001680; 
			ir_rec_buf[8] =0x00000015; 
			ir_rec_buf[9] =0x0000167d; 
			ir_rec_buf[10] =0x00000015; 
			ir_rec_buf[11] =0x0000167e; 
			ir_rec_buf[12] =0x00000015; 
			ir_rec_buf[13] =0x0000167f; 
			ir_rec_buf[14] =0x00000015; 
			ir_rec_buf[15] =0x0000167d; 
			ir_rec_buf[16] =0x00000016; 
			ir_rec_buf[17] =0x0000167c; 
			ir_rec_buf[18] =0x00000017; 
			ir_rec_buf[19] =0x0000167d; 
			ir_rec_buf[20] =0x00000015; 
			ir_rec_buf[21] =0x0000167d; 
			ir_rec_buf[22] =0x00000015; 
			ir_rec_buf[23] =0x000103c0; 
			ir_rec_buf[24] =0x00000015; 
			ir_rec_buf[25] =0x0000167d; 
			ir_rec_buf[26] =0x00000015; 
			ir_rec_buf[27] =0x0000167e; 
			ir_rec_buf[28] =0x00000015; 
			ir_rec_buf[29] =0x0000167e; 
			ir_rec_buf[30] =0x00000015; 
			ir_rec_buf[31] =0x0000167f; 
			ir_rec_buf[32] =0x00000014; 
			ir_rec_buf[33] =0x0000167e; 
			ir_rec_buf[34] =0x00000015; 
			ir_rec_buf[35] =0x0000167f; 
			ir_rec_buf[36] =0x00000015; 
			ir_rec_buf[37] =0x0000167d; 
			ir_rec_buf[38] =0x00000015; 
			ir_rec_buf[39] =0x0000167f; 
			ir_rec_buf[40] =0x00000015; 
			ir_rec_buf[41] =0x00001680; 
			ir_rec_buf[42] =0x00000014; 
			ir_rec_buf[43] =0x0000167f; 
			ir_rec_buf[44] =0x00000017; 
			ir_rec_buf[45] =0x0000167e; 
			ir_rec_buf[46] =0x00000015; 
			ir_rec_buf[47] =0x000103c1; 
			ir_rec_buf[48] =0x00000015; 
			ir_rec_buf[49] =0x0000167f; 
			ir_rec_buf[50] =0x00000015; 
			ir_rec_buf[51] =0x0000167d; 
			ir_rec_buf[52] =0x00000015; 
			ir_rec_buf[53] =0x0000167f; 
			ir_rec_buf[54] =0x00000017; 
			ir_rec_buf[55] =0x0000167e; 
			ir_rec_buf[56] =0x00000015; 
			ir_rec_buf[57] =0x0000167e; 
			ir_rec_buf[58] =0x00000015; 
			ir_rec_buf[59] =0x0000167f; 
			ir_rec_buf[60] =0x00000018; 
			ir_rec_buf[61] =0x0000167d; 
			ir_rec_buf[62] =0x00000015; 
			ir_rec_buf[63] =0x00001680; 
			ir_rec_buf[64] =0x00000015; 
			ir_rec_buf[65] =0x0000167f; 
			ir_rec_buf[66] =0x00000015; 

			#endif
			ir_send(0, ir_rec_buf, 67);

		#endif

		ir_delay_ms(2000);
	}	
	#endif	
}
*/


void		ir_send(uint32_t	zaibo , uint32_t	*send_buf, uint32_t	send_count)
{
	uint32_t	x,y,temp;

	ir_mode(1);

	if(zaibo ==0)
	{
		temp =send_count;
		for(x =0 ; x< temp ; x++)
		{
			y =*send_buf;
			if(x &0x01)
			{
				ir_no_modulation_send(y, 0);
			}
			else
			{
				ir_no_modulation_send(y ,1);

			}
			send_buf ++;
			send_count ++;
		}
	}
	else
	{
		temp =send_count;
		for(x =0 ; x< temp ; x++)
		{
			y =*send_buf;
			if(x &0x01)
			{
				ir_no_modulation_send(y,0);
			}
			else
			{
				ir_modulation_send(zaibo ,y);
			}
			send_buf ++;
			send_count ++;
		}
	}
	
}

/*
static	void		ir_delay(void)
{
	gpio_config(GPIOA_3,OUTPUT, PULL_LOW);
	while(1)
	{
		#if 1			// 1us test
		port_ir_high();
		ir_delay_us(1);
		port_ir_low();
		ir_delay_us(1);
		#endif

		#if 0				// 2us 
		ir_read_timer( );
		cal_timer_us(50, 50, 60, 60);
		port_ir_high();

		ir_read_timer( );
		cal_timer_us(50, 50, 60, 60);
		port_ir_low();

		#endif

		
	}
}
*/

uint8_t  test_flag = 0;

uint8_t		ir_receive(uint32_t overtimer,  uint32_t *zaibo_phase ,uint32_t *rec_buf , uint32_t  com_count ,uint32_t  *return_count)
{
	uint8_t		ret_value, temp, for_cnt ;//, zaibo_return;
	uint32_t	timer_high_temp , timer_low_temp, end_high , end_low;
	uint32_t	zaibo_high_temp , zaibo_low_temp, zaibo_state;	
	uint32_t	freq[6],sub1;
	uint32_t	phase_cnt, total_cnt, overtimer_temp;



	//ir_init();
	ir_mode(0);				//receive mode
	ir_delay_us(5000);

	overtimer_temp =overtimer *1000;
	ir_read_timer( );
	timer_high_over_start =timer_high;		timer_low_over_start =timer_low;		
	f_start =0;		
	f_state =1;

	total_cnt =0;
	ret_value =IR_REC_ERR;
	zaibo_state =0;

	while(1)
	{
			if(f_start ==0)	
			{
				//载波没开始 ，超时查询,载波开始查询
				if(ir_read_io() ==0) 
				{					
					ir_read_timer();
					//timer_high_decode_start =timer_high;  timer_low_decode_start =timer_low;
					timer_high_phase =timer_high; timer_low_phase =timer_low;
					timer_high_temp =timer_high;  timer_low_temp =timer_low;	

					port_ir_low();

					f_start =1;
					f_state =1;

					//zaibo_return =zaibo_check(50, &timer_high_temp, &timer_low_temp, &freq[0]);
					zaibo_check(50, &timer_high_temp, &timer_low_temp, &freq[0]);
					if(1)						
					{
						if(freq[0] ==0)
						{
							//无载波
							zaibo_state =0;
							*zaibo_phase =freq[0];
							goto		ir_check;
						}
						zaibo_state =1;
						zaibo_high_temp =timer_high_temp;
						zaibo_low_temp =timer_low_temp;

						if(zaibo_check(50, &timer_high_temp, &timer_low_temp, &freq[1]))
						{
							if(zaibo_check(50, &timer_high_temp, &timer_low_temp, &freq[2]))
							{
								if(zaibo_check(50, &timer_high_temp, &timer_low_temp, &freq[3]))
								{
									if(zaibo_check(50, &timer_high_temp, &timer_low_temp, &freq[4]))
									{

										#ifdef		IR_ZAIBO_AVER4
											sub1 =cal_timer_us(zaibo_high_temp, zaibo_low_temp, timer_high_temp, timer_low_temp);
											sub1 =sub1 >>2;
											*zaibo_phase =sub1;

											if((sub1 >0x0a) &&(sub1 <60))
											{
												//UART_PRINTF("zaibo = 0x%08x \r\n",sub1);
												
											}
											else			{goto	zaibo_check_error;}

										#else
											//freq[5] =freq[1];
											freq[5] =freq[1] +freq[2] +freq[3] +freq[4];
											freq[5] =freq[5] >>2;
											*zaibo_phase =freq[5];

											if((freq[5] >0x0a) &&(freq[5] <60))
											{
												//UART_PRINTF("freq[5] = 0x%08x \r\n",freq[5]);
											}
											else			{goto	zaibo_check_error;}
										#endif										
									}								
									else		{goto	zaibo_check_error;}											
									
								}
								else		{goto	zaibo_check_error;}											
							}
							else		{goto	zaibo_check_error;}								
						}
						else		{goto	zaibo_check_error;}
						
					}
					else		{goto	zaibo_check_error;}


				ir_check:					
					//载波检测OK

					if(zaibo_state)
					{
						for(for_cnt =0; for_cnt <com_count ; for_cnt++)
						{
							temp =state_check(&f_state ,&timer_high_phase, &timer_low_phase, &phase_cnt ,overtimer+2);
							if(temp ==1)
							{
								ir_rec_buf[total_cnt] =phase_cnt;
								total_cnt +=1;		
							}	
							else
							{
								break;
							}
						}
					}
					else
					{
						//无载波
						sub1 =cal_timer_us(timer_high_phase, timer_low_phase, timer_high_temp, timer_low_temp);
						ir_rec_buf[0] =sub1;
						total_cnt =1;
						f_state =1;
						for(for_cnt =0; for_cnt <com_count ; for_cnt++)
						{
							temp =nozaibo_state_check(&f_state ,&timer_high_phase, &timer_low_phase, &phase_cnt ,overtimer+2, 1);
							if(temp ==1)
							{
								ir_rec_buf[total_cnt] =phase_cnt;
								total_cnt +=1;		
							}	
							else
							{
								break;
							}
						}
						
					}

					
					#if 0
						for(for_cnt =0; for_cnt <100; for_cnt++)
						{
							UART_PRINTF("ir_rec_buf[%d] %08x \r\n", for_cnt,ir_rec_buf[for_cnt]);
						}
						for(;;);
					#endif
					goto		next;
					
				}					
			}

					zaibo_check_error:
					
						
				f_start =0;		
				f_state =1;

				total_cnt =0;
				ret_value =IR_REC_ERR;
				zaibo_state =0;
				//UART_PRINTF("decode error \r\n");

			next:
			
			//检查时间
			ir_read_timer( );
			end_high =timer_high;		end_low =timer_low;			
			sub1 =cal_timer_us(timer_high_over_start, timer_low_over_start, end_high, end_low);
			if(sub1 >overtimer_temp)
			{
				ret_value =IR_OVERTIMER;

				if(total_cnt <10)						
					ret_value =IR_REC_ERR;
				else	
				{
					*return_count =total_cnt;
					 ret_value =IR_REC_OK;
				}	
				break;
			}

			if(total_cnt >=com_count)
			{
				*return_count =total_cnt;
				ret_value =IR_REC_OK;
				break;

			}
						
	}
							
	return	(ret_value);	
}



static	uint32_t	cal_timer_us(uint32_t timer1_high, uint32_t timer1_low, uint32_t timer2_high, uint32_t timer2_low)
{
	uint32_t sub_high,sub_low;
	if(timer2_high >=timer1_high)	
		sub_high =timer2_high -timer1_high;
	else														
		sub_high =0x8000000 -timer1_high+timer2_high; 

	sub_high =sub_high *625;
	if(timer2_low >=timer1_low)
	{
		sub_low =timer2_low -timer1_low;
		sub_high +=sub_low;
	}
	else	
	{
		sub_low =625-timer1_low +timer2_low;
		sub_high +=sub_low;
		sub_high -=625;

	}
	return		sub_high;
}




static	uint8_t		state_check(uint8_t	*state , uint32_t *start_timer_high, uint32_t *start_timer_low, uint32_t *phase_timer, uint32_t overtimer)
{
	uint8_t 		state_temp,io_state,io_state_temp;
	uint8_t			temp_flag;
	uint32_t		temp_high,temp_low, middle_high, middle_low, end_high, end_low;
	uint32_t		sub1, overtimer_temp;
	
	state_temp =*state;
	temp_flag =0;
	overtimer_temp =overtimer *1000;
	io_state =ir_read_io();
	

	temp_high =*start_timer_high;  temp_low =*start_timer_low;
	
	if(state_temp ==1)
	{
		//载波阶段
		ir_read_timer();
		middle_high =timer_high; 	
		middle_low =timer_low; 				

		ir_read_timer();
		end_high =timer_high; 	
		end_low =timer_low; 				

		while(1)
		{
			io_state_temp =ir_read_io();
			if(io_state_temp ==io_state)			
			{
				// check timer
				ir_read_timer();
				end_high =timer_high; 	
				end_low =timer_low; 				
				sub1 =cal_timer_us(middle_high, middle_low, end_high , end_low);					
				if(sub1 >50)
				{
					//phase exit
					temp_flag =1;						
					break;
				}					
			}
			else
			{
				io_state =io_state_temp;
				ir_read_timer();
				middle_high =timer_high; 	
				middle_low =timer_low; 									
			}

			//判断超时
			sub1 =cal_timer_us(temp_high, temp_low, end_high , end_low);					
			if(sub1 >overtimer_temp)
			{
				temp_flag =2;
				break;
			}
				
		}

		if(temp_flag ==1)
		{				
			sub1 =cal_timer_us(temp_high, temp_low, end_high , end_low);	
			sub1 -=20;
			*phase_timer =sub1;		
			*state =0;

			*start_timer_high =end_high;
			*start_timer_low =end_low;
		
			return(1);
		}
		else			
			return 0;
	}
	else
	{
		io_state_temp =ir_read_io();
		if(io_state_temp ==io_state)			
		while(1)
		{
			ir_read_timer();
			end_high =timer_high; 	end_low =timer_low; 				
			sub1 =cal_timer_us(temp_high, temp_low, end_high , end_low);					

			io_state_temp =ir_read_io();
			if(io_state_temp ==0)
			{
				//退出
				temp_flag =1;

				break;				
			}
			else
			{
				if(sub1 >overtimer_temp)
				{
					temp_flag =2;
					break;
				}		
			}
		}		

		if(temp_flag ==1)
		{				
			*phase_timer =sub1;		
			*state =1;

			*start_timer_high =end_high;
			*start_timer_low =end_low;
			
			return(1);
		}
		else if(temp_flag ==2)
		{
			*phase_timer =sub1;		
	 		return(2);
		}

		else			
			return 0;		
	}
}


static	uint8_t		nozaibo_state_check(uint8_t	*state , uint32_t *start_timer_high, uint32_t *start_timer_low, uint32_t *phase_timer, uint32_t overtimer ,uint32_t cash)
{
	uint8_t 		state_temp,io_state_temp;
	uint8_t			temp_flag;
	uint32_t		temp_high,temp_low, end_high, end_low;
	uint32_t		sub1, overtimer_temp;
	
	state_temp =*state;
	temp_flag =0;
	overtimer_temp =overtimer *1000;
	ir_read_io();
	

	temp_high =*start_timer_high;  temp_low =*start_timer_low;

		if(cash ==0)		
			return	0;
		
		while(1)
		{
			ir_read_timer();
			end_high =timer_high; 	end_low =timer_low; 				
			sub1 =cal_timer_us(temp_high, temp_low, end_high , end_low);		

			io_state_temp =ir_read_io();
			if(io_state_temp !=state_temp)
			{
				//退出
				temp_flag =1;
				break;				
			}
			else
			{
				if(sub1 >overtimer_temp)
				{
					temp_flag =2;
					break;
				}		
			}
		}		

		if(temp_flag ==1)
		{				
			*phase_timer =sub1;		
			*state =io_state_temp;

			*start_timer_high =end_high;
			*start_timer_low =end_low;
			
			return(1);
		}
		else if(temp_flag ==2)
		{
			*phase_timer =sub1;		
	 		return(2);
		}

		else			
			return 0;		
}




static	uint8_t		zaibo_check(uint8_t	us_overtimer , uint32_t *start_timer_high, uint32_t *start_timer_low, uint32_t *zaibo)
{
	uint32_t	temp_high,temp_low, middle_high, middle_low ,end_high, end_low;
	uint32_t	sub1;
	uint8_t		temp_flag, ret_value;
	
	temp_high =*start_timer_high;  temp_low =*start_timer_low;
	

	temp_flag =0;
	for(;;)
	{
		//0---1
		if(ir_read_io() ==1)
		{
			ir_read_timer();

			middle_high =timer_high; 	middle_low =timer_low; 				
			sub1 =cal_timer_us(temp_high, temp_low, middle_high, middle_low);
			temp_flag =1;				//next				
			break;
		}
		else
		{
			ir_read_timer();
			middle_high =timer_high; 	middle_low =timer_low; 
			sub1 =cal_timer_us(temp_high, temp_low, middle_high, middle_low);
			if(sub1 >=70)		
			{
				temp_flag =3;		    //超时退出
				break;
			}
		}
	}


	if(temp_flag ==3)		{ret_value =0;	return (ret_value);}			


	for(;;)
	{
		// 1---0
		if(ir_read_io() ==0)
		{
		
			ir_read_timer();
			end_high =timer_high; 	end_low =timer_low; 
			sub1 =cal_timer_us(middle_high, middle_low, end_high , end_low);
			temp_flag =2;
			break;
		}
		else
		{
			ir_read_timer();
			end_high =timer_high; 	end_low =timer_low; 
			sub1 =cal_timer_us(middle_high, middle_low, end_high , end_low);
			//sub1 =sub1 *2;
			if(sub1 >=70)		
			{
				temp_flag =3;				//超时退出
				break;
			}
		}
	}
	

	if(temp_flag ==3)		
	{
		*start_timer_high =middle_high;
		*start_timer_low =middle_low;		
		*zaibo =0;
		ret_value =0;	return (ret_value);
	}
	else	
	{
		*zaibo =cal_timer_us(temp_high, temp_low, end_high, end_low);
		*start_timer_high =end_high;
		*start_timer_low =end_low;		
		return (1);		
	}

}

static	void		ir_read_timer()
{
	lld_evt_time_get_us(&timer_high , &timer_low);
	timer_low =625-timer_low;
}

uint8_t	ir_read_io()
{

#if 0
	uint8_t	cnt, value_high;

	for(cnt =0, value_high++; cnt <5 ; cnt ++)
	{
		if(REG_APB9_IR_CFG &0x10)		value_high +=1;		
	}

	if(value_high >=4)		return		1;
	else if(value_high <2)								return    0;
	else 					return	(2);
#else 
	if(REG_APB9_IR_CFG &0x10)		return 1;
	else 												
		return 0;
			
#endif

}



static		void ir_delay_ms(int num)
{
    int 	y;
    for(y = 0; y < num; y ++ )
    {
        ir_delay_us(1000);
    }
}


static		void ir_delay_us(int num)
{
    int 	y;
    for(y = 0; y < num; y ++ )
    {
        ir_delay_1us();
    }
}



static		void ir_delay_1us(void)
{
    int x;
    for(x = 0; x < 6; x++);
	//	__asm volatile ( "nop");
}





