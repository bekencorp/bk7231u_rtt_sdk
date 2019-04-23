/**
 ****************************************************************************************
 *
 * @file   ayla_task.c
 *
 * @brief AYLA Server Role Task Implementation.
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

#if (BLE_AYLA_SERVER)

#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "atts.h"
#include "common_utils.h"
#include "kernel_mem.h"
#include "ayla.h"
#include "ayla_task.h"

#include "prf_utils.h"

static int ayla_enable_req_handler(kernel_msg_id_t const msgid,
                                   struct ayla_enable_req const *param,
                                   kernel_task_id_t const dest_id,
                                   kernel_task_id_t const src_id)
{
    int msg_status = KERNEL_MSG_SAVED;
    uint8_t state = kernel_state_get(dest_id);

    // check state of the task
    if(state == AYLA_IDLE)
    {
      //  struct ffe0s_env_tag* ffe0s_env = PRF_ENV_GET(FFE0S, ffe0s);

        // Check provided values
        if((param->conidx > BLE_CONNECTION_MAX)
            || (gapc_get_conhdl(param->conidx) == GAP_INVALID_CONHDL))
        {
            // an error occurs, trigg it.
            struct ayla_enable_rsp* rsp = KERNEL_MSG_ALLOC(AYLA_ENABLE_RSP, src_id,
                dest_id, ayla_enable_rsp);
            rsp->conidx = param->conidx;
            rsp->status = (param->conidx > BLE_CONNECTION_MAX) ? GAP_ERR_INVALID_PARAM : PRF_ERR_REQ_DISALLOWED;
            kernel_msg_send(rsp);

            msg_status = KERNEL_MSG_CONSUMED;
        }
       
    }

    return msg_status;
}

static void ayla_ntf_ffa1_val(struct ayla_env_tag* ayla_env,struct ayla_ffa1_value_upd_req const *param)
{
	
	//UART_PRINTF("%s\r\n",__func__);
    //Allocate the GATT notification message
    struct gattc_send_evt_cmd *ayla_value = KERNEL_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
            KERNEL_BUILD_ID(TASK_GATTC, 0), prf_src_task_get(&(ayla_env->prf_env),0),
            gattc_send_evt_cmd, param->length);

    //Fill in the parameter structure
    ayla_value->operation = GATTC_NOTIFY ;//GATTC_INDICATE;
    ayla_value->handle = ayla_get_att_handle(AYLA_IDX_FFA1_VAL_VALUE);
    // pack measured value in database
    ayla_value->length = param->length;
		
	ayla_value->seq_num = param->seq_num;
  
	memcpy(&ayla_value->value[0],param->value,ayla_value->length);
		
    //send notification to peer device
    kernel_msg_send(ayla_value);
}

static int ayla_ffa1_value_upd_req_handler(kernel_msg_id_t const msgid,
                                            struct ayla_ffa1_value_upd_req const *param,
                                            kernel_task_id_t const dest_id,
                                            kernel_task_id_t const src_id)
{
	// UART_PRINTF("%s\r\n",__func__);
    int msg_status = KERNEL_MSG_CONSUMED;
    uint8_t state = kernel_state_get(dest_id);
	
    // check state of the task
    if(state == AYLA_IDLE)
    {
        struct ayla_env_tag* ayla_env = PRF_ENV_GET(AYLA, ayla);
	
        // update the battery level value
        kernel_state_set(dest_id, AYLA_BUSY);       

		ayla_ntf_ffa1_val(ayla_env, param);
		kernel_state_set(dest_id, AYLA_IDLE);   
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
    printf("gattc_att_info_req_ind_handler\r\n");
    uint8_t status = ayla_get_att_idx(param->handle, &att_idx);
    //Send write response
    cfm = KERNEL_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);
    cfm->handle = param->handle;

    if(status == GAP_ERR_NO_ERROR)
    {
        // check if it's a client configuration charms
        if(att_idx == AYLA_IDX_FFA1_VAL_VALUE)
        {
            // attribute length = 2
            cfm->length = 128;
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
    struct gattc_write_cfm * cfm;
    uint8_t att_idx = 0;
    uint8_t conidx = KERNEL_IDX_GET(src_id);
    // retrieve handle information
    printf("gattc_write_req_ind_handler\r\n");
    uint8_t status = ayla_get_att_idx(param->handle,  &att_idx);
    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        struct ayla_env_tag* ayla_env = PRF_ENV_GET(AYLA, ayla);
        // Extract value before check
        uint16_t ntf_cfg = common_read16p(&param->value[0]);

        // Only update configuration if value for stop or notification enable
        if ((att_idx == AYLA_IDX_FFA1_VAL_NTF_CFG)
                && ((ntf_cfg == PRF_CLI_STOP_NTFIND) || (ntf_cfg == PRF_CLI_START_NTF)))
        {

            // Conserve information in environment
            if (ntf_cfg == PRF_CLI_START_NTF)
            {
                // Ntf cfg bit set to 1
                ayla_env->ntf_cfg[conidx] |= (AYLA_FFA1_LVL_NTF_SUP );
            }
            else
            {
                // Ntf cfg bit set to 0
                ayla_env->ntf_cfg[conidx] &= ~(AYLA_FFA1_LVL_NTF_SUP );
            }

            

            // Inform APP of configuration change
            struct ayla_ffa1_value_ntf_cfg_ind * ind = KERNEL_MSG_ALLOC(AYLA_FFA1_VALUE_NTF_CFG_IND,
                    prf_dst_task_get(&(ayla_env->prf_env), conidx), dest_id,
                    ayla_ffa1_value_ntf_cfg_ind);
            ind->conidx = conidx;
            ind->ntf_cfg = ayla_env->ntf_cfg[conidx];
						
            kernel_msg_send(ind);
						
					
        }
		else if (att_idx == AYLA_IDX_FFA1_VAL_VALUE)
		{
        	// Allocate the alert value change indication
        	struct ayla_ffa1_write_ind *ind = KERNEL_MSG_ALLOC_DYN(AYLA_FFA1_WRITE_IND,
                prf_dst_task_get(&(ayla_env->prf_env), conidx),
                dest_id, ayla_ffa1_write_ind,param->length);
			
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
    struct gattc_read_cfm * cfm;
    uint8_t  att_idx = 0;
    uint8_t conidx = KERNEL_IDX_GET(src_id);
    // retrieve handle information
    printf("gattc_read_req_ind_handler\r\n");
    uint8_t status = ayla_get_att_idx(param->handle, &att_idx);

    uint16_t length = 0;
    struct ayla_env_tag* ayla_env = PRF_ENV_GET(AYLA, ayla);
//	UART_PRINTF("att_idx = %d\r\n",att_idx);
    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        if (att_idx == AYLA_IDX_FFA1_VAL_NTF_CFG)
        {
            
			length = 2;		
			 //Send read response
			cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, length);
			cfm->handle = param->handle;
			cfm->status = status;
			cfm->length = length;
			
			uint16_t ntf_cfg = (ayla_env->ntf_cfg[conidx] & AYLA_FFA1_LVL_NTF_SUP) ? PRF_CLI_START_IND: PRF_CLI_STOP_NTFIND;
			common_write16p(cfm->value, ntf_cfg);
			
		 	kernel_msg_send(cfm);
        }
		
       
    }


    return (KERNEL_MSG_CONSUMED);
}   




static int gattc_cmp_evt_handler(kernel_msg_id_t const msgid,  struct gattc_cmp_evt const *param,
                                 kernel_task_id_t const dest_id, kernel_task_id_t const src_id)
{
  	//UART_PRINTF("%s\r\n",__func__);
  	struct ayla_env_tag*ayla_env = PRF_ENV_GET(AYLA, ayla);
    if(param->operation == GATTC_INDICATE)
    {
        // continue operation execution
		struct gattc_cmp_evt *evt = KERNEL_MSG_ALLOC(AYLA_GATTC_CMP_EVT,
        prf_dst_task_get(&(ayla_env->prf_env), 0),
        dest_id, gattc_cmp_evt);
			
		evt->operation = param->operation;
		evt->status = param->status;
		evt->seq_num = param->seq_num;
				
		kernel_state_set(dest_id, AYLA_IDLE); 
		kernel_msg_send(evt);					
    }
		
    return (KERNEL_MSG_CONSUMED);
}

/// Default State handlers definition
const struct kernel_msg_handler ayla_default_state[] =
{
    {AYLA_ENABLE_REQ,                 (kernel_msg_func_t) ayla_enable_req_handler},
    {AYLA_FFA1_VALUE_UPD_REQ,         (kernel_msg_func_t) ayla_ffa1_value_upd_req_handler},
    {GATTC_ATT_INFO_REQ_IND,        (kernel_msg_func_t) gattc_att_info_req_ind_handler},
    {GATTC_WRITE_REQ_IND,           (kernel_msg_func_t) gattc_write_req_ind_handler},
    {GATTC_READ_REQ_IND,            (kernel_msg_func_t) gattc_read_req_ind_handler},
    {GATTC_CMP_EVT,                 (kernel_msg_func_t) gattc_cmp_evt_handler},
};


/// Specifies the message handlers that are common to all states.
const struct kernel_state_handler ayla_default_handler = KERNEL_STATE_HANDLER(ayla_default_state);

#endif /* #if (BLE_AYLA_SERVER) */


 
