/**
 ****************************************************************************************
 *
 * @file ms.c
 *
 * @brief BTL Server Implementation.
 *
 * Copyright (C) beken 2009-2018
 *
 *
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_BTL_SERVER)
#include "attm.h"
#include "btl.h"
#include "btl_task.h"
#include "prf_utils.h"
#include "prf.h"
#include "kernel_mem.h"




/*
 * FFF0 ATTRIBUTES DEFINITION
 ****************************************************************************************
 */

/// Full FFF0 Database Description - Used to add attributes into the database
const struct attm_desc btl_att_db[BTL_IDX_NB] =
{
	//  Service Declaration
	[BTL_IDX_SVC]                  =   {ATT_DECL_PRIMARY_SERVICE,  PERM(RD, ENABLE), 0, 0},
	
	//  Level Characteristic Declaration
	[BTL_IDX_FF01_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC,   PERM(RD, ENABLE), 0, 0},
	//  Level Characteristic Value
	[BTL_IDX_FF01_VAL_VALUE]       =   {ATT_CHAR_BTL_FF01,   PERM(WRITE_REQ, ENABLE), PERM(RI, ENABLE) , 128},
	
	[BTL_IDX_FF02_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC,   PERM(RD, ENABLE), 0, 0},
	//  Level Characteristic Value
	[BTL_IDX_FF02_VAL_VALUE]       =   {ATT_CHAR_BTL_FF02,   PERM(IND, ENABLE) , PERM(RI, ENABLE) , 128},

	//  Level Characteristic - Client Characteristic Configuration Descriptor

	[BTL_IDX_FF02_VAL_IND_CFG]     =   {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},
};/// Macro used to retrieve permission value from access and rights on attribute.


static uint8_t btl_init (struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl,  struct btl_db_cfg* params)
{
    uint16_t shdl;
    struct btl_env_tag*btl_env = NULL;
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    
    //-------------------- allocate memory required for the profile  ---------------------
    btl_env = (struct btl_env_tag* ) kernel_malloc(sizeof(struct btl_env_tag), KERNEL_MEM_ATT_DB);
    memset(btl_env, 0 , sizeof(struct btl_env_tag));

   
    // Service content flag
    uint16_t cfg_flag =  params->cfg_flag;

    // Save database configuration
    btl_env->features |= (params->features) ;
   
    // Check if notifications are supported
    if (params->features == BTL_FF02_LVL_IND_SUP)
    {
        cfg_flag |= BTL_CFG_FLAG_IND_SUP_MASK;
    }

   
    shdl = *start_hdl;

    //Create FFF0 in the DB
    //------------------ create the attribute database for the profile -------------------
    status = attm_svc_create_db(&(shdl), ATT_SVC_BTL, (uint8_t *)&cfg_flag,
            BTL_IDX_NB, NULL, env->task, &btl_att_db[0],
            (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)));
						
    //-------------------- Update profile task information  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {

        // allocate BASS required environment variable
        env->env = (prf_env_t*) btl_env;
        *start_hdl = shdl;
        btl_env->start_hdl = *start_hdl;
        btl_env->prf_env.app_task = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        btl_env->prf_env.prf_task = env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        env->id                     = TASK_ID_BTL;
        env->desc.idx_max           = BTL_IDX_MAX;
        env->desc.state             = btl_env->state;
        env->desc.default_handler   = &btl_default_handler;

        printf("btl_env->start_hdl = 0x%x",btl_env->start_hdl);

        // service is ready, go into an Idle state
        kernel_state_set(env->task, BTL_IDLE);
    }
    else if(btl_env != NULL)
    {
        kernel_free(btl_env);
    }
     
    return (status);
}


static void btl_destroy(struct prf_task_env* env)
{
    struct btl_env_tag* btl_env = (struct btl_env_tag*) env->env;

    // clear on-going operation
    if(btl_env->operation != NULL)
    {
        kernel_free(btl_env->operation);
    }

    // free profile environment variables
    env->env = NULL;
    kernel_free(btl_env);
}

static void btl_create(struct prf_task_env* env, uint8_t conidx)
{
    struct btl_env_tag* btl_env = (struct btl_env_tag*) env->env;
    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);

    // force notification config to zero when peer device is connected
    btl_env->ind_cfg[conidx] = 0;
}


static void btl_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
    struct btl_env_tag* btl_env = (struct btl_env_tag*) env->env;

    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);
    // force notification config to zero when peer device is disconnected
   btl_env->ind_cfg[conidx] = 0;
}


///  Task interface required by profile manager
const struct prf_task_cbs btl_itf =
{
        (prf_init_fnct) btl_init,
        btl_destroy,
        btl_create,
        btl_cleanup,
};


const struct prf_task_cbs* btl_prf_itf_get(void)
{
   return &btl_itf;
}


uint16_t btl_get_att_handle( uint8_t att_idx)
{
		
    struct btl_env_tag *btl_env = PRF_ENV_GET(BTL, btl);
    uint16_t handle = ATT_INVALID_HDL;
   
   
    handle = btl_env->start_hdl;
    printf("btl_get_att_handle  hdl_cursor = 0x%x\r\n",handle);
    // increment index according to expected index
    if(att_idx < BTL_IDX_FF02_VAL_IND_CFG)
    {
        handle += att_idx;
    }
    //  notification
    else if((att_idx == BTL_IDX_FF02_VAL_IND_CFG) && (((btl_env->features ) & 0x02) == BTL_FF02_LVL_IND_SUP))
    {
        handle += BTL_IDX_FF02_VAL_IND_CFG;			
    }	      
    else
    {
        handle = ATT_INVALID_HDL;
    }
    
    return handle;
}

uint8_t btl_get_att_idx(uint16_t handle, uint8_t *att_idx)
{
    struct btl_env_tag* btl_env = PRF_ENV_GET(BTL, btl);
    uint16_t hdl_cursor = btl_env->start_hdl;
    printf("btl_get_att_idx handle = 0x%x hdl_cursor = 0x%x\r\n", handle, hdl_cursor);
    uint8_t status = PRF_APP_ERROR;

    // Browse list of services
    // handle must be greater than current index 
    // check if it's a mandatory index
    if(handle <= (hdl_cursor + BTL_IDX_FF02_VAL_VALUE))
    {
        *att_idx = handle -hdl_cursor;
        status = GAP_ERR_NO_ERROR;
				
		return (status);
    }

    hdl_cursor += BTL_IDX_FF02_VAL_VALUE;

    // check if it's a notify index
    if(((btl_env->features ) & 0x02) == BTL_FF02_LVL_IND_SUP)
    {
        hdl_cursor++;
        if(handle == hdl_cursor)
        {
            *att_idx = BTL_IDX_FF02_VAL_IND_CFG;
            status = GAP_ERR_NO_ERROR;
        }
    }

    hdl_cursor++;
    

    return (status);
}

void btl_exe_operation(void)
{
    struct btl_env_tag* btl_env = PRF_ENV_GET(BTL,btl);
    ASSERT_ERR(btl_env->operation != NULL);
    bool finished = false ;
    uint8_t conidx = GAP_INVALID_CONIDX;


    // Restoring connection information requested
    if(btl_env->operation->id == BTL_ENABLE_REQ)
    {
        struct btl_enable_req  *enable = (struct btl_enable_req *) kernel_msg2param(btl_env->operation);
        conidx = enable->conidx;
        // loop on all services to check if notification should be triggered
        if(((btl_env->ind_cfg[enable->conidx] & 2 ) != 0))
        {
            //trigger notification
          	//ffe0s_notify_ffe1_val(ffe0s_env, enable->conidx);
         
        }
		finished = true;
        
    }
    // ff02 level updated
    else if(btl_env->operation->id == BTL_FF02_VALUE_UPD_REQ)
    {
        struct btl_ff02_value_upd_req * update = (struct btl_ff02_value_upd_req *) kernel_msg2param(btl_env->operation);
				
		conidx = update->conidx;
        
        if((btl_env->ind_cfg[conidx] & 2 ) != 0)
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
        if(btl_env->operation->id == BTL_ENABLE_REQ)
        {
            struct btl_enable_rsp * rsp = KERNEL_MSG_ALLOC(BTL_ENABLE_RSP, btl_env->operation->src_id,
                    btl_env->operation->dest_id, btl_enable_rsp);

            rsp->conidx = conidx;
            rsp->status = GAP_ERR_NO_ERROR;
            kernel_msg_send(rsp);
        }
        else if(btl_env->operation->id == BTL_FF02_VALUE_UPD_REQ)
        {
            struct btl_ff02_value_upd_rsp * rsp = KERNEL_MSG_ALLOC(BTL_FF02_VALUE_UPD_RSP, btl_env->operation->src_id,
                    btl_env->operation->dest_id, btl_ff02_value_upd_rsp);

            rsp->status = GAP_ERR_NO_ERROR;
            kernel_msg_send(rsp);
        }

        // free operation
        kernel_free(btl_env->operation);
        btl_env->operation = NULL;
				
        // go back to idle state
        kernel_state_set(prf_src_task_get(&(btl_env->prf_env), 0), BTL_IDLE);
    }


}


#endif // (BLE_BTL_SERVER)



 
