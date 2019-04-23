/**
 ****************************************************************************************
 *
 * @file ms.c
 *
 * @brief MS Server Implementation.
 *
 * Copyright (C) beken 2009-2018
 *
 *
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_MS_SERVER)
#include "attm.h"
#include "ms.h"
#include "ms_task.h"
#include "prf_utils.h"
#include "prf.h"
#include "kernel_mem.h"




/*
 * FFF0 ATTRIBUTES DEFINITION
 ****************************************************************************************
 */

/// Full FFF0 Database Description - Used to add attributes into the database
const struct attm_desc ms_att_db[MS_IDX_NB] =
{
	//  Service Declaration
	[MS_IDX_SVC]                  =   {ATT_DECL_PRIMARY_SERVICE,  PERM(RD, ENABLE), 0, 0},
	
	//  Level Characteristic Declaration
	[MS_IDX_REVICE_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC,   PERM(RD, ENABLE), 0, 0},
	//  Level Characteristic Value
	[MS_IDX_REVICE_VAL_VALUE]       =   {ATT_CHAR_MS_REVICE,   PERM(WRITE_REQ, ENABLE), PERM(RI, ENABLE) , 250},
	[MS_IDX_REVICE_VAL_USER_DESC]   =   {ATT_DESC_CHAR_USER_DESCRIPTION,  PERM(RD, ENABLE), 0, 0},  
	
	[MS_IDX_SEND_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC,   PERM(RD, ENABLE), 0, 0},
	//  Level Characteristic Value
	[MS_IDX_SEND_VAL_VALUE]       =   {ATT_CHAR_MS_SEND,   PERM(IND, ENABLE) , PERM(RI, ENABLE) , 250},

	//  Level Characteristic - Client Characteristic Configuration Descriptor

	[MS_IDX_SEND_VAL_IND_CFG]     =   {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},
	[MS_IDX_SEND_VAL_USER_DESC]   =   {ATT_DESC_CHAR_USER_DESCRIPTION,  PERM(RD, ENABLE), 0, 0},  
};/// Macro used to retrieve permission value from access and rights on attribute.


static uint8_t ms_init (struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl,  struct ms_db_cfg* params)
{
    uint16_t shdl;
    struct ms_env_tag*ms_env = NULL;
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    
    //-------------------- allocate memory required for the profile  ---------------------
    ms_env = (struct ms_env_tag* ) kernel_malloc(sizeof(struct ms_env_tag), KERNEL_MEM_ATT_DB);
    memset(ms_env, 0 , sizeof(struct ms_env_tag));

   
    // Service content flag
    uint16_t cfg_flag =  params->cfg_flag;

    // Save database configuration
    ms_env->features |= (params->features) ;
   
    // Check if notifications are supported
    if (params->features == MS_SEND_LVL_IND_SUP)
    {
        cfg_flag |= MS_CFG_FLAG_IND_SUP_MASK;
    }

   
    shdl = *start_hdl;

    //Create FFF0 in the DB
    //------------------ create the attribute database for the profile -------------------
    status = attm_svc_create_db(&(shdl), ATT_SVC_MS, (uint8_t *)&cfg_flag,
            MS_IDX_NB, NULL, env->task, &ms_att_db[0],
            (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)));
						
    //-------------------- Update profile task information  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {

        // allocate BASS required environment variable
        env->env = (prf_env_t*) ms_env;
        *start_hdl = shdl;
        ms_env->start_hdl = *start_hdl;
        ms_env->prf_env.app_task = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        ms_env->prf_env.prf_task = env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        env->id                     = TASK_ID_MS;
        env->desc.idx_max           = MS_IDX_MAX;
        env->desc.state             = ms_env->state;
        env->desc.default_handler   = &ms_default_handler;

        printf("ms_env->start_hdl = 0x%x",ms_env->start_hdl);

        // service is ready, go into an Idle state
        kernel_state_set(env->task, MS_IDLE);
    }
    else if(ms_env != NULL)
    {
        kernel_free(ms_env);
    }
     
    return (status);
}


static void ms_destroy(struct prf_task_env* env)
{
    struct ms_env_tag* ms_env = (struct ms_env_tag*) env->env;

    // clear on-going operation
    if(ms_env->operation != NULL)
    {
        kernel_free(ms_env->operation);
    }

    // free profile environment variables
    env->env = NULL;
    kernel_free(ms_env);
}

static void ms_create(struct prf_task_env* env, uint8_t conidx)
{
    struct ms_env_tag* ms_env = (struct ms_env_tag*) env->env;
    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);

    // force notification config to zero when peer device is connected
    ms_env->ind_cfg[conidx] = 0;
}


static void ms_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
    struct ms_env_tag* ms_env = (struct ms_env_tag*) env->env;

    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);
    // force notification config to zero when peer device is disconnected
   ms_env->ind_cfg[conidx] = 0;
}


///  Task interface required by profile manager
const struct prf_task_cbs ms_itf =
{
        (prf_init_fnct) ms_init,
        ms_destroy,
        ms_create,
        ms_cleanup,
};


const struct prf_task_cbs* ms_prf_itf_get(void)
{
   return &ms_itf;
}


uint16_t ms_get_att_handle( uint8_t att_idx)
{
		
    struct ms_env_tag *ms_env = PRF_ENV_GET(MS, ms);
    uint16_t handle = ATT_INVALID_HDL;
   
   
    handle = ms_env->start_hdl;
    printf("ms_get_att_handle  hdl_cursor = 0x%x\r\n",handle);
    // increment index according to expected index
    if(att_idx < MS_IDX_SEND_VAL_IND_CFG)
    {
        handle += att_idx;
    }
    //  notification
    else if((att_idx == MS_IDX_SEND_VAL_IND_CFG) && (((ms_env->features ) & 0x02) == MS_SEND_LVL_IND_SUP))
    {
        handle += MS_IDX_SEND_VAL_IND_CFG;			
    }	      
    else
    {
        handle = ATT_INVALID_HDL;
    }
    
    return handle;
}

uint8_t ms_get_att_idx(uint16_t handle, uint8_t *att_idx)
{
    struct ms_env_tag* ms_env = PRF_ENV_GET(MS, ms);
    uint16_t hdl_cursor = ms_env->start_hdl;
    printf("ms_get_att_idx  hdl_cursor = 0x%x\r\n",hdl_cursor);
    uint8_t status = PRF_APP_ERROR;

    // Browse list of services
    // handle must be greater than current index 
    // check if it's a mandatory index
    if(handle <= (hdl_cursor +MS_IDX_SEND_VAL_VALUE))
    {
        *att_idx = handle -hdl_cursor;
        status = GAP_ERR_NO_ERROR;
				
		return (status);
    }

    hdl_cursor += MS_IDX_SEND_VAL_VALUE;

    // check if it's a notify index
    if(((ms_env->features ) & 0x02) == MS_SEND_LVL_IND_SUP)
    {
        hdl_cursor++;
        if(handle == hdl_cursor)
        {
            *att_idx = MS_IDX_SEND_VAL_IND_CFG;
            status = GAP_ERR_NO_ERROR;
        }
    }

    hdl_cursor++;
    

    return (status);
}

void ms_exe_operation(void)
{
    struct ms_env_tag* ms_env = PRF_ENV_GET(MS,ms);
    ASSERT_ERR(ms_env->operation != NULL);
    bool finished = false ;
    uint8_t conidx = GAP_INVALID_CONIDX;


    // Restoring connection information requested
    if(ms_env->operation->id == MS_ENABLE_REQ)
    {
        struct ms_enable_req  *enable = (struct ms_enable_req *) kernel_msg2param(ms_env->operation);
        conidx = enable->conidx;
        // loop on all services to check if notification should be triggered
        if(((ms_env->ind_cfg[enable->conidx] & 2 ) != 0))
        {
            //trigger notification
          	//ffe0s_notify_ffe1_val(ffe0s_env, enable->conidx);
         
        }
		finished = true;
        
    }
    // fff1 level updated
    else if(ms_env->operation->id == MS_SEND_VALUE_UPD_REQ)
    {
        struct ms_send_value_upd_req * update = (struct ms_send_value_upd_req *) kernel_msg2param(ms_env->operation);
				
		conidx = update->conidx;
        
        if((ms_env->ind_cfg[conidx] & 2 ) != 0)
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
        if(ms_env->operation->id == MS_ENABLE_REQ)
        {
            struct ms_enable_rsp * rsp = KERNEL_MSG_ALLOC(MS_ENABLE_RSP, ms_env->operation->src_id,
                    ms_env->operation->dest_id, ms_enable_rsp);

            rsp->conidx = conidx;
            rsp->status = GAP_ERR_NO_ERROR;
            kernel_msg_send(rsp);
        }
        else if(ms_env->operation->id == MS_SEND_VALUE_UPD_REQ)
        {
            struct ms_send_value_upd_rsp * rsp = KERNEL_MSG_ALLOC(MS_SEND_VALUE_UPD_RSP, ms_env->operation->src_id,
                    ms_env->operation->dest_id, ms_send_value_upd_rsp);

            rsp->status = GAP_ERR_NO_ERROR;
            kernel_msg_send(rsp);
        }

        // free operation
        kernel_free(ms_env->operation);
        ms_env->operation = NULL;
				
        // go back to idle state
        kernel_state_set(prf_src_task_get(&(ms_env->prf_env), 0), MS_IDLE);
    }


}


#endif // (BLE_MS_SERVER)



 
