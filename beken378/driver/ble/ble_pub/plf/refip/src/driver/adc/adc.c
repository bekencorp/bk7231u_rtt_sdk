#include <stdint.h>        // standard integer definition
#include <string.h>        // string manipulation
#include <stddef.h>     // standard definition
#include "BK3435_reg.h"
#include "uart.h"
#include "gpio.h"
#include "adc.h"


void adc_init(uint8_t chanle,uint8_t mode)
{
	uint32_t cfg;

	//REG_AHB0_ICU_ANALOG1_PWD |= (0x01 << 9);
	
	//enable adc clk
	REG_AHB0_ICU_ADCCLKCON &= ~(0x01 << 0);
	//adc div
	REG_AHB0_ICU_ADCCLKCON = (0x5 << 1);
		
	//set special as peripheral func
	gpio_config(GPIOD_0 + chanle,FLOAT,PULL_NONE); 

	//set adc mode/channel/wait clk
	cfg  = ( (mode << BIT_ADC_MODE ) | (chanle << BIT_ADC_CHNL) | (0x01 << BIT_ADC_WAIT_CLK_SETTING));
	REG_APB7_ADC_CFG =  cfg;

	//set adc sample rate/pre div
	cfg |= ((18 << BIT_ADC_SAMPLE_RATE) | (0 << BIT_ADC_PRE_DIV)|(0x0 << BIT_ADC_DIV1_MODE));

	REG_APB7_ADC_CFG =  cfg;
	
	cfg |= (0x0 << BIT_ADC_FILTER);
	REG_APB7_ADC_CFG =  cfg;

	REG_APB7_ADC_CFG |= (0x01 << BIT_ADC_INT_CLEAR);
	REG_APB7_ADC_CFG |= (0x01 << BIT_ADC_EN);
	
	REG_AHB0_ICU_INT_ENABLE |= (0x01 << 8); 
}



uint16_t g_adc_value;
void adc_isr(void)
{
	REG_APB7_ADC_CFG |= (0x01 << BIT_ADC_INT_CLEAR);
	g_adc_value = (REG_APB7_ADC_DAT & 0x3ff);

	//UART_PRINTF("%d\r\n", g_adc_value);
}



