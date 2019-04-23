/**
 ****************************************************************************************
 *
 * @file app_braces.c
 *
 * @brief Bracelet Application Module entry point
 *
 * @auth  gang.cheng
 *
 * @date  2016.09.06
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

#if (BLE_APP_OADS)
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>
#include "app_oads.h"                // Bracese Application Module Definitions
#include "application.h"                     // Application Definitions
#include "app_task.h"                // application task definitions
#include "oads.h"
#include "oads_task.h"               // health thermometer functions
#include "common_bt.h"
#include "prf_types.h"               // Profile common types definition
#include "architect.h"                    // Platform Definitions
#include "prf.h"
#include "kernel_env.h"
#include "lld_evt.h"
#include "uart.h"


		
/*
 * LOCATION FUN DEFINES
 ****************************************************************************************
 */



/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// braces Application Module Environment Structure
struct app_oads_env_tag app_oads_env;



/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void app_oads_init(void)
{

    // Reset the environment
    memset(&app_oads_env, 0, sizeof(struct app_oads_env_tag));
		
 
}

void app_oad_add_oads(void)
{

	 //UART_PRINTF("app_oad_add_oads\r\n");
   struct oads_db_cfg *db_cfg;
		
	 struct gapm_profile_task_add_cmd *req = KERNEL_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                                  TASK_GAPM, TASK_APP,
                                                  gapm_profile_task_add_cmd, sizeof(struct oads_db_cfg));
    // Fill message
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl = 0;//PERM(SVC_AUTH, ENABLE);
    req->prf_task_id = TASK_ID_OADS;
    req->app_task = TASK_APP;
    req->start_hdl = 0; //req->start_hdl = 0; dynamically allocated

	 
	  // Set parameters
    db_cfg = (struct oads_db_cfg* ) req->param;
	 
    // Sending of notifications is supported
    db_cfg->features = OADS_NTF_SUP;
		//UART_PRINTF("app_oad_add_oads d = %x,s = %x\r\n",TASK_GAPM,TASK_APP);
    // Send the message
    kernel_msg_send(req);
}

void app_oads_enable_prf(uint8_t conidx)
{

    app_oads_env.conidx = conidx;

    // Allocate the message
    struct oads_enable_req * req = KERNEL_MSG_ALLOC(OADS_ENABLE_REQ,
                                                prf_get_task_from_id(TASK_ID_OADS),
                                                TASK_APP,
                                                oads_enable_req);

    // Fill in the parameter structure
    req->conidx             = conidx;

    // NTF initial status - Disabled
		req->ffc1_ntf_cfg           = PRF_CLI_STOP_NTFIND;
		req->ffc2_ntf_cfg           = PRF_CLI_STOP_NTFIND;


    // Send the message
    kernel_msg_send(req);
}







static int oads_enable_rsp_handler(kernel_msg_id_t const msgid,
                                    struct oads_enable_rsp const *param,
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
static int app_oads_msg_dflt_handler(kernel_msg_id_t const msgid,
                                     void const *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{
    // Drop the message
		//UART_PRINTF("%s\r\n",__func__);
		//UART_PRINTF("msgid = 0x%04x,destid = 0x%x,srcid = 0x%x\r\n",msgid,dest_id,src_id);
    return (KERNEL_MSG_CONSUMED);
}




static int app_ffc1_writer_req_handler(kernel_msg_id_t const msgid,
                                     struct oads_ffc1_writer_ind *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{
    // Drop the message
	
	  //UART_PRINTF("%s\r\n",__func__);
	
		oadImgIdentifyWrite(0x0, param->length,param->data );
	


    return (KERNEL_MSG_CONSUMED);
}




static int app_ffc2_writer_req_handler(kernel_msg_id_t const msgid,
                                     struct oads_ffc2_writer_ind *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{
    // Drop the message
	
	  //UART_PRINTF("%s\r\n",__func__);
	//	oadImgBlockWrite(0, param->data);


    return (KERNEL_MSG_CONSUMED);
}


void app_ffc1_ntf_req(void)
{
    // Drop the message
	
	  //UART_PRINTF("%s\r\n",__func__);
	
		
	 struct oads_ffc1_upd_req * req = KERNEL_MSG_ALLOC(OADS_FFC1_UPD_REQ,
                                                        prf_get_task_from_id(TASK_ID_OADS),
                                                        TASK_APP,
                                                        oads_ffc1_upd_req);

    // Fill in the parameter structure
		
		req->length = 20;
    req->data[0]   = 100;

    // Send the message
    kernel_msg_send(req);
	
}

void app_ffc2_ntf_req(void)
{
    // Drop the message
	
	  //UART_PRINTF("%s\r\n",__func__);
	
		
	 struct oads_ffc2_upd_req * req = KERNEL_MSG_ALLOC(OADS_FFC2_UPD_REQ,
                                                        prf_get_task_from_id(TASK_ID_OADS),
                                                        TASK_APP,
                                                        oads_ffc2_upd_req);

    // Fill in the parameter structure
		
		req->length = 20;
    req->data[0]   = 50;

    // Send the message
    kernel_msg_send(req);
	
}





static int oads_ffc1_upd_rsp_handler(kernel_msg_id_t const msgid,
                                     struct oads_ffc1_upd_rsp *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{
	
	  //UART_PRINTF("%s\r\n",__func__);
		if(param->status == GAP_ERR_NO_ERROR)
		{
				
						
				
		}
		
		
		return (KERNEL_MSG_CONSUMED);
}

static int oads_ffc2_upd_rsp_handler(kernel_msg_id_t const msgid,
                                     struct oads_ffc2_upd_rsp *param,
                                     kernel_task_id_t const dest_id,
                                     kernel_task_id_t const src_id)
{
	
	  //UART_PRINTF("%s\r\n",__func__);
		if(param->status == GAP_ERR_NO_ERROR)
		{
				
						
				
		}
		
		
		return (KERNEL_MSG_CONSUMED);
}






/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
const struct kernel_msg_handler app_oads_msg_handler_list[] =
{
    // Note: first message is latest message checked by kernel so default is put on top.
    {KERNEL_MSG_DEFAULT_HANDLER,          (kernel_msg_func_t)app_oads_msg_dflt_handler},
		
		{OADS_ENABLE_RSP,         			(kernel_msg_func_t)oads_enable_rsp_handler},
		
    {OADS_FFC1_WRITER_REQ_IND,      (kernel_msg_func_t)app_ffc1_writer_req_handler},
		
		{OADS_FFC1_UPD_RSP,							(kernel_msg_func_t)oads_ffc1_upd_rsp_handler},
		
		{OADS_FFC2_WRITER_REQ_IND,      (kernel_msg_func_t)app_ffc2_writer_req_handler},
		
		{OADS_FFC2_UPD_RSP,							(kernel_msg_func_t)oads_ffc2_upd_rsp_handler},


};

const struct kernel_state_handler app_oads_table_handler =
    {&app_oads_msg_handler_list[0], (sizeof(app_oads_msg_handler_list)/sizeof(struct kernel_msg_handler))};

#endif //BLE_APP_BATT
