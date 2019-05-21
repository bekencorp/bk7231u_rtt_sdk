/**
 ****************************************************************************************
 *
 * @file ayla_conn.h
 *
 * @brief Header file - media Service Server Role
 *
 * Copyright (C) beken 2009-2018
 *
 *
 ****************************************************************************************
 */
#ifndef _AYLA_CONN_H_
#define _AYLA_CONN_H_

/**
 ****************************************************************************************
 * @addtogroup  AYLA_CONN 'Profile' Server
 * @ingroup AYLA_CONN
 * @brief AYLA_CONN 'Profile' Server
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "rwprf_config.h"

#if (BLE_AYLA_CONN_SERVER)

#include "ayla_conn_task.h"
#include "atts.h"
#include "prf_types.h"
#include "prf.h"
#include "ble_compiler.h"
/*
 * DEFINES
 ****************************************************************************************
 */

#define AYLA_CONN_CFG_FLAG_MANDATORY_MASK       (0x7F)
#define AYLA_CONN_CFG_FLAG_NTF_SUP_MASK         (0x08)

#define ATT_DECL_PRIMARY_SERVICE_128 {0x00,0x28,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define ATT_DECL_CHARACTERISTIC_128 {0x03,0x28,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define ATT_DESC_CLIENT_CHAR_CFG_128 {0x02,0x29,0,0,0,0,0,0,0,0,0,0,0,0,0,0}

#define ATT_CHAR_AYLA_CONN_SETO_128 {0x34,0xA8,0x1B,0xEF,0x21,0x1C,0xEA,0x88,0x20,0x45,0xB3,0x4D,0xED,0x69,0x98,0x7E}

///  Service Attributes Indexes
enum
{
	AYLA_CONN_IDX_SVC,	 
	AYLA_CONN_IDX_SETO_VAL_CHAR,
	AYLA_CONN_IDX_SETO_VAL_VALUE,
	AYLA_CONN_IDX_NB,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


///  'Profile' Server environment variable
struct ayla_conn_env_tag
{
    /// profile environment
    prf_env_t prf_env;
   
    /// On-going operation
    struct kernel_msg * operation;
    /// Services Start Handle
    uint16_t start_hdl;
   
    /// BASS task state
    kernel_state_t state[AYLA_CONN_IDX_MAX];
    /// 
    uint8_t rd_value[AYLA_CONN_CHAR_DATA_LEN];

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
const struct prf_task_cbs* ayla_conn_prf_itf_get(void);

uint16_t ayla_conn_get_att_handle(uint8_t att_idx);

uint8_t  ayla_conn_get_att_idx(uint16_t handle, uint8_t *att_idx);

#endif /* #if (BLE_AYLA_CONN_SERVER) */



#endif /*  _BTL_H_ */




