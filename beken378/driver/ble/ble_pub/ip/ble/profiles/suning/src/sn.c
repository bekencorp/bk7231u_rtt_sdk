/**
 ****************************************************************************************
 *
 * @file ms.c
 *
 * @brief SN Server Implementation.
 *
 * Copyright (C) beken 2009-2018
 *
 *
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_SN_SERVER)
#include "attm.h"
#include "sn.h"
#include "sn_task.h"
#include "prf_utils.h"
#include "prf.h"
#include "kernel_mem.h"




/*
 * FFF0 ATTRIBUTES DEFINITION
 ****************************************************************************************
 */

/// Full FFF0 Database Description - Used to add attributes into the database
const struct attm_desc sn_att_db[SN_IDX_NB] =
{
	//  Service Declaration
	[SN_IDX_SVC]                  =   {ATT_DECL_PRIMARY_SERVICE,  PERM(RD, ENABLE), 0, 0},
	
	//  Level Characteristic Declaration
	[SN_IDX_5301_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC,   PERM(RD, ENABLE), 0, 0},
	//  Level Characteristic Value
	[SN_IDX_5301_VAL_VALUE]       =   {ATT_CHAR_SN_5301,   PERM(NTF, ENABLE), PERM(RI, ENABLE) , 128},
	//  Level Characteristic - Client Characteristic Configuration Descriptor

	[SN_IDX_5301_VAL_NTF_CFG]     =   {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},

	[SN_IDX_5302_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC,   PERM(RD, ENABLE), 0, 0},
	//  Level Characteristic Value
	[SN_IDX_5302_VAL_VALUE]       =   {ATT_CHAR_SN_5302,   PERM(WRITE_COMMAND, ENABLE) , PERM(RI, ENABLE) , 128},

};/// Macro used to retrieve permission value from access and rights on attribute.


static uint8_t sn_init (struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl,  struct sn_db_cfg* params)
{
    uint16_t shdl;
    struct sn_env_tag*sn_env = NULL;
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    
    //-------------------- allocate memory required for the profile  ---------------------
    sn_env = (struct sn_env_tag* ) kernel_malloc(sizeof(struct sn_env_tag), KERNEL_MEM_ATT_DB);
    memset(sn_env, 0 , sizeof(struct sn_env_tag));

   
    // Service content flag
    uint16_t cfg_flag =  params->cfg_flag;

    // Save database configuration
    sn_env->features |= (params->features) ;
   
    // Check if notifications are supported
    if (params->features == SN_IDX_5301_VAL_NTF_CFG)
    {
        cfg_flag |= SN_CFG_FLAG_NTF_SUP_MASK;
    }

   
    shdl = *start_hdl;

    //Create FFF0 in the DB
    //------------------ create the attribute database for the profile -------------------
    status = attm_svc_create_db(&(shdl), ATT_SVC_SN, (uint8_t *)&cfg_flag,
            SN_IDX_NB, NULL, env->task, &sn_att_db[0],
            (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)));
						
    //-------------------- Update profile task information  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {

        // allocate BASS required environment variable
        env->env = (prf_env_t*) sn_env;
        *start_hdl = shdl;
        sn_env->start_hdl = *start_hdl;
        sn_env->prf_env.app_task = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        sn_env->prf_env.prf_task = env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        env->id                     = TASK_ID_SN;
        env->desc.idx_max           = SN_IDX_MAX;
        env->desc.state             = sn_env->state;
        env->desc.default_handler   = &sn_default_handler;

        printf("sn_env->start_hdl = 0x%x",sn_env->start_hdl);

        // service is ready, go into an Idle state
        kernel_state_set(env->task, SN_IDLE);
    }
    else if(sn_env != NULL)
    {
        kernel_free(sn_env);
    }
     
    return (status);
}


static void sn_destroy(struct prf_task_env* env)
{
    struct sn_env_tag* sn_env = (struct sn_env_tag*) env->env;

    // clear on-going operation
    if(sn_env->operation != NULL)
    {
        kernel_free(sn_env->operation);
    }

    // free profile environment variables
    env->env = NULL;
    kernel_free(sn_env);
}

static void sn_create(struct prf_task_env* env, uint8_t conidx)
{
    struct sn_env_tag* sn_env = (struct sn_env_tag*) env->env;
    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);

    // force notification config to zero when peer device is connected
    sn_env->ntf_cfg[conidx] = 0;
}


static void sn_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
    struct sn_env_tag* sn_env = (struct sn_env_tag*) env->env;

    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);
    // force notification config to zero when peer device is disconnected
   sn_env->ntf_cfg[conidx] = 0;
}


///  Task interface required by profile manager
const struct prf_task_cbs sn_itf =
{
        (prf_init_fnct) sn_init,
        sn_destroy,
        sn_create,
        sn_cleanup,
};


const struct prf_task_cbs* sn_prf_itf_get(void)
{
   return &sn_itf;
}


uint16_t sn_get_att_handle( uint8_t att_idx)
{
		
    struct sn_env_tag *sn_env = PRF_ENV_GET(SN, sn);
    uint16_t handle = ATT_INVALID_HDL;
   
   
    handle = sn_env->start_hdl;
    printf("sn_get_att_handle  hdl_cursor = 0x%x\r\n",handle);
    // increment index according to expected index
    if(att_idx < SN_IDX_NB)
    {
        handle += att_idx;
    }
    else
    {
        handle = ATT_INVALID_HDL;
    }
    
    return handle;
}

uint8_t sn_get_att_idx(uint16_t handle, uint8_t *att_idx)
{
    struct sn_env_tag* sn_env = PRF_ENV_GET(SN, sn);
    uint16_t hdl_cursor = sn_env->start_hdl;
    printf("sn_get_att_idx  hdl_cursor = 0x%x\r\n",hdl_cursor);
    uint8_t status = PRF_APP_ERROR;

    // Browse list of services
    // handle must be greater than current index 
    // check if it's a mandatory index
    if(handle <= (hdl_cursor + SN_IDX_5302_VAL_VALUE))
    {
        *att_idx = handle -hdl_cursor;
        status = GAP_ERR_NO_ERROR;
				
		return (status);
    }

    return (status);
}

void sn_exe_operation(void)
{
    struct sn_env_tag* sn_env = PRF_ENV_GET(SN,sn);
    ASSERT_ERR(sn_env->operation != NULL);
    bool finished = false ;
    uint8_t conidx = GAP_INVALID_CONIDX;


    // Restoring connection information requested
    if(sn_env->operation->id == SN_ENABLE_REQ)
    {
        struct sn_enable_req  *enable = (struct sn_enable_req *) kernel_msg2param(sn_env->operation);
        conidx = enable->conidx;
        // loop on all services to check if notification should be triggered
        if(((sn_env->ntf_cfg[enable->conidx] & 1 ) != 0))
        {
            //trigger notification
          	//ffe0s_notify_ffe1_val(ffe0s_env, enable->conidx);
         
        }
		finished = true;
        
    }
    // ff02 level updated
    else if(sn_env->operation->id == SN_5301_VALUE_UPD_REQ)
    {
        struct sn_5301_value_upd_req * update = (struct sn_5301_value_upd_req *) kernel_msg2param(sn_env->operation);
				
		conidx = update->conidx;
        
        if((sn_env->ntf_cfg[conidx] & 1 ) != 0)
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
        if(sn_env->operation->id == SN_ENABLE_REQ)
        {
            struct sn_enable_rsp * rsp = KERNEL_MSG_ALLOC(SN_ENABLE_RSP, sn_env->operation->src_id,
                    sn_env->operation->dest_id, sn_enable_rsp);

            rsp->conidx = conidx;
            rsp->status = GAP_ERR_NO_ERROR;
            kernel_msg_send(rsp);
        }
        else if(sn_env->operation->id == SN_5301_VALUE_UPD_REQ)
        {
            struct sn_5301_value_upd_rsp * rsp = KERNEL_MSG_ALLOC(SN_5301_VALUE_UPD_RSP, sn_env->operation->src_id,
                    sn_env->operation->dest_id, sn_5301_value_upd_rsp);

            rsp->status = GAP_ERR_NO_ERROR;
            kernel_msg_send(rsp);
        }

        // free operation
        kernel_free(sn_env->operation);
        sn_env->operation = NULL;
				
        // go back to idle state
        kernel_state_set(prf_src_task_get(&(sn_env->prf_env), 0), SN_IDLE);
    }


}


#endif // (BLE_SN_SERVER)



 
