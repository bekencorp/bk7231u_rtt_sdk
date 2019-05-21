/**
 ****************************************************************************************
 *
 * @file ayla_wifi.c
 *
 * @brief AYLA_WIFI Server Implementation.
 *
 * Copyright (C) beken 2009-2018
 *
 *
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_AYLA_WIFI_SERVER)
#include "attm.h"
#include "ayla_wifi.h"
#include "ayla_wifi_task.h"
#include "prf_utils.h"
#include "prf.h"
#include "kernel_mem.h"




/*
 * FFF0 ATTRIBUTES DEFINITION
 ****************************************************************************************
 */

#if 0
/// Full FFF0 Database Description - Used to add attributes into the database
const struct attm_desc ayla_wifi_att_db[AYLA_WIFI_IDX_NB] =
{
	//  Service Declaration
	[AYLA_WIFI_IDX_SVC]                  =   {ATT_DECL_PRIMARY_SERVICE,  PERM(RD, ENABLE), 0, 0},
	
	//  Level Characteristic Declaration
	[AYLA_WIFI_IDX_FFA1_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC,   PERM(RD, ENABLE), 0, 0},
	//  Level Characteristic Value
	[AYLA_WIFI_IDX_FFA1_VAL_VALUE]       =   {ATT_CHAR_AYLA_WIFI_FFA1,   PERM(NTF, ENABLE)|PERM(WRITE_REQ, ENABLE), PERM(RI, ENABLE) , 128},
	//  Level Characteristic - Client Characteristic Configuration Descriptor

	[AYLA_WIFI_IDX_FFA1_VAL_NTF_CFG]     =   {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},

};/// Macro used to retrieve permission value from access and rights on attribute.
#endif

const uint8_t ayla_wifi_service[16] = {0x96,0x4B,0x12,0xAB,0x87,0xE2,0xFC,0xA9,0x6E,0x4D,0xCF,0x3E,0x66,0xFE,0xF0,0x1C};

/// Full FFF0 Database Description - Used to add attributes into the database
const struct attm_desc_128 ayla_wifi_att_db[AYLA_WIFI_IDX_NB] =
{
	//  Service Declaration
	[AYLA_WIFI_IDX_SVC]                =   {ATT_DECL_PRIMARY_SERVICE_128,  PERM(RD, ENABLE), 0, 0},
	
	//  Level Characteristic Declaration
	[AYLA_WIFI_IDX_CONN_VAL_CHAR]      =   {ATT_DECL_CHARACTERISTIC_128,   PERM(RD, ENABLE), 0, 0},
	//  Level Characteristic Value
	[AYLA_WIFI_IDX_CONN_VAL_VALUE]     =   {ATT_CHAR_AYLA_WIFI_CONN_128,   PERM(WRITE_REQ, ENABLE), PERM(RI, ENABLE)|PERM(UUID_LEN, UUID_128) , 128},
	
	[AYLA_WIFI_IDX_WPS_VAL_CHAR]       =   {ATT_DECL_CHARACTERISTIC_128,   PERM(RD, ENABLE), 0, 0},
	[AYLA_WIFI_IDX_WPS_VAL_VALUE]      =   {ATT_CHAR_AYLA_WIFI_WPS_128,  PERM(WRITE_REQ, ENABLE), PERM(RI, ENABLE)|PERM(UUID_LEN, UUID_128) , 128},
	
	[AYLA_WIFI_IDX_STATU_VAL_CHAR]     =   {ATT_DECL_CHARACTERISTIC_128,   PERM(RD, ENABLE), 0, 0},
	[AYLA_WIFI_IDX_STATU_VAL_VALUE]    =   {ATT_CHAR_AYLA_WIFI_STATU_128, PERM(RD, ENABLE)|PERM(NTF, ENABLE), PERM(RI, ENABLE)|PERM(UUID_LEN, UUID_128) , 128},
	[AYLA_WIFI_IDX_STATU_VAL_NTF_CFG]  =   {ATT_DESC_CLIENT_CHAR_CFG_128, PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},
	
	[AYLA_WIFI_IDX_SCAN_VAL_CHAR]      =   {ATT_DECL_CHARACTERISTIC_128,   PERM(RD, ENABLE), 0, 0},
	[AYLA_WIFI_IDX_SCAN_VAL_VALUE]     =   {ATT_CHAR_AYLA_WIFI_SCAN_128,   PERM(WRITE_REQ, ENABLE), PERM(RI, ENABLE)|PERM(UUID_LEN, UUID_128) , 128},
	
	[AYLA_WIFI_IDX_SCRE_VAL_CHAR]      =   {ATT_DECL_CHARACTERISTIC_128,   PERM(RD, ENABLE), 0, 0},
	[AYLA_WIFI_IDX_SCRE_VAL_VALUE]     =   {ATT_CHAR_AYLA_WIFI_SCRE_128,   PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE)|PERM(NTF, ENABLE), PERM(RI, ENABLE)|PERM(UUID_LEN, UUID_128) , 128},
	[AYLA_WIFI_IDX_SCRE_VAL_NTF_CFG]   =   {ATT_DESC_CLIENT_CHAR_CFG_128, PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},
	
};/// Macro used to retrieve permission value from access and rights on attribute.

static uint8_t ayla_wifi_init (struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl,  struct ayla_wifi_db_cfg* params)
{
    uint16_t shdl;
    struct ayla_wifi_env_tag* ayla_wifi_env = NULL;
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    
    //-------------------- allocate memory required for the profile  ---------------------
    ayla_wifi_env = (struct ayla_wifi_env_tag* ) kernel_malloc(sizeof(struct ayla_wifi_env_tag), KERNEL_MEM_ATT_DB);
    memset(ayla_wifi_env, 0 , sizeof(struct ayla_wifi_env_tag));

   
    // Service content flag
    uint16_t cfg_flag =  params->cfg_flag;

    // Save database configuration
    ayla_wifi_env->features |= (params->features) ;
   
    shdl = *start_hdl;

    //Create FFF0 in the DB
    //------------------ create the attribute database for the profile -------------------
    status = attm_svc_create_db_128(&(shdl), ayla_wifi_service, (uint8_t *)&cfg_flag,
            AYLA_WIFI_IDX_NB, NULL, env->task, &ayla_wifi_att_db[0],
            (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS | PERM_MASK_SVC_UUID_LEN)));
						
    //-------------------- Update profile task information  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {

        // allocate BASS required environment variable
        env->env = (prf_env_t*) ayla_wifi_env;
        *start_hdl = shdl;
        ayla_wifi_env->start_hdl = *start_hdl;
        ayla_wifi_env->prf_env.app_task = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        ayla_wifi_env->prf_env.prf_task = env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        env->id                     = TASK_ID_AYLA_WIFI;
        env->desc.idx_max           = AYLA_WIFI_IDX_MAX;
        env->desc.state             = ayla_wifi_env->state;
        env->desc.default_handler   = &ayla_wifi_default_handler;

        printf("ayla_wifi_env->start_hdl = 0x%x",ayla_wifi_env->start_hdl);

        // service is ready, go into an Idle state
        kernel_state_set(env->task, AYLA_WIFI_IDLE);
    }
    else if(ayla_wifi_env != NULL)
    {
        kernel_free(ayla_wifi_env);
    }
     
    return (status);
}


static void ayla_wifi_destroy(struct prf_task_env* env)
{
    struct ayla_wifi_env_tag* ayla_wifi_env = (struct ayla_wifi_env_tag*) env->env;

    // clear on-going operation
    if(ayla_wifi_env->operation != NULL)
    {
        kernel_free(ayla_wifi_env->operation);
    }

    // free profile environment variables
    env->env = NULL;
    kernel_free(ayla_wifi_env);
}

static void ayla_wifi_create(struct prf_task_env* env, uint8_t conidx)
{
    struct ayla_wifi_env_tag* ayla_wifi_env = (struct ayla_wifi_env_tag*) env->env;
    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);

    // force notification config to zero when peer device is connected
    ayla_wifi_env->statu_ntf_ind_cfg[conidx] = 0;
    ayla_wifi_env->scre_ntf_ind_cfg[conidx] = 0;
}


static void ayla_wifi_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
    struct ayla_wifi_env_tag* ayla_wifi_env = (struct ayla_wifi_env_tag*) env->env;

    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);
    // force notification config to zero when peer device is disconnected
    ayla_wifi_env->statu_ntf_ind_cfg[conidx] = 0;
    ayla_wifi_env->scre_ntf_ind_cfg[conidx] = 0;
}


///  Task interface required by profile manager
const struct prf_task_cbs ayla_wifi_itf =
{
        (prf_init_fnct) ayla_wifi_init,
        ayla_wifi_destroy,
        ayla_wifi_create,
        ayla_wifi_cleanup,
};


const struct prf_task_cbs* ayla_wifi_prf_itf_get(void)
{
   return &ayla_wifi_itf;
}


uint16_t ayla_wifi_get_att_handle( uint8_t att_idx)
{
		
    struct ayla_wifi_env_tag *ayla_wifi_env = PRF_ENV_GET(AYLA_WIFI, ayla_wifi);
    uint16_t handle = ATT_INVALID_HDL;
   
   
    handle = ayla_wifi_env->start_hdl;
    printf("ayla_wifi_get_att_handle  hdl_cursor = 0x%x\r\n",handle);
    // increment index according to expected index
    if(att_idx < AYLA_WIFI_IDX_NB)
    {
        handle += att_idx;
    }
    else
    {
        handle = ATT_INVALID_HDL;
    }
    
    return handle;
}

uint8_t ayla_wifi_get_att_idx(uint16_t handle, uint8_t *att_idx)
{
    struct ayla_wifi_env_tag* ayla_wifi_env = PRF_ENV_GET(AYLA_WIFI, ayla_wifi);
    uint16_t hdl_cursor = ayla_wifi_env->start_hdl;
    printf("ayla_wifi_get_att_idx  hdl_cursor = 0x%x, handle:0x%x\r\n", hdl_cursor, handle);
    uint8_t status = PRF_APP_ERROR;

    // Browse list of services
    // handle must be greater than current index 
    // check if it's a mandatory index
    if(handle <= (hdl_cursor + AYLA_WIFI_IDX_SCRE_VAL_NTF_CFG))
    {
        *att_idx = handle -hdl_cursor;
        status = GAP_ERR_NO_ERROR;
				
		return (status);
    }

    return (status);
}

#endif // (BLE_AYLA_WIFI_SERVER)



 
