/**
 ****************************************************************************************
 *
 * @file app_ayla_gen.c
 *
 * @brief AYLA_GEN Application Module entry point
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

#if (BLE_APP_AYLA_GEN)
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>
#include "app_ayla_gen.h"                //  Application Module Definitions
#include "application.h"                     // Application Definitions
#include "app_task.h"                // application task definitions
#include "ayla_gen_task.h"               // health thermometer functions
#include "common_bt.h"
#include "prf_types.h"               // Profile common types definition
#include "architect.h"                    // Platform Definitions
#include "prf.h"
#include "ayla_gen.h"
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
struct app_ayla_gen_env_tag app_ayla_gen_env;

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void app_ayla_gen_init(void)
{
    // Reset the environment
    memset(&app_ayla_gen_env, 0, sizeof(struct app_ayla_gen_env_tag));
}


void app_ayla_gen_add_gen(void)
{

    printf("app_ayla_gen_add_gen\r\n");

   	struct ayla_gen_db_cfg *db_cfg;
		
	struct gapm_profile_task_add_cmd *req = KERNEL_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                                  TASK_GAPM, TASK_APP,
                                                  gapm_profile_task_add_cmd, sizeof(struct ayla_gen_db_cfg));
    // Fill message
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl = 0;
    req->prf_task_id = TASK_ID_AYLA_GEN;
    req->app_task = TASK_APP;
    req->start_hdl = 0; //req->start_hdl = 0; dynamically allocated

	  
	//Set parameters
    db_cfg = (struct ayla_gen_db_cfg* ) req->param;
	db_cfg->cfg_flag = 0xffff;
    //Send the message
    kernel_msg_send(req);
}


void app_ayla_gen_enable_prf(uint8_t conidx)
{

    app_ayla_gen_env.conidx = conidx;

    // Allocate the message
    struct ayla_gen_enable_req * req = KERNEL_MSG_ALLOC(AYLA_GEN_ENABLE_REQ,
                                                prf_get_task_from_id(TASK_ID_AYLA_GEN),
                                                TASK_APP,
                                                ayla_gen_enable_req);
    // Fill in the parameter structure
    req->conidx             = conidx;
    
    // Send the message
    kernel_msg_send(req);
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
static int app_ayla_gen_msg_dflt_handler(kernel_msg_id_t const msgid,
                                     void const *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{
    // Drop the message
    return (KERNEL_MSG_CONSUMED);
}

static int ayla_gen_idid_write_req_ind_handler(kernel_msg_id_t const msgid,
                                     struct ayla_gen_idid_write_ind *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{    
    if(ble_write_cb != NULL)
    {
        ble_write_cb(0x0005,param->value,param->length);
    }
    	
    return (KERNEL_MSG_CONSUMED);
}

static int ayla_gen_dnid_write_req_ind_handler(kernel_msg_id_t const msgid,
                                     struct ayla_gen_dnid_write_ind *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{    
    if(ble_write_cb != NULL)
    {
        ble_write_cb(0x0006,param->value,param->length);
    }
    	
    return (KERNEL_MSG_CONSUMED);
}


static int ayla_gen_gattc_cmp_evt_handler(kernel_msg_id_t const msgid,  struct gattc_cmp_evt const *param,
                                 kernel_task_id_t const dest_id, kernel_task_id_t const src_id)
{	
    bk_printf("%s,operation = %x,status = 0x%x,seq_num = 0x%x\r\n",__func__,param->operation,param->status,param->seq_num);
    return KERNEL_MSG_CONSUMED;
}


/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
const struct kernel_msg_handler app_ayla_gen_msg_handler_list[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KERNEL_MSG_DEFAULT_HANDLER,         (kernel_msg_func_t)app_ayla_gen_msg_dflt_handler},
    {AYLA_GEN_IDID_WRITE_IND,            (kernel_msg_func_t)ayla_gen_idid_write_req_ind_handler},
    {AYLA_GEN_DNID_WRITE_IND,            (kernel_msg_func_t)ayla_gen_dnid_write_req_ind_handler},
	{AYLA_GEN_GATTC_CMP_EVT,             (kernel_msg_func_t)ayla_gen_gattc_cmp_evt_handler},
};

const struct kernel_state_handler app_ayla_gen_table_handler =
    {&app_ayla_gen_msg_handler_list[0], (sizeof(app_ayla_gen_msg_handler_list)/sizeof(struct kernel_msg_handler))};

#endif //(BLE_APP_AYLA_GEN)

