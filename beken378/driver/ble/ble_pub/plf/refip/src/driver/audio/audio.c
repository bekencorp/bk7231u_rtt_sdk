/**
****************************************************************************************
*
* @file audio.c
*
* @brief audio initialization and specific functions
*
* Copyright (C) Beken 2009-2016
*
* $Rev: $
*
****************************************************************************************
*/

/**
****************************************************************************************
* @addtogroup AUDIO
* @ingroup AUDIO
* @brief AUDIO
*
* This is the driver block for AUDIO
* @{
****************************************************************************************
*/


#include <stddef.h>     // standard definition
#include "BK3435_reg.h"
#include "audio.h"

#include "gpio.h"
#include "uart.h"
#include "application.h"
#include "app_task.h"
#include "app_hid.h"



typedef enum
{
	VOICE_RECORD_FREE,
	VOICE_RECORD_BUSY,
}voice_record_state;

uint16_t pcm_sample_col;
int16_t pcm_sample_buffer[MAX_PCM_BUF_ROW][MAX_PCM_BUF_LEN];
uint8_t pcm_index_in = 0x0;
uint8_t pcm_index_out = 0x0;
static voice_record_state  voice_start_flag = VOICE_RECORD_FREE;


/*******************************************************************************
 * Function: audio_init
 * Description: audio_init
 * Input:  void
 * Output: void
 * Return: void
 * Others: void
*******************************************************************************/
void audio_init(void)
{	
	AUDIO_CONFIG = (0x01 << ADC_ENABLE_BIT); 

	ADC_CONFIG_0 =  (58<<18)|(1<<16)|0x00003A22;
	ADC_CONFIG_1 = 0x8BBF3A22;
	ADC_CONFIG_2 = 0xC9E6751C;
	
    ADC_FIFO_CONFIG &= ~(0x1F << ADC_FIFO_THRESHOLD_BIT);
	ADC_FIFO_CONFIG  |=  (12 << ADC_FIFO_THRESHOLD_BIT);
	
	//REG_AHB0_ICU_ANALOG1_PWD |= (0x01 << 10) | (0x01<< 9);
	//REG_AHB0_ICU_RNGCLKCON &= ~(0X01 << 1);
	
	//UART_PRINTF("REG_AHB0_ICU_ANALOG1_PWD = 0x%08x ,");
}


/*******************************************************************************
 * Function: audio_int_en
 * Description: audio_int_en 
 * Input: void
 * Output: void
 * Return: void
 * Others: void
*******************************************************************************/
void audio_int_en(void)
{
	REG_AHB0_ICU_ANALOG1_PWD |= ((0x01 << 10) | (0x01<< 9));
	REG_AHB0_ICU_RNGCLKCON &= ~(0X01 << 1);
	ADC_FIFO_CONFIG  |=  (0X01 << ADC_FIFO_INT_EN_BIT);
}


/*******************************************************************************
 * Function: audio_int_dis
 * Description: audio_int_dis 
 * Input: void
 * Output: void
 * Return: void
 * Others: void
*******************************************************************************/
void audio_int_dis(void)
{
	ADC_FIFO_CONFIG  &=  ~(0X01 << ADC_FIFO_INT_EN_BIT);
	REG_AHB0_ICU_RNGCLKCON |= (0X01 << 1);
	REG_AHB0_ICU_ANALOG1_PWD &= ~((0x01 << 10) | (0x01<< 9));
}


/*******************************************************************************
 * Function: audio_isr
 * Description: audio_isr 
 * Input: void
 * Output: void
 * Return: void
 * Others: void
*******************************************************************************/
void audio_isr(void)
{
	uint16_t data = 0;
	
	//read adc fifo data until fifo is null.
	while(!(ADC_STAUTS & (0x01 << ADC_STATUS_FIFO_EMPTY_BIT)))
	{
		data = (ADC_FPORT & 0xffff);
		if(data > 32767)
		{
			pcm_sample_buffer[pcm_index_in][pcm_sample_col]= data - 65536;
		}
		else
		{
			pcm_sample_buffer[pcm_index_in][pcm_sample_col]= data;
		}
		pcm_sample_col++;
		if(pcm_sample_col >(MAX_PCM_BUF_LEN - 1))
		{
			pcm_sample_col = 0;
			
			//send encode evt to app task
			app_send_encode_evt();
			
			if(++pcm_index_in > (MAX_PCM_BUF_ROW - 1))
			{
				pcm_index_in = 0;
			}
		}
	}
}


/*******************************************************************************
 * Function: audio_start
 * Description: 
 * Input: void
 * Output: void
 * Return: void
 * Others: void
*******************************************************************************/
void audio_start(void)
{
	if(voice_start_flag == VOICE_RECORD_FREE)
	{
		voice_start_flag = VOICE_RECORD_BUSY;
		audio_int_en();
		encode_voice_init();
		pcm_sample_col = 0;
		pcm_index_in = 0;
		pcm_index_out = 0;
		app_hid_env.audio_start = 1;

		//UART_PRINTF("audio start\r\n");
	}
}



/*******************************************************************************
 * Function: audio_stop
 * Description: 
 * Input: void
 * Output: void
 * Return: void
 * Others: void
*******************************************************************************/
void audio_stop(void)
{
	audio_int_dis();
	pcm_index_in = 0;
	pcm_index_out = 0;
	app_hid_env.encode_tx_cnt = 0;
	app_hid_env.audio_start = 0;

	if(voice_start_flag == VOICE_RECORD_BUSY)
	{
		voice_start_flag = VOICE_RECORD_FREE;
		print_debug_status();
		
		print_debug_info();
		
		//UART_PRINTF("audio stop\r\n");
	}
}




