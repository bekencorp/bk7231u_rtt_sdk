/**
 ****************************************************************************************
 *
 * @file app_ffe0.c
 *
 * @brief ffe0 Application Module entry point
 *
 * @auth  gang.cheng
 *
 * @date  2017.03.29
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

#if (BLE_APP_FFE0)
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>
#include "app_ffe0.h"                // Battery Application Module Definitions
#include "application.h"                     // Application Definitions
#include "app_task.h"                // application task definitions
#include "ffe0s_task.h"               // health thermometer functions
#include "common_bt.h"
#include "prf_types.h"               // Profile common types definition
#include "architect.h"                    // Platform Definitions
#include "prf.h"
#include "ffe0s.h"
#include "kernel_timer.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// fff0 Application Module Environment Structure
struct app_ffe0_env_tag app_ffe0_env;

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void app_ffe0_init(void)
{
    // Reset the environment
    memset(&app_ffe0_env, 0, sizeof(struct app_ffe0_env_tag));
}


void app_ffe0_add_ffe0s(void)
{

   	struct ffe0s_db_cfg *db_cfg;
		
	struct gapm_profile_task_add_cmd *req = KERNEL_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                                  TASK_GAPM, TASK_APP,
                                                  gapm_profile_task_add_cmd, sizeof(struct ffe0s_db_cfg));
    // Fill message
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl = 0;
    req->prf_task_id = TASK_ID_FFE0S;
    req->app_task = TASK_APP;
    req->start_hdl = 0; //req->start_hdl = 0; dynamically allocated

	  
	//Set parameters
    db_cfg = (struct ffe0s_db_cfg* ) req->param;
	db_cfg->cfg_flag = 0x1f;
    //Sending of notifications is supported
    db_cfg->features = FFE0_FFE1_LVL_NTF_SUP;
    //Send the message
    kernel_msg_send(req);
}


void app_ffe0_enable_prf(uint8_t conidx)
{

    app_ffe0_env.conidx = conidx;

    // Allocate the message
    struct ffe0s_enable_req * req = KERNEL_MSG_ALLOC(FFE0S_ENABLE_REQ,
                                                prf_get_task_from_id(TASK_ID_FFE0S),
                                                TASK_APP,
                                                ffe0s_enable_req);
    // Fill in the parameter structure
    req->conidx             = conidx;

    // NTF initial status - Disabled
    req->ntf_cfg           = PRF_CLI_STOP_NTFIND;
    
    // Send the message
    kernel_msg_send(req);
}




void app_ffe1_send_val(uint8_t len,uint8_t *buf,uint16_t seq_num)
{
    // Allocate the message
    struct ffe0s_ffe1_value_upd_req * req = KERNEL_MSG_ALLOC_DYN(FFE0S_FFE1_VALUE_UPD_REQ,
                                                        prf_get_task_from_id(TASK_ID_FFE0S),
                                                        TASK_APP,
                                                        ffe0s_ffe1_value_upd_req,len);

    // Fill in the parameter structure
	req->seq_num = seq_num;
	req->length = len;
    memcpy(req->value,buf,len);

    // Send the message
    kernel_msg_send(req);
}

static int ffe0s_ffe1_level_ntf_cfg_ind_handler(kernel_msg_id_t const msgid,
                                               struct ffe0s_ffe1_value_ntf_cfg_ind const *param,
                                               kernel_task_id_t const dest_id,
                                               kernel_task_id_t const src_id)
{
   
	//UART_PRINTF("param->ntf_cfg = %x\r\n",param->ntf_cfg);

	app_ffe0_env.ffe1_ntf_cfg = param->ntf_cfg;

	if(param->ntf_cfg == PRF_CLI_STOP_NTFIND)
	{
		;
	}else
	{
		;
	}
    
    return (KERNEL_MSG_CONSUMED);
}

static int ffe1_level_upd_handler(kernel_msg_id_t const msgid,
                                      struct ffe0s_ffe1_value_upd_rsp const *param,
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
static int app_ffe0_msg_dflt_handler(kernel_msg_id_t const msgid,
                                     void const *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{
    // Drop the message
    return (KERNEL_MSG_CONSUMED);
}



static int ffe1_writer_req_handler(kernel_msg_id_t const msgid,
                                     struct ffe0s_ffe1_writer_ind *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{
    // Drop the message
	//UART_PRINTF("FFE1 param->value = 0x ");
	uint8_t i = 0;
	for( i = 0;i < param->length;i++)
	{
		//UART_PRINTF("%02x ",param->value[i]);
	}

	//UART_PRINTF("\r\n");
    return (KERNEL_MSG_CONSUMED);
}


static int ffe1_read_req_handler(kernel_msg_id_t const msgid,
                                     struct ffe0s_ffe1_read_req_ind *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{
    // Drop the message
	// UART_PRINTF("%s\r\n",__func__);
	 uint8_t len = 20;
	
	 static uint8_t a = 20;
	 struct ffe0s_ffe1_read_rsp * rsp = KERNEL_MSG_ALLOC_DYN(FFE0S_FFE1_READ_RSP,
                                                        prf_get_task_from_id(TASK_ID_FFE0S),
                                                        TASK_APP,
                                                        ffe0s_ffe1_read_rsp,len);

    // Fill in the parameter structure
	rsp->length = 20;
	rsp->handle = param->handle;
	rsp->rsp_id = param->rsp_id;
	rsp->value[0]   = a++;
    // Send the message
    kernel_msg_send(rsp);

		
		
    return (KERNEL_MSG_CONSUMED);
}



static int ffe0_gattc_cmp_evt_handler(kernel_msg_id_t const msgid,  struct gattc_cmp_evt const *param,
                                 kernel_task_id_t const dest_id, kernel_task_id_t const src_id)
{
	
    return KERNEL_MSG_CONSUMED;
}


/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
const struct kernel_msg_handler app_ffe0_msg_handler_list[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KERNEL_MSG_DEFAULT_HANDLER,         (kernel_msg_func_t)app_ffe0_msg_dflt_handler},
    {FFE0S_FFE1_VALUE_NTF_CFG_IND,   (kernel_msg_func_t)ffe0s_ffe1_level_ntf_cfg_ind_handler},
    {FFE0S_FFE1_VALUE_UPD_RSP,       (kernel_msg_func_t)ffe1_level_upd_handler},
	{FFE0S_FFE1_WRITER_REQ_IND,		 (kernel_msg_func_t)ffe1_writer_req_handler},
	{FFE0S_FFE1_READ_REQ_IND,		 (kernel_msg_func_t)ffe1_read_req_handler},
	{FFE0S_GATTC_CMP_EVT,            (kernel_msg_func_t)ffe0_gattc_cmp_evt_handler},
};

const struct kernel_state_handler app_ffe0_table_handler =
    {&app_ffe0_msg_handler_list[0], (sizeof(app_ffe0_msg_handler_list)/sizeof(struct kernel_msg_handler))};

#endif //BLE_APP_BATT
