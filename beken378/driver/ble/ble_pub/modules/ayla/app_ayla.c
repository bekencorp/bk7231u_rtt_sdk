/**
 ****************************************************************************************
 *
 * @file app_ayla.c
 *
 * @brief AYLA Application Module entry point
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

#if (BLE_APP_AYLA)
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>
#include "app_ayla.h"                //  Application Module Definitions
#include "application.h"                     // Application Definitions
#include "app_task.h"                // application task definitions
#include "ayla_task.h"               // health thermometer functions
#include "common_bt.h"
#include "prf_types.h"               // Profile common types definition
#include "architect.h"                    // Platform Definitions
#include "prf.h"
#include "ayla.h"
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
struct app_ayla_env_tag app_ayla_env;

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void app_ayla_init(void)
{
    // Reset the environment
    memset(&app_ayla_env, 0, sizeof(struct app_ayla_env_tag));
}


void app_ayla_add_ayla(void)
{

    printf("app_ayla_add_ayla\r\n");

   	struct ayla_db_cfg *db_cfg;
		
	struct gapm_profile_task_add_cmd *req = KERNEL_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                                  TASK_GAPM, TASK_APP,
                                                  gapm_profile_task_add_cmd, sizeof(struct ayla_db_cfg));
    // Fill message
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl = 0;
    req->prf_task_id = TASK_ID_AYLA;
    req->app_task = TASK_APP;
    req->start_hdl = 0; //req->start_hdl = 0; dynamically allocated

	  
	//Set parameters
    db_cfg = (struct ayla_db_cfg* ) req->param;
	db_cfg->cfg_flag = 0xffff;
    //Sending of notifications is supported
    db_cfg->features = AYLA_FFA1_LVL_NTF_SUP;
    //Send the message
    kernel_msg_send(req);
}


void app_ayla_enable_prf(uint8_t conidx)
{

    app_ayla_env.conidx = conidx;

    // Allocate the message
    struct ayla_enable_req * req = KERNEL_MSG_ALLOC(AYLA_ENABLE_REQ,
                                                prf_get_task_from_id(TASK_ID_AYLA),
                                                TASK_APP,
                                                ayla_enable_req);
    // Fill in the parameter structure
    req->conidx             = conidx;

    // IND initial status - Disabled
    req->ind_cfg           = PRF_CLI_STOP_NTFIND;
    
    // Send the message
    kernel_msg_send(req);
}

void app_ayla_ffa1_val(uint8_t len,uint8_t *buf,uint16_t seq_num)//send data
{
    int i;

    // Allocate the message
    struct ayla_ffa1_value_upd_req * req = KERNEL_MSG_ALLOC_DYN(AYLA_FFA1_VALUE_UPD_REQ,
                                                        prf_get_task_from_id(TASK_ID_AYLA),
                                                        TASK_APP,
                                                        ayla_ffa1_value_upd_req,len);

    // Fill in the parameter structure
	req->seq_num = seq_num;
	req->length = len;
    memcpy(req->value,buf,len);

    // Send the message
    kernel_msg_send(req);
}

static int ayla_ffa1_level_ntf_cfg_ind_handler(kernel_msg_id_t const msgid,
                                               struct ayla_ffa1_value_ntf_cfg_ind const *param,
                                               kernel_task_id_t const dest_id,
                                               kernel_task_id_t const src_id)
{
	app_ayla_env.send_ntf_cfg = param->ntf_cfg;

	if(param->ntf_cfg == PRF_CLI_STOP_NTFIND)
	{
		;
	}else
	{
		;
	}
    
    return (KERNEL_MSG_CONSUMED);
}

static int ayla_ffa1_level_upd_handler(kernel_msg_id_t const msgid,
                                      struct ayla_ffa1_value_upd_rsp const *param,
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
static int app_ayla_msg_dflt_handler(kernel_msg_id_t const msgid,
                                     void const *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{
    // Drop the message
    return (KERNEL_MSG_CONSUMED);
}



static int ayla_ffa1_write_handler(kernel_msg_id_t const msgid,
                                     struct ayla_ffa1_write_ind *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{
    if(ble_write_cb)
    {
        ble_write_cb(0xFFA1,param->value,param->length);
    }

    /*
    if(param->value[0]== 0x55)
    {
        app_ayla_ffa1_val(param->length,param->value,0xff);
    }
    */
    
    return (KERNEL_MSG_CONSUMED);
}





static int ayla_gattc_cmp_evt_handler(kernel_msg_id_t const msgid,  struct gattc_cmp_evt const *param,
                                 kernel_task_id_t const dest_id, kernel_task_id_t const src_id)
{

    if(param->operation == GATTC_NOTIFY  && param->status == GAP_ERR_NO_ERROR)
     {
        
    }
	
    return KERNEL_MSG_CONSUMED;
}


/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
const struct kernel_msg_handler app_ayla_msg_handler_list[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KERNEL_MSG_DEFAULT_HANDLER,         (kernel_msg_func_t)app_ayla_msg_dflt_handler},
    {AYLA_FFA1_VALUE_NTF_CFG_IND,   (kernel_msg_func_t)ayla_ffa1_level_ntf_cfg_ind_handler},
    {AYLA_FFA1_VALUE_UPD_RSP,       (kernel_msg_func_t)ayla_ffa1_level_upd_handler},
	{AYLA_FFA1_WRITE_IND,		 (kernel_msg_func_t)ayla_ffa1_write_handler},
	{AYLA_GATTC_CMP_EVT,            (kernel_msg_func_t)ayla_gattc_cmp_evt_handler},
};

const struct kernel_state_handler app_ayla_table_handler =
    {&app_ayla_msg_handler_list[0], (sizeof(app_ayla_msg_handler_list)/sizeof(struct kernel_msg_handler))};

#endif //(BLE_APP_AYLA)

