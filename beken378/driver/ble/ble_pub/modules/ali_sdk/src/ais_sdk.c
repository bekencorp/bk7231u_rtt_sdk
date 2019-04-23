/**
 ****************************************************************************************
 *
 * @file ais_sdk.c
 *
 * @brief ais SDK Module entry point
 *
 * @auth  gang.cheng
 *
 * @date  2017.09.19
 *
 * Copyright (C) Beken 2009-2016
 *
 *
 ****************************************************************************************
 */
  
/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_APP_FEB3)

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "common_math.h"
#include "ais_sdk.h"                // Battery Application Module Definitions
#include "app_task.h"                // application task definitions
#include "AIS_task.h"               // health thermometer functions

#include "common_bt.h"
#include "prf_types.h"               // Profile common types definition
#include "prf.h"
#include "AIS.h"
#include "kernel_timer.h"

#include "wlan_ui_pub.h"
#include "uart_pub.h"
#include "ble_pub.h"

/*
 * DEFINES
 ****************************************************************************************
 */
 
#define PRINT_LOG  0

#define POST_SEQ_NUM 0x50
#define POST_FAST_SEQ_NUM 0x52

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
/// ais sdk Module Environment Structure
struct ais_sdk_env_tag ais_sdk_env;

txdone txdone_cb = NULL;
/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void app_feb3_init(void)
{
    memset(&ais_sdk_env, 0, sizeof(struct ais_sdk_env_tag));
}

void app_ais_add_feb3s(void)
{

	//	UART_PRINTF("%s\r\n",__func__);
    struct feb3s_db_cfg *db_cfg;
		
    struct gapm_profile_task_add_cmd *req = KERNEL_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                                  TASK_GAPM, TASK_APP,
                                                  gapm_profile_task_add_cmd, sizeof(struct feb3s_db_cfg));
    // Fill message
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl = 0;
    req->prf_task_id = TASK_ID_FEB3S;
    req->app_task = TASK_APP;
    req->start_hdl = 0; //req->start_hdl = 0; dynamically allocated

	  
	//Set parameters
    db_cfg = (struct feb3s_db_cfg* ) req->param;
    db_cfg->cfg_flag = 0xffff;
    //Sending of notifications is supported
    db_cfg->features = FEB3S_FED6_VAL_IND_SUP | FEB3S_FED8_VAL_NTF_SUP;;
    //Send the message
    kernel_msg_send(req);
}


void app_feb3_enable_prf(uint8_t conidx)
{

	//	UART_PRINTF("%s\r\n",__func__);
    ais_sdk_env.conidx = conidx;

    // Allocate the message
    struct feb3s_enable_req * req = KERNEL_MSG_ALLOC(FEB3S_ENABLE_REQ,
                                                prf_get_task_from_id(TASK_ID_FEB3S),
                                                TASK_APP,
                                                feb3s_enable_req);
    // Fill in the parameter structure
    req->conidx             = conidx;

    // NTF initial status - Disabled
    req->ntf_cfg           = PRF_CLI_STOP_NTFIND;
		
    req->ind_cfg           = PRF_CLI_STOP_NTFIND;
    
    // Send the message
    kernel_msg_send(req);
}


ble_err_t feb3_send_fed6_ind_value(uint32_t len,uint8_t *buf,uint16_t seq_num)
{
    OSStatus ret; 
    
    ble_err_t status = ERR_SUCCESS;

    if(ais_sdk_env.fed6_ind_send_allow == 1)
    {
        // Allocate the message
        struct feb3s_fed6_value_ind_upd_req * req = KERNEL_MSG_ALLOC_DYN(FEB3S_FED6_VALUE_IND_UPD_REQ,
                                                        prf_get_task_from_id(TASK_ID_FEB3S),
                                                        TASK_APP,
                                                        feb3s_fed6_value_ind_upd_req,len);

        req->length = len;
        memcpy(req->value,buf,len);
        req->seq_num = POST_SEQ_NUM;

        kernel_msg_send(req);

        ais_sdk_env.fed6_ind_send_allow = 0;
    }
    else
    {
            status = ERR_GATT_INDICATE_FAIL;
    }

    return status;
}


ble_err_t feb3_send_fed8_ntf_value(uint32_t len,uint8_t *buf,uint16_t seq_num)
{
    ble_err_t status = ERR_SUCCESS;

    if(ais_sdk_env.fed8_ntf_send_allow == 1)
    {
        // Allocate the message
        struct feb3s_fed8_value_ntf_upd_req * req = KERNEL_MSG_ALLOC_DYN(FEB3S_FED8_VALUE_NTF_UPD_REQ,
                                                        prf_get_task_from_id(TASK_ID_FEB3S),
                                                        TASK_APP,
                                                        feb3s_fed8_value_ntf_upd_req,len);
        req->length = len;
        memcpy(req->value,buf,len);
        req->seq_num = POST_FAST_SEQ_NUM;
    
        kernel_msg_send(req);
        ais_sdk_env.fed8_ntf_send_allow = 0;
    }
    else
    {
        status = ERR_GATT_NOTIFY_FAIL;
    }
        
    return status;
}



static int feb3s_fed6_cfg_change_ind_handler(kernel_msg_id_t const msgid,
                                               struct feb3s_fed6_value_ind_cfg_ind const *param,
                                               kernel_task_id_t const dest_id,
                                               kernel_task_id_t const src_id)
{
    OSStatus ret;
    
#if PRINT_LOG
    //UART_PRINTF("fed6_ind_cfg handle = 0x%x,fed6_ind_cfg = %x\r\n",param->handle,param->ind_cfg);
#endif

    ais_sdk_env.fed6_ind_cfg = param->ind_cfg;

    if(param->ind_cfg != 0)
    {
        ais_sdk_env.fed6_ind_send_allow = 1;
    }
    else
    {
        ais_sdk_env.fed6_ind_send_allow = 0;
    }      

    if(ble_event_cb != NULL)
    {
        ble_event_cb(BLE_CFG_INDICATE, &(param->ind_cfg));
    }   
    return (KERNEL_MSG_CONSUMED);
}

static int feb3s_fed8_cfg_change_ntf_handler(kernel_msg_id_t const msgid,
                                               struct feb3s_fed8_value_ntf_cfg_ind const *param,
                                               kernel_task_id_t const dest_id,
                                               kernel_task_id_t const src_id)
{
   
#if PRINT_LOG
    //UART_PRINTF("fed8_->ntf_cfg handle = 0x%x,fed8_ntf_cfg = %x\r\n",param->handle,param->ntf_cfg);
#endif

    ais_sdk_env.fed8_ntf_cfg = param->ntf_cfg;
    
    if(param->ntf_cfg != 0)
    {
        ais_sdk_env.fed8_ntf_send_allow = 1;
    }
    else
    {
        ais_sdk_env.fed8_ntf_send_allow = 0;
    }
	
    if(ble_event_cb != NULL)
    {
        ble_event_cb(BLE_CFG_NOTIFY, &(param->ntf_cfg));
    }
	  
    return (KERNEL_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int app_feb3_msg_dflt_handler(kernel_msg_id_t const msgid,
                                     void const *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{
#if PRINT_LOG
    //UART_PRINTF("%s,msgid = 0x%04x,src_id = 0x%04x\r\n",__func__,msgid,src_id);
#endif
    // Drop the message
    return (KERNEL_MSG_CONSUMED);
}


static int feb3s_fed5_write_req_ind_handler(kernel_msg_id_t const msgid,
                                     struct feb3s_fed5_writer_req_ind *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{    
    if(ble_write_cb != NULL)
    {
        ble_write_cb(0xFED5,param->value,param->length);
    }
    	
    return (KERNEL_MSG_CONSUMED);
}

static int feb3s_fed7_write_cmd_ind_handler(kernel_msg_id_t const msgid,
                                     struct feb3s_fed5_writer_req_ind *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{    
    if(ble_write_cb != NULL)
    {
        ble_write_cb(0xFED7,param->value,param->length);
    }
    
    return (KERNEL_MSG_CONSUMED);
}

static int feb3s_fed4_read_req_ind_handler(kernel_msg_id_t const msgid,
                                     struct feb3s_fed4_read_req_ind *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{
    // Drop the message
    //UART_PRINTF("%s\r\n",__func__);

    struct feb3s_fed4_read_rsp * rsp = KERNEL_MSG_ALLOC_DYN(FEB3S_FED4_READ_RSP,
                                                        prf_get_task_from_id(TASK_ID_FEB3S),
                                                        TASK_APP,
                                                        feb3s_fed4_read_rsp,20);

	  // Fill in the parameter structure
    rsp->handle = param->handle;
    rsp->rsp_id = param->rsp_id;
	
    // Send the message
    kernel_msg_send(rsp);
	
    return (KERNEL_MSG_CONSUMED);
}


static int feb3_gattc_cmp_evt_handler(kernel_msg_id_t const msgid,  struct gattc_cmp_evt const *param,
                                kernel_task_id_t const dest_id, kernel_task_id_t const src_id)
{
    //UART_PRINTF("%s,operation = %x,status = 0x%x,seq_num = 0x%x\r\n",__func__,param->operation,param->status,param->seq_num);
		
    if((param->operation == GATTC_NOTIFY) && (param->status == GAP_ERR_NO_ERROR))
    {
        ais_sdk_env.fed8_ntf_send_allow = 1;
    }
    if((param->operation == GATTC_INDICATE) && (param->status == GAP_ERR_NO_ERROR))
    {   		
        ais_sdk_env.fed6_ind_send_allow = 1;
        
        if(txdone_cb)
        {
            txdone_cb(GAP_ERR_NO_ERROR);
        } 
    }	
		
    return KERNEL_MSG_CONSUMED;
}

#include "alinkapp.h"
struct gapm_start_advertise_cmd g_param;

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
const struct kernel_msg_handler app_feb3_msg_handler_list[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KERNEL_MSG_DEFAULT_HANDLER,         (kernel_msg_func_t)app_feb3_msg_dflt_handler},
    {FEB3S_FED5_WRITER_REQ_IND,				(kernel_msg_func_t)feb3s_fed5_write_req_ind_handler},
    {FEB3S_FED7_WRITER_CMD_IND,				(kernel_msg_func_t)feb3s_fed7_write_cmd_ind_handler},				
    {FEB3S_FED6_VALUE_IND_CFG_IND,   (kernel_msg_func_t)feb3s_fed6_cfg_change_ind_handler},
    {FEB3S_FED8_VALUE_NTF_CFG_IND,   (kernel_msg_func_t)feb3s_fed8_cfg_change_ntf_handler},
    {FEB3S_FED4_READ_REQ_IND,					(kernel_msg_func_t)feb3s_fed4_read_req_ind_handler},		
    {FEB3S_GATTC_CMP_EVT,            (kernel_msg_func_t)feb3_gattc_cmp_evt_handler},						
};


const struct kernel_state_handler app_feb3_table_handler =
    {&app_feb3_msg_handler_list[0], (sizeof(app_feb3_msg_handler_list)/sizeof(struct kernel_msg_handler))};

#endif //BLE_APP_FEB3
