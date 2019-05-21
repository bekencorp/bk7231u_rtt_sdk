/**
 ****************************************************************************************
 *
 * @file ayla_wifi.h
 *
 * @brief Header file - media Service Server Role
 *
 * Copyright (C) beken 2009-2018
 *
 *
 ****************************************************************************************
 */
#ifndef _AYLA_WIFI_H_
#define _AYLA_WIFI_H_

/**
 ****************************************************************************************
 * @addtogroup  AYLA_WIFI 'Profile' Server
 * @ingroup AYLA_WIFI
 * @brief AYLA_WIFI 'Profile' Server
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "rwprf_config.h"

#if (BLE_AYLA_WIFI_SERVER)

#include "ayla_wifi_task.h"
#include "atts.h"
#include "prf_types.h"
#include "prf.h"
#include "ble_compiler.h"
/*
 * DEFINES
 ****************************************************************************************
 */

#define AYLA_WIFI_CFG_FLAG_MANDATORY_MASK       (0x7F)
#define AYLA_WIFI_CFG_FLAG_NTF_SUP_MASK         (0x08)

#define ATT_DECL_PRIMARY_SERVICE_128 {0x00,0x28,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define ATT_DECL_CHARACTERISTIC_128 {0x03,0x28,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define ATT_DESC_CLIENT_CHAR_CFG_128 {0x02,0x29,0,0,0,0,0,0,0,0,0,0,0,0,0,0}

#define ATT_CHAR_AYLA_WIFI_CONN_128 {0x6F,0xF1,0xD8,0x54,0xF8,0x00,0xE5,0x94,0x35,0x4E,0x71,0x2B,0x6A,0xAF,0x80,0x1F}
#define ATT_CHAR_AYLA_WIFI_WPS_128 {0x6F,0xF1,0xD8,0x54,0xF8,0x00,0xE5,0x94,0x35,0x4E,0x71,0x2B,0x6B,0xAF,0x80,0x1F}
#define ATT_CHAR_AYLA_WIFI_STATU_128 {0x6F,0xF1,0xD8,0x54,0xF8,0x00,0xE5,0x94,0x35,0x4E,0x71,0x2B,0x6C,0xAF,0x80,0x1F}
#define ATT_CHAR_AYLA_WIFI_SCAN_128 {0x6F,0xF1,0xD8,0x54,0xF8,0x00,0xE5,0x94,0x35,0x4E,0x71,0x2B,0x6D,0xAF,0x80,0x1F}
#define ATT_CHAR_AYLA_WIFI_SCRE_128 {0x6F,0xF1,0xD8,0x54,0xF8,0x00,0xE5,0x94,0x35,0x4E,0x71,0x2B,0x6E,0xAF,0x80,0x1F}

///  Service Attributes Indexes
enum
{
	AYLA_WIFI_IDX_SVC,	 
	AYLA_WIFI_IDX_CONN_VAL_CHAR,
	AYLA_WIFI_IDX_CONN_VAL_VALUE,
	AYLA_WIFI_IDX_WPS_VAL_CHAR,
	AYLA_WIFI_IDX_WPS_VAL_VALUE,
	AYLA_WIFI_IDX_STATU_VAL_CHAR,
	AYLA_WIFI_IDX_STATU_VAL_VALUE,
	AYLA_WIFI_IDX_STATU_VAL_NTF_CFG,
	AYLA_WIFI_IDX_SCAN_VAL_CHAR,
	AYLA_WIFI_IDX_SCAN_VAL_VALUE,
	AYLA_WIFI_IDX_SCRE_VAL_CHAR,
	AYLA_WIFI_IDX_SCRE_VAL_VALUE,
	AYLA_WIFI_IDX_SCRE_VAL_NTF_CFG,
	AYLA_WIFI_IDX_NB,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


///  'Profile' Server environment variable
struct ayla_wifi_env_tag
{
    /// profile environment
    prf_env_t prf_env;
   
    /// On-going operation
    struct kernel_msg * operation;
    /// Services Start Handle
    uint16_t start_hdl;
   
    /// BASS task state
    kernel_state_t state[AYLA_WIFI_IDX_MAX];
    /// 
    uint8_t rd_value[AYLA_WIFI_CHAR_DATA_LEN];

    uint16_t statu_ntf_ind_cfg[BLE_CONNECTION_MAX];

    uint16_t scre_ntf_ind_cfg[BLE_CONNECTION_MAX];
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
const struct prf_task_cbs* ayla_wifi_prf_itf_get(void);

uint16_t ayla_wifi_get_att_handle(uint8_t att_idx);

uint8_t  ayla_wifi_get_att_idx(uint16_t handle, uint8_t *att_idx);

#endif /* #if (BLE_AYLA_WIFI_SERVER) */



#endif /*  _BTL_H_ */




