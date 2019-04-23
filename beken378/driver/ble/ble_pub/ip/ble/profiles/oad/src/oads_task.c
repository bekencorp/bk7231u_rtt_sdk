/**
 ****************************************************************************************
 *
 * @file   braces_task.c
 *
 * @brief barcelet Server Role Task Implementation.
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

#if (BLE_OADS_SERVER)


#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "atts.h"
#include "common_utils.h"
#include "oads.h"
#include "oads_task.h"

#include "prf_utils.h"
#include "uart.h"


static int oads_enable_req_handler(kernel_msg_id_t const msgid,
                                   struct oads_enable_req const *param,
                                   kernel_task_id_t const dest_id,
                                   kernel_task_id_t const src_id)
{
	//UART_PRINTF("%s \r\n",__func__);
    int msg_status = KERNEL_MSG_SAVED;
    uint8_t state = kernel_state_get(dest_id);

    // check state of the task
    struct oads_env_tag* oads_env = PRF_ENV_GET(OADS,oads);

    // Check provided values
    if((param->conidx > BLE_CONNECTION_MAX)
        || (gapc_get_conhdl(param->conidx) == GAP_INVALID_CONHDL))
    {
        // an error occurs, trigg it.
        struct oads_enable_rsp* rsp = KERNEL_MSG_ALLOC(OADS_ENABLE_RSP, src_id,
            dest_id, oads_enable_rsp);
        rsp->conidx = param->conidx;
        rsp->status = (param->conidx > BLE_CONNECTION_MAX) ? GAP_ERR_INVALID_PARAM : PRF_ERR_REQ_DISALLOWED;
        kernel_msg_send(rsp);

        msg_status = KERNEL_MSG_CONSUMED;
    }
    else
    {
        //put task in a busy state
        msg_status = KERNEL_MSG_NO_FREE;
		//kernel_state_set(dest_id, OADS_BUSY);
        oads_env->ffc1_ntf_cfg[param->conidx] = param->ffc1_ntf_cfg;
		oads_env->ffc2_ntf_cfg[param->conidx] = param->ffc2_ntf_cfg;
						
        oads_env->operation = kernel_param2msg(param);
		oads_env->cursor = 0;
        // trigger notification
        //oads_exe_operation();		
    }

    return msg_status;
}



static int oads_ffc1_upd_req_handler(kernel_msg_id_t const msgid,
                                            struct oads_ffc1_upd_req const *param,
                                            kernel_task_id_t const dest_id,
                                            kernel_task_id_t const src_id)
{
	uart_putchar("oads_ffc1_upd_req_handler \r\n");

    int msg_status = KERNEL_MSG_SAVED;
	
 	/*   
 	uint8_t state = kernel_state_get(dest_id);
	
    // check state of the task
    if(state == OADS_IDLE)
    {
        struct oads_env_tag* oads_env = PRF_ENV_GET(OADS, oads);

        // Check provided values
        if((param->length <= OADS_FFC1_DATA_LEN))
        {
            // update the  value
			memcpy(&oads_env->ffc1_value[0],&param->data[0],param->length);
            // put task in a busy state
            msg_status = KERNEL_MSG_NO_FREE;
         	//kernel_state_set(dest_id, OADS_BUSY);
            oads_env->operation = kernel_param2msg(param);
          	oads_env->cursor = 0;					
            //trigger notification			
			//oads_exe_operation();		 
        }
        else
        {					
            // an error occurs, trigg it.
            struct oads_ffc1_upd_rsp * rsp = KERNEL_MSG_ALLOC(OADS_FFC1_UPD_RSP, src_id,
                    dest_id, oads_ffc1_upd_rsp);

            rsp->status = PRF_ERR_INVALID_PARAM;
            kernel_msg_send(rsp);
            msg_status = KERNEL_MSG_CONSUMED;
        }
    }
	*/
    return (msg_status);
}



static int oads_ffc2_upd_req_handler(kernel_msg_id_t const msgid,
                                            struct oads_ffc2_upd_req const *param,
                                            kernel_task_id_t const dest_id,
                                            kernel_task_id_t const src_id)
{
	//UART_PRINTF("%s \r\n",__func__);
	
    int msg_status = KERNEL_MSG_SAVED;
 	/*   
 	uint8_t state = kernel_state_get(dest_id);
	
    // check state of the task
    if(state == OADS_IDLE)
    {
        struct oads_env_tag* oads_env = PRF_ENV_GET(OADS, oads);

        // Check provided values
        if((param->length <= OADS_FFC2_DATA_LEN))
        {
            // update the battery level value
			memcpy(&oads_env->ffc2_value[0],&param->data[0],param->length);
            // put task in a busy state
            msg_status = KERNEL_MSG_NO_FREE;
			//kernel_state_set(dest_id, OADS_BUSY);
            oads_env->operation = kernel_param2msg(param);
          	oads_env->cursor = 0;					
            //trigger notification
			//oads_exe_operation();
        }
        else
        {					
            // an error occurs, trigg it.
            struct oads_ffc2_upd_rsp * rsp = KERNEL_MSG_ALLOC(OADS_FFC2_UPD_RSP, src_id,
                    dest_id, oads_ffc2_upd_rsp);

            rsp->status = PRF_ERR_INVALID_PARAM;
            kernel_msg_send(rsp);
            msg_status = KERNEL_MSG_CONSUMED;
        }
    }
	*/
    return (msg_status);
}





static int gattc_write_req_ind_handler(kernel_msg_id_t const msgid, struct gattc_write_req_ind const *param,
                                      kernel_task_id_t const dest_id, kernel_task_id_t const src_id)
{

 //  UART_PRINTF("oad %s \r\n",__func__);
   uint8_t status = ATT_ERR_NO_ERROR;
	
   int msg_status = KERNEL_MSG_CONSUMED;
	
   uint8_t conidx = KERNEL_IDX_GET(src_id);
    // retrieve handle information
		
    // If the attribute has been found, status is ATT_ERR_NO_ERROR
    if(kernel_state_get(dest_id) == OADS_IDLE)
    {
				
        struct oads_env_tag* oads_env = PRF_ENV_GET(OADS, oads);
		if(oads_env == NULL)
		{
			while(1){ //UART_PRINTF("oads_env == null\r\n");};
		}
		uint16_t ntf_cfg = common_read16p(&param->value[0]);
		if(((oads_env->features & 0x01) == OADS_NTF_SUP) && ((ntf_cfg == PRF_CLI_STOP_NTFIND) || (ntf_cfg == PRF_CLI_START_NTF)))
		{
			
			if(param->handle == (oads_env->oads_start_hdl + OADS_IDX_FFC1_LVL_NTF_CFG))
			{	
				//UART_PRINTF("OADS_IDX_FFC1_LVL_NTF_CFG ntf_cfg = %d\r\n",ntf_cfg);
				if (ntf_cfg == PRF_CLI_START_NTF)
				{
						
						// Ntf cfg bit set to 1
						oads_env->ffc1_ntf_cfg[conidx] |= (OADS_NTF_SUP);
				}
				else
				{	
						// Ntf cfg bit set to 0
						oads_env->ffc1_ntf_cfg[conidx] &= ~(OADS_NTF_SUP);
				}			
			}
			else if(param->handle == (oads_env->oads_start_hdl + OADS_IDX_FFC2_LVL_NTF_CFG))
			{
			 	//UART_PRINTF("OADS_IDX_FFC2_LVL_NTF_CFG ntf_cfg = %d\r\n",ntf_cfg);
				if (ntf_cfg == PRF_CLI_START_NTF)
				{
						// Ntf cfg bit set to 1
						oads_env->ffc2_ntf_cfg[conidx] |= (OADS_NTF_SUP);
				}
				else
				{
						// Ntf cfg bit set to 0
						oads_env->ffc2_ntf_cfg[conidx] &= ~(OADS_NTF_SUP);
				}
			}
		}	
				
		if(param->handle == (oads_env->oads_start_hdl + OADS_IDX_FFC1_LVL_VAL))
		{   
			//UART_PRINTF("OADS_IDX_FFC1_LVL_VAL value = ");
			//for(int i = 0;i<param->length;i++)
			//{
			//	UART_PRINTF("%x ",param->value[i]);
			//}
			//UART_PRINTF("\r\n");
							
			memcpy(&oads_env->ffc1_value[0],&param->value[0],param->length);
		
			oadImgIdentifyWrite(0,param->length,oads_env->ffc1_value);
				
		}
		else if(param->handle == (oads_env->oads_start_hdl + OADS_IDX_FFC2_LVL_VAL))
		{			
			memcpy(&oads_env->ffc2_value[0],&param->value[0],param->length);
			oadImgBlockWrite( 0, oads_env->ffc2_value);
		}
			
		//Send write response
		struct gattc_write_cfm *cfm = KERNEL_MSG_ALLOC(
    				GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);
		//Send write response
		cfm->handle = param->handle;
		cfm->status = status;
	  	kernel_msg_send(cfm);
    }
	else if(kernel_state_get(dest_id) == OADS_BUSY)
    {
		//UART_PRINTF("OADS_BUSY\r\n");
      	msg_status = KERNEL_MSG_SAVED;
    }


    return (msg_status);
}   



static int gattc_read_req_ind_handler(kernel_msg_id_t const msgid, struct gattc_read_req_ind const *param,
                                      kernel_task_id_t const dest_id, kernel_task_id_t const src_id)
{
    //UART_PRINTF("oads %s \r\n",__func__);
	uint8_t status = ATT_ERR_NO_ERROR;
	int msg_status = KERNEL_MSG_CONSUMED;
	struct gattc_read_cfm * cfm;
	struct oads_env_tag* oads_env = PRF_ENV_GET(OADS, oads);
	uint16_t length = 0;
	uint8_t value[20] = {0};

	if(kernel_state_get(dest_id) == OADS_IDLE)
	{
		 //UART_PRINTF("oads OADS_IDLE read param->handle = 0x%x \r\n",param->handle);
		
		if(param->handle == (oads_env->oads_start_hdl + OADS_IDX_FFC1_LVL_NTF_CFG))
		{
			length = sizeof(uint16_t);
			memcpy(value,&oads_env->ffc1_ntf_cfg[0],length);
			//UART_PRINTF("Read c1 cfg \r\n");
		}
		else if(param->handle == (oads_env->oads_start_hdl + OADS_IDX_FFC2_LVL_NTF_CFG))
		{
			length = sizeof(uint16_t);
			memcpy(value,&oads_env->ffc2_ntf_cfg[0],length);
			//UART_PRINTF("Read c2 cfg \r\n");
		}
		
		else if(param->handle == (oads_env->oads_start_hdl + OADS_IDX_FFC1_LVL_VAL))
		{
			length = OADS_FFC1_DATA_LEN*sizeof(uint8_t);
			memcpy(value,&oads_env->ffc1_value[0],length);
			//UART_PRINTF("Read c1 val\r\n");
		}
		else if(param->handle == (oads_env->oads_start_hdl + OADS_IDX_FFC2_LVL_VAL))
		{
			length = OADS_FFC2_DATA_LEN*sizeof(uint8_t);
			memcpy(value,&oads_env->ffc2_value[0],length);
			//UART_PRINTF("Read c2 val\r\n");
		}
		
		else if(param->handle == (oads_env->oads_start_hdl + OADS_IDX_FFC1_USER_DECL))
		{
			length = sizeof("Img Identify");
			memcpy(value,"Img Identify",length);
			//UART_PRINTF("Read c1 DECL val\r\n");
		}
		
		else if(param->handle == (oads_env->oads_start_hdl + OADS_IDX_FFC2_USER_DECL))
		{
			length = sizeof("Img Block");
			memcpy(value,"Img Block",length);
			//UART_PRINTF("Read c2 DECL val\r\n");
		}
			
		else
		{
			  //UART_PRINTF("Read status ATT_ERR_REQUEST_NOT_SUPPORTED \r\n");
			  status = ATT_ERR_REQUEST_NOT_SUPPORTED;
		}
			
		cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, length);
		cfm->length = length;
		memcpy(cfm->value, value, length);
		cfm->handle = param->handle;
		cfm->status = status;
		
		// Send value to peer device.
      	kernel_msg_send(cfm);
			
	  	//UART_PRINTF("oad read send cfm\r\n");
		
	}
	else if(kernel_state_get(dest_id) == OADS_BUSY)
	{
			//UART_PRINTF("oads OADS_BUSY \r\n");
			msg_status = KERNEL_MSG_SAVED;
	}
		
	//UART_PRINTF("oads %s end!\r\n",__func__);

    return (msg_status);
}   


static int gattc_cmp_evt_handler(kernel_msg_id_t const msgid,  struct gattc_cmp_evt const *param,
                                 kernel_task_id_t const dest_id, kernel_task_id_t const src_id)
{
    if(param->operation == GATTC_NOTIFY)
    {
        //continue operation execution	
		//oads_exe_operation();

    }
    return (KERNEL_MSG_CONSUMED);
}

/// Default State handlers definition
const struct kernel_msg_handler oads_default_state[] =
{
    {OADS_ENABLE_REQ,             (kernel_msg_func_t) oads_enable_req_handler},
    {OADS_FFC1_UPD_REQ,           (kernel_msg_func_t) oads_ffc1_upd_req_handler},
	{OADS_FFC2_UPD_REQ, 		  (kernel_msg_func_t)	oads_ffc2_upd_req_handler},
    {GATTC_WRITE_REQ_IND,    	    (kernel_msg_func_t) gattc_write_req_ind_handler},
    {GATTC_READ_REQ_IND,            (kernel_msg_func_t) gattc_read_req_ind_handler},
    {GATTC_CMP_EVT,                 (kernel_msg_func_t) gattc_cmp_evt_handler},
};

/// Specifies the message handlers that are common to all states.
const struct kernel_state_handler braces_default_handler = KERNEL_STATE_HANDLER(oads_default_state);

#endif /* #if (BLE_OADS_SERVER) */




 
