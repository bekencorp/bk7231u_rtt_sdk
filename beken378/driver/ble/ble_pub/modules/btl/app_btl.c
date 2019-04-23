/**
 ****************************************************************************************
 *
 * @file app_btl.c
 *
 * @brief Media Application Module entry point
 *
 * @auth  gang.cheng
 *
 * @date  2018.09.17
 *
 * Copyright (C) Beken 2009-2018
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

#if (BLE_APP_BTL)
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>
#include "app_btl.h"                //  Application Module Definitions
#include "application.h"                     // Application Definitions
#include "app_task.h"                // application task definitions
#include "btl_task.h"               // health thermometer functions
#include "common_bt.h"
#include "prf_types.h"               // Profile common types definition
#include "architect.h"                    // Platform Definitions
#include "prf.h"
#include "btl.h"
#include "kernel_timer.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

///  Application Module Environment Structure
struct app_btl_env_tag app_btl_env;

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void app_btl_init(void)
{
    // Reset the environment
    memset(&app_btl_env, 0, sizeof(struct app_btl_env_tag));
}


void app_btl_add_btl(void)
{

    printf("app_btl_add_btl\r\n");

   	struct btl_db_cfg *db_cfg;
		
	struct gapm_profile_task_add_cmd *req = KERNEL_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                                  TASK_GAPM, TASK_APP,
                                                  gapm_profile_task_add_cmd, sizeof(struct btl_db_cfg));
    // Fill message
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl = 0;
    req->prf_task_id = TASK_ID_BTL;
    req->app_task = TASK_APP;
    req->start_hdl = 0; //req->start_hdl = 0; dynamically allocated

	  
	//Set parameters
    db_cfg = (struct btl_db_cfg* ) req->param;
	db_cfg->cfg_flag = 0xffff;
    //Sending of notifications is supported
    db_cfg->features = BTL_FF02_LVL_IND_SUP;
    //Send the message
    kernel_msg_send(req);
}


void app_btl_enable_prf(uint8_t conidx)
{

    app_btl_env.conidx = conidx;

    // Allocate the message
    struct btl_enable_req * req = KERNEL_MSG_ALLOC(BTL_ENABLE_REQ,
                                                prf_get_task_from_id(TASK_ID_BTL),
                                                TASK_APP,
                                                btl_enable_req);
    // Fill in the parameter structure
    req->conidx             = conidx;

    // IND initial status - Disabled
    req->ind_cfg           = PRF_CLI_STOP_NTFIND;
    
    // Send the message
    kernel_msg_send(req);
}


void app_btl_ff02_val(uint8_t len,uint8_t *buf,uint16_t seq_num)//send data
{


    printf("app_btl_ff02_val\r\n");
    // Allocate the message
    struct btl_ff02_value_upd_req * req = KERNEL_MSG_ALLOC_DYN(BTL_FF02_VALUE_UPD_REQ,
                                                        prf_get_task_from_id(TASK_ID_BTL),
                                                        TASK_APP,
                                                        btl_ff02_value_upd_req,len);

    // Fill in the parameter structure
	req->seq_num = seq_num;
	req->length = len;
    memcpy(req->value,buf,len);

    // Send the message
    kernel_msg_send(req);
}

static int btl_ff02_level_ind_cfg_ind_handler(kernel_msg_id_t const msgid,
                                               struct btl_ff02_value_ind_cfg_ind const *param,
                                               kernel_task_id_t const dest_id,
                                               kernel_task_id_t const src_id)
{
   
	printf("param->ind_cfg = %x\r\n",param->ind_cfg);

	app_btl_env.send_ind_cfg = param->ind_cfg;

	if(param->ind_cfg == PRF_CLI_STOP_NTFIND)
	{
		;
	}else
	{
		;
	}
    
    return (KERNEL_MSG_CONSUMED);
}

static int btl_ff02_level_upd_handler(kernel_msg_id_t const msgid,
                                      struct btl_ff02_value_upd_rsp const *param,
                                      kernel_task_id_t const dest_id,
                                      kernel_task_id_t const src_id)
{

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
static int app_btl_msg_dflt_handler(kernel_msg_id_t const msgid,
                                     void const *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{
    // Drop the message
    return (KERNEL_MSG_CONSUMED);
}



static int btl_ff01_req_handler(kernel_msg_id_t const msgid,
                                     struct btl_ff01_ind *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{
    if(ble_write_cb)
    {
        ble_write_cb(0xFF01,param->value,param->length);
    }

    /*
    if(param->value[0]== 0x55)
    {
        app_btl_ff02_val(param->length,param->value,0xff);
    }
    */
    return (KERNEL_MSG_CONSUMED);
}





static int btl_gattc_cmp_evt_handler(kernel_msg_id_t const msgid,  struct gattc_cmp_evt const *param,
                                 kernel_task_id_t const dest_id, kernel_task_id_t const src_id)
{

    if(param->operation == GATTC_INDICATE  && param->status == GAP_ERR_NO_ERROR)
     {
        
    }
	
    return KERNEL_MSG_CONSUMED;
}


/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
const struct kernel_msg_handler app_btl_msg_handler_list[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KERNEL_MSG_DEFAULT_HANDLER,         (kernel_msg_func_t)app_btl_msg_dflt_handler},
    {BTL_FF02_VALUE_IND_CFG_IND,   (kernel_msg_func_t)btl_ff02_level_ind_cfg_ind_handler},
    {BTL_FF02_VALUE_UPD_RSP,       (kernel_msg_func_t)btl_ff02_level_upd_handler},
	{BTL_FF01_REQ_IND,		 (kernel_msg_func_t)btl_ff01_req_handler},
	{BTL_GATTC_CMP_EVT,            (kernel_msg_func_t)btl_gattc_cmp_evt_handler},
};

const struct kernel_state_handler app_btl_table_handler =
    {&app_btl_msg_handler_list[0], (sizeof(app_btl_msg_handler_list)/sizeof(struct kernel_msg_handler))};

#endif //(BLE_APP_BTL)

