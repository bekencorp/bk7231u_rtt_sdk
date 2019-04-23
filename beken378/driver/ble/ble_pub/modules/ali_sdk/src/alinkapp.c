/**
 ****************************************************************************************
 *
 * @file alinkapp.c
 *
 * @brief ais coustom Module entry point
 *
 * @auth  gang.cheng
 *
 * @date  2017.11.08
 *
 * Copyright (C) Beken 2009-2016
 *
 *
 ****************************************************************************************
 */



#include "api_export.h"
#include "uart.h"
#include "alinkapp.h"
#include <stdio.h>
static const char *Alink_EVENT[4] = 
{
	"CONNECTED",
	"DISCONNECTED",
	"AUTHENTICATED",
	"TX_DONE"

};

uint8_t auth_flag = 0;
void alink_status_changed_cb(alink_event_t event)
{
	printf("alink_status_changed_cb EVENT = 0x%x,%s\r\n",event,Alink_EVENT[event]);
	if(event == 2)
	{
			auth_flag = 1;
	}
	if(event == 1)
	{
			auth_flag = 0;
	}
	
}


void alink_set_dev_status_cb(uint8_t *buffer, uint32_t length)
{
	printf("alink_set_dev_status_cb \r\n");
	for(uint8_t i = 0; i < length; i ++)
	{
		printf("buffer[%d] = 0x%02x\r\n",i,buffer[i]);
	}
 
}

uint8_t send_buf[300];
void alink_get_dev_status_cb(uint8_t *buffer, uint32_t length)
{
	printf("alink_get_dev_status_cb length = 0x%x\r\n\r\n\r\n",length);
	
	send_buf[0] = 0x00 | ((auth_flag == 1) ? 0x10 : 0x00);
	send_buf[1] = 0x03;
	for(uint32_t i = 2; i < length; i ++)
	{
			send_buf[i] = buffer[i];
	//	UART_PRINTF("buffer[%d] = 0x%02x\r\n",i,buffer[i]);
	}
//	printf("************************TX start******************************************\r\n");
	alink_post(send_buf,length);

}





























