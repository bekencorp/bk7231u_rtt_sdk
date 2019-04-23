/**
 ****************************************************************************************
 *
 * @file ffe0s.c
 *
 * @brief fff0 Server Implementation.
 *
 * Copyright (C) beken 2009-2015
 *
 *
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_FFE0_SERVER)
#include "attm.h"
#include "ffe0s.h"
#include "ffe0s_task.h"
#include "prf_utils.h"
#include "prf.h"
#include "kernel_mem.h"




/*
 * FFF0 ATTRIBUTES DEFINITION
 ****************************************************************************************
 */

/// Full FFF0 Database Description - Used to add attributes into the database
const struct attm_desc ffe0_att_db[FFE0S_IDX_NB] =
{
	// FFF0 Service Declaration
	[FFE0S_IDX_SVC]                  =   {ATT_DECL_PRIMARY_SERVICE,  PERM(RD, ENABLE), 0, 0},
	// fff1 Level Characteristic Declaration
	[FFE0S_IDX_FFE1_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC,   PERM(RD, ENABLE), 0, 0},
	// fff1 Level Characteristic Value
	[FFE0S_IDX_FFE1_VAL_VALUE]       =   {ATT_CHAR_FFE0_FFE1,   PERM(WRITE_REQ, ENABLE)|PERM(RD, ENABLE)|PERM(NTF, ENABLE) , PERM(RI, ENABLE) , FFE0_FFE1_DATA_LEN * 10 * sizeof(uint16_t)},
	// fff1 Level Characteristic - Client Characteristic Configuration Descriptor
	[FFE0S_IDX_FFE1_VAL_NTF_CFG]     =   {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},
	[FFE0S_IDX_FFE1_VAL_USER_DESC]   =   {ATT_DESC_CHAR_USER_DESCRIPTION,  PERM(RD, ENABLE), 0, 0},  
};/// Macro used to retrieve permission value from access and rights on attribute.


static uint8_t ffe0s_init (struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl,  struct ffe0s_db_cfg* params)
{
    uint16_t shdl;
    struct ffe0s_env_tag* ffe0s_env = NULL;
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    
    //-------------------- allocate memory required for the profile  ---------------------
    ffe0s_env = (struct ffe0s_env_tag* ) kernel_malloc(sizeof(struct ffe0s_env_tag), KERNEL_MEM_ATT_DB);
    memset(ffe0s_env, 0 , sizeof(struct ffe0s_env_tag));

   
    // Service content flag
    uint8_t cfg_flag =  params->cfg_flag;

    // Save database configuration
    ffe0s_env->features |= (params->features) ;
   
    // Check if notifications are supported
    if (params->features == FFE0_FFE1_LVL_NTF_SUP)
    {
        cfg_flag |= FFE0_CFG_FLAG_NTF_SUP_MASK;
    }

   
    shdl = *start_hdl;

    //Create FFF0 in the DB
    //------------------ create the attribute database for the profile -------------------
    status = attm_svc_create_db(&(shdl), ATT_SVC_FFE0, (uint8_t *)&cfg_flag,
            FFE0S_IDX_NB, NULL, env->task, &ffe0_att_db[0],
            (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)));
						
    //-------------------- Update profile task information  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {

        // allocate BASS required environment variable
        env->env = (prf_env_t*) ffe0s_env;
        *start_hdl = shdl;
        ffe0s_env->start_hdl = *start_hdl;
        ffe0s_env->prf_env.app_task = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        ffe0s_env->prf_env.prf_task = env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        env->id                     = TASK_ID_FFE0S;
        env->desc.idx_max           = FFE0S_IDX_MAX;
        env->desc.state             = ffe0s_env->state;
        env->desc.default_handler   = &ffe0s_default_handler;

        // service is ready, go into an Idle state
        kernel_state_set(env->task, FFE0S_IDLE);
    }
    else if(ffe0s_env != NULL)
    {
        kernel_free(ffe0s_env);
    }
     
    return (status);
}


static void ffe0s_destroy(struct prf_task_env* env)
{
    struct ffe0s_env_tag* ffe0s_env = (struct ffe0s_env_tag*) env->env;

    // clear on-going operation
    if(ffe0s_env->operation != NULL)
    {
        kernel_free(ffe0s_env->operation);
    }

    // free profile environment variables
    env->env = NULL;
    kernel_free(ffe0s_env);
}

static void ffe0s_create(struct prf_task_env* env, uint8_t conidx)
{
    struct ffe0s_env_tag* ffe0s_env = (struct ffe0s_env_tag*) env->env;
    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);

    // force notification config to zero when peer device is connected
    ffe0s_env->ntf_cfg[conidx] = 0;
}


static void ffe0s_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
    struct ffe0s_env_tag* ffe0s_env = (struct ffe0s_env_tag*) env->env;

    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);
    // force notification config to zero when peer device is disconnected
    ffe0s_env->ntf_cfg[conidx] = 0;
}


/// BASS Task interface required by profile manager
const struct prf_task_cbs ffe0s_itf =
{
        (prf_init_fnct) ffe0s_init,
        ffe0s_destroy,
        ffe0s_create,
        ffe0s_cleanup,
};


const struct prf_task_cbs* ffe0s_prf_itf_get(void)
{
   return &ffe0s_itf;
}


uint16_t ffe0s_get_att_handle( uint8_t att_idx)
{
		
    struct ffe0s_env_tag *ffe0s_env = PRF_ENV_GET(FFE0S, ffe0s);
    uint16_t handle = ATT_INVALID_HDL;
   
   
    handle = ffe0s_env->start_hdl;

    // increment index according to expected index
    if(att_idx < FFE0S_IDX_FFE1_VAL_NTF_CFG)
    {
        handle += att_idx;
    }
    // FFF1 notification
    else if((att_idx == FFE0S_IDX_FFE1_VAL_NTF_CFG) && (((ffe0s_env->features ) & 0x01) == FFE0_FFE1_LVL_NTF_SUP))
    {
        handle += FFE0S_IDX_FFE1_VAL_NTF_CFG;			
    }	      
    else
    {
        handle = ATT_INVALID_HDL;
    }
    
    return handle;
}

uint8_t ffe0s_get_att_idx(uint16_t handle, uint8_t *att_idx)
{
    struct ffe0s_env_tag* ffe0s_env = PRF_ENV_GET(FFE0S, ffe0s);
    uint16_t hdl_cursor = ffe0s_env->start_hdl;
    uint8_t status = PRF_APP_ERROR;

    // Browse list of services
    // handle must be greater than current index 
    // check if it's a mandatory index
    if(handle <= (hdl_cursor + FFE0S_IDX_FFE1_VAL_VALUE))
    {
        *att_idx = handle -hdl_cursor;
        status = GAP_ERR_NO_ERROR;
				
		return (status);
    }

    hdl_cursor += FFE0S_IDX_FFE1_VAL_VALUE;

    // check if it's a notify index
    if(((ffe0s_env->features ) & 0x01) == FFE0_FFE1_LVL_NTF_SUP)
    {
        hdl_cursor++;
        if(handle == hdl_cursor)
        {
            *att_idx = FFE0S_IDX_FFE1_VAL_NTF_CFG;
            status = GAP_ERR_NO_ERROR;
        }
    }

    hdl_cursor++;
    

    return (status);
}

void ffe0s_exe_operation(void)
{
    struct ffe0s_env_tag* ffe0s_env = PRF_ENV_GET(FFE0S, ffe0s);
    ASSERT_ERR(ffe0s_env->operation != NULL);
    bool finished = false ;
    uint8_t conidx = GAP_INVALID_CONIDX;


    // Restoring connection information requested
    if(ffe0s_env->operation->id == FFE0S_ENABLE_REQ)
    {
        struct ffe0s_enable_req  *enable = (struct ffe0s_enable_req *) kernel_msg2param(ffe0s_env->operation);
        conidx = enable->conidx;
        // loop on all services to check if notification should be triggered
        if(((ffe0s_env->ntf_cfg[enable->conidx] & 1 ) != 0))
        {
            //trigger notification
          	//ffe0s_notify_ffe1_val(ffe0s_env, enable->conidx);
         
        }
		finished = true;
        
    }
    // fff1 level updated
    else if(ffe0s_env->operation->id == FFE0S_FFE1_VALUE_UPD_REQ)
    {
        struct ffe0s_ffe1_value_upd_req * update = (struct ffe0s_ffe1_value_upd_req *) kernel_msg2param(ffe0s_env->operation);
				
		conidx = update->conidx;
        
        if((ffe0s_env->ntf_cfg[conidx] & 1 ) != 0)
         {
            //trigger notification
			//ffe0s_notify_ffe1_val(ffe0s_env,conidx);
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
        if(ffe0s_env->operation->id == FFE0S_ENABLE_REQ)
        {
            struct ffe0s_enable_rsp * rsp = KERNEL_MSG_ALLOC(FFE0S_ENABLE_RSP, ffe0s_env->operation->src_id,
                    ffe0s_env->operation->dest_id, ffe0s_enable_rsp);

            rsp->conidx = conidx;
            rsp->status = GAP_ERR_NO_ERROR;
            kernel_msg_send(rsp);
        }
        else if(ffe0s_env->operation->id == FFE0S_FFE1_VALUE_UPD_REQ)
        {
            struct ffe0s_ffe1_value_upd_rsp * rsp = KERNEL_MSG_ALLOC(FFE0S_FFE1_VALUE_UPD_RSP, ffe0s_env->operation->src_id,
                    ffe0s_env->operation->dest_id, ffe0s_ffe1_value_upd_rsp);

            rsp->status = GAP_ERR_NO_ERROR;
            kernel_msg_send(rsp);
        }

        // free operation
        kernel_free(ffe0s_env->operation);
        ffe0s_env->operation = NULL;
				
        // go back to idle state
        kernel_state_set(prf_src_task_get(&(ffe0s_env->prf_env), 0), FFE0S_IDLE);
    }


}


#endif // (BLE_FFE0_SERVER)



 
