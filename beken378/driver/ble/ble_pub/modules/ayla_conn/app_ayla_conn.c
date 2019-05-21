/**
 ****************************************************************************************
 *
 * @file app_ayla_conn.c
 *
 * @brief AYLA_CONN Application Module entry point
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

#if (BLE_APP_AYLA_CONN)
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>
#include "app_ayla_conn.h"                //  Application Module Definitions
#include "application.h"                     // Application Definitions
#include "app_task.h"                // application task definitions
#include "ayla_conn_task.h"               // health thermometer functions
#include "common_bt.h"
#include "prf_types.h"               // Profile common types definition
#include "architect.h"                    // Platform Definitions
#include "prf.h"
#include "ayla_conn.h"
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
struct app_ayla_conn_env_tag app_ayla_conn_env;

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void app_ayla_conn_init(void)
{
    // Reset the environment
    memset(&app_ayla_conn_env, 0, sizeof(struct app_ayla_conn_env_tag));
}


void app_ayla_conn_add_conn(void)
{

    printf("app_ayla_conn_add_conn\r\n");

   	struct ayla_conn_db_cfg *db_cfg;
		
	struct gapm_profile_task_add_cmd *req = KERNEL_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                                  TASK_GAPM, TASK_APP,
                                                  gapm_profile_task_add_cmd, sizeof(struct ayla_conn_db_cfg));
    // Fill message
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl = PERM(SVC_UUID_LEN, UUID_128);
    req->prf_task_id = TASK_ID_AYLA_CONN;
    req->app_task = TASK_APP;
    req->start_hdl = 0; //req->start_hdl = 0; dynamically allocated

	  
	//Set parameters
    db_cfg = (struct ayla_conn_db_cfg* ) req->param;
	db_cfg->cfg_flag = 0xffff;
    //Send the message
    kernel_msg_send(req);
}


void app_ayla_conn_enable_prf(uint8_t conidx)
{

    app_ayla_conn_env.conidx = conidx;

    // Allocate the message
    struct ayla_conn_enable_req * req = KERNEL_MSG_ALLOC(AYLA_CONN_ENABLE_REQ,
                                                prf_get_task_from_id(TASK_ID_AYLA_CONN),
                                                TASK_APP,
                                                ayla_conn_enable_req);
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
static int app_ayla_conn_msg_dflt_handler(kernel_msg_id_t const msgid,
                                     void const *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{
    // Drop the message
    return (KERNEL_MSG_CONSUMED);
}

static int ayla_conn_seto_write_req_ind_handler(kernel_msg_id_t const msgid,
                                     struct ayla_conn_seto_write_ind *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{    
    if(ble_write_cb != NULL)
    {
        ble_write_cb(0x69ED,param->value,param->length);
    }
    	
    return (KERNEL_MSG_CONSUMED);
}

static int ayla_conn_gattc_cmp_evt_handler(kernel_msg_id_t const msgid,  struct gattc_cmp_evt const *param,
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
const struct kernel_msg_handler app_ayla_conn_msg_handler_list[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KERNEL_MSG_DEFAULT_HANDLER,         (kernel_msg_func_t)app_ayla_conn_msg_dflt_handler},
    {AYLA_CONN_SETO_WRITE_IND,           (kernel_msg_func_t)ayla_conn_seto_write_req_ind_handler},
	{AYLA_CONN_GATTC_CMP_EVT,            (kernel_msg_func_t)ayla_conn_gattc_cmp_evt_handler},
};

const struct kernel_state_handler app_ayla_conn_table_handler =
    {&app_ayla_conn_msg_handler_list[0], (sizeof(app_ayla_conn_msg_handler_list)/sizeof(struct kernel_msg_handler))};

#endif //(BLE_APP_AYLA_CONN)

