/**
 ****************************************************************************************
 *
 * @file ayla_gen.h
 *
 * @brief Header file - media Service Server Role
 *
 * Copyright (C) beken 2009-2018
 *
 *
 ****************************************************************************************
 */
#ifndef _AYLA_GEN_H_
#define _AYLA_GEN_H_

/**
 ****************************************************************************************
 * @addtogroup  AYLA_GEN 'Profile' Server
 * @ingroup AYLA_GEN
 * @brief AYLA_GEN 'Profile' Server
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "rwprf_config.h"

#if (BLE_AYLA_GEN_SERVER)

#include "ayla_gen_task.h"
#include "atts.h"
#include "prf_types.h"
#include "prf.h"
#include "ble_compiler.h"
/*
 * DEFINES
 ****************************************************************************************
 */

#define AYLA_GEN_CFG_FLAG_MANDATORY_MASK       (0x7F)
#define AYLA_GEN_CFG_FLAG_NTF_SUP_MASK         (0x08)

#define ATT_DECL_PRIMARY_SERVICE_128 {0x00,0x28,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define ATT_DECL_CHARACTERISTIC_128 {0x03,0x28,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define ATT_DESC_CLIENT_CHAR_CFG_128 {0x02,0x29,0,0,0,0,0,0,0,0,0,0,0,0,0,0}

#define ATT_CHAR_AYLA_GEN_DUID_128 {0xD0,0xBC,0xB9,0x1B,0xB2,0xF1,0x1A,0x99,0x5B,0x43,0x28,0xFE,0x01,0x00,0x00,0x00}
#define ATT_CHAR_AYLA_GEN_OEMID_128 {0xD0,0xBC,0xB9,0x1B,0xB2,0xF1,0x1A,0x99,0x5B,0x43,0x28,0xFE,0x02,0x00,0x00,0x00}
#define ATT_CHAR_AYLA_GEN_OEMMID_128 {0xD0,0xBC,0xB9,0x1B,0xB2,0xF1,0x1A,0x99,0x5B,0x43,0x28,0xFE,0x03,0x00,0x00,0x00}
#define ATT_CHAR_AYLA_GEN_TVID_128 {0xD0,0xBC,0xB9,0x1B,0xB2,0xF1,0x1A,0x99,0x5B,0x43,0x28,0xFE,0x04,0x00,0x00,0x00}
#define ATT_CHAR_AYLA_GEN_IDID_128 {0xD0,0xBC,0xB9,0x1B,0xB2,0xF1,0x1A,0x99,0x5B,0x43,0x28,0xFE,0x05,0x00,0x00,0x00}
#define ATT_CHAR_AYLA_GEN_DNID_128 {0xD0,0xBC,0xB9,0x1B,0xB2,0xF1,0x1A,0x99,0x5B,0x43,0x28,0xFE,0x04,0x00,0x00,0x00}


///  Service Attributes Indexes
enum
{
	AYLA_GEN_IDX_SVC,	 
	AYLA_GEN_IDX_DUID_VAL_CHAR,
	AYLA_GEN_IDX_DUID_VAL_VALUE,
	AYLA_GEN_IDX_OEMID_VAL_CHAR,
	AYLA_GEN_IDX_OEMID_VAL_VALUE,
	AYLA_GEN_IDX_OEMMID_VAL_CHAR,
	AYLA_GEN_IDX_OEMMID_VAL_VALUE,
	AYLA_GEN_IDX_TVID_VAL_CHAR,
	AYLA_GEN_IDX_TVID_VAL_VALUE,
	AYLA_GEN_IDX_IDID_VAL_CHAR,
	AYLA_GEN_IDX_IDID_VAL_VALUE,
	AYLA_GEN_IDX_DNID_VAL_CHAR,
	AYLA_GEN_IDX_DNID_VAL_VALUE,
	AYLA_GEN_IDX_NB,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


///  'Profile' Server environment variable
struct ayla_gen_env_tag
{
    /// profile environment
    prf_env_t prf_env;
   
    /// On-going operation
    struct kernel_msg * operation;
    /// Services Start Handle
    uint16_t start_hdl;
   
    /// BASS task state
    kernel_state_t state[AYLA_GEN_IDX_MAX];
    /// 
    uint8_t rd_value[AYLA_GEN_CHAR_DATA_LEN];

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
const struct prf_task_cbs* ayla_gen_prf_itf_get(void);

uint16_t ayla_gen_get_att_handle(uint8_t att_idx);

uint8_t  ayla_gen_get_att_idx(uint16_t handle, uint8_t *att_idx);

#endif /* #if (BLE_AYLA_GEN_SERVER) */



#endif /*  _BTL_H_ */




