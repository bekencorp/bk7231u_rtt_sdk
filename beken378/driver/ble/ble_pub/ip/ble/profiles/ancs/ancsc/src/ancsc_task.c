/**
 ****************************************************************************************
 *
 * @file ancsc_task.c
 *
 * @brief ANCSC Locator Task implementation.
 *
 * Copyright (C) BeKen 2009-2015
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup ANCSCTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_ANCS_CLIENT)
#include "ancs_common.h"
#include "ancsc.h"
#include "ancsc_task.h"
#include "gattc_task.h"
#include "prf_utils.h"
#include "gap.h"
#include "gapc.h"
#include "gapc_task.h"
#include <stdint.h>

#include "kernel_mem.h"

#define ATT_SVC_ANCS_UUID   (ATT_UUID_16(0x00D0))

#define ATT_SVC_ANCS_UUID128  {0xD0, 0x00, 0x2D, 0x12, 0x1E, 0x4B, 0x0F, 0xA4, \
                          0x99, 0x4E, 0xCE, 0xB5, 0x31, 0xF4, 0x05, 0x79}

	
												
#define ATT_CHAR_NTF_UUID   (ATT_UUID_16(0x1DBD))

#define ATT_CHAR_NTF_UUID128 {0xBD, 0x1D, 0xA2, 0x99, 0xE6, 0x25, 0x58, 0x8C, \
                          0xD9, 0x42, 0x01, 0x63, 0x0D, 0x12, 0xBF, 0x9F}

											
#define ATT_CHAR_CNTL_POINT_UUID		(ATT_UUID_16(0xD9D9))
#define ATT_CHAR_CNTL_POINT_UUID128 {0xD9, 0xD9, 0xAA, 0xFD, 0xE6, 0x9B, 0x21, 0x98, \
                          0xA8, 0x49, 0xE1, 0x45, 0xF3, 0xD8, 0xD1, 0x69}

											
#define ATT_CHAR_DATA_SOURCE_UUID			(ATT_UUID_16(0x7BFB))
#define ATT_CHAR_DATA_SOURCE_UUID128 {0xFB, 0x7B, 0x7C, 0xCE, 0x6A, 0xB3, 0x44, 0xBE, \
                          0xB5, 0x4B, 0xD6, 0x24, 0xE9, 0xC6, 0xEA, 0x22}
 


/// table used to retrieve ancs service characteristics information
const struct prf_char_def       ancsc_ancs_char[ANCS_SOURCE_CHAR_MAX] =
{
	
				[ANCSC_CNTL_POINT_CHAR] = {ATT_CHAR_CNTL_POINT_UUID,
                                  ATT_OPTIONAL,
                                  ATT_CHAR_PROP_WR | ATT_CHAR_PROP_EXT_PROP},
				
        [ANCSC_NTF_SOURCE_CHAR] = {ATT_CHAR_NTF_UUID,
                                  ATT_MANDATORY,
                                  ATT_CHAR_PROP_NTF},
				
				
				
				[ANCSC_DATA_SOURCE_CHAR] = {ATT_CHAR_DATA_SOURCE_UUID,
                                  ATT_OPTIONAL,
                                  ATT_CHAR_PROP_NTF},
};

/// State machine used to retrieve ancs Service characteristic description information
const struct prf_char_desc_def   ancsc_ancs_char_desc[ANCSC_DESC_MAX] =
{
     /// 
    [ANCSC_DESC_NTF_SRC_CFG]     = {ATT_DESC_CLIENT_CHAR_CFG,   ATT_MANDATORY, ANCSC_NTF_SOURCE_CHAR},

		[ANCSC_DESC_DATA_SRC_CFG]    = {ATT_DESC_CLIENT_CHAR_CFG,   ATT_MANDATORY, ANCSC_DATA_SOURCE_CHAR},

};



/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref ANCSC_ENABLE_REQ message.
 * The handler enables the Ancs profile - Client Role.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int ancsc_enable_req_handler(kernel_msg_id_t const msgid,
                                    struct ancsc_enable_req const *param,
                                    kernel_task_id_t const dest_id,
                                    kernel_task_id_t const src_id)
{
    // Status

	  int msg_status = KERNEL_MSG_CONSUMED;
    uint8_t status = GAP_ERR_NO_ERROR;

    uint8_t state = kernel_state_get(dest_id);
    uint8_t conidx = KERNEL_IDX_GET(dest_id);
		uint8_t uuid[ATT_UUID_128_LEN] = ATT_SVC_ANCS_UUID128;
    // ANCS Client Role Task Environment
    struct ancsc_env_tag *ancsc_env = PRF_ENV_GET(ANCSC, ancsc);

    ASSERT_INFO(ancsc_env != NULL, dest_id, src_id);
    if((state == ANCSC_IDLE) && (ancsc_env->env[conidx] == NULL))
    {
        // allocate environment variable for task instance
        ancsc_env->env[conidx] = (struct ancsc_cnx_env*) kernel_malloc(sizeof(struct ancsc_cnx_env),KERNEL_MEM_ATT_DB);
        memset(ancsc_env->env[conidx], 0, sizeof(struct ancsc_cnx_env));

        //Config connection, start discovering
        if(param->con_type == PRF_CON_DISCOVERY)
        {
						
									
            //start discovering IAS on peer
            prf_disc_svc_send_uuid128(&(ancsc_env->prf_env), conidx, uuid);
						ancsc_env->env[conidx]->operation = kernel_param2msg(param);		
            // Go to BUSY state
            kernel_state_set(dest_id, ANCSC_BUSY);
					  msg_status = KERNEL_MSG_NO_FREE;
        }
        //normal connection, get saved att details
        else
        {
            ancsc_env->env[conidx]->ancsc = param->ancsc;

            //send APP confirmation that can start normal connection 
            ancsc_enable_rsp_send(ancsc_env, conidx, GAP_ERR_NO_ERROR);
        }
    }
    else if(state != ANCSC_FREE)
    {
        status = PRF_ERR_REQ_DISALLOWED;
    }

    // send an error if request fails
    if(status != GAP_ERR_NO_ERROR)
    {
        ancsc_enable_rsp_send(ancsc_env, conidx, status);
    }

    return (msg_status);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_SDP_SVC_IND_HANDLER message.
 * The handler stores the found service details for service discovery.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_sdp_svc_ind_handler(kernel_msg_id_t const msgid,
                                     struct gattc_sdp_svc_ind const *ind,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{
    uint8_t state = kernel_state_get(dest_id);
	//uart_printf("%s\r\n",__func__);
    if(state == ANCSC_BUSY)
    {
        uint8_t conidx = KERNEL_IDX_GET(dest_id);
        // Get the address of the environment
        struct ancsc_env_tag *ancsc_env = PRF_ENV_GET(ANCSC, ancsc);

        ASSERT_INFO(ancsc_env != NULL, dest_id, src_id);
        ASSERT_INFO(ancsc_env->env[conidx] != NULL, dest_id, src_id);
        if(ancsc_env->env[conidx]->nb_svc == 0)
        {
		
						
            // Retrieve ANCS characteristic
            prf_extract_svc_info(ind, ANCS_SOURCE_CHAR_MAX, &ancsc_ancs_char[0],
                    &(ancsc_env->env[conidx]->ancsc.chars[0]), ANCSC_DESC_MAX, &ancsc_ancs_char_desc[0],  &(ancsc_env->env[conidx]->ancsc.descs[0]));
				
            //Even if we get multiple responses we only store 1 range
            ancsc_env->env[conidx]->ancsc.svc.shdl = ind->start_hdl;
            ancsc_env->env[conidx]->ancsc.svc.ehdl = ind->end_hdl;
						uart_printf("start= 0x%x,end= 0x%x\r\n",ancsc_env->env[conidx]->ancsc.svc.shdl,ancsc_env->env[conidx]->ancsc.svc.ehdl);
				/// characteristic info

				}
			
			  //uart_printf("ANCSC_CNTL_POINT_CHAR  char handle = 0x%x,value handle = 0x%x,prop = 0x%x\r\n",ancsc_env->env[conidx]->ancsc.chars[ANCSC_CNTL_POINT_CHAR].char_hdl,ancsc_env->env[conidx]->ancsc.chars[ANCSC_CNTL_POINT_CHAR].val_hdl,
																					ancsc_env->env[conidx]->ancsc.chars[ANCSC_CNTL_POINT_CHAR].prop);
				
				//uart_printf("ANCSC_NTF_SOURCE_CHAR  char handle = 0x%x,value handle = 0x%x,prop = 0x%x\r\n",ancsc_env->env[conidx]->ancsc.chars[ANCSC_NTF_SOURCE_CHAR].char_hdl,ancsc_env->env[conidx]->ancsc.chars[ANCSC_NTF_SOURCE_CHAR].val_hdl,
																					ancsc_env->env[conidx]->ancsc.chars[ANCSC_NTF_SOURCE_CHAR].prop);
			
				//uart_printf("ANCSC_DATA_SOURCE_CHAR  char handle = 0x%x,value handle = 0x%x,prop = 0x%x\r\n",ancsc_env->env[conidx]->ancsc.chars[ANCSC_DATA_SOURCE_CHAR].char_hdl,ancsc_env->env[conidx]->ancsc.chars[ANCSC_DATA_SOURCE_CHAR].val_hdl,
																					ancsc_env->env[conidx]->ancsc.chars[ANCSC_DATA_SOURCE_CHAR].prop);
			
				//uart_printf("ANCSC_DESC_NTF_SRC_CFG  desc handle = 0x%x \r\n",ancsc_env->env[conidx]->ancsc.descs[ANCSC_DESC_NTF_SRC_CFG].desc_hdl);
			
				
				//uart_printf("ANCSC_DESC_DATA_SRC_CFG  desc handle = 0x%x \r\n",ancsc_env->env[conidx]->ancsc.descs[ANCSC_DESC_DATA_SRC_CFG].desc_hdl);
			
				
        ancsc_env->env[conidx]->nb_svc++;
				
				
    }

    return (KERNEL_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref ANCSC_CFG_NTF_SRC_INDNTF_REQ message.
 * The handler enable/disables the ANCSS profile - Target Role.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int ancsc_ntf_src_ntf_cfg_req_handler(kernel_msg_id_t const msgid,
                                       struct ancsc_ntf_src_ntf_cfg_req const *param,
                                       kernel_task_id_t const dest_id,
                                       kernel_task_id_t const src_id)
{
   
    int msg_status = KERNEL_MSG_CONSUMED;
    uint8_t state = kernel_state_get(dest_id);
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    if(state == ANCSC_IDLE)
    {
        uint8_t conidx = KERNEL_IDX_GET(dest_id);
        // Get the address of the environment
        struct ancsc_env_tag *ancsc_env = PRF_ENV_GET(ANCSC, ancsc);
        ASSERT_INFO(ancsc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if(ancsc_env->env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        // check parameter range
        else if( param->ntf_cfg > PRF_CLI_START_NTF )
        {
            status = PRF_ERR_INVALID_PARAM;
        }
        else
        {
            uint16_t handle = ancsc_env->env[conidx]->ancsc.descs[ANCSC_DESC_NTF_SRC_CFG].desc_hdl;
            status = PRF_ERR_INEXISTENT_HDL;

            if(handle == ATT_INVALID_HDL)
            {
                status = PRF_ERR_INEXISTENT_HDL;
            }
            else
            {
                status = GAP_ERR_NO_ERROR;
                // Send GATT Write Request
                prf_gatt_write_ntf_ind(&ancsc_env->prf_env, conidx, handle, param->ntf_cfg);
                // store context of request and go into busy state
                ancsc_env->env[conidx]->operation = kernel_param2msg(param);
                kernel_state_set(dest_id, ANCSC_BUSY);
                msg_status = KERNEL_MSG_NO_FREE;
            }
        }
    }
    // process message later
    else if (state == ANCSC_BUSY)
    {
        status = GAP_ERR_NO_ERROR;
        msg_status = KERNEL_MSG_SAVED;
    }


    // request cannot be performed
    if(status != GAP_ERR_NO_ERROR)
    {
        struct ancsc_ntf_src_ntf_cfg_rsp * rsp = KERNEL_MSG_ALLOC(ANCSC_CFG_NTF_SRC_INDNTF_RSP,
                src_id, dest_id, ancsc_ntf_src_ntf_cfg_rsp);
        // set error status
        rsp->status = status;
       
        kernel_msg_send(rsp);
    }

    return (msg_status);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref ANCSC_CFG_DATA_SRC_INDNTF_REQ message.
 * The handler enable/disables the ANCSS profile - Target Role.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int ancsc_data_src_ntf_cfg_req_handler(kernel_msg_id_t const msgid,
                                       struct ancsc_data_src_ntf_cfg_req const *param,
                                       kernel_task_id_t const dest_id,
                                       kernel_task_id_t const src_id)
{
    //uart_printf("%s\r\n",__func__);
    int msg_status = KERNEL_MSG_CONSUMED;
    uint8_t state = kernel_state_get(dest_id);
    uint8_t status = PRF_ERR_REQ_DISALLOWED;

    if(state == ANCSC_IDLE)
    {
        uint8_t conidx = KERNEL_IDX_GET(dest_id);
        // Get the address of the environment
        struct ancsc_env_tag *ancsc_env = PRF_ENV_GET(ANCSC, ancsc);
        ASSERT_INFO(ancsc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if(ancsc_env->env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        // check parameter range
        else if( param->ntf_cfg > PRF_CLI_START_NTF )
        {
            status = PRF_ERR_INVALID_PARAM;
        }
        else
        {
            uint16_t handle = ancsc_env->env[conidx]->ancsc.descs[ANCSC_DESC_DATA_SRC_CFG].desc_hdl;
            status = PRF_ERR_INEXISTENT_HDL;

            if(handle == ATT_INVALID_HDL)
            {
                status = PRF_ERR_INEXISTENT_HDL;
            }
            else
            {
                status = GAP_ERR_NO_ERROR;
								//uart_printf("data desc handle = 0x%x\r\n",handle);
                // Send GATT Write Request
                prf_gatt_write_ntf_ind(&ancsc_env->prf_env, conidx, handle, param->ntf_cfg);
                // store context of request and go into busy state
                ancsc_env->env[conidx]->operation = kernel_param2msg(param);
                kernel_state_set(dest_id, ANCSC_BUSY);
                msg_status = KERNEL_MSG_NO_FREE;
            }
        }
    }
    // process message later
    else if (state == ANCSC_BUSY)
    {
				//uart_printf("write data desc cfg busy !!!!\r\n");
        status = GAP_ERR_NO_ERROR;
        msg_status = KERNEL_MSG_SAVED;
    }


    // request cannot be performed
    if(status != GAP_ERR_NO_ERROR)
    {
        struct ancsc_data_src_ntf_cfg_rsp * rsp = KERNEL_MSG_ALLOC(ANCSC_CFG_DATA_SRC_INDNTF_RSP,
                src_id, dest_id, ancsc_data_src_ntf_cfg_rsp);
        // set error status
        rsp->status = status;
       
        kernel_msg_send(rsp);
    }

    return (msg_status);
}


/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_CMP_EVT message.
 * This generic event is received for different requests, so need to keep track.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_cmp_evt_handler(kernel_msg_id_t const msgid,
                                struct gattc_cmp_evt const *param,
                                kernel_task_id_t const dest_id,
                                kernel_task_id_t const src_id)
{
	
	// uart_printf("ancsc %s\r\n operation =0x%x,status = 0x%x,seq_num = 0x%x\r\n",__func__,param->operation,param->status,param->seq_num);

   uint8_t state = kernel_state_get(dest_id);
    uint8_t conidx = KERNEL_IDX_GET(dest_id);
    // Get the address of the environment
    struct ancsc_env_tag *ancsc_env = PRF_ENV_GET(ANCSC, ancsc);

		
    // sanity check
    if((state == ANCSC_BUSY) && (ancsc_env->env[conidx] != NULL)
            && (ancsc_env->env[conidx]->operation != NULL))
    {
        switch(ancsc_env->env[conidx]->operation->id)
        {
            case ANCSC_ENABLE_REQ:
            {
                uint8_t status = param->status;

                if (param->status == ATT_ERR_NO_ERROR)
                {
                    // check characteristic validity
                    if(ancsc_env->env[conidx]->nb_svc > 0)
                    {
                          
                       
                            status = prf_check_svc_char_validity(ANCS_SOURCE_CHAR_MAX, &(ancsc_env->env[conidx]->ancsc.chars[0]),
                                    &(ancsc_ancs_char[0]));

                            // check descriptor validity
                            if(status == GAP_ERR_NO_ERROR)
                            {
                                struct prf_char_desc_def ancsc_desc[ANCSC_DESC_MAX];
                                memcpy(ancsc_desc, ancsc_ancs_char_desc, sizeof(ancsc_ancs_char_desc));

                               
                                if ((ancsc_env->env[conidx]->ancsc.chars[ANCSC_NTF_SOURCE_CHAR].prop & ATT_CHAR_PROP_NTF) == ATT_CHAR_PROP_NTF)
                                {
                                    ancsc_desc[ANCSC_DESC_NTF_SRC_CFG].req_flag = ATT_MANDATORY;
                                }
																
																if ((ancsc_env->env[conidx]->ancsc.chars[ANCSC_DATA_SOURCE_CHAR].prop & ATT_CHAR_PROP_NTF) == ATT_CHAR_PROP_NTF)
                                {
                                     ancsc_desc[ANCSC_DESC_DATA_SRC_CFG].req_flag = ATT_MANDATORY;
                                }
																

                                status = prf_check_svc_char_desc_validity(ANCSC_DESC_MAX,
                                        ancsc_env->env[conidx]->ancsc.descs, ancsc_desc,
                                        ancsc_env->env[conidx]->ancsc.chars);
															
                            }
                        
                    }
                    // no services found
                    else
                    {
                        status = PRF_ERR_STOP_DISC_CHAR_MISSING;
                    }

                }
                ancsc_enable_rsp_send(ancsc_env, conidx, status);
            }break;
            
            case ANCSC_CFG_NTF_SRC_INDNTF_REQ:
            {
                struct ancsc_ntf_src_ntf_cfg_req* req = (struct ancsc_ntf_src_ntf_cfg_req*) kernel_msg2param(ancsc_env->env[conidx]->operation);

                struct ancsc_ntf_src_ntf_cfg_rsp * rsp = KERNEL_MSG_ALLOC(ANCSC_CFG_NTF_SRC_INDNTF_RSP,
                        prf_dst_task_get(&(ancsc_env->prf_env), conidx), dest_id, ancsc_ntf_src_ntf_cfg_rsp);
                // set error status
                rsp->status = param->status;

                kernel_msg_send(rsp);
            }break;
						
					  case ANCSC_CFG_DATA_SRC_INDNTF_REQ:
            {
                struct ancsc_data_src_ntf_cfg_req* req = (struct ancsc_data_src_ntf_cfg_req*) kernel_msg2param(ancsc_env->env[conidx]->operation);

                struct ancsc_data_src_ntf_cfg_rsp * rsp = KERNEL_MSG_ALLOC(ANCSC_CFG_DATA_SRC_INDNTF_RSP,
                        prf_dst_task_get(&(ancsc_env->prf_env), conidx), dest_id, ancsc_data_src_ntf_cfg_rsp);
                // set error status
                rsp->status =param->status;

                kernel_msg_send(rsp);
            }break;
            default:
            {
                // Not Expected at all
                ASSERT_ERR(0);
            }break;
        }

        // operation is over - go back to idle state
        kernel_free(ancsc_env->env[conidx]->operation);
        ancsc_env->env[conidx]->operation = NULL;
        kernel_state_set(dest_id, ANCSC_IDLE);
    }

    return (KERNEL_MSG_CONSUMED);
}

static int ancsc_msg_default_handler(kernel_msg_id_t const msgid,
                                void const *param,
                                kernel_task_id_t const dest_id,
                                kernel_task_id_t const src_id)
{
	  //uart_printf("%s ,msgid = 0x%x,dest_id = 0x%x,src_id = 0x%x\r\n",__func__,msgid,dest_id,src_id);
	
		
		return (KERNEL_MSG_CONSUMED);
}


static int ancsc_wr_cntl_point_req_handler(kernel_msg_id_t const msgid,
                                         struct Get_Notification_Attribute_Command const *param,
                                         kernel_task_id_t const dest_id,
                                         kernel_task_id_t const src_id)
{
	//uart_printf("ancsc %s\r\n",__func__);
	uint8_t conidx = KERNEL_IDX_GET(dest_id);
	struct ancsc_env_tag *ancsc_env = PRF_ENV_GET(ANCSC, ancsc);
	
	uint16_t handle = ancsc_env->env[conidx]->ancsc.chars[ANCSC_CNTL_POINT_CHAR].val_hdl;
	uint16_t length = param->length;
	
	uint8_t *buf = (uint8_t *)(&param->cmd[0]);
	
	//uart_printf("cntlpoint handle = 0x%x\r\n",handle);
	prf_gatt_write(&(ancsc_env->prf_env), conidx,handle, buf,length,GATTC_WRITE);
	
//	for(int i = 0;i < length;i++)
//	{
//			uart_printf("buf[%d] = 0x%x\r\n",i,buf[i]);
//	}
	
	
	return (KERNEL_MSG_CONSUMED);
}


static int gattc_event_ind_handler(kernel_msg_id_t const msgid,
                                         struct gattc_event_ind const *param,
                                         kernel_task_id_t const dest_id,
                                         kernel_task_id_t const src_id)
																					 
{
		uint8_t conidx = KERNEL_IDX_GET(dest_id);
	  struct ancsc_env_tag *ancsc_env = PRF_ENV_GET(ANCSC, ancsc);
	
	//	uart_printf("ancsc %s\r\n",__func__);
		
		if(ancsc_env->env[conidx]->ancsc.chars[ANCSC_NTF_SOURCE_CHAR].val_hdl == param->handle)
		{
			  struct notificationCharInd * ind = KERNEL_MSG_ALLOC(ANCSC_NOTIFICATION_IND,
                        prf_dst_task_get(&(ancsc_env->prf_env), conidx), dest_id, notificationCharInd);
                // set error status
        
				 ind->length = param->length;
				 memcpy((uint8_t *)&(ind->ntf.EventID),param->value,param->length);
				 
				 kernel_msg_send(ind);
		}
		
		if(ancsc_env->env[conidx]->ancsc.chars[ANCSC_DATA_SOURCE_CHAR].val_hdl == param->handle)
		{
		  //	uart_printf("ancsc data source coming\r\n");
			
			  struct dataCharInd * ind = KERNEL_MSG_ALLOC_DYN(ANCSC_DATA_SOURCE_IND,
                        prf_dst_task_get(&(ancsc_env->prf_env), conidx), dest_id, dataCharInd,param->length);
                // set error status
        
				 ind->length = param->length;
				 memcpy((uint8_t *)&(ind->value[0]),param->value,param->length);
					 
				 kernel_msg_send(ind);
		}
		
		return (KERNEL_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */


/// Default State handlers definition
const struct kernel_msg_handler ancsc_default_state[] =
	
{
	  {KERNEL_MSG_DEFAULT_HANDLER,								(kernel_msg_func_t)ancsc_msg_default_handler},
    {ANCSC_ENABLE_REQ,            				  (kernel_msg_func_t)ancsc_enable_req_handler},
    {GATTC_CMP_EVT,                					(kernel_msg_func_t)gattc_cmp_evt_handler},
    {GATTC_SDP_SVC_IND,            					(kernel_msg_func_t)gattc_sdp_svc_ind_handler},
    {ANCSC_CFG_NTF_SRC_INDNTF_REQ,          (kernel_msg_func_t)ancsc_ntf_src_ntf_cfg_req_handler},
		
		{ANCSC_CFG_DATA_SRC_INDNTF_REQ,         (kernel_msg_func_t)ancsc_data_src_ntf_cfg_req_handler},
		{ANCSC_WR_CNTL_POINT_REQ,								(kernel_msg_func_t)ancsc_wr_cntl_point_req_handler},
		{GATTC_EVENT_IND ,											(kernel_msg_func_t)gattc_event_ind_handler},
		
};

/// Specifies the message handlers that are common to all states.
const struct kernel_state_handler ancsc_default_handler = KERNEL_STATE_HANDLER(ancsc_default_state);

#endif //BLE_ANCS_CLIENT

/// @} ANCSCTASK
