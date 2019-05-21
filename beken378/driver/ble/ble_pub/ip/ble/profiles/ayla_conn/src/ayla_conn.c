/**
 ****************************************************************************************
 *
 * @file ayla_conn.c
 *
 * @brief AYLA_CONN Server Implementation.
 *
 * Copyright (C) beken 2009-2018
 *
 *
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_AYLA_CONN_SERVER)
#include "attm.h"
#include "ayla_conn.h"
#include "ayla_conn_task.h"
#include "prf_utils.h"
#include "prf.h"
#include "kernel_mem.h"




/*
 * FFF0 ATTRIBUTES DEFINITION
 ****************************************************************************************
 */

#if 0
/// Full FFF0 Database Description - Used to add attributes into the database
const struct attm_desc ayla_conn_att_db[AYLA_CONN_IDX_NB] =
{
	//  Service Declaration
	[AYLA_CONN_IDX_SVC]                  =   {ATT_DECL_PRIMARY_SERVICE,  PERM(RD, ENABLE), 0, 0},
	
	//  Level Characteristic Declaration
	[AYLA_CONN_IDX_FFA1_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC,   PERM(RD, ENABLE), 0, 0},
	//  Level Characteristic Value
	[AYLA_CONN_IDX_FFA1_VAL_VALUE]       =   {ATT_CHAR_AYLA_CONN_FFA1,   PERM(NTF, ENABLE)|PERM(WRITE_REQ, ENABLE), PERM(RI, ENABLE) , 128},
	//  Level Characteristic - Client Characteristic Configuration Descriptor

	[AYLA_CONN_IDX_FFA1_VAL_NTF_CFG]     =   {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},

};/// Macro used to retrieve permission value from access and rights on attribute.
#endif

const uint8_t ayla_conn_service[16] = {0xDC, 0x9A, 0xD5, 0x5B, 0xB2, 0xFA, 0x36, 0xAE, 0x73, 0x48, 0xB6, 0x59, 0x41, 0xEC, 0xE3, 0xFC};

/// Full FFF0 Database Description - Used to add attributes into the database
const struct attm_desc_128 ayla_conn_att_db[AYLA_CONN_IDX_NB] =
{
	//  Service Declaration
	[AYLA_CONN_IDX_SVC]                  =   {ATT_DECL_PRIMARY_SERVICE_128,  PERM(RD, ENABLE), 0, 0},
	
	//  Level Characteristic Declaration
	[AYLA_CONN_IDX_SETO_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC_128,   PERM(RD, ENABLE), 0, 0},
	//  Level Characteristic Value
	[AYLA_CONN_IDX_SETO_VAL_VALUE]       =   {ATT_CHAR_AYLA_CONN_SETO_128,   PERM(WRITE_REQ, ENABLE), PERM(RI, ENABLE)|(0x2 << 13) , 128},
	
};/// Macro used to retrieve permission value from access and rights on attribute.

static uint8_t ayla_conn_init (struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl,  struct ayla_conn_db_cfg* params)
{
    uint16_t shdl;
    struct ayla_conn_env_tag* ayla_conn_env = NULL;
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    
    //-------------------- allocate memory required for the profile  ---------------------
    ayla_conn_env = (struct ayla_conn_env_tag* ) kernel_malloc(sizeof(struct ayla_conn_env_tag), KERNEL_MEM_ATT_DB);
    memset(ayla_conn_env, 0 , sizeof(struct ayla_conn_env_tag));

   
    // Service content flag
    uint16_t cfg_flag =  params->cfg_flag;

    // Save database configuration
    ayla_conn_env->features |= (params->features) ;
   
    shdl = *start_hdl;

    //Create FFF0 in the DB
    //------------------ create the attribute database for the profile -------------------
    status = attm_svc_create_db_128(&(shdl), ayla_conn_service, (uint8_t *)&cfg_flag,
            AYLA_CONN_IDX_NB, NULL, env->task, &ayla_conn_att_db[0],
            (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS | PERM_MASK_SVC_UUID_LEN)));
						
    //-------------------- Update profile task information  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {

        // allocate BASS required environment variable
        env->env = (prf_env_t*) ayla_conn_env;
        *start_hdl = shdl;
        ayla_conn_env->start_hdl = *start_hdl;
        ayla_conn_env->prf_env.app_task = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        ayla_conn_env->prf_env.prf_task = env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        env->id                     = TASK_ID_AYLA_CONN;
        env->desc.idx_max           = AYLA_CONN_IDX_MAX;
        env->desc.state             = ayla_conn_env->state;
        env->desc.default_handler   = &ayla_conn_default_handler;

        printf("ayla_conn_env->start_hdl = 0x%x",ayla_conn_env->start_hdl);

        // service is ready, go into an Idle state
        kernel_state_set(env->task, AYLA_CONN_IDLE);
    }
    else if(ayla_conn_env != NULL)
    {
        kernel_free(ayla_conn_env);
    }
     
    return (status);
}


static void ayla_conn_destroy(struct prf_task_env* env)
{
    struct ayla_conn_env_tag* ayla_conn_env = (struct ayla_conn_env_tag*) env->env;

    // clear on-going operation
    if(ayla_conn_env->operation != NULL)
    {
        kernel_free(ayla_conn_env->operation);
    }

    // free profile environment variables
    env->env = NULL;
    kernel_free(ayla_conn_env);
}

static void ayla_conn_create(struct prf_task_env* env, uint8_t conidx)
{
    struct ayla_conn_env_tag* ayla_conn_env = (struct ayla_conn_env_tag*) env->env;
    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);

    // force notification config to zero when peer device is connected
    ayla_conn_env->ntf_cfg[conidx] = 0;
}


static void ayla_conn_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
    struct ayla_conn_env_tag* ayla_conn_env = (struct ayla_conn_env_tag*) env->env;

    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);
    // force notification config to zero when peer device is disconnected
    ayla_conn_env->ntf_cfg[conidx] = 0;
}


///  Task interface required by profile manager
const struct prf_task_cbs ayla_conn_itf =
{
        (prf_init_fnct) ayla_conn_init,
        ayla_conn_destroy,
        ayla_conn_create,
        ayla_conn_cleanup,
};


const struct prf_task_cbs* ayla_conn_prf_itf_get(void)
{
   return &ayla_conn_itf;
}


uint16_t ayla_conn_get_att_handle( uint8_t att_idx)
{
		
    struct ayla_conn_env_tag *ayla_conn_env = PRF_ENV_GET(AYLA_CONN, ayla_conn);
    uint16_t handle = ATT_INVALID_HDL;
   
   
    handle = ayla_conn_env->start_hdl;
    printf("ayla_conn_get_att_handle  hdl_cursor = 0x%x\r\n",handle);
    // increment index according to expected index
    if(att_idx < AYLA_CONN_IDX_NB)
    {
        handle += att_idx;
    }
    else
    {
        handle = ATT_INVALID_HDL;
    }
    
    return handle;
}

uint8_t ayla_conn_get_att_idx(uint16_t handle, uint8_t *att_idx)
{
    struct ayla_conn_env_tag* ayla_conn_env = PRF_ENV_GET(AYLA_CONN, ayla_conn);
    uint16_t hdl_cursor = ayla_conn_env->start_hdl;
    printf("ayla_conn_get_att_idx  hdl_cursor = 0x%x, handle:0x%x\r\n", hdl_cursor, handle);
    uint8_t status = PRF_APP_ERROR;

    // Browse list of services
    // handle must be greater than current index 
    // check if it's a mandatory index
    if(handle <= (hdl_cursor + AYLA_CONN_IDX_SETO_VAL_VALUE))
    {
        *att_idx = handle -hdl_cursor;
        status = GAP_ERR_NO_ERROR;
				
		return (status);
    }

    return (status);
}

#endif // (BLE_AYLA_CONN_SERVER)



 
