/**
 ****************************************************************************************
 *
 * @file ayla.h
 *
 * @brief Header file - media Service Server Role
 *
 * Copyright (C) beken 2009-2018
 *
 *
 ****************************************************************************************
 */
#ifndef _AYLA_H_
#define _AYLA_H_

/**
 ****************************************************************************************
 * @addtogroup  AYLA 'Profile' Server
 * @ingroup AYLA
 * @brief AYLA 'Profile' Server
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "rwprf_config.h"

#if (BLE_AYLA_SERVER)

#include "ayla_task.h"
#include "atts.h"
#include "prf_types.h"
#include "prf.h"
#include "ble_compiler.h"
/*
 * DEFINES
 ****************************************************************************************
 */

#define AYLA_CFG_FLAG_MANDATORY_MASK       (0x7F)
#define AYLA_CFG_FLAG_NTF_SUP_MASK         (0x08)

enum
{		
	ATT_SVC_AYLA 			   = ATT_UUID_16(0xFFA0),

	ATT_CHAR_AYLA_FFA1         =  ATT_UUID_16(0xFFA1),
		
};

///  Service Attributes Indexes
enum
{
	AYLA_IDX_SVC,	 
	AYLA_IDX_FFA1_VAL_CHAR,
	AYLA_IDX_FFA1_VAL_VALUE,
	AYLA_IDX_FFA1_VAL_NTF_CFG,
	AYLA_IDX_NB,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


///  'Profile' Server environment variable
struct ayla_env_tag
{
    /// profile environment
    prf_env_t prf_env;
   
    /// On-going operation
    struct kernel_msg * operation;
    /// Services Start Handle
    uint16_t start_hdl;
   
    /// BASS task state
    kernel_state_t state[AYLA_IDX_MAX];
    /// 
    uint16_t ntf_cfg[BLE_CONNECTION_MAX];
    /// Database features
    uint16_t features;	
};



/**
 ****************************************************************************************
 * @brief Retrieve 5300 service profile interface
 *
 * @return 5300 service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* ayla_prf_itf_get(void);

uint16_t ayla_get_att_handle(uint8_t att_idx);

uint8_t  ayla_get_att_idx(uint16_t handle, uint8_t *att_idx);

void ayla_exe_operation(void);

#endif /* #if (BLE_AYLA_SERVER) */



#endif /*  _BTL_H_ */




