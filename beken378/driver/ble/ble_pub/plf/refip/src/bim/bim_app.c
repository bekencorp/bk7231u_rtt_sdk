

#include <stdint.h>
#include "bim_com.h"
#include "bim_updataImage.h"
#include "bim_application.h"
#include "BK3435_reg.h"
#include "bim_uart.h"

#define Uart_Write_Byte(v)               (REG_APB3_UART_PORT=v)

typedef void (*FUNCPTR)(void);


void delay(uint32_t t)
{
	for(int i  = 1000;i> 0;i--)
	{
		for(int j = 0;j< t;j++)
		{
			;
		}
	}
}
void bim_main(void)	
{
	int32_t main_point = SEC_IMAGE_RUN_CADDR;

	//uart_init(115200);
	//UART_PRINTF("boot uart_init  OK!!1\r\n");
	
	//UART_PRINTF("bim_main \r\n");
	//udi_init_bim_env(ICU_CLK_64M);
	udi_init_bim_env(ICU_CLK_16M);
	//udi_erase_image_sec();
	
	//test_crc();
	//fflash_wr_protect_16k();
	
	//delay(90000);
	 flash_wr_protect_none();
	
	//udi_erase_image_sec();
	//udi_erase_backup_sec();
	//udi_updata_backup_to_image_sec();
	
	//udi_updata_image_to_backup_sec();
	//test_erase_time();
	//while(1);
	
	
	if(1 == udi_select_sec())
	{
		 udi_init_bim_env(ICU_CLK_16M);
		//fflash_wp_ALL();
		//test_crc();
		//while(1);
		//UART_PRINTF("image-main_RUN ADDR = 0x%x\r\n",main_point);
		(*(FUNCPTR)main_point)();
	}else
	{
		//UART_PRINTF("image status error\r\n");
	}
	//flash_rw_test();
	
	
	while(1)
	{ 
	   //UART_PRINTF("error!!!!!!!\r\n");
	}

}

