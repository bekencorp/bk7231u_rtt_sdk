/**
 ****************************************************************************************
 *
 * @file   AIS_task.c
 *
 * @brief AIS Server Role Task Implementation.
 *
 * Copyright (C) Beken 2009-2015
 *
 *
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_FEB3_SERVER)

#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "atts.h"
#include "common_utils.h"
#include "kernel_mem.h"
#include "AIS.h"
#include "AIS_task.h"

#include "prf_utils.h"
#include "uart.h"
#include "ble_pub.h"

static int feb3s_enable_req_handler(kernel_msg_id_t const msgid,
                                   struct feb3s_enable_req const *param,
                                   kernel_task_id_t const dest_id,
                                   kernel_task_id_t const src_id)
{
    //UART_PRINTF("%s\r\n",__func__);
    int msg_status = KERNEL_MSG_SAVED;
    uint8_t state = kernel_state_get(dest_id);

    // check state of the task
    if(state == FEB3S_IDLE)
    {
        struct feb3s_env_tag* feb3s_env = PRF_ENV_GET(FEB3S, feb3s);

        // Check provided values
        if((param->conidx > BLE_CONNECTION_MAX)
            || (gapc_get_conhdl(param->conidx) == GAP_INVALID_CONHDL))
        {
            // an error occurs, trigg it.
            struct feb3s_enable_rsp* rsp = KERNEL_MSG_ALLOC(FEB3S_ENABLE_RSP, src_id,
                dest_id, feb3s_enable_rsp);
            rsp->conidx = param->conidx;
            rsp->status = (param->conidx > BLE_CONNECTION_MAX) ? GAP_ERR_INVALID_PARAM : PRF_ERR_REQ_DISALLOWED;
            kernel_msg_send(rsp);

            msg_status =  KERNEL_MSG_CONSUMED;
        }
       
    }

    return msg_status;
}

static void feb3s_Indicate_fed6_val(struct feb3s_env_tag* feb3s_env,struct feb3s_fed6_value_ind_upd_req const *param)
{
	
	//  UART_PRINTF("%s\r\n",__func__);
    //Allocate the GATT notification message
    struct gattc_send_evt_cmd *fed6_value = KERNEL_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
            KERNEL_BUILD_ID(TASK_GATTC, 0), prf_src_task_get(&(feb3s_env->prf_env),0),
            gattc_send_evt_cmd, param->length);

    //Fill in the parameter structure
    fed6_value->operation = GATTC_INDICATE;
    fed6_value->handle = feb3s_get_att_handle(FEB3S_IDX_FED6_VAL_VALUE);
    // pack measured value in database
    fed6_value->length = param->length;
		fed6_value->seq_num = param->seq_num;
  
//		UART_PRINTF("fed6_value->handle = 0x%x\r\n",fed6_value->handle);
		memcpy(&fed6_value->value[0],param->value,fed6_value->length);
	
		memcpy(feb3s_env->fed6_value,param->value,fed6_value->length);
		feb3s_env->fed6_len = fed6_value->length;
    //send notification to peer device
    kernel_msg_send(fed6_value);
}

static void feb3s_Notify_fed8_val(struct feb3s_env_tag* feb3s_env,struct feb3s_fed8_value_ntf_upd_req const *param)
{
	
	//  UART_PRINTF("%s\r\n",__func__);
    //Allocate the GATT notification message
    struct gattc_send_evt_cmd *fed8_value = KERNEL_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
            KERNEL_BUILD_ID(TASK_GATTC, 0), prf_src_task_get(&(feb3s_env->prf_env),0),
            gattc_send_evt_cmd, param->length);

    //Fill in the parameter structure
    fed8_value->operation = GATTC_NOTIFY;
    fed8_value->handle = feb3s_get_att_handle(FEB3S_IDX_FED8_VAL_VALUE);
	//	UART_PRINTF("fed8_value->handle = 0x%x,param->length = %d\r\n",fed8_value->handle,param->length);
    // pack measured value in database
		fed8_value->length = param->length;
		if(fed8_value->length > 20)
		{
			fed8_value->length = 20;
		}
    
		fed8_value->seq_num = param->seq_num;
  
		memcpy(&fed8_value->value[0],param->value,fed8_value->length);
		
		feb3s_env->fed8_len = param->length;
		
    //send notification to peer device
    kernel_msg_send(fed8_value);
}

static int feb3_fed6_value_ind_upd_req_handler(kernel_msg_id_t const msgid,
                                            struct feb3s_fed6_value_ind_upd_req const *param,
                                            kernel_task_id_t const dest_id,
                                            kernel_task_id_t const src_id)
{
	 // UART_PRINTF("%s\r\n",__func__);
    int msg_status = KERNEL_MSG_CONSUMED;
    uint8_t state = kernel_state_get(dest_id);
	
    // check state of the task
    if(state == FEB3S_IDLE)
    {
        struct feb3s_env_tag* feb3s_env = PRF_ENV_GET(FEB3S, feb3s);

	
        // update the battery level value
        kernel_state_set(dest_id, FEB3S_BUSY);       

				feb3s_Indicate_fed6_val(feb3s_env, param);
			//	ke_state_set(dest_id, FEB3S_IDLE);   
				msg_status =  KERNEL_MSG_CONSUMED;							       
    }
		else
		{
				 //UART_PRINTF("KE_MSG_SAVED6\r\n");
				 msg_status = KERNEL_MSG_SAVED;
		}

    return (msg_status);
}

static int feb3_fed8_value_ntf_upd_req_handler(kernel_msg_id_t const msgid,
                                            struct feb3s_fed8_value_ntf_upd_req const *param,
                                            kernel_task_id_t const dest_id,
                                            kernel_task_id_t const src_id)
{
	// UART_PRINTF("%s\r\n",__func__);
    int msg_status = KERNEL_MSG_CONSUMED;
    uint8_t state = kernel_state_get(dest_id);
	
    // check state of the task
    if(state == FEB3S_IDLE)
    {
        struct feb3s_env_tag* feb3s_env = PRF_ENV_GET(FEB3S, feb3s);

	
        // update the battery level value
        kernel_state_set(dest_id, FEB3S_BUSY);       

	  	 feb3s_Notify_fed8_val(feb3s_env, param);
	//	ke_state_set(dest_id, FEB3S_IDLE);   
		   msg_status = KERNEL_MSG_CONSUMED;							       
    }
	else
	{
		 //UART_PRINTF("KE_MSG_SAVED8\r\n");
		 msg_status = KERNEL_MSG_SAVED;
	}

    return (msg_status);
  }



  
static int gattc_att_info_req_ind_handler(kernel_msg_id_t const msgid,
        struct gattc_att_info_req_ind *param,
        kernel_task_id_t const dest_id,
        kernel_task_id_t const src_id)
{
		//UART_PRINTF("feb3s %s\r\n",__func__);
    struct gattc_att_info_cfm * cfm;
    uint8_t  att_idx = 0;
    // retrieve handle information
    uint8_t status = feb3s_get_att_idx(param->handle, &att_idx);

    //Send write response
    cfm = KERNEL_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);
    cfm->handle = param->handle;

    if(status == GAP_ERR_NO_ERROR)
    {
        // check if it's a client configuration char
        if(att_idx == FFB3S_IDX_FED6_VAL_IND_CFG)
        {
            // CCC attribute length = 2
            cfm->length = 2;
        }
				else if(att_idx == FFB3S_IDX_FED8_VAL_NTF_CFG)
				{
						cfm->length = 2;
				}
        // not expected request
        else
        {
            cfm->length = 0;
            status = ATT_ERR_WRITE_NOT_PERMITTED;
        }
    }

    cfm->status = status;
    kernel_msg_send(cfm);

    return (KERNEL_MSG_CONSUMED);
}



static int gattc_write_req_ind_handler(kernel_msg_id_t const msgid, struct gattc_write_req_ind const *param,
                                      kernel_task_id_t const dest_id, kernel_task_id_t const src_id)
{
	 // UART_PRINTF("feb3s %s\r\n",__func__);
    struct gattc_write_cfm * cfm;
    uint8_t att_idx = 0;
    uint8_t conidx = KERNEL_IDX_GET(src_id);
    // retrieve handle information
    uint8_t status = feb3s_get_att_idx(param->handle,  &att_idx);
	//UART_PRINTF("param->handle = 0x%x ,att_idx =  0x%x\r\n",param->handle,att_idx);
    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        struct feb3s_env_tag* feb3s_env = PRF_ENV_GET(FEB3S, feb3s);
        // Extract value before check
        uint16_t ntf_cfg = common_read16p(&param->value[0]);

        // Only update configuration if value for stop or notification enable
        if ((att_idx == FFB3S_IDX_FED6_VAL_IND_CFG)
                && ((ntf_cfg == PRF_CLI_STOP_NTFIND) || (ntf_cfg == PRF_CLI_START_IND)))
        {
		//				UART_PRINTF("4 \r\n");
            // Conserve information in environment
            if (ntf_cfg == PRF_CLI_START_IND)
            {		
							//UART_PRINTF("5 \r\n");
                // Ntf cfg bit set to 1
                feb3s_env->ind_cfg[conidx] |= (FEB3S_FED6_VAL_IND_SUP );
            }
            else
            {	
//							UART_PRINTF("6 \r\n");
                // Ntf cfg bit set to 0
                feb3s_env->ind_cfg[conidx] &= ~(FEB3S_FED6_VAL_IND_SUP );
            }

            // Inform APP of configuration change
            struct feb3s_fed6_value_ind_cfg_ind * ind = KERNEL_MSG_ALLOC(FEB3S_FED6_VALUE_IND_CFG_IND,
                    prf_dst_task_get(&(feb3s_env->prf_env), conidx), dest_id,
                    feb3s_fed6_value_ind_cfg_ind);
            ind->conidx = conidx;
						ind->handle = param->handle;
            ind->ind_cfg = feb3s_env->ind_cfg[conidx];
						
            kernel_msg_send(ind);
						
					
        }
				else if ((att_idx == FFB3S_IDX_FED8_VAL_NTF_CFG)
                && ((ntf_cfg == PRF_CLI_STOP_NTFIND) || (ntf_cfg == PRF_CLI_START_NTF)))
        {
				//		UART_PRINTF("7 \r\n");
            // Conserve information in environment
            if (ntf_cfg == PRF_CLI_START_NTF)
            {	
				//			UART_PRINTF("8 \r\n");
                // Ntf cfg bit set to 1
                feb3s_env->ntf_cfg[conidx] |= (FEB3S_FED8_VAL_NTF_SUP );
            }
            else
            {	
//							UART_PRINTF("9 \r\n");
                // Ntf cfg bit set to 0
                feb3s_env->ntf_cfg[conidx] &= ~(FEB3S_FED8_VAL_NTF_SUP );
            }

            // Inform APP of configuration change
            struct feb3s_fed8_value_ntf_cfg_ind * ind = KERNEL_MSG_ALLOC(FEB3S_FED8_VALUE_NTF_CFG_IND,
                    prf_dst_task_get(&(feb3s_env->prf_env), conidx), dest_id,
                    feb3s_fed8_value_ntf_cfg_ind);
            ind->conidx = conidx;
						ind->handle = param->handle;
            ind->ntf_cfg = feb3s_env->ntf_cfg[conidx];
						
            kernel_msg_send(ind);
						
					
        }
		else if (att_idx == FEB3S_IDX_FED5_VAL_VALUE)
		{		
		//	UART_PRINTF("10 \r\n");
        	// Allocate the alert value change indication
        	struct feb3s_fed5_writer_req_ind *ind = KERNEL_MSG_ALLOC_DYN(FEB3S_FED5_WRITER_REQ_IND,
                prf_dst_task_get(&(feb3s_env->prf_env), conidx),
                dest_id, feb3s_fed5_writer_req_ind,param->length);
			
        	// Fill in the parameter structure	
			memcpy(ind->value,&param->value[0],param->length);
			memcpy(feb3s_env->fed5_value,&param->value[0],param->length);
			feb3s_env->fed5_len	= param->length;
			ind->conidx = conidx;
			ind->handle = param->handle;
			ind->length = param->length;
			// Send the message
			kernel_msg_send(ind);
		}
		else if (att_idx == FEB3S_IDX_FED7_VAL_VALUE)
		{		
			//UART_PRINTF("11 \r\n");
        	// Allocate the alert value change indication
        	struct feb3s_fed7_writer_cmd_ind *ind = KERNEL_MSG_ALLOC_DYN(FEB3S_FED7_WRITER_CMD_IND,
                prf_dst_task_get(&(feb3s_env->prf_env), conidx),
                dest_id, feb3s_fed7_writer_cmd_ind,param->length);
			
        	// Fill in the parameter structure	
			memcpy(ind->value,&param->value[0],param->length);
			memcpy(feb3s_env->fed7_value,&param->value[0],param->length);
			feb3s_env->fed7_len	= param->length;
			ind->conidx = conidx;
			ind->handle = param->handle;
			ind->length = param->length;
			// Send the message
			kernel_msg_send(ind);
		}
    else
    {		
				//	UART_PRINTF("12 \r\n");
          status = PRF_APP_ERROR;
    }

    }

    //Send write response
    cfm = KERNEL_MSG_ALLOC(GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);
    cfm->handle = param->handle;
    cfm->status = status;
    kernel_msg_send(cfm);

    return ( KERNEL_MSG_CONSUMED);
}   



static int gattc_read_req_ind_handler(kernel_msg_id_t const msgid, struct gattc_read_req_ind const *param,
                                      kernel_task_id_t const dest_id, kernel_task_id_t const src_id)
{
 // 	UART_PRINTF("%s\r\n",__func__);
    struct gattc_read_cfm * cfm;
    uint8_t  att_idx = 0;
    uint8_t conidx = KERNEL_IDX_GET(src_id);
    // retrieve handle information
    uint8_t status = feb3s_get_att_idx(param->handle, &att_idx);
    uint16_t length = 0;
    struct feb3s_env_tag* feb3s_env = PRF_ENV_GET(FEB3S, feb3s);
//	  UART_PRINTF("param->handle = 0x%x ,att_idx =  0x%x,status = 0x%x\r\n",param->handle,att_idx,status);
    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        if ((att_idx == FFB3S_IDX_FED6_VAL_IND_CFG) || (att_idx == FFB3S_IDX_FED8_VAL_NTF_CFG))
        {
		//			UART_PRINTF("1 \r\n");
					length = 2;

					//Send read response
					cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, length);
					cfm->handle = param->handle;
					cfm->status = status;
					cfm->length = length;
					if (att_idx == FFB3S_IDX_FED6_VAL_IND_CFG)
					{
			//			UART_PRINTF("2 \r\n");
						uint16_t ntf_cfg = (feb3s_env->ind_cfg[conidx] & FEB3S_FED6_VAL_IND_SUP) ? PRF_CLI_START_IND : PRF_CLI_STOP_NTFIND;
						common_write16p(cfm->value, ntf_cfg);
					}
					else if (att_idx == FFB3S_IDX_FED8_VAL_NTF_CFG)
					{
			//			UART_PRINTF("3 \r\n");
						uint16_t ntf_cfg = (feb3s_env->ntf_cfg[conidx] & FEB3S_FED8_VAL_NTF_SUP) ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;
						common_write16p(cfm->value, ntf_cfg);
					}		
					kernel_msg_send(cfm);
			}
			else if(att_idx == FEB3S_IDX_FED4_VAL_VALUE)
			{		
			//		UART_PRINTF("4 \r\n");
					/*
					struct feb3s_fed4_read_req_ind *ind = KERNEL_MSG_ALLOC(FEB3S_FED4_READ_REQ_IND,
					prf_dst_task_get(&(feb3s_env->prf_env), conidx),
					dest_id, feb3s_fed4_read_req_ind);

					ind->handle = param->handle;
					ind->rsp_id = src_id;
					kernel_msg_send(ind);
					*/
					if(ble_read_cb != NULL)
                    {
                        ble_read_cb(0xFED4,feb3s_env->fed4_value,feb3s_env->fed4_len);
                    }
					
					cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, feb3s_env->fed4_len);
					cfm->handle = param->handle;
					cfm->status = status;
					cfm->length = feb3s_env->fed4_len;
					memcpy(cfm->value,feb3s_env->fed4_value,feb3s_env->fed5_len);
					kernel_msg_send(cfm);
			}
			else if(att_idx == FEB3S_IDX_FED5_VAL_VALUE)
			{		
		//			UART_PRINTF("55 \r\n");
					cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, feb3s_env->fed5_len);
					cfm->handle = param->handle;
					cfm->status = status;
					cfm->length = feb3s_env->fed5_len;
					memcpy(cfm->value,feb3s_env->fed5_value,feb3s_env->fed5_len);
					kernel_msg_send(cfm);
			}
			else if(att_idx == FEB3S_IDX_FED6_VAL_VALUE)
			{		
			//		UART_PRINTF("66 \r\n");
					cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, feb3s_env->fed6_len);
					cfm->handle = param->handle;
					cfm->status = status;
					cfm->length = feb3s_env->fed6_len;
					memcpy(cfm->value,feb3s_env->fed6_value,feb3s_env->fed6_len);
					kernel_msg_send(cfm);
			}
			else if(att_idx == FEB3S_IDX_FED7_VAL_VALUE)
			{		
	//				UART_PRINTF("77 \r\n");
					cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, feb3s_env->fed7_len);
					cfm->handle = param->handle;
					cfm->status = status;
					cfm->length = feb3s_env->fed7_len;
					memcpy(cfm->value,feb3s_env->fed7_value,feb3s_env->fed7_len);
					kernel_msg_send(cfm);
			}
			else if(att_idx == FEB3S_IDX_FED8_VAL_VALUE)
			{		
	//				UART_PRINTF("88 \r\n");
					cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, feb3s_env->fed8_len);
					cfm->handle = param->handle;
					cfm->status = status;
					cfm->length = feb3s_env->fed8_len;
					memcpy(cfm->value,feb3s_env->fed8_value,feb3s_env->fed8_len);
					kernel_msg_send(cfm);
			}else
			{	  
	//				UART_PRINTF("99 \r\n");
					cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, 1);
					cfm->handle = param->handle;
					cfm->status = status;
					cfm->length = 0;
					kernel_msg_send(cfm);
			}
    }
    
       

    return (KERNEL_MSG_CONSUMED);
}   

static int feb3_fed4_read_rsp_handler(kernel_msg_id_t const msgid, struct feb3s_fed4_read_rsp const *param,
                                      kernel_task_id_t const dest_id, kernel_task_id_t const src_id)
{
	//UART_PRINTF("%s\r\n",__func__);
	struct gattc_read_cfm * cfm;
	
	cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, param->rsp_id, dest_id, gattc_read_cfm, param->length);
	cfm->handle = param->handle;
	cfm->status = GAP_ERR_NO_ERROR;
	cfm->length = param->length;
	memcpy(cfm->value,param->value,param->length);
	
	kernel_msg_send(cfm);
	
	return ( KERNEL_MSG_CONSUMED);
}


static int gattc_cmp_evt_handler(kernel_msg_id_t const msgid,  struct gattc_cmp_evt const *param,
                                 kernel_task_id_t const dest_id, kernel_task_id_t const src_id)
{
  //	UART_PRINTF("feb3 %s\r\n",__func__);
  	struct feb3s_env_tag* feb3s_env = PRF_ENV_GET(FEB3S, feb3s);  
    {
        // continue operation execution
		    struct gattc_cmp_evt *evt = KERNEL_MSG_ALLOC(FEB3S_GATTC_CMP_EVT,
        prf_dst_task_get(&(feb3s_env->prf_env), 0),
        dest_id, gattc_cmp_evt);
			
				evt->operation = param->operation;
				evt->status = param->status;
				evt->seq_num = param->seq_num;
				
				kernel_state_set(dest_id, FEB3S_IDLE); 
				kernel_msg_send(evt);					
    }
		
    return (KERNEL_MSG_CONSUMED);
}

/// Default State handlers definition
const struct kernel_msg_handler feb3s_default_state[] =
{
    {FEB3S_ENABLE_REQ,              (kernel_msg_func_t) feb3s_enable_req_handler},
    {FEB3S_FED6_VALUE_IND_UPD_REQ,  (kernel_msg_func_t) feb3_fed6_value_ind_upd_req_handler},
		{FEB3S_FED8_VALUE_NTF_UPD_REQ,  (kernel_msg_func_t) feb3_fed8_value_ntf_upd_req_handler},
    {GATTC_ATT_INFO_REQ_IND,        (kernel_msg_func_t) gattc_att_info_req_ind_handler},
    {GATTC_WRITE_REQ_IND,           (kernel_msg_func_t) gattc_write_req_ind_handler},
    {GATTC_READ_REQ_IND,            (kernel_msg_func_t) gattc_read_req_ind_handler},
	  {FEB3S_FED4_READ_RSP,			      (kernel_msg_func_t) feb3_fed4_read_rsp_handler},
    {GATTC_CMP_EVT,                 (kernel_msg_func_t) gattc_cmp_evt_handler},
		
};


/// Specifies the message handlers that are common to all states.
const struct kernel_state_handler feb3s_default_handler = KERNEL_STATE_HANDLER(feb3s_default_state);

#endif /* #if (BLE_FEB3_SERVER) */


 
