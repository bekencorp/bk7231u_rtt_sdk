/**
 ****************************************************************************************
 *
 * @file sn.h
 *
 * @brief Header file - media Service Server Role
 *
 * Copyright (C) beken 2009-2018
 *
 *
 ****************************************************************************************
 */
#ifndef _SN_H_
#define _SN_H_

/**
 ****************************************************************************************
 * @addtogroup  SN 'Profile' Server
 * @ingroup SN
 * @brief SN 'Profile' Server
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "rwprf_config.h"

#if (BLE_SN_SERVER)

#include "sn_task.h"
#include "atts.h"
#include "prf_types.h"
#include "prf.h"
#include "ble_compiler.h"
/*
 * DEFINES
 ****************************************************************************************
 */

#define SN_CFG_FLAG_MANDATORY_MASK       (0x7F)
#define SN_CFG_FLAG_NTF_SUP_MASK         (0x08)

enum
{		
	ATT_SVC_SN 			   = ATT_UUID_16(0x5300),

	ATT_CHAR_SN_5301         =  ATT_UUID_16(0x5301),

	ATT_CHAR_SN_5302		   = ATT_UUID_16(0x5302),
		
};

///  Service Attributes Indexes
enum
{
	SN_IDX_SVC,	 
	SN_IDX_5301_VAL_CHAR,
	SN_IDX_5301_VAL_VALUE,
	SN_IDX_5301_VAL_NTF_CFG,
	SN_IDX_5302_VAL_CHAR,
	SN_IDX_5302_VAL_VALUE,
	SN_IDX_NB,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


///  'Profile' Server environment variable
struct sn_env_tag
{
    /// profile environment
    prf_env_t prf_env;
   
    /// On-going operation
    struct kernel_msg * operation;
    /// Services Start Handle
    uint16_t start_hdl;
   
    /// BASS task state
    kernel_state_t state[SN_IDX_MAX];
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
const struct prf_task_cbs* sn_prf_itf_get(void);

uint16_t sn_get_att_handle(uint8_t att_idx);

uint8_t  sn_get_att_idx(uint16_t handle, uint8_t *att_idx);

void sn_exe_operation(void);

#endif /* #if (BLE_SN_SERVER) */



#endif /*  _BTL_H_ */




