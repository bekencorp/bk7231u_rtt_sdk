/**
 ****************************************************************************************
 *
 * @file app_fff0.c
 *
 * @brief fff0 Application Module entry point
 *
 * @auth  gang.cheng
 *
 * @date  2016.05.31
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

#if (BLE_APP_FFF0)
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>
#include "app_fff0.h"              // Battery Application Module Definitions
#include "application.h"                    // Application Definitions
#include "app_task.h"             // application task definitions
#include "Fff0s_task.h"           // health thermometer functions
#include "common_bt.h"
#include "prf_types.h"             // Profile common types definition
#include "architect.h"                    // Platform Definitions
#include "prf.h"
#include "fff0s.h"
#include "kernel_timer.h"
#include "uart.h"

#include "app_hid.h"


/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// fff0 Application Module Environment Structure
struct app_fff0_env_tag app_fff0_env;

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */


void app_fff0_init(void)
{

    // Reset the environment
    memset(&app_fff0_env, 0, sizeof(struct app_fff0_env_tag));

    // Initial battery level: 100
    app_fff0_env.fff1_lvl = 100;
}

void app_fff0_add_fff0s(void)
{

   struct fff0s_db_cfg *db_cfg;
		
   struct gapm_profile_task_add_cmd *req = KERNEL_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                                  TASK_GAPM, TASK_APP,
                                                  gapm_profile_task_add_cmd, sizeof(struct fff0s_db_cfg));
    // Fill message
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl =   0;//PERM(SVC_AUTH, ENABLE);
    req->prf_task_id = TASK_ID_FFF0S;
    req->app_task = TASK_APP;
    req->start_hdl = 0; //req->start_hdl = 0; dynamically allocated

	 
    // Set parameters
    db_cfg = (struct fff0s_db_cfg* ) req->param;
	 
    // Sending of notifications is supported
    db_cfg->features = FFF0_FFF1_LVL_NTF_SUP;
    // Send the message
    kernel_msg_send(req);
}

void app_fff0_enable_prf(uint8_t conidx)
{

    app_fff0_env.conidx = conidx;

    // Allocate the message
    struct fff0s_enable_req * req = KERNEL_MSG_ALLOC(FFF0S_ENABLE_REQ,
                                                prf_get_task_from_id(TASK_ID_FFF0S),
                                                TASK_APP,
                                                fff0s_enable_req);

    // Fill in the parameter structure
    req->conidx             = conidx;

    // NTF initial status - Disabled
    req->ntf_cfg        = PRF_CLI_STOP_NTFIND;
    req->old_fff1_lvl   = 60;

    // Send the message
    kernel_msg_send(req);
}




void app_fff1_send_lvl(uint8_t fff1_lvl)
{
    ASSERT_ERR(fff1_lvl <= FFF1_LVL_MAX);

    // Allocate the message
    struct fff0s_fff1_level_upd_req * req = KERNEL_MSG_ALLOC(FFF0S_FFF1_LEVEL_UPD_REQ,
                                                        prf_get_task_from_id(TASK_ID_FFF0S),
                                                        TASK_APP,
                                                        fff0s_fff1_level_upd_req);

    // Fill in the parameter structure	
    req->length = 20;
    req->fff1_level[0] = fff1_lvl;

    // Send the message
    kernel_msg_send(req);
}

static int fff0s_fff1_level_ntf_cfg_ind_handler(kernel_msg_id_t const msgid,
                                               struct fff0s_fff1_level_ntf_cfg_ind const *param,
                                               kernel_task_id_t const dest_id,
                                               kernel_task_id_t const src_id)
{
	//UART_PRINTF("param->ntf_cfg = %x\r\n",param->ntf_cfg);
	if(param->ntf_cfg == PRF_CLI_STOP_NTFIND)
	{
		kernel_timer_clear(FFF0S_FFF1_LEVEL_PERIOD_NTF,dest_id);
	}else
	{
		//kernel_timer_set(FFF0S_FFF1_LEVEL_PERIOD_NTF,dest_id , 300);
	}
    
    return (KERNEL_MSG_CONSUMED);
}

static int fff1_level_upd_handler(kernel_msg_id_t const msgid,
                                      struct fff0s_fff1_level_upd_rsp const *param,
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
static int app_fff0_msg_dflt_handler(kernel_msg_id_t const msgid,
                                     void const *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{
    // Drop the message
    return (KERNEL_MSG_CONSUMED);
}

static int fff2_writer_req_handler(kernel_msg_id_t const msgid,
                                     struct fff0s_fff2_writer_ind *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{
    // Drop the message
	//UART_PRINTF("FFF2 param->value = 0x ");
	for(uint8_t i = 0;i < param->length;i++)
	{
		//UART_PRINTF("%02x ",param->fff2_value[i]);
	}
	//UART_PRINTF("\r\n");
		
    return (KERNEL_MSG_CONSUMED);
}


static int fff1_period_ntf_handler(kernel_msg_id_t const msgid,
                                               struct fff0s_fff1_level_ntf_cfg_ind const *param,
                                               kernel_task_id_t const dest_id,
                                               kernel_task_id_t const src_id)
{
	static uint8_t count = 0;

    //UART_PRINTF("fff1_period_ntf_handler\r\n");		
    count++;
    if(count == 100)
    {
	 count = 0;
    }
    app_fff1_send_lvl(count);
    kernel_timer_set(FFF0S_FFF1_LEVEL_PERIOD_NTF,dest_id , 20);
		
    return (KERNEL_MSG_CONSUMED);
}



/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
const struct kernel_msg_handler app_fff0_msg_handler_list[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KERNEL_MSG_DEFAULT_HANDLER,        (kernel_msg_func_t)app_fff0_msg_dflt_handler},
    {FFF0S_FFF1_LEVEL_NTF_CFG_IND,  (kernel_msg_func_t)fff0s_fff1_level_ntf_cfg_ind_handler},
    {FFF0S_FFF1_LEVEL_UPD_RSP,      (kernel_msg_func_t)fff1_level_upd_handler},
    {FFF0S_FFF2_WRITER_REQ_IND,		(kernel_msg_func_t)fff2_writer_req_handler},
    {FFF0S_FFF1_LEVEL_PERIOD_NTF,	(kernel_msg_func_t)fff1_period_ntf_handler},
};

const struct kernel_state_handler app_fff0_table_handler =
    {&app_fff0_msg_handler_list[0], (sizeof(app_fff0_msg_handler_list)/sizeof(struct kernel_msg_handler))};

#endif //BLE_APP_BATT

