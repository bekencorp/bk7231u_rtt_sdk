/**
 ****************************************************************************************
 *
 * @file ayla.c
 *
 * @brief AYLA Server Implementation.
 *
 * Copyright (C) beken 2009-2018
 *
 *
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_AYLA_SERVER)
#include "attm.h"
#include "ayla.h"
#include "ayla_task.h"
#include "prf_utils.h"
#include "prf.h"
#include "kernel_mem.h"




/*
 * FFF0 ATTRIBUTES DEFINITION
 ****************************************************************************************
 */

/// Full FFF0 Database Description - Used to add attributes into the database
const struct attm_desc ayla_att_db[AYLA_IDX_NB] =
{
	//  Service Declaration
	[AYLA_IDX_SVC]                  =   {ATT_DECL_PRIMARY_SERVICE,  PERM(RD, ENABLE), 0, 0},
	
	//  Level Characteristic Declaration
	[AYLA_IDX_FFA1_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC,   PERM(RD, ENABLE), 0, 0},
	//  Level Characteristic Value
	[AYLA_IDX_FFA1_VAL_VALUE]       =   {ATT_CHAR_AYLA_FFA1,   PERM(NTF, ENABLE)|PERM(WRITE_REQ, ENABLE), PERM(RI, ENABLE) , 128},
	//  Level Characteristic - Client Characteristic Configuration Descriptor

	[AYLA_IDX_FFA1_VAL_NTF_CFG]     =   {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},

};/// Macro used to retrieve permission value from access and rights on attribute.


static uint8_t ayla_init (struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl,  struct ayla_db_cfg* params)
{
    uint16_t shdl;
    struct ayla_env_tag* ayla_env = NULL;
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    
    //-------------------- allocate memory required for the profile  ---------------------
    ayla_env = (struct ayla_env_tag* ) kernel_malloc(sizeof(struct ayla_env_tag), KERNEL_MEM_ATT_DB);
    memset(ayla_env, 0 , sizeof(struct ayla_env_tag));

   
    // Service content flag
    uint16_t cfg_flag =  params->cfg_flag;

    // Save database configuration
    ayla_env->features |= (params->features) ;
   
    // Check if notifications are supported
    if (params->features == AYLA_IDX_FFA1_VAL_NTF_CFG)
    {
        cfg_flag |= AYLA_CFG_FLAG_NTF_SUP_MASK;
    }

   
    shdl = *start_hdl;

    //Create FFF0 in the DB
    //------------------ create the attribute database for the profile -------------------
    status = attm_svc_create_db(&(shdl), ATT_SVC_AYLA, (uint8_t *)&cfg_flag,
            AYLA_IDX_NB, NULL, env->task, &ayla_att_db[0],
            (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)));
						
    //-------------------- Update profile task information  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {

        // allocate BASS required environment variable
        env->env = (prf_env_t*) ayla_env;
        *start_hdl = shdl;
        ayla_env->start_hdl = *start_hdl;
        ayla_env->prf_env.app_task = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        ayla_env->prf_env.prf_task = env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        env->id                     = TASK_ID_AYLA;
        env->desc.idx_max           = AYLA_IDX_MAX;
        env->desc.state             = ayla_env->state;
        env->desc.default_handler   = &ayla_default_handler;

        printf("ayla_env->start_hdl = 0x%x",ayla_env->start_hdl);

        // service is ready, go into an Idle state
        kernel_state_set(env->task, AYLA_IDLE);
    }
    else if(ayla_env != NULL)
    {
        kernel_free(ayla_env);
    }
     
    return (status);
}


static void ayla_destroy(struct prf_task_env* env)
{
    struct ayla_env_tag* ayla_env = (struct ayla_env_tag*) env->env;

    // clear on-going operation
    if(ayla_env->operation != NULL)
    {
        kernel_free(ayla_env->operation);
    }

    // free profile environment variables
    env->env = NULL;
    kernel_free(ayla_env);
}

static void ayla_create(struct prf_task_env* env, uint8_t conidx)
{
    struct ayla_env_tag* ayla_env = (struct ayla_env_tag*) env->env;
    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);

    // force notification config to zero when peer device is connected
    ayla_env->ntf_cfg[conidx] = 0;
}


static void ayla_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
    struct ayla_env_tag* ayla_env = (struct ayla_env_tag*) env->env;

    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);
    // force notification config to zero when peer device is disconnected
   ayla_env->ntf_cfg[conidx] = 0;
}


///  Task interface required by profile manager
const struct prf_task_cbs ayla_itf =
{
        (prf_init_fnct) ayla_init,
        ayla_destroy,
        ayla_create,
        ayla_cleanup,
};


const struct prf_task_cbs* ayla_prf_itf_get(void)
{
   return &ayla_itf;
}


uint16_t ayla_get_att_handle( uint8_t att_idx)
{
		
    struct ayla_env_tag *ayla_env = PRF_ENV_GET(AYLA, ayla);
    uint16_t handle = ATT_INVALID_HDL;
   
   
    handle = ayla_env->start_hdl;
    printf("ayla_get_att_handle  hdl_cursor = 0x%x\r\n",handle);
    // increment index according to expected index
    if(att_idx < AYLA_IDX_NB)
    {
        handle += att_idx;
    }
    else
    {
        handle = ATT_INVALID_HDL;
    }
    
    return handle;
}

uint8_t ayla_get_att_idx(uint16_t handle, uint8_t *att_idx)
{
    struct ayla_env_tag* ayla_env = PRF_ENV_GET(AYLA, ayla);
    uint16_t hdl_cursor = ayla_env->start_hdl;
    printf("ayla_get_att_idx  hdl_cursor = 0x%x, handle:0x%x\r\n", hdl_cursor, handle);
    uint8_t status = PRF_APP_ERROR;

    // Browse list of services
    // handle must be greater than current index 
    // check if it's a mandatory index
    if(handle <= (hdl_cursor + AYLA_IDX_FFA1_VAL_NTF_CFG))
    {
        *att_idx = handle -hdl_cursor;
        status = GAP_ERR_NO_ERROR;
				
		return (status);
    }

    return (status);
}

void ayla_exe_operation(void)
{
    struct ayla_env_tag* ayla_env = PRF_ENV_GET(AYLA,ayla);
    ASSERT_ERR(ayla_env->operation != NULL);
    bool finished = false ;
    uint8_t conidx = GAP_INVALID_CONIDX;


    // Restoring connection information requested
    if(ayla_env->operation->id == AYLA_ENABLE_REQ)
    {
        struct ayla_enable_req  *enable = (struct ayla_enable_req *) kernel_msg2param(ayla_env->operation);
        conidx = enable->conidx;
        // loop on all services to check if notification should be triggered
        if(((ayla_env->ntf_cfg[enable->conidx] & 1 ) != 0))
        {
            //trigger notification
          	//ffe0s_notify_ffe1_val(ffe0s_env, enable->conidx);
         
        }
		finished = true;
        
    }
    // ff02 level updated
    else if(ayla_env->operation->id == AYLA_FFA1_VALUE_UPD_REQ)
    {
        struct ayla_ffa1_value_upd_req * update = (struct ayla_ffa1_value_upd_req *) kernel_msg2param(ayla_env->operation);
				
		conidx = update->conidx;
        
        if((ayla_env->ntf_cfg[conidx] & 1 ) != 0)
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
        if(ayla_env->operation->id == AYLA_ENABLE_REQ)
        {
            struct ayla_enable_rsp * rsp = KERNEL_MSG_ALLOC(AYLA_ENABLE_RSP, ayla_env->operation->src_id,
                    ayla_env->operation->dest_id, ayla_enable_rsp);

            rsp->conidx = conidx;
            rsp->status = GAP_ERR_NO_ERROR;
            kernel_msg_send(rsp);
        }
        else if(ayla_env->operation->id == AYLA_FFA1_VALUE_UPD_REQ)
        {
            struct ayla_ffa1_value_upd_rsp * rsp = KERNEL_MSG_ALLOC(AYLA_FFA1_VALUE_UPD_RSP, ayla_env->operation->src_id,
                    ayla_env->operation->dest_id, ayla_ffa1_value_upd_rsp);

            rsp->status = GAP_ERR_NO_ERROR;
            kernel_msg_send(rsp);
        }

        // free operation
        kernel_free(ayla_env->operation);
        ayla_env->operation = NULL;
				
        // go back to idle state
        kernel_state_set(prf_src_task_get(&(ayla_env->prf_env), 0), AYLA_IDLE);
    }


}


#endif // (BLE_AYLA_SERVER)



 
