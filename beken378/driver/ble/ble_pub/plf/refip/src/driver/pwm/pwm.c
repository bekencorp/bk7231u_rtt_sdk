/**
****************************************************************************************
*
* @file pwm.c
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
* @addtogroup PWM
* @ingroup PWM
* @brief PWM
*
* This is the driver block for pwm
* @{
****************************************************************************************
*/

#include <stdint.h>        // standard integer definition
#include <string.h>        // string manipulation
#include <stddef.h>     // standard definition
#include "BK3435_reg.h"
#include "pwm.h"
#include "gpio.h"
#include "lld_evt.h"

#include "uart.h"


static void (*p_PWM_Int_Handler[PWM_CHANNEL_NUMBER_MAX])(unsigned char ucPWM_channel);


void pwm_init(PWM_DRV_DESC *pwm_drv_desc)
{
	if(pwm_drv_desc == NULL)
	{
		return;
	}
	if (pwm_drv_desc->channel > PWM_CHANNEL_NUMBER_MAX)
    {
        return;
    }
	if (pwm_drv_desc->duty_cycle > pwm_drv_desc->end_value)
    {
        return;
    }
	// enable GPIO second function
    if ((pwm_drv_desc->mode & 0x0C) != 0x04)
    {
		REG_APB5_GPIOB_CFG &= (~ GPIO_CONFIG_X_SECOND_FUNCTION_MASK(pwm_drv_desc->channel));
    }
	
	//Config clk
	ICU_PWM_CLK_PWM_X_PWD_CLEAR(pwm_drv_desc->channel);
    if (pwm_drv_desc->mode & 0x10)
    {
        // select 16MHz
        ICU_PWM_CLK_PWM_X_SEL_16MHZ(pwm_drv_desc->channel);
    }
    else
    {
        // select 32KHz
        ICU_PWM_CLK_PWM_X_SEL_32KHZ(pwm_drv_desc->channel);
    }
	//Config duty_cycle and end value
	REG_PWM_X_CNT(pwm_drv_desc->channel) = 
        ((((unsigned long)pwm_drv_desc->duty_cycle << PWM_CNT_DUTY_CYCLE_POSI) & PWM_CNT_DUTY_CYCLE_MASK)
       + (((unsigned long)pwm_drv_desc->end_value << PWM_CNT_END_VALUE_POSI) & PWM_CNT_END_VALUE_MASK));

    REG_PWM_CTRL = (REG_PWM_CTRL & (~(0x0F << (0x04 *  pwm_drv_desc->channel))))
             | ((pwm_drv_desc->mode & 0x0F) << (0x04 *  pwm_drv_desc->channel));

	if (pwm_drv_desc->mode & 0x02)    // int enable
	{
		// install interrupt handler
        p_PWM_Int_Handler[pwm_drv_desc->channel] = pwm_drv_desc->p_Int_Handler;
		//ICU_INT_ENABLE_SET(ICU_INT_ENABLE_IRQ_PWM_X_MASK(pwm_drv_desc->channel));
		REG_AHB0_ICU_INT_ENABLE |=  (ICU_INT_ENABLE_IRQ_PWM_X_MASK(pwm_drv_desc->channel)); 
	}
	else
	{
		p_PWM_Int_Handler[pwm_drv_desc->channel] = NULL;
		ICU_INT_ENABLE_CLEAR(ICU_INT_ENABLE_IRQ_PWM_X_MASK(pwm_drv_desc->channel));
	}
}


void pwm_enable(unsigned char ucChannel)
{
    if (ucChannel > PWM_CHANNEL_NUMBER_MAX)
    {
        return;
    }

    REG_PWM_CTRL |= PWM_CTRL_PWM_X_ENABLE_SET(ucChannel);
}

void pwm_disable(unsigned char ucChannel)
{
    if (ucChannel > PWM_CHANNEL_NUMBER_MAX)
    {
        return;
    }

    // 芯片内部设计要求, 这里必须要同时关闭中断使能, 否则照样会触发中断
    REG_PWM_CTRL &= (~(PWM_CTRL_PWM_X_ENABLE_SET(ucChannel) | PWM_CTRL_PWM_X_INT_ENABLE_SET(ucChannel)));
}

void pwm_int_enable(unsigned char ucChannel)
{
    if (ucChannel > PWM_CHANNEL_NUMBER_MAX)
    {
        return;
    }

    REG_PWM_CTRL |= PWM_CTRL_PWM_X_INT_ENABLE_SET(ucChannel);
}

void pwm_int_disable(unsigned char ucChannel)
{
    if (ucChannel > PWM_CHANNEL_NUMBER_MAX)
    {
        return;
    }
    REG_PWM_CTRL &= (~PWM_CTRL_PWM_X_INT_ENABLE_SET(ucChannel));
}

unsigned short pwm_capture_value_get(unsigned char ucChannel)
{
    if (ucChannel > PWM_CHANNEL_NUMBER_MAX)
    {
        return 0;
    }

    return REG_PWM_X_CAP_OUT(ucChannel);
}

void pwm_int_handler_clear(unsigned char ucChannel)
{
    if (ucChannel > PWM_CHANNEL_NUMBER_MAX)
    {
        return;
    }
    p_PWM_Int_Handler[ucChannel] = NULL;
}

void pwm_isr(void)
{
    int i;
    unsigned long ulIntStatus;

    ulIntStatus = REG_PWM_INTR;

    for (i=0; i<PWM_CHANNEL_NUMBER_MAX; i++)
    {
        if (ulIntStatus & (0x01<<i))
        {
            if (p_PWM_Int_Handler[i] != NULL)
            {
                (void)p_PWM_Int_Handler[i]((unsigned char)i);
            }
        }
    }
    do
    {
        REG_PWM_INTR = ulIntStatus;
    } while (REG_PWM_INTR & ulIntStatus & REG_PWM_INTR_MASK);   // delays
}

