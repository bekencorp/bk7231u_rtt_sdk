/**
 ****************************************************************************************
 *
 * @file intc.c
 *
 * @brief Definition of the Interrupt Controller (INTCTRL) API.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup INTC
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "ble_compiler.h"        // for inline functions
#include "architect.h"            // arch definition
#include "common_math.h"         // math library definition
#include "rwip_config.h"     // stack configuration

#if defined(CFG_BT)
#include "rwbt.h"            // rwbt core
#endif //CFG_BT
#if defined(CFG_BLE)
#include "rwble.h"           // rwble core
#endif //CFG_BLE

#include "intc.h"            // interrupt controller
#include "reg_intc.h"        // intc registers
#if PLF_UART
#include "uart.h"            // uart definitions
#endif //PLF_UART
#include "icu.h"
#include "gpio.h"
#include "audio.h"
#include "adc.h"
#include "pwm.h"
#include "rtc.h"
#include "spi.h"

/*
 * DEFINES
 ****************************************************************************************
 */

#define RWBT_INT      COMMON_BIT(INTC_BT)
#define PCM_INT       COMMON_BIT(INTC_PCM)
#define TIMER_INT     COMMON_BIT(INTC_TIMER1)
#define UART_INT      COMMON_BIT(INTC_UART)
#define RWBLE_INT     COMMON_BIT(INTC_BLE)
#define DMA_INT       COMMON_BIT(INTC_DMA)

// enable the supported interrupts
#define PLF_INT     (UART_INT | DMA_INT)
#if defined(CFG_BT)
#define BT_INT      (RWBT_INT)
#else
#define BT_INT       0
#endif // #if defined(CFG_BT)
#if defined(CFG_BLE)
#define BLE_INT     (RWBLE_INT)
#else
#define BLE_INT      0
#endif // #if defined(CFG_BLE)



/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
/// locally define this type to be able to qualify the array.
typedef void (*void_fn)(void);

/*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */
void intc_spurious(void);
void IRQ_Exception(void);

/*
 * CONSTANT DATA DEFINITIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
void intc_spurious(void)
{
    // force error
    ASSERT_ERR(0);
}

void intc_init(void)
{
    // Clear all interrupts
    intc_enable_clear(INT_IRQ_BIT | FIQ_IRQ_BIT);
	
    // enable the supported interrupts
    intc_enable_set(INT_IRQ_BIT | FIQ_IRQ_BIT);
	
    intc_module_enable_set(INT_BLE_bit | INT_UART_bit);
}

void intc_stat_clear(void)
{
    // Clear all interrupts
    intc_status_clear(0xFFFF);
    
}

void IRQ_Exception(void)
{
	uint32_t IntStat;
	uint32_t irq_status;

    // sanity check
    ASSERT_ERR(intc_status_get() != 0);
	IntStat = intc_status_get();

#if (DEEP_SLEEP)
	cpu_wakeup();
#endif
	
   	// call the function handler	
	if(IntStat & INT_STATUS_UART_bit)
	{
		irq_status |= INT_STATUS_UART_bit;
		uart_isr();
	}
	if(IntStat & INT_STATUS_SDM_bit)
	{
		irq_status |= INT_STATUS_SDM_bit;
		audio_isr();
	}
	if(IntStat & INT_STATUS_GPIO_bit)
	{
		irq_status |= INT_STATUS_GPIO_bit;
		gpio_isr();
	}
	if(IntStat & INT_STATUS_ADC_bit)
	{
		irq_status |= INT_STATUS_ADC_bit;
		adc_isr();
	}
	if(IntStat & INT_STATUS_PWM1_bit)
	{
		irq_status |= INT_STATUS_PWM1_bit;
		pwm_isr();
	}
	if(IntStat & INT_STATUS_RTC_bit)
	{
		irq_status |= INT_STATUS_RTC_bit;
		rtc_isr();
	}
	/*
	if(IntStat & INT_STATUS_I2C_bit)
	{
		irq_status |= INT_STATUS_I2C_bit;
		i2c_isr();
	}
	*/
	intc_status_clear(irq_status);	
	
}

void FIQ_Exception(void)
{
	uint32_t IntStat;
	uint32_t fiq_status;
	IntStat = intc_status_get();

#if (DEEP_SLEEP)
	cpu_wakeup();
#endif

    // call the function handler
	if(IntStat & INT_STATUS_BLE_bit)
	{
		fiq_status |= INT_STATUS_BLE_bit;

		rwble_isr();
	}

	if(IntStat & INT_STATUS_LBD_bit) 
	{
		fiq_status |= INT_STATUS_LBD_bit;
	}
	if(IntStat & INT_STATUS_GPIO_bit)
	{
		fiq_status |= INT_STATUS_GPIO_bit;
		gpio_isr();
	}
	intc_status_clear(IntStat); 
}


void Undefined_Exception(void)
{
	while(1)
	{
		//UART_PRINTF("%s \r\n",__func__);
		uart_putchar("Undefined_Exception\r\n");
	}

}
void SoftwareInterrupt_Exception(void)
{
	while(1)
	{
		//UART_PRINTF("%s \r\n",__func__);
		uart_putchar("SoftwareInterrupt_Exception\r\n");
	}

}
void PrefetchAbort_Exception(void)
{
	while(1)
	{
		//UART_PRINTF("%s \r\n",__func__);
		uart_putchar("PrefetchAbort_Exception\r\n");
	}

}
void DataAbort_Exception(void)
{
	while(1)
	{
		//UART_PRINTF("%s \r\n",__func__);
		uart_putchar("DataAbort_Exception\r\n");
	}

}
	
void Reserved_Exception(void)
{
	while(1)
	{
		//UART_PRINTF("%s \r\n",__func__);
		uart_putchar("Reserved_Exception\r\n");
	}

}

#pragma ARM
/*Do not change the function*/

__IRQ  FAST_IRQ_ENTRY  void  SYSirq_IRQ_Handler(void)
{

	__asm volatile
	{
		bl IRQ_Exception
	};
}


__FIQ void FAST_FIQ_ENTRY SYSirq_FIQ_Handler(void)
{

	__asm volatile
	{
		bl FIQ_Exception
	};
}


/// @} INTC
