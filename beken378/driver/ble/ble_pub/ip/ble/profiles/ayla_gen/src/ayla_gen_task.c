/**
 ****************************************************************************************
 *
 * @file   ayla_gen_task.c
 *
 * @brief AYLA_GEN Server Role Task Implementation.
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

#if (BLE_AYLA_GEN_SERVER)

#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "atts.h"
#include "common_utils.h"
#include "kernel_mem.h"
#include "ayla_gen.h"
#include "ayla_gen_task.h"

#include "prf_utils.h"
#include "ble_pub.h"

static int ayla_gen_enable_req_handler(kernel_msg_id_t const msgid,
                                   struct ayla_gen_enable_req const *param,
                                   kernel_task_id_t const dest_id,
                                   kernel_task_id_t const src_id)
{
    int msg_status = KERNEL_MSG_SAVED;
    uint8_t state = kernel_state_get(dest_id);

    // check state of the task
    if(state == AYLA_GEN_IDLE)
    {
      //  struct ffe0s_env_tag* ffe0s_env = PRF_ENV_GET(FFE0S, ffe0s);

        // Check provided values
        if((param->conidx > BLE_CONNECTION_MAX)
            || (gapc_get_conhdl(param->conidx) == GAP_INVALID_CONHDL))
        {
            // an error occurs, trigg it.
            struct ayla_gen_enable_rsp* rsp = KERNEL_MSG_ALLOC(AYLA_GEN_ENABLE_RSP, src_id,
                dest_id, ayla_gen_enable_rsp);
            rsp->conidx = param->conidx;
            rsp->status = (param->conidx > BLE_CONNECTION_MAX) ? GAP_ERR_INVALID_PARAM : PRF_ERR_REQ_DISALLOWED;
            kernel_msg_send(rsp);

            msg_status = KERNEL_MSG_CONSUMED;
        }
       
    }

    return msg_status;
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
    uint8_t status = ayla_gen_get_att_idx(param->handle, &att_idx);
    //Send write response
    cfm = KERNEL_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);
    cfm->handle = param->handle;

    if(status == GAP_ERR_NO_ERROR)
    {
        // check if it's a client configuration charms
        if((att_idx == AYLA_GEN_IDX_IDID_VAL_VALUE) || (att_idx == AYLA_GEN_IDX_DNID_VAL_VALUE))
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

static int gattc_read_req_ind_handler(kernel_msg_id_t const msgid, struct gattc_read_req_ind const *param,
                                      kernel_task_id_t const dest_id, kernel_task_id_t const src_id)
{
//	UART_PRINTF("%s\r\n",__func__);
    struct gattc_read_cfm * cfm;
    uint8_t  att_idx = 0;
    uint8_t conidx = KERNEL_IDX_GET(src_id);
    // retrieve handle information
    printf("gattc_read_req_ind_handler\r\n");
    uint8_t status = ayla_gen_get_att_idx(param->handle, &att_idx);

    uint16_t length = 0;
    struct ayla_gen_env_tag* ayla_gen_env = PRF_ENV_GET(AYLA_GEN, ayla_gen);
//	UART_PRINTF("att_idx = %d\r\n",att_idx);
    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        if (att_idx == AYLA_GEN_IDX_DUID_VAL_VALUE)
        {
			if(ble_read_cb != NULL)
            {
                length = ble_read_cb(0x0001,ayla_gen_env->rd_value,sizeof(ayla_gen_env->rd_value));
            }

            bk_printf("length:%d\r\n", length);
            for(int i = 0; i < length; i++)
            {
                bk_printf("0x%x ", ayla_gen_env->rd_value[i]);
            }
            bk_printf("\r\n");
			
			cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, length);
            cfm->length = length;
			cfm->handle = param->handle;
			cfm->status = status;
			memcpy(cfm->value,ayla_gen_env->rd_value,cfm->length);
			kernel_msg_send(cfm);
        }
        else if (att_idx == AYLA_GEN_IDX_OEMID_VAL_VALUE)
        {
			if(ble_read_cb != NULL)
            {
                length = ble_read_cb(0x0002,ayla_gen_env->rd_value,sizeof(ayla_gen_env->rd_value));
            }
			
			cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, length);
            cfm->length = length;
			cfm->handle = param->handle;
			cfm->status = status;
			memcpy(cfm->value,ayla_gen_env->rd_value,cfm->length);
			kernel_msg_send(cfm);
        }
        else if (att_idx == AYLA_GEN_IDX_OEMMID_VAL_VALUE)
        {
			if(ble_read_cb != NULL)
            {
                length = ble_read_cb(0x0003,ayla_gen_env->rd_value,sizeof(ayla_gen_env->rd_value));
            }
			
			cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, length);
            cfm->length = length;
			cfm->handle = param->handle;
			cfm->status = status;
			memcpy(cfm->value,ayla_gen_env->rd_value,cfm->length);
			kernel_msg_send(cfm);
        }
        else if (att_idx == AYLA_GEN_IDX_TVID_VAL_VALUE)
        {
			if(ble_read_cb != NULL)
            {
                length = ble_read_cb(0x0004,ayla_gen_env->rd_value,sizeof(ayla_gen_env->rd_value));
            }
			
			cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, length);
            cfm->length = length;
			cfm->handle = param->handle;
			cfm->status = status;
			memcpy(cfm->value,ayla_gen_env->rd_value,cfm->length);
			kernel_msg_send(cfm);
        }
        else if (att_idx == AYLA_GEN_IDX_DNID_VAL_VALUE)
        {
			if(ble_read_cb != NULL)
            {
                length = ble_read_cb(0x0006,ayla_gen_env->rd_value,sizeof(ayla_gen_env->rd_value));
            }
			
			cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, length);
            cfm->length = length;
			cfm->handle = param->handle;
			cfm->status = status;
			memcpy(cfm->value,ayla_gen_env->rd_value,cfm->length);
			kernel_msg_send(cfm);
        }
        else
        {	  
            cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, 1);
            cfm->handle = param->handle;
            cfm->status = status;
            cfm->length = 0;
            kernel_msg_send(cfm);
		}
    }

    return (KERNEL_MSG_CONSUMED);
}   

static int gattc_write_req_ind_handler(kernel_msg_id_t const msgid, struct gattc_write_req_ind const *param,
                                      kernel_task_id_t const dest_id, kernel_task_id_t const src_id)
{
    struct gattc_write_cfm * cfm;
    uint8_t att_idx = 0;
    uint8_t conidx = KERNEL_IDX_GET(src_id);
    // retrieve handle information
    uint8_t status = ayla_gen_get_att_idx(param->handle,  &att_idx);
    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        struct ayla_gen_env_tag* ayla_gen_env = PRF_ENV_GET(AYLA_GEN, ayla_gen);
        // Extract value before check
        
		if (att_idx == AYLA_GEN_IDX_IDID_VAL_VALUE)
		{
        	// Allocate the alert value change indication
        	struct ayla_gen_idid_write_ind *ind = KERNEL_MSG_ALLOC_DYN(AYLA_GEN_IDID_WRITE_IND,
                prf_dst_task_get(&(ayla_gen_env->prf_env), conidx),
                dest_id, ayla_gen_idid_write_ind,param->length);
			
        	// Fill in the parameter structure	
			memcpy(ind->value,&param->value[0],param->length);
			ind->conidx = conidx;
			ind->length = param->length;
			// Send the message
			kernel_msg_send(ind);
		}
        else if (att_idx == AYLA_GEN_IDX_DNID_VAL_VALUE)
		{
        	// Allocate the alert value change indication
        	struct ayla_gen_dnid_write_ind *ind = KERNEL_MSG_ALLOC_DYN(AYLA_GEN_DNID_WRITE_IND,
                prf_dst_task_get(&(ayla_gen_env->prf_env), conidx),
                dest_id, ayla_gen_dnid_write_ind,param->length);
			
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

static int gattc_cmp_evt_handler(kernel_msg_id_t const msgid,  struct gattc_cmp_evt const *param,
                                 kernel_task_id_t const dest_id, kernel_task_id_t const src_id)
{
  	//UART_PRINTF("%s\r\n",__func__);
  	struct ayla_gen_env_tag*ayla_gen_env = PRF_ENV_GET(AYLA_GEN, ayla_gen);
    if(param->operation == GATTC_INDICATE)
    {
        // continue operation execution
		struct gattc_cmp_evt *evt = KERNEL_MSG_ALLOC(AYLA_GEN_GATTC_CMP_EVT,
        prf_dst_task_get(&(ayla_gen_env->prf_env), 0),
        dest_id, gattc_cmp_evt);
			
		evt->operation = param->operation;
		evt->status = param->status;
		evt->seq_num = param->seq_num;
				
		kernel_state_set(dest_id, AYLA_GEN_IDLE); 
		kernel_msg_send(evt);					
    }
		
    return (KERNEL_MSG_CONSUMED);
}

/// Default State handlers definition
const struct kernel_msg_handler ayla_gen_default_state[] =
{
    {AYLA_GEN_ENABLE_REQ,           (kernel_msg_func_t) ayla_gen_enable_req_handler},
    {GATTC_ATT_INFO_REQ_IND,        (kernel_msg_func_t) gattc_att_info_req_ind_handler},
    {GATTC_READ_REQ_IND,            (kernel_msg_func_t) gattc_read_req_ind_handler},
    {GATTC_WRITE_REQ_IND,            (kernel_msg_func_t) gattc_write_req_ind_handler},
    {GATTC_CMP_EVT,                 (kernel_msg_func_t) gattc_cmp_evt_handler},
};


/// Specifies the message handlers that are common to all states.
const struct kernel_state_handler ayla_gen_default_handler = KERNEL_STATE_HANDLER(ayla_gen_default_state);

#endif /* #if (BLE_AYLA_GEN_SERVER) */


 
