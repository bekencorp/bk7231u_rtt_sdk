/**
 ****************************************************************************************
 *
 * @file   ayla_wifi_task.c
 *
 * @brief AYLA_WIFI Server Role Task Implementation.
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

#if (BLE_AYLA_WIFI_SERVER)

#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "atts.h"
#include "common_utils.h"
#include "kernel_mem.h"
#include "ayla_wifi.h"
#include "ayla_wifi_task.h"

#include "prf_utils.h"
#include "ble_pub.h"

static int ayla_wifi_enable_req_handler(kernel_msg_id_t const msgid,
                                   struct ayla_wifi_enable_req const *param,
                                   kernel_task_id_t const dest_id,
                                   kernel_task_id_t const src_id)
{
    int msg_status = KERNEL_MSG_SAVED;
    uint8_t state = kernel_state_get(dest_id);

    // check state of the task
    if(state == AYLA_WIFI_IDLE)
    {
      //  struct ffe0s_env_tag* ffe0s_env = PRF_ENV_GET(FFE0S, ffe0s);

        // Check provided values
        if((param->conidx > BLE_CONNECTION_MAX)
            || (gapc_get_conhdl(param->conidx) == GAP_INVALID_CONHDL))
        {
            // an error occurs, trigg it.
            struct ayla_wifi_enable_rsp* rsp = KERNEL_MSG_ALLOC(AYLA_WIFI_ENABLE_RSP, src_id,
                dest_id, ayla_wifi_enable_rsp);
            rsp->conidx = param->conidx;
            rsp->status = (param->conidx > BLE_CONNECTION_MAX) ? GAP_ERR_INVALID_PARAM : PRF_ERR_REQ_DISALLOWED;
            kernel_msg_send(rsp);

            msg_status = KERNEL_MSG_CONSUMED;
        }
       
    }

    return msg_status;
}

static void ayla_wifi_statu_ntf_val(struct ayla_wifi_env_tag* ayla_wifi_env,struct ayla_wifi_statu_ntf_upd_req const *param)
{
	
	//  UART_PRINTF("%s\r\n",__func__);
    //Allocate the GATT notification message
    struct gattc_send_evt_cmd *statu_value = KERNEL_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
            KERNEL_BUILD_ID(TASK_GATTC, 0), prf_src_task_get(&(ayla_wifi_env->prf_env),0),
            gattc_send_evt_cmd, param->length);

    //Fill in the parameter structure
    statu_value->operation = GATTC_NOTIFY;
    statu_value->handle = ayla_wifi_get_att_handle(AYLA_WIFI_IDX_STATU_VAL_VALUE);
    // pack measured value in databaseS
    statu_value->length = param->length;
    statu_value->seq_num = param->seq_num;
  
    //UART_PRINTF("fed6_value->handle = 0x%x\r\n",fed6_value->handle);
    memcpy(&statu_value->value[0],param->value,statu_value->length);

    //send notification to peer device
    kernel_msg_send(statu_value);
}


static int ayla_wifi_statu_ntf_upd_req_handler(kernel_msg_id_t const msgid,
                                            struct ayla_wifi_statu_ntf_upd_req const *param,
                                            kernel_task_id_t const dest_id,
                                            kernel_task_id_t const src_id)
{
    int msg_status = KERNEL_MSG_CONSUMED;
    uint8_t state = kernel_state_get(dest_id);
	
    // check state of the task
    if(state == AYLA_WIFI_IDLE)
    {
        struct feb3s_env_tag* ayla_wifi_env = PRF_ENV_GET(AYLA_WIFI, ayla_wifi);
	
        // update the battery level value
        kernel_state_set(dest_id, AYLA_WIFI_BUSY);       

        ayla_wifi_statu_ntf_val(ayla_wifi_env, param);
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

static void ayla_wifi_scre_ntf_val(struct ayla_wifi_env_tag* ayla_wifi_env,struct ayla_wifi_scre_ntf_upd_req const *param)
{
	
	//  UART_PRINTF("%s\r\n",__func__);
    //Allocate the GATT notification message
    struct gattc_send_evt_cmd *scre_value = KERNEL_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
            KERNEL_BUILD_ID(TASK_GATTC, 0), prf_src_task_get(&(ayla_wifi_env->prf_env),0),
            gattc_send_evt_cmd, param->length);

    //Fill in the parameter structure
    scre_value->operation = GATTC_NOTIFY;
    scre_value->handle = ayla_wifi_get_att_handle(AYLA_WIFI_IDX_SCRE_VAL_VALUE);
    // pack measured value in databaseS
    scre_value->length = param->length;
    scre_value->seq_num = param->seq_num;
  
    //UART_PRINTF("fed6_value->handle = 0x%x\r\n",fed6_value->handle);
    memcpy(&scre_value->value[0],param->value,scre_value->length);

    //send notification to peer device
    kernel_msg_send(scre_value);
}

static int ayla_wifi_scre_ntf_upd_req_handler(kernel_msg_id_t const msgid,
                                            struct ayla_wifi_scre_ntf_upd_req const *param,
                                            kernel_task_id_t const dest_id,
                                            kernel_task_id_t const src_id)
{
    int msg_status = KERNEL_MSG_CONSUMED;
    uint8_t state = kernel_state_get(dest_id);
	
    // check state of the task
    if(state == AYLA_WIFI_IDLE)
    {
        struct feb3s_env_tag* ayla_wifi_env = PRF_ENV_GET(AYLA_WIFI, ayla_wifi);
	
        // update the battery level value
        kernel_state_set(dest_id, AYLA_WIFI_BUSY);       

        ayla_wifi_scre_ntf_val(ayla_wifi_env, param);
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

  
static int gattc_att_info_req_ind_handler(kernel_msg_id_t const msgid,
        struct gattc_att_info_req_ind *param,
        kernel_task_id_t const dest_id,
        kernel_task_id_t const src_id)
{

    struct gattc_att_info_cfm * cfm;
    uint8_t  att_idx = 0;
    // retrieve handle information
    printf("gattc_att_info_req_ind_handler\r\n");
    uint8_t status = ayla_wifi_get_att_idx(param->handle, &att_idx);
    //Send write response
    cfm = KERNEL_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);
    cfm->handle = param->handle;

    if(status == GAP_ERR_NO_ERROR)
    {
        // check if it's a client configuration charms
        if((att_idx == AYLA_WIFI_IDX_CONN_VAL_VALUE) || (att_idx == AYLA_WIFI_IDX_WPS_VAL_VALUE)
            || (att_idx == AYLA_WIFI_IDX_SCAN_VAL_VALUE) || (att_idx == AYLA_WIFI_IDX_STATU_VAL_NTF_CFG)
            || (att_idx == AYLA_WIFI_IDX_SCRE_VAL_VALUE) || (att_idx == AYLA_WIFI_IDX_SCRE_VAL_NTF_CFG))
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
    uint8_t status = ayla_wifi_get_att_idx(param->handle, &att_idx);

    uint16_t length = 0;
    struct ayla_wifi_env_tag* ayla_wifi_env = PRF_ENV_GET(AYLA_WIFI, ayla_wifi);
//	UART_PRINTF("att_idx = %d\r\n",att_idx);
    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        if (att_idx == AYLA_WIFI_IDX_STATU_VAL_VALUE)
        {
			if(ble_read_cb != NULL)
            {
                length = ble_read_cb(0xAF6C,ayla_wifi_env->rd_value,sizeof(ayla_wifi_env->rd_value));
            }

            bk_printf("length:%d\r\n", length);
            for(int i = 0; i < length; i++)
            {
                bk_printf("0x%x ", ayla_wifi_env->rd_value[i]);
            }
            bk_printf("\r\n");
			
			cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, length);
            cfm->length = length;
			cfm->handle = param->handle;
			cfm->status = status;
			memcpy(cfm->value,ayla_wifi_env->rd_value,cfm->length);
			kernel_msg_send(cfm);
        }
        else if (att_idx == AYLA_WIFI_IDX_SCRE_VAL_VALUE)
        {
			if(ble_read_cb != NULL)
            {
                length = ble_read_cb(0xAF6E,ayla_wifi_env->rd_value,sizeof(ayla_wifi_env->rd_value));
            }
			
			cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, length);
            cfm->length = length;
			cfm->handle = param->handle;
			cfm->status = status;
			memcpy(cfm->value,ayla_wifi_env->rd_value,cfm->length);
			kernel_msg_send(cfm);
        }
        else if (att_idx == AYLA_WIFI_IDX_STATU_VAL_NTF_CFG)
        {	
            length = 2;
			cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, length);
            cfm->length = length;
			cfm->handle = param->handle;
			cfm->status = status;
            uint16_t ntf_cfg = (ayla_wifi_env->statu_ntf_ind_cfg[conidx] & AYLA_WIFI_LVL_NTF_SUP) ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;
			common_write16p(cfm->value, ntf_cfg);
			kernel_msg_send(cfm);
        }
        else if (att_idx == AYLA_WIFI_IDX_SCRE_VAL_NTF_CFG)
        {
			length = 2;
			cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, length);
            cfm->length = length;
			cfm->handle = param->handle;
			cfm->status = status;
            uint16_t ntf_cfg = (ayla_wifi_env->scre_ntf_ind_cfg[conidx] & AYLA_WIFI_LVL_NTF_SUP) ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;
			common_write16p(cfm->value, ntf_cfg);
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
    uint8_t status = ayla_wifi_get_att_idx(param->handle,  &att_idx);
    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        struct ayla_wifi_env_tag* ayla_wifi_env = PRF_ENV_GET(AYLA_WIFI, ayla_wifi);
        // Extract value before check
        uint16_t ntf_cfg = common_read16p(&param->value[0]);
        
		if (att_idx == AYLA_WIFI_IDX_STATU_VAL_NTF_CFG)
		{
        	// Allocate the alert value change indication
        	struct ayla_wifi_statu_value_ntf_cfg_ind *ind = KERNEL_MSG_ALLOC_DYN(AYLA_WIFI_STATU_NTF_CFG_IND,
                prf_dst_task_get(&(ayla_wifi_env->prf_env), conidx),
                dest_id, ayla_wifi_statu_value_ntf_cfg_ind,param->length);

            if (ntf_cfg == PRF_CLI_START_IND)
            {		
							//UART_PRINTF("5 \r\n");
                // Ntf cfg bit set to 1
                ayla_wifi_env->statu_ntf_ind_cfg[conidx] |= (AYLA_WIFI_LVL_IND_SUP );
            }
            else if (ntf_cfg == PRF_CLI_START_NTF)
            {		
							//UART_PRINTF("5 \r\n");
                // Ntf cfg bit set to 1
                ayla_wifi_env->statu_ntf_ind_cfg[conidx] |= (AYLA_WIFI_LVL_NTF_SUP );
            }
            else
            {	
//							UART_PRINTF("6 \r\n");
                // Ntf cfg bit set to 0
                ayla_wifi_env->statu_ntf_ind_cfg[conidx] = AYLA_WIFI_LVL_NTF_NOT_SUP;
            }
			
        	// Fill in the parameter structure	
            ind->conidx = conidx;
            ind->handle = param->handle;
            ind->ntf_cfg = ayla_wifi_env->statu_ntf_ind_cfg[conidx];
			// Send the message
			kernel_msg_send(ind);
		}
        else if (att_idx == AYLA_WIFI_IDX_SCRE_VAL_NTF_CFG)
		{
        	// Allocate the alert value change indication
            struct ayla_wifi_scre_value_ntf_cfg_ind *ind = KERNEL_MSG_ALLOC_DYN(AYLA_WIFI_SCRE_NTF_CFG_IND,
                prf_dst_task_get(&(ayla_wifi_env->prf_env), conidx),
                dest_id, ayla_wifi_scre_value_ntf_cfg_ind,param->length);

            if (ntf_cfg == PRF_CLI_START_IND)
            {		
							//UART_PRINTF("5 \r\n");
                // Ntf cfg bit set to 1
                ayla_wifi_env->scre_ntf_ind_cfg[conidx] |= (AYLA_WIFI_LVL_IND_SUP );
            }
            else if (ntf_cfg == PRF_CLI_START_NTF)
            {		
							//UART_PRINTF("5 \r\n");
                // Ntf cfg bit set to 1
                ayla_wifi_env->scre_ntf_ind_cfg[conidx] |= (AYLA_WIFI_LVL_NTF_SUP );
            }
            else
            {	
//							UART_PRINTF("6 \r\n");
                // Ntf cfg bit set to 0
                ayla_wifi_env->scre_ntf_ind_cfg[conidx] = AYLA_WIFI_LVL_NTF_NOT_SUP;
            }
        	// Fill in the parameter structure	
            ind->conidx = conidx;
            ind->handle = param->handle;
            ind->ntf_cfg = ayla_wifi_env->scre_ntf_ind_cfg[conidx];
			// Send the message
			kernel_msg_send(ind);
		}
        else if (att_idx == AYLA_WIFI_IDX_CONN_VAL_VALUE)
		{
        	// Allocate the alert value change indication
        	struct ayla_wifi_conn_write_ind *ind = KERNEL_MSG_ALLOC_DYN(AYLA_WIFI_CONN_WRITE_IND,
                prf_dst_task_get(&(ayla_wifi_env->prf_env), conidx),
                dest_id, ayla_wifi_conn_write_ind,param->length);
			
        	// Fill in the parameter structure	
			memcpy(ind->value,&param->value[0],param->length);
			ind->conidx = conidx;
			ind->length = param->length;
			// Send the message
			kernel_msg_send(ind);
		}
        else if (att_idx == AYLA_WIFI_IDX_WPS_VAL_VALUE)
		{
        	// Allocate the alert value change indication
        	struct ayla_wifi_wps_write_ind *ind = KERNEL_MSG_ALLOC_DYN(AYLA_WIFI_WPS_WRITE_IND,
                prf_dst_task_get(&(ayla_wifi_env->prf_env), conidx),
                dest_id, ayla_wifi_wps_write_ind,param->length);
			
        	// Fill in the parameter structure	
			memcpy(ind->value,&param->value[0],param->length);
			ind->conidx = conidx;
			ind->length = param->length;
			// Send the message
			kernel_msg_send(ind);
		}
        else if (att_idx == AYLA_WIFI_IDX_SCAN_VAL_VALUE)
		{
        	// Allocate the alert value change indication
        	struct ayla_wifi_scan_write_ind *ind = KERNEL_MSG_ALLOC_DYN(AYLA_WIFI_SCAN_WRITE_IND,
                prf_dst_task_get(&(ayla_wifi_env->prf_env), conidx),
                dest_id, ayla_wifi_scan_write_ind,param->length);
			
        	// Fill in the parameter structure	
			memcpy(ind->value,&param->value[0],param->length);
			ind->conidx = conidx;
			ind->length = param->length;
			// Send the message
			kernel_msg_send(ind);
		}
        else if (att_idx == AYLA_WIFI_IDX_SCRE_VAL_VALUE)
		{
        	// Allocate the alert value change indication
        	struct ayla_wifi_scre_write_ind *ind = KERNEL_MSG_ALLOC_DYN(AYLA_WIFI_SCRE_WRITE_IND,
                prf_dst_task_get(&(ayla_wifi_env->prf_env), conidx),
                dest_id, ayla_wifi_scre_write_ind,param->length);
			
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
  	struct ayla_wifi_env_tag*ayla_wifi_env = PRF_ENV_GET(AYLA_WIFI, ayla_wifi);
    {
        // continue operation execution
		struct gattc_cmp_evt *evt = KERNEL_MSG_ALLOC(AYLA_WIFI_GATTC_CMP_EVT,
        prf_dst_task_get(&(ayla_wifi_env->prf_env), 0),
        dest_id, gattc_cmp_evt);
			
		evt->operation = param->operation;
		evt->status = param->status;
		evt->seq_num = param->seq_num;
				
		kernel_state_set(dest_id, AYLA_WIFI_IDLE); 
		kernel_msg_send(evt);					
    }
		
    return (KERNEL_MSG_CONSUMED);
}

/// Default State handlers definition
const struct kernel_msg_handler ayla_wifi_default_state[] =
{
    {AYLA_WIFI_ENABLE_REQ,          (kernel_msg_func_t) ayla_wifi_enable_req_handler},
    {AYLA_WIFI_STATU_NTF_UPD_REQ,   (kernel_msg_func_t) ayla_wifi_statu_ntf_upd_req_handler},
    {AYLA_WIFI_SCRE_NTF_UPD_REQ,    (kernel_msg_func_t) ayla_wifi_scre_ntf_upd_req_handler},
    {GATTC_ATT_INFO_REQ_IND,        (kernel_msg_func_t) gattc_att_info_req_ind_handler},
    {GATTC_READ_REQ_IND,            (kernel_msg_func_t) gattc_read_req_ind_handler},
    {GATTC_WRITE_REQ_IND,           (kernel_msg_func_t) gattc_write_req_ind_handler},
    {GATTC_CMP_EVT,                 (kernel_msg_func_t) gattc_cmp_evt_handler},
};


/// Specifies the message handlers that are common to all states.
const struct kernel_state_handler ayla_wifi_default_handler = KERNEL_STATE_HANDLER(ayla_wifi_default_state);

#endif /* #if (BLE_AYLA_WIFI_SERVER) */


 
