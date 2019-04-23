/**
 ****************************************************************************************
 *
 * @file fff0s.c
 *
 * @brief fff0 Server Implementation.
 *
 * Copyright (C) beken 2009-2015
 *
 *
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_FFF0_SERVER)
#include "attm.h"
#include "fff0s.h"
#include "fff0s_task.h"
#include "prf_utils.h"
#include "prf.h"
#include "kernel_mem.h"




/*
 * FFF0 ATTRIBUTES DEFINITION
 ****************************************************************************************
 */
 
/// Full FFF0 Database Description - Used to add attributes into the database
const struct attm_desc fff0_att_db[FFF0S_IDX_NB] =
{
    // FFF0 Service Declaration
    [FFF0S_IDX_SVC]            =   {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), 0, 0},

	[FFF0S_IDX_FFF2_LVL_CHAR]  =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    //  Characteristic Value
    [FFF0S_IDX_FFF2_LVL_VAL]   =   {ATT_USER_SERVER_CHAR_FFF2,PERM(WRITE_COMMAND, ENABLE), PERM(RI, ENABLE), FFF0_FFF2_DATA_LEN *sizeof(uint8_t)},

	// fff1 Level Characteristic Declaration
	[FFF0S_IDX_FFF1_LVL_CHAR]  =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // fff1 Level Characteristic Value
    [FFF0S_IDX_FFF1_LVL_VAL]   =   {ATT_USER_SERVER_CHAR_FFF1, PERM(WRITE_COMMAND, ENABLE) , PERM(RI, ENABLE), FFF0_FFF1_DATA_LEN * sizeof(uint8_t)},

	// fff1 Level Characteristic - Client Characteristic Configuration Descriptor
	[FFF0S_IDX_FFF1_LVL_NTF_CFG] = {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},

};/// Macro used to retrieve permission value from access and rights on attribute.


static uint8_t fff0s_init (struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl,  struct fff0s_db_cfg* params)
{
    uint16_t shdl;
    struct fff0s_env_tag* fff0s_env = NULL;
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    
    //-------------------- allocate memory required for the profile  ---------------------
    fff0s_env = (struct fff0s_env_tag* ) kernel_malloc(sizeof(struct fff0s_env_tag), KERNEL_MEM_ATT_DB);
    memset(fff0s_env, 0 , sizeof(struct fff0s_env_tag));

    // Service content flag
    uint8_t cfg_flag = FFF0S_CFG_FLAG_MANDATORY_MASK;

    // Save database configuration
    fff0s_env->features |= (params->features) ;
   
    // Check if notifications are supported
    if (params->features == FFF0_FFF1_LVL_NTF_SUP)
    {
        cfg_flag |= FFF0_CFG_FLAG_NTF_SUP_MASK;
    }
    shdl = *start_hdl;

    //Create FFF0 in the DB
    //------------------ create the attribute database for the profile -------------------
    status = attm_svc_create_db(&(shdl), ATT_USER_SERVER_FFF0, (uint8_t *)&cfg_flag,
            FFF0S_IDX_NB, NULL, env->task, &fff0_att_db[0],
            (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)));
				


    //Set optional permissions
    if (status == GAP_ERR_NO_ERROR)
    {
        //Set optional permissions
        if(params->features == FFF0_FFF1_LVL_NTF_SUP)
        {
            // Battery Level characteristic value permissions
            uint16_t perm = PERM(RD, ENABLE) | PERM(NTF, ENABLE);

            attm_att_set_permission(shdl + FFF0S_IDX_FFF1_LVL_VAL, perm, 0);
        }
    }

    //-------------------- Update profile task information  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {

        // allocate BASS required environment variable
        env->env = (prf_env_t*) fff0s_env;
        *start_hdl = shdl;
        fff0s_env->start_hdl = *start_hdl;
        fff0s_env->prf_env.app_task = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        fff0s_env->prf_env.prf_task = env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        env->id                     = TASK_ID_FFF0S;
        env->desc.idx_max           = FFF0S_IDX_MAX;
        env->desc.state             = fff0s_env->state;
        env->desc.default_handler   = &fff0s_default_handler;

        // service is ready, go into an Idle state
        kernel_state_set(env->task, FFF0S_IDLE);
    }
    else if(fff0s_env != NULL)
    {
        kernel_free(fff0s_env);
    }
     
    return (status);
}


static void fff0s_destroy(struct prf_task_env* env)
{
    struct fff0s_env_tag* fff0s_env = (struct fff0s_env_tag*) env->env;

    // clear on-going operation
    if(fff0s_env->operation != NULL)
    {
        kernel_free(fff0s_env->operation);
    }

    // free profile environment variables
    env->env = NULL;
    kernel_free(fff0s_env);
}

static void fff0s_create(struct prf_task_env* env, uint8_t conidx)
{
    struct fff0s_env_tag* fff0s_env = (struct fff0s_env_tag*) env->env;
    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);

    // force notification config to zero when peer device is connected
    fff0s_env->ntf_cfg[conidx] = 0;
}


static void fff0s_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
    struct fff0s_env_tag* fff0s_env = (struct fff0s_env_tag*) env->env;

    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);
    // force notification config to zero when peer device is disconnected
    fff0s_env->ntf_cfg[conidx] = 0;
}


static void fff0s_notify_fff1_lvl(struct fff0s_env_tag* fff0s_env,uint8_t conidx)
{
    // Allocate the GATT notification message
    struct gattc_send_evt_cmd *fff1_lvl = KERNEL_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
            KERNEL_BUILD_ID(TASK_GATTC, conidx), prf_src_task_get(&(fff0s_env->prf_env),0),
            gattc_send_evt_cmd, sizeof(uint8_t)* FFF0_FFF1_DATA_LEN);

    // Fill in the parameter structure
    fff1_lvl->operation = GATTC_NOTIFY;
    fff1_lvl->handle = fff0s_get_att_handle(FFF0S_IDX_FFF1_LVL_VAL);
    // pack measured value in database
    fff1_lvl->length = FFF0_FFF1_DATA_LEN * sizeof(uint8_t);
  	//fff1_lvl->value[0] = fff0s_env->fff1_lvl[0];
	memcpy(&fff1_lvl->value[0],&fff0s_env->fff1_lvl[0],fff1_lvl->length);
    // send notification to peer device
    kernel_msg_send(fff1_lvl);
}



/// BASS Task interface required by profile manager
const struct prf_task_cbs fff0s_itf =
{
        (prf_init_fnct) fff0s_init,
        fff0s_destroy,
        fff0s_create,
        fff0s_cleanup,
};


const struct prf_task_cbs* fff0s_prf_itf_get(void)
{
   return &fff0s_itf;
}


uint16_t fff0s_get_att_handle( uint8_t att_idx)
{
		
    struct fff0s_env_tag *fff0s_env = PRF_ENV_GET(FFF0S, fff0s);
    uint16_t handle = ATT_INVALID_HDL;
   
    handle = fff0s_env->start_hdl;

    // increment index according to expected index
    if(att_idx < FFF0S_IDX_FFF1_LVL_NTF_CFG)
    {
        handle += att_idx;
    }
    // FFF1 notification
    else if((att_idx == FFF0S_IDX_FFF1_LVL_NTF_CFG) && (((fff0s_env->features ) & 0x01) == FFF0_FFF1_LVL_NTF_SUP))
    {
        handle += FFF0S_IDX_FFF1_LVL_NTF_CFG;			
    }		      
    else
    {
        handle = ATT_INVALID_HDL;
    }
    

    return handle;
}

uint8_t fff0s_get_att_idx(uint16_t handle, uint8_t *att_idx)
{
	

    struct fff0s_env_tag* fff0s_env = PRF_ENV_GET(FFF0S, fff0s);
    uint16_t hdl_cursor = fff0s_env->start_hdl;
    uint8_t status = PRF_APP_ERROR;

    // Browse list of services
    // handle must be greater than current index 
    // check if it's a mandatory index
    if(handle <= (hdl_cursor + FFF0S_IDX_FFF1_LVL_VAL))
    {
        *att_idx = handle -hdl_cursor;
        status = GAP_ERR_NO_ERROR;
        
    }
    hdl_cursor += FFF0S_IDX_FFF1_LVL_VAL;

    // check if it's a notify index
    if(((fff0s_env->features ) & 0x01) == FFF0_FFF1_LVL_NTF_SUP)
    {
        hdl_cursor++;
        if(handle == hdl_cursor)
        {
            *att_idx = FFF0S_IDX_FFF1_LVL_NTF_CFG;
            status = GAP_ERR_NO_ERROR;
          
        }
    }
    hdl_cursor++;
    
    return (status);
}

void fff0s_exe_operation(void)
{
    struct fff0s_env_tag* fff0s_env = PRF_ENV_GET(FFF0S, fff0s);
    ASSERT_ERR(fff0s_env->operation != NULL);
    bool finished = false ;
    uint8_t conidx = GAP_INVALID_CONIDX;


    // Restoring connection information requested
    if(fff0s_env->operation->id == FFF0S_ENABLE_REQ)
    {
        struct fff0s_enable_req  *enable = (struct fff0s_enable_req *) kernel_msg2param(fff0s_env->operation);
        conidx = enable->conidx;
		
        // loop on all services to check if notification should be triggered
        if(((fff0s_env->ntf_cfg[enable->conidx] & 1 ) != 0))
        {
            // trigger notification
            fff0s_notify_fff1_lvl(fff0s_env, enable->conidx);
         
        }
		finished = true;
        
    }
    // fff1 level updated
    else if(fff0s_env->operation->id == FFF0S_FFF1_LEVEL_UPD_REQ)
    {
        struct fff0s_fff1_level_upd_req * update = (struct fff0s_fff1_level_upd_req *) kernel_msg2param(fff0s_env->operation);
				
		conidx = update->conidx;
        
        if((fff0s_env->ntf_cfg[conidx] & 1 ) != 0)
         {
        	// trigger notification
			fff0s_notify_fff1_lvl(fff0s_env,conidx);
         }
		finished = true;
     
    }
    // default, should not happen
    else
    {
			
        ASSERT_ERR(0);
    }

    // check if operation is finished
    if(finished)
    {
        // trigger response message
        if(fff0s_env->operation->id == FFF0S_ENABLE_REQ)
        {
            struct fff0s_enable_rsp * rsp = KERNEL_MSG_ALLOC(FFF0S_ENABLE_RSP, fff0s_env->operation->src_id,
                    fff0s_env->operation->dest_id, fff0s_enable_rsp);

            rsp->conidx = conidx;
            rsp->status = GAP_ERR_NO_ERROR;
            kernel_msg_send(rsp);
        }
        else if(fff0s_env->operation->id == FFF0S_FFF1_LEVEL_UPD_REQ)
        {
            struct fff0s_fff1_level_upd_rsp * rsp = KERNEL_MSG_ALLOC(FFF0S_FFF1_LEVEL_UPD_RSP, fff0s_env->operation->src_id,
                    fff0s_env->operation->dest_id, fff0s_fff1_level_upd_rsp);

            rsp->status = GAP_ERR_NO_ERROR;
            kernel_msg_send(rsp);
        }

        // free operation
        kernel_free(fff0s_env->operation);
        fff0s_env->operation = NULL;
				
        // go back to idle state
        kernel_state_set(prf_src_task_get(&(fff0s_env->prf_env), 0), FFF0S_IDLE);
    }
}


#endif // (BLE_fff0_SERVER)


 
