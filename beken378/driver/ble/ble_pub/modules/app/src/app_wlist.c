/**
 ****************************************************************************************
 *
 * @file app_wlist.c
 *
 * @brief Application entry point
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"          // SW configuration

#if (BLE_APP_PRESENT)
#include <string.h>
#include "rwapp_config.h"
#include "app_task.h"             // Application task Definition
#include "app_wlist.h"
#include "application.h"                     // Application Definition
#include "gap.h"                     // GAP Definition
#include "gapm_task.h"         // GAP Manager Task API
#include "gapc_task.h"           // GAP Controller Task API

#include "common_bt.h"                  // Common BT Definition
#include "common_math.h"             // Common Maths Definition
#include "kernel_timer.h"
#include "uart.h"



void appm_get_wlist_size(void)  //size set by BLE_WHITELIST_MAX
{
	 /*  rsp:
	  * GAPM_CMP_EVT: When operation completed.
	  * GAPM_WHITE_LIST_SIZE_IND: If white list size is requested
	  */
	 //UART_PRINTF("%s\r\n",__func__);
	 struct gapm_white_list_mgt_cmd* cmd = KERNEL_MSG_ALLOC(GAPM_WHITE_LIST_MGT_CMD,
                    TASK_GAPM, TASK_APP, gapm_white_list_mgt_cmd);
	 
	 
	cmd->operation = GAPM_GET_WLIST_SIZE;
	 
	kernel_msg_send(cmd);
}

int app_gapm_white_list_ind_handler(kernel_msg_id_t const msgid,
                                struct gapm_white_list_size_ind const *param,
                                kernel_task_id_t const dest_id,
                                kernel_task_id_t const src_id)
																
{
	//UART_PRINTF("%s\r\n",__func__);
	
	
	//UART_PRINTF("size = %x\r\n",param->size);
	
	
	return (KERNEL_MSG_CONSUMED);
}


void appm_add_dev_to_wlist(struct gap_bdaddr bdaddr)
{
	 /*  rsp:
	  * GAPM_CMP_EVT: When operation completed.
	  */
	
	 //UART_PRINTF("%s\r\n",__func__);
	 struct gapm_white_list_mgt_cmd* cmd = KERNEL_MSG_ALLOC_DYN(GAPM_WHITE_LIST_MGT_CMD,
                    TASK_GAPM, TASK_APP, gapm_white_list_mgt_cmd,sizeof(struct gap_bdaddr));
	 
	 
	  cmd->operation = GAPM_ADD_DEV_IN_WLIST;
	  cmd->nb = 1;
	  cmd->devices[0] = bdaddr;
	 
	  //UART_PRINTF("addr_type = %x\r\n", cmd->devices[0].addr_type);
	  //UART_PRINTF("addr = ");
	  for(int i = 0;i < sizeof(bd_addr_t);i++)
	  {
		//UART_PRINTF("%x ",cmd->devices[0].addr.addr[i]);
	  }
	  //UART_PRINTF("\r\n");
		
	  kernel_msg_send(cmd);
}


void appm_rmv_dev_from_wlist(struct gap_bdaddr bdaddr)
{
	 /*  rsp:
	  *  GAPM_CMP_EVT: When operation completed.
	  */
	
	 //UART_PRINTF("%s\r\n",__func__);
	 struct gapm_white_list_mgt_cmd* cmd = KERNEL_MSG_ALLOC_DYN(GAPM_WHITE_LIST_MGT_CMD,
                    TASK_GAPM, TASK_APP, gapm_white_list_mgt_cmd,sizeof(struct gap_bdaddr));
	 
	 
	  cmd->operation = GAPM_RMV_DEV_FRM_WLIST;
	  cmd->nb = 1;
	  cmd->devices[0] = bdaddr;
	 
	  //UART_PRINTF("addr_type = %x\r\n", cmd->devices[0].addr_type);
	  //UART_PRINTF("addr = ");
	  for(int i = 0;i < sizeof(bd_addr_t);i++)
	  {
	//	UART_PRINTF("%x ",cmd->devices[0].addr.addr[i]);
	  }
	  //UART_PRINTF("\r\n");
		
	  kernel_msg_send(cmd);
	 

}




void appm_clear_wlist(void)
{
	 /*  rsp:
	  * GAPM_CMP_EVT: When operation completed.
	  * GAPM_WHITE_LIST_SIZE_IND: If white list size is requested
	  */
	
	 //UART_PRINTF("%s\r\n",__func__);
	 struct gapm_white_list_mgt_cmd* cmd = KERNEL_MSG_ALLOC(GAPM_WHITE_LIST_MGT_CMD,
                    TASK_GAPM, TASK_APP, gapm_white_list_mgt_cmd);
	 
	 
	cmd->operation = GAPM_CLEAR_WLIST;
	 
	kernel_msg_send(cmd);
}



 //Get Resolving List Size ; size set by BLE_RESOL_ADDR_LIST_MAX
void appm_get_ral_list_size(void)
{
	  /*  rsp:
	   * GAPM_CMP_EVT: When operation completed.
	   * GAPM_RAL_SIZE_IND: If resolving list size is requested.
	   * GAPM_RAL_ADDR_IND: If local or peer address is requested.
	   */
	
	 //UART_PRINTF("%s\r\n",__func__);
	 struct gapm_ral_mgt_cmd* cmd = KERNEL_MSG_ALLOC(GAPM_RAL_MGT_CMD,
                    TASK_GAPM, TASK_APP, gapm_ral_mgt_cmd);
	 
	 
	 cmd->operation = GAPM_GET_RAL_SIZE;
	 
	 kernel_msg_send(cmd);
	 

}


int app_gapm_ral_list_size_ind_handler(kernel_msg_id_t const msgid,
                                struct gapm_ral_size_ind const *param,
                                kernel_task_id_t const dest_id,
                                kernel_task_id_t const src_id)
																
{
	//UART_PRINTF("%s\r\n",__func__);	
	
	//UART_PRINTF("size = %x\r\n",param->size);

	return (KERNEL_MSG_CONSUMED);
}



void appm_add_ral_dev_to_list(struct gap_ral_dev_info ral_dev) 
{

	
	 //UART_PRINTF("%s\r\n",__func__);
	 struct gapm_ral_mgt_cmd* cmd = KERNEL_MSG_ALLOC_DYN(GAPM_RAL_MGT_CMD,
                    TASK_GAPM, TASK_APP, gapm_ral_mgt_cmd,sizeof(struct gap_ral_dev_info));
	 
	 
	 cmd->operation = GAPM_ADD_DEV_IN_RAL;
	 cmd->nb = 1;
	 cmd->devices[0] = ral_dev;
	 
	 //UART_PRINTF("addr_type = %x\r\n",cmd->devices[0].addr_type);
	 
	 //UART_PRINTF("addr = ");
	 for(int i = 0;i < sizeof(bd_addr_t);i++)
	 {
		//UART_PRINTF("%x ",cmd->devices[0].addr.addr[i]); 
	 }
	 //UART_PRINTF("\r\n");
	
	 //UART_PRINTF("peer_irk = ");
	 for(int i = 0;i < GAP_KEY_LEN;i++)
	 {
		//UART_PRINTF("%x ",cmd->devices[0].peer_irk[i]); 
	 }
	 //UART_PRINTF("\r\n");
	
	 //UART_PRINTF("local_irk = ");
	 for(int i = 0;i < GAP_KEY_LEN;i++)
	 {
		//UART_PRINTF("%x ",cmd->devices[0].local_irk[i]); 
	 }
	// UART_PRINTF("\r\n");
	 
	 kernel_msg_send(cmd);
	 
}



void appm_rmv_ral_dev_from_list(struct gap_ral_dev_info ral_dev) 
{
	 //UART_PRINTF("%s\r\n",__func__);
	 struct gapm_ral_mgt_cmd* cmd = KERNEL_MSG_ALLOC_DYN(GAPM_RAL_MGT_CMD,
                    TASK_GAPM, TASK_APP, gapm_ral_mgt_cmd,sizeof(struct gap_ral_dev_info));
	 
	 
	 cmd->operation = GAPM_RMV_DEV_FRM_RAL;
	 cmd->nb = 1;
	 cmd->devices[0] = ral_dev;

	 /*
	 UART_PRINTF("addr_type = %x\r\n",cmd->devices[0].addr_type);
	 
	 UART_PRINTF("addr = ");
	 for(int i = 0;i < sizeof(bd_addr_t);i++)
	 {
		UART_PRINTF("%x ",cmd->devices[0].addr.addr[i]); 
	 }
	 UART_PRINTF("\r\n");
	
	 UART_PRINTF("peer_irk = ");
	 for(int i = 0;i < GAP_KEY_LEN;i++)
	 {
		UART_PRINTF("%x ",cmd->devices[0].peer_irk[i]); 
	 }
	 UART_PRINTF("\r\n");
	
	 UART_PRINTF("local_irk = ");
	 for(int i = 0;i < GAP_KEY_LEN;i++)
	 {
		UART_PRINTF("%x ",cmd->devices[0].local_irk[i]); 
	 }
	 UART_PRINTF("\r\n");
	*/
	 kernel_msg_send(cmd);
}



void appm_get_ral_loc_addr(struct gap_bdaddr bdaddr) 
{
	/*rsp:
	  *GAPM_CMP_EVT: When operation completed.
	  *GAPM_RAL_SIZE_IND: If resolving list size is requested.
	  *GAPM_RAL_ADDR_IND: If local or peer address is requested.
	*/
	
	 //UART_PRINTF("%s\r\n",__func__);
	 struct gapm_ral_mgt_cmd* cmd = KERNEL_MSG_ALLOC(GAPM_RAL_MGT_CMD,
                    TASK_GAPM, TASK_APP, gapm_ral_mgt_cmd);
	 
	 
	 cmd->operation = GAPM_GET_RAL_LOC_ADDR;
	 cmd->nb = 1;
	 memcpy(cmd->devices[0].addr.addr, bdaddr.addr.addr,sizeof(bd_addr_t));
	 cmd->devices[0].addr_type = bdaddr.addr_type;
	 
	 
	 kernel_msg_send(cmd);
	 

}

void appm_get_ral_peer_addr(struct gap_bdaddr bdaddr) 
{
	/*  rsp:
		GAPM_CMP_EVT: When operation completed.
		GAPM_RAL_SIZE_IND: If resolving list size is requested.
		GAPM_RAL_ADDR_IND: If local or peer address is requested.
	*/
	
	 //UART_PRINTF("%s\r\n",__func__);
	 struct gapm_ral_mgt_cmd* cmd = KERNEL_MSG_ALLOC(GAPM_RAL_MGT_CMD,
                    TASK_GAPM, TASK_APP, gapm_ral_mgt_cmd);
	 
	 
	 cmd->operation = GAPM_GET_RAL_PEER_ADDR;
	 
	 cmd->nb = 1;
	 memcpy(cmd->devices[0].addr.addr, bdaddr.addr.addr,sizeof(bd_addr_t));
	 cmd->devices[0].addr_type = bdaddr.addr_type;
	 
	 kernel_msg_send(cmd);
	 

}


void appm_clear_ral_list(void) //clear Resolving List
{
	/*  rsp:
	   GAPM_CMP_EVT: When operation completed.
	   GAPM_RAL_SIZE_IND: If resolving list size is requested.
	   GAPM_RAL_ADDR_IND: If local or peer address is requested.
	*/
	
	 //UART_PRINTF("%s\r\n",__func__);
	 struct gapm_ral_mgt_cmd* cmd = KERNEL_MSG_ALLOC(GAPM_RAL_MGT_CMD,
                    TASK_GAPM, TASK_APP, gapm_ral_mgt_cmd);
	 
	 
	 cmd->operation = GAPM_CLEAR_RAL;
	 
	 kernel_msg_send(cmd);

}


int app_gapm_ral_addr_ind_handler(kernel_msg_id_t const msgid,
                                struct gapm_ral_addr_ind const *param,
                                kernel_task_id_t const dest_id,
                                kernel_task_id_t const src_id)
																
{
/*
	UART_PRINTF("%s\r\n",__func__);
	
	
	UART_PRINTF("operation = %x\r\n",param->operation);
	UART_PRINTF("addr_type = %x\r\n",param->addr.addr_type);
	UART_PRINTF("addr = ");
	for(int i = 0;i < sizeof(bd_addr_t);i++)
	{
		UART_PRINTF("%x ",param->addr.addr.addr[i]);
	}
	UART_PRINTF("\r\n");
	*/
	return (KERNEL_MSG_CONSUMED);
}


void appm_gapm_resolv_dev_addr(bd_addr_t addr,struct gap_sec_key irk)
{
	  //UART_PRINTF("%s\r\n",__func__);
	  struct gapm_resolv_addr_cmd* cmd = KERNEL_MSG_ALLOC_DYN(GAPM_RESOLV_ADDR_CMD,
                    TASK_GAPM, TASK_APP, gapm_resolv_addr_cmd,sizeof(struct gap_sec_key));
	 
	  cmd->operation = GAPM_RESOLV_ADDR;
	  cmd->nb_key = 1;
	 
	  memcpy(cmd->addr.addr,addr.addr,sizeof(bd_addr_t));
	  /*
	  UART_PRINTF("addr = ");
	  for(int i = 0; i < sizeof(bd_addr_t);i ++)
	  {
		UART_PRINTF(" %x",cmd->addr.addr[i]);
	  }
	  UART_PRINTF("\r\n");
	  */
	  memcpy(cmd->irk[0].key,irk.key,sizeof(struct gap_sec_key));
	  /*
	  UART_PRINTF("irk = ");
	  for(int i = 0; i < sizeof(struct gap_sec_key);i ++)
	  {
		UART_PRINTF(" %x",cmd->irk[0].key[i]);
	  }
	  UART_PRINTF("\r\n");
	  */
	 
	  kernel_msg_send(cmd);
}


int app_gapm_resolv_dev_addr_ind_handler(kernel_msg_id_t const msgid,
                                struct gapm_addr_solved_ind const *param,
                                kernel_task_id_t const dest_id,
                                kernel_task_id_t const src_id)

{
/*
  	UART_PRINTF("%s\r\n",__func__);
	
	UART_PRINTF("addr = ");
	for(int i = 0; i < sizeof(bd_addr_t);i ++)
	{
		UART_PRINTF(" %x",param->addr.addr[i]);
	}
	UART_PRINTF("\r\n");
	UART_PRINTF("irk = ");
	for(int i = 0; i < sizeof(struct gap_sec_key);i ++)
    {
		UART_PRINTF(" %x",param->irk.key[i]);
	}
	UART_PRINTF("\r\n");
	*/
	return (KERNEL_MSG_CONSUMED);
}


void appm_gapm_gen_rand_addr(void)
{
	 //UART_PRINTF("%s\r\n",__func__);
	 struct gapm_gen_rand_addr_cmd* cmd = KERNEL_MSG_ALLOC(GAPM_GEN_RAND_ADDR_CMD,
                    TASK_GAPM, TASK_APP, gapm_gen_rand_addr_cmd);
	 

	 cmd->operation = GAPM_GEN_RAND_ADDR;
	 cmd->prand[0] = (uint8_t)common_rand_word();
	 cmd->prand[1] = (uint8_t)common_rand_word();
	 cmd->prand[2] = (uint8_t)common_rand_word();
	 
	 cmd->rnd_type = GAP_STATIC_ADDR;
	 /*
	  ///  - GAP_STATIC_ADDR: Static random address
      ///  - GAP_NON_RSLV_ADDR: Private non resolvable address
      ///  - GAP_RSLV_ADDR: Private resolvable address
      ///  - uint8_t rnd_type;
	 */
	 
	  kernel_msg_send(cmd);
}

int app_gapm_gen_rand_addr_ind_handler(kernel_msg_id_t const msgid,
                                struct gapm_dev_bdaddr_ind const *param,
                                kernel_task_id_t const dest_id,
                                kernel_task_id_t const src_id)
																
{
/*
	UART_PRINTF("%s\r\n",__func__);
	UART_PRINTF("param->addr.addr_type = 0x%x \r\n",param->addr.addr_type);
 	UART_PRINTF("param->addr.addr = ");
	for(int i = 0; i < sizeof(bd_addr_t);i ++)
	{
		UART_PRINTF(" %x",param->addr.addr.addr[i]);
	}
	UART_PRINTF("\r\n");
*/
	return (KERNEL_MSG_CONSUMED);
}

void appm_gapm_gen_rand_number(void)
{
	 //UART_PRINTF("%s\r\n",__func__);
	 struct gapm_gen_rand_nb_cmd* cmd = KERNEL_MSG_ALLOC(GAPM_GEN_RAND_NB_CMD,
                    TASK_GAPM, TASK_APP, gapm_gen_rand_nb_cmd);
	 

	  cmd->operation = GAPM_GEN_RAND_NB;
	 
	  kernel_msg_send(cmd);
}


int app_gapm_gen_rand_number_ind_handler(kernel_msg_id_t const msgid,
                                struct gapm_gen_rand_nb_ind const *param,
                                kernel_task_id_t const dest_id,
                                kernel_task_id_t const src_id)
																
{
/*
	UART_PRINTF("%s\r\n",__func__);
	
	UART_PRINTF("param->randnb.nb = ");
	for(int i = 0;i < GAP_RAND_NB_LEN;i++)
	{
		UART_PRINTF("%x ",param->randnb.nb[i]);
	}
	UART_PRINTF("\r\n");
*/	
	return (KERNEL_MSG_CONSUMED);
}

void appm_gapm_set_irk(struct gap_sec_key irk) 
{
	//UART_PRINTF("%s\r\n",__func__);
	struct gapm_set_irk_cmd *cmd = KERNEL_MSG_ALLOC(GAPM_SET_IRK_CMD,
	            TASK_GAPM, TASK_APP, gapm_set_irk_cmd);

	cmd->operation = GAPM_SET_IRK;
	memcpy(cmd->irk.key,irk.key,sizeof(struct gap_sec_key));

/*
	UART_PRINTF("irk.key = ");
	for(int i =0;i < sizeof(struct gap_sec_key);i ++)
	{
	UART_PRINTF("%x ",cmd->irk.key[i]);
	}
	UART_PRINTF("\r\n");
*/	
	kernel_msg_send(cmd);
}

/*
void app_cfg_test(void)
{
	

	static uint8_t test_cnt = 13;
	UART_PRINTF("%s,test_cnt = %d\r\n",__func__,test_cnt);
	
	switch(test_cnt)
	{
		case 0:
		{
			appm_get_wlist_size();
		  	test_cnt++;
			kernel_timer_set(APP_PERIOD_TIMER_FOR_TEST,TASK_APP,200);
		
		}break;
		
		case 1:
		{
			appm_clear_wlist();
			test_cnt++;
			kernel_timer_set(APP_PERIOD_TIMER_FOR_TEST,TASK_APP,200);
			
		}break;
		
		case 2:
		{
			appm_get_wlist_size();
			test_cnt++;
			kernel_timer_set(APP_PERIOD_TIMER_FOR_TEST,TASK_APP,200);
		}break;
		
		case 3:
		{
			struct gap_bdaddr bdaddr;
			bdaddr.addr_type = 1;
			bdaddr.addr.addr[0] = 1;
			bdaddr.addr.addr[1] = 2;
			bdaddr.addr.addr[2] = 3;
			bdaddr.addr.addr[3] = 4;
			bdaddr.addr.addr[4] = 5;
			bdaddr.addr.addr[5] = 6;
			appm_add_dev_to_wlist(bdaddr);
			test_cnt++;
			kernel_timer_set(APP_PERIOD_TIMER_FOR_TEST,TASK_APP,200);
		}break;
		
		case 4:
		{
			struct gap_bdaddr bdaddr;
			bdaddr.addr_type = 1;
			bdaddr.addr.addr[0] = 2;
			bdaddr.addr.addr[1] = 2;
			bdaddr.addr.addr[2] = 3;
			bdaddr.addr.addr[3] = 4;
			bdaddr.addr.addr[4] = 5;
			bdaddr.addr.addr[5] = 6;
			appm_add_dev_to_wlist(bdaddr);
			test_cnt++;
			kernel_timer_set(APP_PERIOD_TIMER_FOR_TEST,TASK_APP,200);
		}break;
		
		case 5:
		{
			struct gap_bdaddr bdaddr;
			bdaddr.addr_type = 1;
			bdaddr.addr.addr[0] = 3;
			bdaddr.addr.addr[1] = 2;
			bdaddr.addr.addr[2] = 3;
			bdaddr.addr.addr[3] = 4;
			bdaddr.addr.addr[4] = 5;
			bdaddr.addr.addr[5] = 6;
			appm_add_dev_to_wlist(bdaddr);
			test_cnt++;
			kernel_timer_set(APP_PERIOD_TIMER_FOR_TEST,TASK_APP,200);
		}break;
		
		
		case 6:
		{
			struct gap_bdaddr bdaddr;
			bdaddr.addr_type = 1;
			bdaddr.addr.addr[0] = 3;
			bdaddr.addr.addr[1] = 2;
			bdaddr.addr.addr[2] = 3;
			bdaddr.addr.addr[3] = 4;
			bdaddr.addr.addr[4] = 5;
			bdaddr.addr.addr[5] = 6;
			appm_rmv_dev_from_wlist(bdaddr);
			test_cnt++;
			kernel_timer_set(APP_PERIOD_TIMER_FOR_TEST,TASK_APP,200);
		}break;
		
		
		
		case 7:
		{
			struct gap_bdaddr bdaddr;
			bdaddr.addr_type = 1;
			bdaddr.addr.addr[0] = 4;
			bdaddr.addr.addr[1] = 2;
			bdaddr.addr.addr[2] = 3;
			bdaddr.addr.addr[3] = 4;
			bdaddr.addr.addr[4] = 5;
			bdaddr.addr.addr[5] = 6;
			appm_add_dev_to_wlist(bdaddr);
			test_cnt++;
			kernel_timer_set(APP_PERIOD_TIMER_FOR_TEST,TASK_APP,200);
		}break;
		
		case 8:
		{
			appm_get_wlist_size();
			test_cnt++;
		
		}break;
		
		
		case 9:
		{
			struct gap_ral_dev_info ral_dev;
			uint8_t key_len = KEY_LEN;
			ral_dev.addr_type = 1;
			ral_dev.addr.addr[0] = 0;
			ral_dev.addr.addr[1] = 1;
			ral_dev.addr.addr[2] = 2;
			ral_dev.addr.addr[3] = 3;
			ral_dev.addr.addr[4] = 4;
			ral_dev.addr.addr[5] = 5;
			
			if (nvds_get(NVDS_TAG_LOC_IRK, &key_len, ral_dev.local_irk) != NVDS_OK)
			{
			
				UART_PRINTF("nvds_get NVDS_TAG_LOC_IRK fail!!!\r\n");
			}
			
			if (nvds_get(NVDS_TAG_LOC_IRK, &key_len, ral_dev.peer_irk) != NVDS_OK)
			{
			
				UART_PRINTF("nvds_get NVDS_TAG_LOC_IRK fail!!!\r\n");
			}
			
			ral_dev.peer_irk[0]++;
					
			appm_add_ral_dev_to_list( ral_dev);
			
			test_cnt++;
			kernel_timer_set(APP_PERIOD_TIMER_FOR_TEST,TASK_APP,200);
		}break;
		
			
		case 10:
		{
			appm_get_ral_list_size();
			test_cnt++;
			kernel_timer_set(APP_PERIOD_TIMER_FOR_TEST,TASK_APP,200);
		}break;
		
		case 11:
		{
			struct gap_bdaddr bdaddr;
			bdaddr.addr_type = 1;
			bdaddr.addr.addr[0] = 0;
			bdaddr.addr.addr[1] = 1;
			bdaddr.addr.addr[2] = 2;
			bdaddr.addr.addr[3] = 3;
			bdaddr.addr.addr[4] = 4;
			bdaddr.addr.addr[5] = 5;
			appm_get_ral_loc_addr(bdaddr);
			
			test_cnt++;
			kernel_timer_set(APP_PERIOD_TIMER_FOR_TEST,TASK_APP,200);
		}break;
		
		case 12:
		{
			struct gap_bdaddr bdaddr;
			bdaddr.addr_type = 1;
			bdaddr.addr.addr[0] = 0;
			bdaddr.addr.addr[1] = 1;
			bdaddr.addr.addr[2] = 2;
			bdaddr.addr.addr[3] = 3;
			bdaddr.addr.addr[4] = 4;
			bdaddr.addr.addr[5] = 5;
			appm_get_ral_peer_addr(bdaddr);
			test_cnt++;
		//	kernel_timer_set(APP_PERIOD_TIMER_FOR_TEST,TASK_APP,200);
		}break;
		
		case 13:
		{
			appm_gapm_gen_rand_number();
			test_cnt++;
			kernel_timer_set(APP_PERIOD_TIMER_FOR_TEST,TASK_APP,200);
		}break;
		
		case 14:
		{
			appm_gapm_gen_rand_addr();
			test_cnt++;
			kernel_timer_set(APP_PERIOD_TIMER_FOR_TEST,TASK_APP,200);
		}break;
		
		case 15:
		{
			appm_gapm_gen_rand_addr();
			test_cnt++;
			kernel_timer_set(APP_PERIOD_TIMER_FOR_TEST,TASK_APP,200);
		}break;
		
		case 16:
		{
			uint8_t key_len = KEY_LEN;
			struct gap_sec_key irk;
			if (nvds_get(NVDS_TAG_LOC_IRK, &key_len, irk.key)!= NVDS_OK)
			{
			
				UART_PRINTF("nvds_get NVDS_TAG_LOC_IRK fail!!!\r\n");
			}
			irk.key[0] = 0xaa;
			appm_gapm_set_irk(irk);
			test_cnt++;
		//	kernel_timer_set(APP_PERIOD_TIMER_FOR_TEST,TASK_APP,200);
		}break;
		
		case 20:
		{
			bd_addr_t addr;
			struct gap_sec_key irk;
			addr.addr[0] = 0;
			addr.addr[1] = 1;
			addr.addr[2] = 2;
			addr.addr[3] = 3;
			addr.addr[4] = 4;
			addr.addr[5] = 5;
			for(int i = 0;i < sizeof(struct gap_sec_key);i++)
			{
				irk.key[i]  = i + 1;
			}
			
			appm_gapm_resolv_dev_addr(addr,irk);
			test_cnt++;
		//	kernel_timer_set(APP_PERIOD_TIMER_FOR_TEST,TASK_APP,200);
		}break;
		
		default:
			break;
	}
}
*/
#endif //(BLE_APP_PRESENT)

/// @} APP


