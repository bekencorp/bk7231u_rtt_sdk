/**
 ****************************************************************************************
 *
 * @file AIS.c
 *
 * @brief FEB3 Server Implementation.
 *
 * Copyright (C) beken 2009-2015
 *
 *
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_FEB3_SERVER)
#include "attm.h"
#include "AIS.h"
#include "AIS_task.h"
#include "prf_utils.h"
#include "prf.h"
#include "kernel_mem.h"

#include "uart.h"



/*
 * FFF0 ATTRIBUTES DEFINITION
 ****************************************************************************************
 */

/// Full FEB3 Database Description - Used to add attributes into the database
const struct attm_desc feb3_att_db[FEB3S_IDX_NB] =
{
	// FEB3 Service Declaration
	[FEB3S_IDX_SVC]                  =   {ATT_DECL_PRIMARY_SERVICE,  PERM(RD, ENABLE), 0, 0},
	// fed4  Characteristic Declaration
	[FEB3S_IDX_FED4_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC,   PERM(RD, ENABLE), 0, 0},
	// fed4  Characteristic Value
	[FEB3S_IDX_FED4_VAL_VALUE]       =   {ATT_CHAR_FEB3_FED4,        PERM(RD, ENABLE) , PERM(RI, ENABLE) , FEB3_CHAR_DATA_LEN },
	
	
	// fed5  Characteristic Declaration
	[FEB3S_IDX_FED5_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC,   PERM(RD, ENABLE), 0, 0},
	// fed5  Characteristic Value
	[FEB3S_IDX_FED5_VAL_VALUE]       =   {ATT_CHAR_FEB3_FED5,   PERM(WRITE_REQ, ENABLE)|PERM(RD, ENABLE) , PERM(RI, ENABLE) , FEB3_CHAR_DATA_LEN},
	
	// fed6  Characteristic Declaration
	[FEB3S_IDX_FED6_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC,   PERM(RD, ENABLE), 0, 0},
	// fed6  Characteristic Value
	[FEB3S_IDX_FED6_VAL_VALUE]       =   {ATT_CHAR_FEB3_FED6,        PERM(IND, ENABLE)|PERM(RD, ENABLE) , PERM(RI, ENABLE) , FEB3_CHAR_DATA_LEN},
	[FFB3S_IDX_FED6_VAL_IND_CFG]     =   {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},
	
	// fed7  Characteristic Declaration
	[FEB3S_IDX_FED7_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC,   PERM(RD, ENABLE), 0, 0},
	// fed7  Characteristic Value
	[FEB3S_IDX_FED7_VAL_VALUE]       =   {ATT_CHAR_FEB3_FED7,       PERM(WRITE_COMMAND, ENABLE)|PERM(RD, ENABLE) , PERM(RI, ENABLE) , FEB3_CHAR_DATA_LEN},
	
/*  PERM_POS_WRITE_REQ */
	// fed8  Characteristic Declaration
	[FEB3S_IDX_FED8_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC,   PERM(RD, ENABLE), 0, 0},
	// fed8  Characteristic Value
	[FEB3S_IDX_FED8_VAL_VALUE]       =   {ATT_CHAR_FEB3_FED8,        PERM(NTF, ENABLE)|PERM(RD, ENABLE) , PERM(RI, ENABLE) , FEB3_CHAR_DATA_LEN},
	[FFB3S_IDX_FED8_VAL_NTF_CFG]     =   {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},
	
};/// Macro used to retrieve permission value from access and rights on attribute.


static uint8_t feb3s_init (struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl,  struct feb3s_db_cfg* params)
{
	
	//	UART_PRINTF("%s\r\n",__func__);
    uint16_t shdl;
    struct feb3s_env_tag* feb3s_env = NULL;
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    
    //-------------------- allocate memory required for the profile  ---------------------
    feb3s_env = (struct feb3s_env_tag* ) kernel_malloc(sizeof(struct feb3s_env_tag), KERNEL_MEM_ATT_DB);
    memset(feb3s_env, 0 , sizeof(struct feb3s_env_tag));

   
    // Service content flag
    uint32_t cfg_flag = FEB3S_CFG_FLAG_MANDATORY_MASK;// params->cfg_flag;

    // Save database configuration
    feb3s_env->features |= (params->features) ;
   
    // Check if notifications are supported
    if (params->features & FEB3S_FED6_VAL_IND_SUP)
    {
        cfg_flag |= FED6_CFG_FLAG_IND_SUP_MASK;
    }
		
		if (params->features & FEB3S_FED8_VAL_NTF_SUP)
    {
        cfg_flag |= FED8_CFG_FLAG_NTF_SUP_MASK;
    }

   
    shdl = *start_hdl;

    //Create FFF0 in the DB
    //------------------ create the attribute database for the profile -------------------
    status = attm_svc_create_db(&(shdl), ATT_SVC_FEB3, (uint8_t *)&cfg_flag,
            FEB3S_IDX_NB, NULL, env->task, &feb3_att_db[0],
            (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)));
						
    //-------------------- Update profile task information  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {

        // allocate BASS required environment variable
        env->env = (prf_env_t*) feb3s_env;
        *start_hdl = shdl;
        feb3s_env->start_hdl = *start_hdl;
        feb3s_env->prf_env.app_task = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        feb3s_env->prf_env.prf_task = env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        env->id                     = TASK_ID_FEB3S;
        env->desc.idx_max           = FEB3S_IDX_MAX;
        env->desc.state             = feb3s_env->state;
        env->desc.default_handler   = &feb3s_default_handler;

        // service is ready, go into an Idle state
        kernel_state_set(env->task, FEB3S_IDLE);
    }
    else if(feb3s_env != NULL)
    {
        kernel_free(feb3s_env);
    }
     
    return (status);
}


static void feb3s_destroy(struct prf_task_env* env)
{
	//	UART_PRINTF("%s\r\n",__func__);
    struct feb3s_env_tag* feb3s_env = (struct feb3s_env_tag*) env->env;

    // clear on-going operation
    if(feb3s_env->operation != NULL)
    {
        kernel_free(feb3s_env->operation);
    }

    // free profile environment variables
    env->env = NULL;
    kernel_free(feb3s_env);
}

static void feb3s_create(struct prf_task_env* env, uint8_t conidx)
{
	//		UART_PRINTF("%s\r\n",__func__);
    struct feb3s_env_tag* feb3s_env = (struct feb3s_env_tag*) env->env;
    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);

    // force notification config to zero when peer device is connected
    feb3s_env->ntf_cfg[conidx] = 0;
	  feb3s_env->ind_cfg[conidx] = 0;
}


static void feb3s_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
	//		UART_PRINTF("%s\r\n",__func__);
    struct feb3s_env_tag* feb3s_env = (struct feb3s_env_tag*) env->env;

    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);
    // force notification config to zero when peer device is disconnected
    feb3s_env->ntf_cfg[conidx] = 0;
		feb3s_env->ind_cfg[conidx] = 0;
}


/// BASS Task interface required by profile manager
const struct prf_task_cbs feb3s_itf =
{
        (prf_init_fnct) feb3s_init,
        feb3s_destroy,
        feb3s_create,
        feb3s_cleanup,
};


const struct prf_task_cbs* feb3s_prf_itf_get(void)
{
	// UART_PRINTF("%s\r\n",__func__);
   return &feb3s_itf;
}


uint16_t feb3s_get_att_handle( uint8_t att_idx)
{
		
    struct feb3s_env_tag *feb3s_env = PRF_ENV_GET(FEB3S, feb3s);
    uint16_t handle = ATT_INVALID_HDL;
  // 	UART_PRINTF("%s\r\n",__func__);

   
    handle = feb3s_env->start_hdl;

		if((feb3s_env->features  & FEB3S_FED6_VAL_IND_SUP) &&  (feb3s_env->features  & FEB3S_FED8_VAL_NTF_SUP))
		{
					if(att_idx <= FFB3S_IDX_FED8_VAL_NTF_CFG)
					{
							handle += att_idx;	
					}else
					{
							handle = ATT_INVALID_HDL;
					
					}
					return handle ;
		}
		
		else if((feb3s_env->features  & FEB3S_FED6_VAL_IND_SUP))
		{
					if(att_idx <= FEB3S_IDX_FED8_VAL_VALUE)
					{
							handle += att_idx;	
					}else
					{
							handle = ATT_INVALID_HDL;
					
					}
					return handle ;
					
		}else if((feb3s_env->features  & FEB3S_FED8_VAL_NTF_SUP))
		{
					if(att_idx <= FEB3S_IDX_FED8_VAL_VALUE)
					{
							handle += att_idx;	
					}else
					{
							handle = ATT_INVALID_HDL;
					
					}
					return handle ;
		}
			
		
		if(att_idx <= FEB3S_IDX_FED8_VAL_CHAR)
		{
						handle += att_idx;	
		}else
		{
						handle = ATT_INVALID_HDL;
				
		}
		

    return handle;
}

uint8_t feb3s_get_att_idx(uint16_t handle, uint8_t *att_idx)
{
	  //UART_PRINTF("%s\r\n",__func__);
    struct feb3s_env_tag* feb3s_env = PRF_ENV_GET(FEB3S, feb3s);
    uint16_t hdl_cursor = feb3s_env->start_hdl;
    uint8_t status = PRF_APP_ERROR;

    // Browse list of services
    // handle must be greater than current index 
    // check if it's a mandatory index
    if(handle <= (hdl_cursor + FFB3S_IDX_FED8_VAL_NTF_CFG))
    {
        *att_idx = handle - hdl_cursor;
        status = GAP_ERR_NO_ERROR;
				
				return (status);
    }

    hdl_cursor += FEB3S_IDX_FED6_VAL_VALUE;

    // check if it's a notify index
    if(((feb3s_env->features ) & FEB3S_FED6_VAL_IND_SUP) == FEB3S_FED6_VAL_IND_SUP)
    {
        hdl_cursor++;
        if(handle == hdl_cursor)
        {
            *att_idx = FFB3S_IDX_FED6_VAL_IND_CFG;
            status = GAP_ERR_NO_ERROR;
        }
    }
		
		if(handle <= (hdl_cursor + FEB3S_IDX_FED8_VAL_VALUE))
    {
        *att_idx = handle - hdl_cursor;
        status = GAP_ERR_NO_ERROR;
				
				return (status);
    }
			
		hdl_cursor++;
		
		if(((feb3s_env->features ) & FEB3S_FED8_VAL_NTF_SUP) == FEB3S_FED8_VAL_NTF_SUP)
    {
        hdl_cursor++;
        if(handle == hdl_cursor)
        {
            *att_idx = FFB3S_IDX_FED8_VAL_NTF_CFG;
            status = GAP_ERR_NO_ERROR;
        }
    }


    return (status);
}

void feb3s_exe_operation(void)
{
	
//	UART_PRINTF("%s\r\n",__func__);
    struct feb3s_env_tag* feb3s_env = PRF_ENV_GET(FEB3S, feb3s);
    ASSERT_ERR(feb3s_env->operation != NULL);
    bool finished = false ;
    uint8_t conidx = GAP_INVALID_CONIDX;


    // Restoring connection information requested
    if(feb3s_env->operation->id == FEB3S_ENABLE_REQ)
    {
        struct feb3s_enable_req  *enable = (struct feb3s_enable_req *) kernel_msg2param(feb3s_env->operation);
        conidx = enable->conidx;
        // loop on all services to check if notification should be triggered
        if(((feb3s_env->ntf_cfg[enable->conidx] & 1 ) != 0))
        {
            //trigger notification
          	//ffe0s_notify_ffe1_val(ffe0s_env, enable->conidx);
         
        }
				finished = true;
        
    }
    // fff1 level updated
    else if(feb3s_env->operation->id == FEB3S_FED6_VALUE_IND_UPD_REQ)
    {
        struct feb3s_fed6_value_ind_upd_req * update = (struct feb3s_fed6_value_ind_upd_req *) kernel_msg2param(feb3s_env->operation);
				
		   conidx = update->conidx;
        
        if((feb3s_env->ntf_cfg[conidx] & 1 ) != 0)
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
        if(feb3s_env->operation->id == FEB3S_ENABLE_REQ)
        {
            struct feb3s_enable_rsp * rsp = KERNEL_MSG_ALLOC(FEB3S_ENABLE_RSP, feb3s_env->operation->src_id,
                    feb3s_env->operation->dest_id, feb3s_enable_rsp);

            rsp->conidx = conidx;
            rsp->status = GAP_ERR_NO_ERROR;
            kernel_msg_send(rsp);
        }
        else if(feb3s_env->operation->id == FEB3S_FED6_VALUE_IND_UPD_REQ)
        {
            struct feb3s_fed6_value_ind_upd_rsp * rsp = KERNEL_MSG_ALLOC(FEB3S_FED6_VALUE_IND_UPD_RSP, feb3s_env->operation->src_id,
                    feb3s_env->operation->dest_id, feb3s_fed6_value_ind_upd_rsp);

            rsp->status = GAP_ERR_NO_ERROR;
            kernel_msg_send(rsp);
        }

        // free operation
        kernel_free(feb3s_env->operation);
        feb3s_env->operation = NULL;
				
        // go back to idle state
        kernel_state_set(prf_src_task_get(&(feb3s_env->prf_env), 0), FEB3S_IDLE);
    }


}


#endif // (BLE_FEB3_SERVER)



 
