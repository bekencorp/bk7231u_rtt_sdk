/**
 ****************************************************************************************
 *
 * @file   btl_task.c
 *
 * @brief BTL Server Role Task Implementation.
 *
 * Copyright (C) Beken 2009-2018
 *
 *
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_BTL_SERVER)

#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "atts.h"
#include "common_utils.h"
#include "kernel_mem.h"
#include "btl.h"
#include "btl_task.h"

#include "prf_utils.h"

static int btl_enable_req_handler(kernel_msg_id_t const msgid,
                                   struct btl_enable_req const *param,
                                   kernel_task_id_t const dest_id,
                                   kernel_task_id_t const src_id)
{
    int msg_status = KERNEL_MSG_SAVED;
    uint8_t state = kernel_state_get(dest_id);

    // check state of the task
    if(state == BTL_IDLE)
    {
      //  struct ffe0s_env_tag* ffe0s_env = PRF_ENV_GET(FFE0S, ffe0s);

        // Check provided values
        if((param->conidx > BLE_CONNECTION_MAX)
            || (gapc_get_conhdl(param->conidx) == GAP_INVALID_CONHDL))
        {
            // an error occurs, trigg it.
            struct btl_enable_rsp* rsp = KERNEL_MSG_ALLOC(BTL_ENABLE_RSP, src_id,
                dest_id, btl_enable_rsp);
            rsp->conidx = param->conidx;
            rsp->status = (param->conidx > BLE_CONNECTION_MAX) ? GAP_ERR_INVALID_PARAM : PRF_ERR_REQ_DISALLOWED;
            kernel_msg_send(rsp);

            msg_status = KERNEL_MSG_CONSUMED;
        }
       
    }

    return msg_status;
}

static void btl_ind_ff02_val(struct btl_env_tag* btl_env,struct btl_ff02_value_upd_req const *param)
{
	
	//UART_PRINTF("%s\r\n",__func__);
    //Allocate the GATT notification message
    struct gattc_send_evt_cmd *btl_value = KERNEL_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
            KERNEL_BUILD_ID(TASK_GATTC, 0), prf_src_task_get(&(btl_env->prf_env),0),
            gattc_send_evt_cmd, param->length);

    //Fill in the parameter structure
    btl_value->operation = GATTC_INDICATE ;//GATTC_NOTIFY;
    btl_value->handle = btl_get_att_handle(BTL_IDX_FF02_VAL_VALUE);
    // pack measured value in database
    btl_value->length = param->length;
		
	btl_value->seq_num = param->seq_num;
  
	memcpy(&btl_value->value[0],param->value,btl_value->length);
		
    //send notification to peer device
    kernel_msg_send(btl_value);
}

static int btl_ff02_value_upd_req_handler(kernel_msg_id_t const msgid,
                                            struct btl_ff02_value_upd_req const *param,
                                            kernel_task_id_t const dest_id,
                                            kernel_task_id_t const src_id)
{
	// UART_PRINTF("%s\r\n",__func__);
    int msg_status = KERNEL_MSG_CONSUMED;
    uint8_t state = kernel_state_get(dest_id);
	
    // check state of the task
    if(state == BTL_IDLE)
    {
        struct btl_env_tag* btl_env = PRF_ENV_GET(BTL, btl);
	
        // update the battery level value
        kernel_state_set(dest_id, BTL_BUSY);       

		btl_ind_ff02_val(btl_env, param);
		kernel_state_set(dest_id, BTL_IDLE);   
		msg_status = KERNEL_MSG_CONSUMED;							       
    }
	else
	{
		msg_status = KERNEL_MSG_SAVED;
	}

    return (msg_status);
  }


  
static int gattc_att_info_req_ind_handler(kernel_msg_id_t const msgid,
        struct gattc_att_info_req_ind *param,
        kernel_task_id_t const dest_id,
        kernel_task_id_t const src_id)
{

    struct gattc_att_info_cfm * cfm;
    uint8_t  att_idx = 0;
    // retrieve handle information
    uint8_t status = btl_get_att_idx(param->handle, &att_idx);
    printf("btl gattc_att_info_req_ind_handler param->handle = %x\r\n",param->handle);    
    //Send write response
    cfm = KERNEL_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);
    cfm->handle = param->handle;

    if(status == GAP_ERR_NO_ERROR)
    {
        // check if it's a client configuration charms
        if(att_idx == BTL_IDX_FF01_VAL_VALUE)
        {
            // attribute length = 2
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
	printf("%s\r\n",__func__);
    struct gattc_write_cfm * cfm;
    uint8_t att_idx = 0;
    uint8_t conidx = KERNEL_IDX_GET(src_id);
    // retrieve handle information
    uint8_t status = btl_get_att_idx(param->handle,  &att_idx);
	printf("att_idx = 0x%x\r\n",att_idx);	
    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        struct btl_env_tag* btl_env = PRF_ENV_GET(BTL, btl);
        // Extract value before check
        uint16_t ind_cfg = common_read16p(&param->value[0]);

        // Only update configuration if value for stop or notification enable
        if ((att_idx == BTL_IDX_FF02_VAL_IND_CFG)
                && ((ind_cfg == PRF_CLI_STOP_NTFIND) || (ind_cfg == PRF_CLI_START_IND)))
        {

            // Conserve information in environment
            if (ind_cfg == PRF_CLI_START_IND)
            {
                // Ntf cfg bit set to 1
                btl_env->ind_cfg[conidx] |= (BTL_FF02_LVL_IND_SUP );
            }
            else
            {
                // Ntf cfg bit set to 0
                btl_env->ind_cfg[conidx] &= ~(BTL_FF02_LVL_IND_SUP );
            }

            

            // Inform APP of configuration change
            struct btl_ff02_value_ind_cfg_ind * ind = KERNEL_MSG_ALLOC(BTL_FF02_VALUE_IND_CFG_IND,
                    prf_dst_task_get(&(btl_env->prf_env), conidx), dest_id,
                    btl_ff02_value_ind_cfg_ind);
            ind->conidx = conidx;
            ind->ind_cfg = btl_env->ind_cfg[conidx];
						
            kernel_msg_send(ind);
						
					
        }
		else if (att_idx == BTL_IDX_FF01_VAL_VALUE)
		{
        	// Allocate the alert value change indication
        	struct btl_ff01_ind *ind = KERNEL_MSG_ALLOC_DYN(BTL_FF01_REQ_IND,
                prf_dst_task_get(&(btl_env->prf_env), conidx),
                dest_id, btl_ff01_ind,param->length);
			
        	// Fill in the parameter structure	
			memcpy(ind->value,&param->value[0],param->length);
			ind->conidx = conidx;
			ind->length = param->length;
			// Send the message
			kernel_msg_send(ind);
		}
        else
        {
            status = PRF_APP_ERROR;
        }

    }

    //Send write response
    cfm = KERNEL_MSG_ALLOC(GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);
    cfm->handle = param->handle;
    cfm->status = status;
    kernel_msg_send(cfm);

    return (KERNEL_MSG_CONSUMED);
}   



static int gattc_read_req_ind_handler(kernel_msg_id_t const msgid, struct gattc_read_req_ind const *param,
                                      kernel_task_id_t const dest_id, kernel_task_id_t const src_id)
{
//	UART_PRINTF("%s\r\n",__func__);
     printf("btl gattc_read_req_ind_handler param->handle = %x\r\n",param->handle);    
    struct gattc_read_cfm * cfm;
    uint8_t  att_idx = 0;
    uint8_t conidx = KERNEL_IDX_GET(src_id);
    // retrieve handle information
    uint8_t status = btl_get_att_idx(param->handle, &att_idx);

    printf("att_idx = 0x%x \r\n",att_idx);
    uint16_t length = 0;
    struct btl_env_tag* btl_env = PRF_ENV_GET(BTL, btl);
//	UART_PRINTF("att_idx = %d\r\n",att_idx);
    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        if (att_idx == BTL_IDX_FF02_VAL_IND_CFG)
        {
            
			length = 2;
            printf("length =2 \r\n");
		
			 //Send read response
			cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, length);
			cfm->handle = param->handle;
			cfm->status = status;
			cfm->length = length;
			
			uint16_t ind_cfg = (btl_env->ind_cfg[conidx] & BTL_FF02_LVL_IND_SUP) ? PRF_CLI_START_IND: PRF_CLI_STOP_NTFIND;
			common_write16p(cfm->value, ind_cfg);
			
		 	kernel_msg_send(cfm);
        }
		
       
    }


    return (KERNEL_MSG_CONSUMED);
}   




static int gattc_cmp_evt_handler(kernel_msg_id_t const msgid,  struct gattc_cmp_evt const *param,
                                 kernel_task_id_t const dest_id, kernel_task_id_t const src_id)
{
  	//UART_PRINTF("%s\r\n",__func__);
  	struct btl_env_tag*btl_env = PRF_ENV_GET(BTL, btl);
    if(param->operation == GATTC_INDICATE)
    {
        // continue operation execution
		struct gattc_cmp_evt *evt = KERNEL_MSG_ALLOC(BTL_GATTC_CMP_EVT,
        prf_dst_task_get(&(btl_env->prf_env), 0),
        dest_id, gattc_cmp_evt);
			
		evt->operation = param->operation;
		evt->status = param->status;
		evt->seq_num = param->seq_num;
				
		kernel_state_set(dest_id, BTL_IDLE); 
		kernel_msg_send(evt);					
    }
		
    return (KERNEL_MSG_CONSUMED);
}

/// Default State handlers definition
const struct kernel_msg_handler btl_default_state[] =
{
    {BTL_ENABLE_REQ,                 (kernel_msg_func_t) btl_enable_req_handler},
    {BTL_FF02_VALUE_UPD_REQ,         (kernel_msg_func_t) btl_ff02_value_upd_req_handler},
    {GATTC_ATT_INFO_REQ_IND,        (kernel_msg_func_t) gattc_att_info_req_ind_handler},
    {GATTC_WRITE_REQ_IND,           (kernel_msg_func_t) gattc_write_req_ind_handler},
    {GATTC_READ_REQ_IND,            (kernel_msg_func_t) gattc_read_req_ind_handler},
    {GATTC_CMP_EVT,                 (kernel_msg_func_t) gattc_cmp_evt_handler},
};


/// Specifies the message handlers that are common to all states.
const struct kernel_state_handler btl_default_handler = KERNEL_STATE_HANDLER(btl_default_state);

#endif /* #if (BLE_BTL_SERVER) */


 
