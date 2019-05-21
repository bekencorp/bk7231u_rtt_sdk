/**
 ****************************************************************************************
 *
 * @file ayla_wifi_task.h
 *
 * @brief Header file - AYLA_WIFI Service Server Role Task.
 *
 * Copyright (C) RivieraWaves 2009-2018
 *
 *
 ****************************************************************************************
 */


#ifndef _AYLA_WIFI_TASK_H_
#define _AYLA_WIFI_TASK_H_


#include "rwprf_config.h"
#if (BLE_AYLA_WIFI_SERVER)
#include <stdint.h>
#include "rwip_task.h" // Task definitions
#include "ble_compiler.h"
/*
 * DEFINES
 ****************************************************************************************
 */

///Maximum number of AYLA_WIFI Server task instances
#define AYLA_WIFI_IDX_MAX     0x01
///Maximal number of AYLA_WIFI that can be added in the DB

#define  AYLA_WIFI_CHAR_DATA_LEN  128

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Possible states of the AYLA_WIFI task
enum ayla_wifi_state
{
    /// Idle state
    AYLA_WIFI_IDLE,
    /// busy state
    AYLA_WIFI_BUSY,
    /// Number of defined states.
    AYLA_WIFI_STATE_MAX
};

/// Messages for FFA0 Server
enum ayla_wifi_msg_id
{
	/// Start the FFA0 Server - at connection used to restore bond data
	AYLA_WIFI_ENABLE_REQ  =  TASK_FIRST_MSG(TASK_ID_AYLA_WIFI),

	/// Confirmation of the AYLA_WIFI Server start
	AYLA_WIFI_ENABLE_RSP,

    AYLA_WIFI_STATU_NTF_UPD_REQ,

    AYLA_WIFI_SCRE_NTF_UPD_REQ,

    AYLA_WIFI_CONN_WRITE_IND,

    AYLA_WIFI_WPS_WRITE_IND,

    AYLA_WIFI_STATU_NTF_CFG_IND,

    AYLA_WIFI_SCAN_WRITE_IND,

    AYLA_WIFI_SCRE_WRITE_IND,

    AYLA_WIFI_SCRE_NTF_CFG_IND,

	AYLA_WIFI_GATTC_CMP_EVT
};

/// Features Flag Masks
enum ayla_wifi_features
{
    /// FFA1 Level Characteristic doesn't support notifications
    AYLA_WIFI_LVL_NTF_NOT_SUP = 0,
    /// FFA1 Level Characteristic support notifications
    AYLA_WIFI_LVL_NTF_SUP = 1 << 0,
    AYLA_WIFI_LVL_IND_SUP = 1 << 1,
};
/*
 * APIs Structures
 ****************************************************************************************
 */

/// Parameters for the database creation
struct ayla_wifi_db_cfg
{
    /// Number of FFA0 to add
	uint8_t ayla_wifi_nb;

	uint16_t cfg_flag;
	/// Features of each FFA0 instance
	uint16_t features;
};

/// Parameters of the @ref  message
struct ayla_wifi_enable_req
{
    /// connection index
    uint8_t  conidx;
    ///  Configuration
    uint16_t  ind_cfg;
    
  	uint16_t  ntf_cfg;
    
};


/// Parameters of the @ref  message
struct ayla_wifi_enable_rsp
{
    /// connection index
    uint8_t conidx;
    ///status
    uint8_t status;
};

struct ayla_wifi_statu_ntf_upd_req
{
    ///  instance
    uint8_t conidx;

    uint16_t length;

    uint16_t seq_num;
    /// ffe1 Level
    uint8_t value[__ARRAY_EMPTY];
};

struct ayla_wifi_scre_ntf_upd_req
{
    ///  instance
    uint8_t conidx;

    uint16_t length;

    uint16_t seq_num;
    /// ffe1 Level
    uint8_t value[__ARRAY_EMPTY];
};

struct ayla_wifi_conn_write_ind
{   
    uint8_t conidx; /// Connection index
    uint8_t length;
    uint8_t value[__ARRAY_EMPTY];
};

struct ayla_wifi_wps_write_ind
{   
    uint8_t conidx; /// Connection index
    uint8_t length;
    uint8_t value[__ARRAY_EMPTY];
};

struct ayla_wifi_scan_write_ind
{   
    uint8_t conidx; /// Connection index
    uint8_t length;
    uint8_t value[__ARRAY_EMPTY];
};

struct ayla_wifi_scre_write_ind
{   
    uint8_t conidx; /// Connection index
    uint8_t length;
    uint8_t value[__ARRAY_EMPTY];
};

struct ayla_wifi_statu_value_ntf_cfg_ind
{
    /// connection index
    uint8_t  conidx;
    uint16_t handle;
    ///Notification Configuration
    uint16_t  ntf_cfg;
};

struct ayla_wifi_scre_value_ntf_cfg_ind
{
    /// connection index
    uint8_t  conidx;
    uint16_t handle;
    ///Notification Configuration
    uint16_t  ntf_cfg;
};

/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */

extern const struct kernel_state_handler ayla_wifi_default_handler;
#endif // BLE_AYLA_WIFI_SERVER


#endif /* _AYLA_WIFI_TASK_H_ */

