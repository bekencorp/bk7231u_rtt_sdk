/**
 ****************************************************************************************
 *
 * @file   sn_task.c
 *
 * @brief SN Server Role Task Implementation.
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

#if (BLE_SN_SERVER)

#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "atts.h"
#include "common_utils.h"
#include "kernel_mem.h"
#include "sn.h"
#include "sn_task.h"

#include "prf_utils.h"

static int sn_enable_req_handler(kernel_msg_id_t const msgid,
                                   struct sn_enable_req const *param,
                                   kernel_task_id_t const dest_id,
                                   kernel_task_id_t const src_id)
{
    int msg_status = KERNEL_MSG_SAVED;
    uint8_t state = kernel_state_get(dest_id);

    // check state of the task
    if(state == SN_IDLE)
    {
      //  struct ffe0s_env_tag* ffe0s_env = PRF_ENV_GET(FFE0S, ffe0s);

        // Check provided values
        if((param->conidx > BLE_CONNECTION_MAX)
            || (gapc_get_conhdl(param->conidx) == GAP_INVALID_CONHDL))
        {
            // an error occurs, trigg it.
            struct sn_enable_rsp* rsp = KERNEL_MSG_ALLOC(SN_ENABLE_RSP, src_id,
                dest_id, sn_enable_rsp);
            rsp->conidx = param->conidx;
            rsp->status = (param->conidx > BLE_CONNECTION_MAX) ? GAP_ERR_INVALID_PARAM : PRF_ERR_REQ_DISALLOWED;
            kernel_msg_send(rsp);

            msg_status = KERNEL_MSG_CONSUMED;
        }
       
    }

    return msg_status;
}

static void sn_ntf_5301_val(struct sn_env_tag* sn_env,struct sn_5301_value_upd_req const *param)
{
	
	//UART_PRINTF("%s\r\n",__func__);
    //Allocate the GATT notification message
    struct gattc_send_evt_cmd *sn_value = KERNEL_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
            KERNEL_BUILD_ID(TASK_GATTC, 0), prf_src_task_get(&(sn_env->prf_env),0),
            gattc_send_evt_cmd, param->length);

    //Fill in the parameter structure
    sn_value->operation = GATTC_NOTIFY ;//GATTC_INDICATE;
    sn_value->handle = sn_get_att_handle(SN_IDX_5301_VAL_VALUE);
    // pack measured value in database
    sn_value->length = param->length;
		
	sn_value->seq_num = param->seq_num;
  
	memcpy(&sn_value->value[0],param->value,sn_value->length);
		
    //send notification to peer device
    kernel_msg_send(sn_value);
}

static int sn_5301_value_upd_req_handler(kernel_msg_id_t const msgid,
                                            struct sn_5301_value_upd_req const *param,
                                            kernel_task_id_t const dest_id,
                                            kernel_task_id_t const src_id)
{
	// UART_PRINTF("%s\r\n",__func__);
    int msg_status = KERNEL_MSG_CONSUMED;
    uint8_t state = kernel_state_get(dest_id);
	
    // check state of the task
    if(state == SN_IDLE)
    {
        struct sn_env_tag* sn_env = PRF_ENV_GET(SN, sn);
	
        // update the battery level value
        kernel_state_set(dest_id, SN_BUSY);       

		sn_ntf_5301_val(sn_env, param);
		kernel_state_set(dest_id, SN_IDLE);   
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
    uint8_t status = sn_get_att_idx(param->handle, &att_idx);
    //Send write response
    cfm = KERNEL_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);
    cfm->handle = param->handle;

    if(status == GAP_ERR_NO_ERROR)
    {
        // check if it's a client configuration charms
        if(att_idx == SN_IDX_5302_VAL_VALUE)
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
    uint8_t status = sn_get_att_idx(param->handle,  &att_idx);
    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        struct sn_env_tag* sn_env = PRF_ENV_GET(SN, sn);
        // Extract value before check
        uint16_t ntf_cfg = common_read16p(&param->value[0]);

        // Only update configuration if value for stop or notification enable
        if ((att_idx == SN_IDX_5301_VAL_NTF_CFG)
                && ((ntf_cfg == PRF_CLI_STOP_NTFIND) || (ntf_cfg == PRF_CLI_START_NTF)))
        {

            // Conserve information in environment
            if (ntf_cfg == PRF_CLI_START_NTF)
            {
                // Ntf cfg bit set to 1
                sn_env->ntf_cfg[conidx] |= (SN_5301_LVL_NTF_SUP );
            }
            else
            {
                // Ntf cfg bit set to 0
                sn_env->ntf_cfg[conidx] &= ~(SN_5301_LVL_NTF_SUP );
            }

            

            // Inform APP of configuration change
            struct sn_5301_value_ntf_cfg_ind * ind = KERNEL_MSG_ALLOC(SN_5301_VALUE_NTF_CFG_IND,
                    prf_dst_task_get(&(sn_env->prf_env), conidx), dest_id,
                    sn_5301_value_ntf_cfg_ind);
            ind->conidx = conidx;
            ind->ntf_cfg = sn_env->ntf_cfg[conidx];
						
            kernel_msg_send(ind);
						
					
        }
		else if (att_idx == SN_IDX_5302_VAL_VALUE)
		{
        	// Allocate the alert value change indication
        	struct sn_5302_write_ind *ind = KERNEL_MSG_ALLOC_DYN(SN_5302_WRITE_IND,
                prf_dst_task_get(&(sn_env->prf_env), conidx),
                dest_id, sn_5302_write_ind,param->length);
			
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
    uint8_t status = sn_get_att_idx(param->handle, &att_idx);

    uint16_t length = 0;
    struct sn_env_tag* sn_env = PRF_ENV_GET(SN, sn);
//	UART_PRINTF("att_idx = %d\r\n",att_idx);
    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        if (att_idx == SN_IDX_5301_VAL_NTF_CFG)
        {
            
			length = 2;		
			 //Send read response
			cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, length);
			cfm->handle = param->handle;
			cfm->status = status;
			cfm->length = length;
			
			uint16_t ntf_cfg = (sn_env->ntf_cfg[conidx] & SN_5301_LVL_NTF_SUP) ? PRF_CLI_START_IND: PRF_CLI_STOP_NTFIND;
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
  	struct sn_env_tag*sn_env = PRF_ENV_GET(SN, sn);
    if(param->operation == GATTC_INDICATE)
    {
        // continue operation execution
		struct gattc_cmp_evt *evt = KERNEL_MSG_ALLOC(SN_GATTC_CMP_EVT,
        prf_dst_task_get(&(sn_env->prf_env), 0),
        dest_id, gattc_cmp_evt);
			
		evt->operation = param->operation;
		evt->status = param->status;
		evt->seq_num = param->seq_num;
				
		kernel_state_set(dest_id, SN_IDLE); 
		kernel_msg_send(evt);					
    }
		
    return (KERNEL_MSG_CONSUMED);
}

/// Default State handlers definition
const struct kernel_msg_handler sn_default_state[] =
{
    {SN_ENABLE_REQ,                 (kernel_msg_func_t) sn_enable_req_handler},
    {SN_5301_VALUE_UPD_REQ,         (kernel_msg_func_t) sn_5301_value_upd_req_handler},
    {GATTC_ATT_INFO_REQ_IND,        (kernel_msg_func_t) gattc_att_info_req_ind_handler},
    {GATTC_WRITE_REQ_IND,           (kernel_msg_func_t) gattc_write_req_ind_handler},
    {GATTC_READ_REQ_IND,            (kernel_msg_func_t) gattc_read_req_ind_handler},
    {GATTC_CMP_EVT,                 (kernel_msg_func_t) gattc_cmp_evt_handler},
};


/// Specifies the message handlers that are common to all states.
const struct kernel_state_handler sn_default_handler = KERNEL_STATE_HANDLER(sn_default_state);

#endif /* #if (BLE_SN_SERVER) */


 
