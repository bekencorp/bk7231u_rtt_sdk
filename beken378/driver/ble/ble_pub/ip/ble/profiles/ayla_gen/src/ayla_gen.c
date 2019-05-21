/**
 ****************************************************************************************
 *
 * @file ayla_gen.c
 *
 * @brief AYLA_GEN Server Implementation.
 *
 * Copyright (C) beken 2009-2018
 *
 *
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_AYLA_GEN_SERVER)
#include "attm.h"
#include "ayla_gen.h"
#include "ayla_gen_task.h"
#include "prf_utils.h"
#include "prf.h"
#include "kernel_mem.h"




/*
 * FFF0 ATTRIBUTES DEFINITION
 ****************************************************************************************
 */

#if 0
/// Full FFF0 Database Description - Used to add attributes into the database
const struct attm_desc ayla_gen_att_db[AYLA_GEN_IDX_NB] =
{
	//  Service Declaration
	[AYLA_GEN_IDX_SVC]                  =   {ATT_DECL_PRIMARY_SERVICE,  PERM(RD, ENABLE), 0, 0},
	
	//  Level Characteristic Declaration
	[AYLA_GEN_IDX_FFA1_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC,   PERM(RD, ENABLE), 0, 0},
	//  Level Characteristic Value
	[AYLA_GEN_IDX_FFA1_VAL_VALUE]       =   {ATT_CHAR_AYLA_GEN_FFA1,   PERM(NTF, ENABLE)|PERM(WRITE_REQ, ENABLE), PERM(RI, ENABLE) , 128},
	//  Level Characteristic - Client Characteristic Configuration Descriptor

	[AYLA_GEN_IDX_FFA1_VAL_NTF_CFG]     =   {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},

};/// Macro used to retrieve permission value from access and rights on attribute.
#endif

const uint8_t ayla_gen_service[2] = {0x28, 0xFE};

/// Full FFF0 Database Description - Used to add attributes into the database
const struct attm_desc_128 ayla_gen_att_db[AYLA_GEN_IDX_NB] =
{
	//  Service Declaration
	[AYLA_GEN_IDX_SVC]                  =   {ATT_DECL_PRIMARY_SERVICE_128,  PERM(RD, ENABLE), 0, 0},
	
	//  Level Characteristic Declaration
	[AYLA_GEN_IDX_DUID_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC_128,   PERM(RD, ENABLE), 0, 0},
	//  Level Characteristic Value
	[AYLA_GEN_IDX_DUID_VAL_VALUE]       =   {ATT_CHAR_AYLA_GEN_DUID_128,   PERM(RD, ENABLE), PERM(RI, ENABLE)|(0x2 << 13) , 128},
	
	[AYLA_GEN_IDX_OEMID_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC_128,   PERM(RD, ENABLE), 0, 0},
	[AYLA_GEN_IDX_OEMID_VAL_VALUE]      =   {ATT_CHAR_AYLA_GEN_OEMID_128,  PERM(RD, ENABLE), PERM(RI, ENABLE)|(0x2 << 13) , 128},
	
	[AYLA_GEN_IDX_OEMMID_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC_128,   PERM(RD, ENABLE), 0, 0},
	[AYLA_GEN_IDX_OEMMID_VAL_VALUE]     =   {ATT_CHAR_AYLA_GEN_OEMMID_128, PERM(RD, ENABLE), PERM(RI, ENABLE)|(0x2 << 13) , 128},
	
	[AYLA_GEN_IDX_TVID_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC_128,   PERM(RD, ENABLE), 0, 0},
	[AYLA_GEN_IDX_TVID_VAL_VALUE]       =   {ATT_CHAR_AYLA_GEN_TVID_128,   PERM(RD, ENABLE), PERM(RI, ENABLE)|(0x2 << 13) , 128},
	
	[AYLA_GEN_IDX_IDID_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC_128,   PERM(RD, ENABLE), 0, 0},
	[AYLA_GEN_IDX_IDID_VAL_VALUE]       =   {ATT_CHAR_AYLA_GEN_IDID_128,   PERM(WRITE_REQ, ENABLE), PERM(RI, ENABLE)|(0x2 << 13) , 128},
	
	[AYLA_GEN_IDX_DNID_VAL_CHAR]        =   {ATT_DECL_CHARACTERISTIC_128,   PERM(RD, ENABLE), 0, 0},
	[AYLA_GEN_IDX_DNID_VAL_VALUE]       =   {ATT_CHAR_AYLA_GEN_DNID_128,   PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), PERM(RI, ENABLE)|(0x2 << 13) , 128},

};/// Macro used to retrieve permission value from access and rights on attribute.

static uint8_t ayla_gen_init (struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl,  struct ayla_gen_db_cfg* params)
{
    uint16_t shdl;
    struct ayla_gen_env_tag* ayla_gen_env = NULL;
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    
    //-------------------- allocate memory required for the profile  ---------------------
    ayla_gen_env = (struct ayla_gen_env_tag* ) kernel_malloc(sizeof(struct ayla_gen_env_tag), KERNEL_MEM_ATT_DB);
    memset(ayla_gen_env, 0 , sizeof(struct ayla_gen_env_tag));

   
    // Service content flag
    uint16_t cfg_flag =  params->cfg_flag;

    // Save database configuration
    ayla_gen_env->features |= (params->features) ;
   
    shdl = *start_hdl;

    //Create FFF0 in the DB
    //------------------ create the attribute database for the profile -------------------
    status = attm_svc_create_db_128(&(shdl), ayla_gen_service, (uint8_t *)&cfg_flag,
            AYLA_GEN_IDX_NB, NULL, env->task, &ayla_gen_att_db[0],
            (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS | PERM_MASK_SVC_UUID_LEN)));
						
    //-------------------- Update profile task information  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {

        // allocate BASS required environment variable
        env->env = (prf_env_t*) ayla_gen_env;
        *start_hdl = shdl;
        ayla_gen_env->start_hdl = *start_hdl;
        ayla_gen_env->prf_env.app_task = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        ayla_gen_env->prf_env.prf_task = env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        env->id                     = TASK_ID_AYLA_GEN;
        env->desc.idx_max           = AYLA_GEN_IDX_MAX;
        env->desc.state             = ayla_gen_env->state;
        env->desc.default_handler   = &ayla_gen_default_handler;

        printf("ayla_gen_env->start_hdl = 0x%x",ayla_gen_env->start_hdl);

        // service is ready, go into an Idle state
        kernel_state_set(env->task, AYLA_GEN_IDLE);
    }
    else if(ayla_gen_env != NULL)
    {
        kernel_free(ayla_gen_env);
    }
     
    return (status);
}


static void ayla_gen_destroy(struct prf_task_env* env)
{
    struct ayla_gen_env_tag* ayla_gen_env = (struct ayla_gen_env_tag*) env->env;

    // clear on-going operation
    if(ayla_gen_env->operation != NULL)
    {
        kernel_free(ayla_gen_env->operation);
    }

    // free profile environment variables
    env->env = NULL;
    kernel_free(ayla_gen_env);
}

static void ayla_gen_create(struct prf_task_env* env, uint8_t conidx)
{
    struct ayla_gen_env_tag* ayla_gen_env = (struct ayla_gen_env_tag*) env->env;
    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);

    // force notification config to zero when peer device is connected
    ayla_gen_env->ntf_cfg[conidx] = 0;
}


static void ayla_gen_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
    struct ayla_gen_env_tag* ayla_gen_env = (struct ayla_gen_env_tag*) env->env;

    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);
    // force notification config to zero when peer device is disconnected
   ayla_gen_env->ntf_cfg[conidx] = 0;
}


///  Task interface required by profile manager
const struct prf_task_cbs ayla_gen_itf =
{
        (prf_init_fnct) ayla_gen_init,
        ayla_gen_destroy,
        ayla_gen_create,
        ayla_gen_cleanup,
};


const struct prf_task_cbs* ayla_gen_prf_itf_get(void)
{
   return &ayla_gen_itf;
}


uint16_t ayla_gen_get_att_handle( uint8_t att_idx)
{
		
    struct ayla_gen_env_tag *ayla_gen_env = PRF_ENV_GET(AYLA_GEN, ayla_gen);
    uint16_t handle = ATT_INVALID_HDL;
   
   
    handle = ayla_gen_env->start_hdl;
    printf("ayla_gen_get_att_handle  hdl_cursor = 0x%x\r\n",handle);
    // increment index according to expected index
    if(att_idx < AYLA_GEN_IDX_NB)
    {
        handle += att_idx;
    }
    else
    {
        handle = ATT_INVALID_HDL;
    }
    
    return handle;
}

uint8_t ayla_gen_get_att_idx(uint16_t handle, uint8_t *att_idx)
{
    struct ayla_gen_env_tag* ayla_gen_env = PRF_ENV_GET(AYLA_GEN, ayla_gen);
    uint16_t hdl_cursor = ayla_gen_env->start_hdl;
    printf("ayla_gen_get_att_idx  hdl_cursor = 0x%x, handle:0x%x\r\n", hdl_cursor, handle);
    uint8_t status = PRF_APP_ERROR;

    // Browse list of services
    // handle must be greater than current index 
    // check if it's a mandatory index
    if(handle <= (hdl_cursor + AYLA_GEN_IDX_DNID_VAL_VALUE))
    {
        *att_idx = handle -hdl_cursor;
        status = GAP_ERR_NO_ERROR;
				
		return (status);
    }

    return (status);
}

#endif // (BLE_AYLA_GEN_SERVER)



 
