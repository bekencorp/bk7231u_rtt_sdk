/**
 ****************************************************************************************
 *
 * @file   fff0s_task.c
 *
 * @brief FFF0 Server Role Task Implementation.
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

#if (BLE_FFF0_SERVER)

#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "atts.h"
#include "common_utils.h"
#include "fff0s.h"
#include "fff0s_task.h"

#include "prf_utils.h"


static int fff0s_enable_req_handler(kernel_msg_id_t const msgid,
                                   struct fff0s_enable_req const *param,
                                   kernel_task_id_t const dest_id,
                                   kernel_task_id_t const src_id)
{
    int msg_status = KERNEL_MSG_SAVED;
    uint8_t state = kernel_state_get(dest_id);

    // check state of the task
    if(state == FFF0S_IDLE)
    {
        struct fff0s_env_tag* fff0s_env = PRF_ENV_GET(FFF0S, fff0s);

        // Check provided values
        if((param->conidx > BLE_CONNECTION_MAX)
            || (gapc_get_conhdl(param->conidx) == GAP_INVALID_CONHDL))
        {
            // an error occurs, trigg it.
            struct fff0s_enable_rsp* rsp = KERNEL_MSG_ALLOC(FFF0S_ENABLE_RSP, src_id,
                dest_id, fff0s_enable_rsp);
            rsp->conidx = param->conidx;
            rsp->status = (param->conidx > BLE_CONNECTION_MAX) ? GAP_ERR_INVALID_PARAM : PRF_ERR_REQ_DISALLOWED;
            kernel_msg_send(rsp);

            msg_status = KERNEL_MSG_CONSUMED;
        }
        else
        {
            // put task in a busy state
            msg_status = KERNEL_MSG_NO_FREE;
            kernel_state_set(dest_id, FFF0S_BUSY);
            fff0s_env->ntf_cfg[param->conidx] = param->ntf_cfg;
            fff0s_env->operation = kernel_param2msg(param);

            // trigger notification
            fff0s_exe_operation();
        }
    }

    return msg_status;
}



static int fff0s_fff1_level_upd_req_handler(kernel_msg_id_t const msgid,
                                            struct fff0s_fff1_level_upd_req const *param,
                                            kernel_task_id_t const dest_id,
                                            kernel_task_id_t const src_id)
{
    int msg_status = KERNEL_MSG_SAVED;
    uint8_t state = kernel_state_get(dest_id);
	
    // check state of the task
    if(state == FFF0S_IDLE)
    {
        struct fff0s_env_tag* fff0s_env = PRF_ENV_GET(FFF0S, fff0s);

        // Check provided values
        if((param->fff1_level[0] <= FFF1_LVL_MAX))
        {
            // update the battery level value
			memcpy(&fff0s_env->fff1_lvl[0],&param->fff1_level[0],param->length);
            // put task in a busy state
            msg_status = KERNEL_MSG_NO_FREE;
            kernel_state_set(dest_id, FFF0S_BUSY);
            fff0s_env->operation = kernel_param2msg(param);
          						
            // trigger notification
            fff0s_exe_operation();		 
        }
        else
        {					
            // an error occurs, trigg it.
            struct fff0s_fff1_level_upd_rsp * rsp = KERNEL_MSG_ALLOC(FFF0S_FFF1_LEVEL_UPD_RSP, src_id,
                    dest_id, fff0s_fff1_level_upd_rsp);

            rsp->status = PRF_ERR_INVALID_PARAM;
            kernel_msg_send(rsp);
            msg_status = KERNEL_MSG_CONSUMED;
        }
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
    uint8_t status = fff0s_get_att_idx(param->handle, &att_idx);

    //Send write response
    cfm = KERNEL_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);
    cfm->handle = param->handle;

    if(status == GAP_ERR_NO_ERROR)
    {
        // check if it's a client configuration char
        if(att_idx == FFF0S_IDX_FFF1_LVL_NTF_CFG)
        {
            // CCC attribute length = 2
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
    struct gattc_write_cfm * cfm;
    uint8_t att_idx = 0;
    uint8_t conidx = KERNEL_IDX_GET(src_id);
    // retrieve handle information
    uint8_t status = fff0s_get_att_idx(param->handle,  &att_idx);
		
    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        struct fff0s_env_tag* fff0s_env = PRF_ENV_GET(FFF0S, fff0s);
        // Extract value before check
        uint16_t ntf_cfg = common_read16p(&param->value[0]);

        // Only update configuration if value for stop or notification enable
        if ((att_idx == FFF0S_IDX_FFF1_LVL_NTF_CFG)
                && ((ntf_cfg == PRF_CLI_STOP_NTFIND) || (ntf_cfg == PRF_CLI_START_NTF)))
        {

            // Conserve information in environment
            if (ntf_cfg == PRF_CLI_START_NTF)
            {
                // Ntf cfg bit set to 1
                fff0s_env->ntf_cfg[conidx] |= (FFF0_FFF1_LVL_NTF_SUP );
            }
            else
            {
                // Ntf cfg bit set to 0
                fff0s_env->ntf_cfg[conidx] &= ~(FFF0_FFF1_LVL_NTF_SUP );
            }

            // Inform APP of configuration change
            struct fff0s_fff1_level_ntf_cfg_ind * ind = KERNEL_MSG_ALLOC(FFF0S_FFF1_LEVEL_NTF_CFG_IND,
                    prf_dst_task_get(&(fff0s_env->prf_env), conidx), dest_id,
                    fff0s_fff1_level_ntf_cfg_ind);
            ind->conidx = conidx;
            ind->ntf_cfg = fff0s_env->ntf_cfg[conidx];
						
            kernel_msg_send(ind);			
        }
		else if (att_idx == FFF0S_IDX_FFF2_LVL_VAL)
		{
			// Allocate the alert value change indication
			struct fff0s_fff2_writer_ind *ind = KERNEL_MSG_ALLOC(FFF0S_FFF2_WRITER_REQ_IND,
			        prf_dst_task_get(&(fff0s_env->prf_env), conidx),
			        dest_id, fff0s_fff2_writer_ind);
			
			// Fill in the parameter structure	
			memcpy(ind->fff2_value,&param->value[0],param->length);
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
    struct gattc_read_cfm * cfm;
    uint8_t  att_idx = 0;
    uint8_t conidx = KERNEL_IDX_GET(src_id);
    // retrieve handle information
    uint8_t status = fff0s_get_att_idx(param->handle, &att_idx);
    uint16_t length = 0;
    struct fff0s_env_tag* fff0s_env = PRF_ENV_GET(FFF0S, fff0s);

    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        // read notification information
        if (att_idx == FFF0S_IDX_FFF1_LVL_VAL)
        {
            length = FFF0_FFF1_DATA_LEN * sizeof(uint8_t);
        }
        // read notification information
        else if (att_idx == FFF0S_IDX_FFF1_LVL_NTF_CFG)
        {
            length = sizeof(uint16_t);
        }
        
        else
        {
            status = PRF_APP_ERROR;
        }
    }

    //Send write response
    cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, length);
    cfm->handle = param->handle;
    cfm->status = status;
    cfm->length = length;
   
    if (status == GAP_ERR_NO_ERROR)
    {
        // read notification information
        if (att_idx == FFF0S_IDX_FFF1_LVL_VAL)
        {
            cfm->value[0] = fff0s_env->fff1_lvl[0];
        }
        // retrieve notification config
        else if (att_idx == FFF0S_IDX_FFF1_LVL_NTF_CFG)
        {
            uint16_t ntf_cfg = (fff0s_env->ntf_cfg[conidx] & FFF0_FFF1_LVL_NTF_SUP) ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;
            common_write16p(cfm->value, ntf_cfg);
        }  
        else
        {
            /* Not Possible */
        }
    }

    kernel_msg_send(cfm);

    return (KERNEL_MSG_CONSUMED);
}   


static int gattc_cmp_evt_handler(kernel_msg_id_t const msgid,  struct gattc_cmp_evt const *param,
                                 kernel_task_id_t const dest_id, kernel_task_id_t const src_id)
{
    if(param->operation == GATTC_NOTIFY)
    {
        //continue operation execution
      	//fff0s_exe_operation();
    }
    return (KERNEL_MSG_CONSUMED);
}

/// Default State handlers definition
const struct kernel_msg_handler fff0s_default_state[] =
{
    {FFF0S_ENABLE_REQ,              (kernel_msg_func_t) fff0s_enable_req_handler},
    {FFF0S_FFF1_LEVEL_UPD_REQ,      (kernel_msg_func_t) fff0s_fff1_level_upd_req_handler},
    {GATTC_ATT_INFO_REQ_IND,        (kernel_msg_func_t) gattc_att_info_req_ind_handler},
    {GATTC_WRITE_REQ_IND,           (kernel_msg_func_t) gattc_write_req_ind_handler},
    {GATTC_READ_REQ_IND,            (kernel_msg_func_t) gattc_read_req_ind_handler},
    {GATTC_CMP_EVT,                 (kernel_msg_func_t) gattc_cmp_evt_handler},
};

/// Specifies the message handlers that are common to all states.
const struct kernel_state_handler fff0s_default_handler = KERNEL_STATE_HANDLER(fff0s_default_state);

#endif /* #if (BLE_FFF0_SERVER) */


