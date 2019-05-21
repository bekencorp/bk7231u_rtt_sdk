/**
 ****************************************************************************************
 *
 * @file ayla_conn_task.h
 *
 * @brief Header file - AYLA_CONN Service Server Role Task.
 *
 * Copyright (C) RivieraWaves 2009-2018
 *
 *
 ****************************************************************************************
 */


#ifndef _AYLA_CONN_TASK_H_
#define _AYLA_CONN_TASK_H_


#include "rwprf_config.h"
#if (BLE_AYLA_CONN_SERVER)
#include <stdint.h>
#include "rwip_task.h" // Task definitions
#include "ble_compiler.h"
/*
 * DEFINES
 ****************************************************************************************
 */

///Maximum number of AYLA_CONN Server task instances
#define AYLA_CONN_IDX_MAX     0x01
///Maximal number of AYLA_CONN that can be added in the DB

#define  AYLA_CONN_CHAR_DATA_LEN  128

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Possible states of the AYLA_CONN task
enum ayla_conn_state
{
    /// Idle state
    AYLA_CONN_IDLE,
    /// busy state
    AYLA_CONN_BUSY,
    /// Number of defined states.
    AYLA_CONN_STATE_MAX
};

/// Messages for FFA0 Server
enum ayla_conn_msg_id
{
	/// Start the FFA0 Server - at connection used to restore bond data
	AYLA_CONN_ENABLE_REQ  =  TASK_FIRST_MSG(TASK_ID_AYLA_CONN),

	/// Confirmation of the AYLA_CONN Server start
	AYLA_CONN_ENABLE_RSP,

    AYLA_CONN_SETO_WRITE_IND,

	AYLA_CONN_GATTC_CMP_EVT
};

/// Features Flag Masks
enum ayla_conn_features
{
    /// FFA1 Level Characteristic doesn't support notifications
    AYLA_CONN_LVL_NTF_NOT_SUP = 0,
    /// FFA1 Level Characteristic support notifications
    AYLA_CONN_LVL_NTF_SUP = 1 << 0,
    AYLA_CONN_LVL_IND_SUP = 1 << 1,
};
/*
 * APIs Structures
 ****************************************************************************************
 */

/// Parameters for the database creation
struct ayla_conn_db_cfg
{
    /// Number of FFA0 to add
	uint8_t ayla_conn_nb;

	uint16_t cfg_flag;
	/// Features of each FFA0 instance
	uint16_t features;
};

/// Parameters of the @ref  message
struct ayla_conn_enable_req
{
    /// connection index
    uint8_t  conidx;
    ///  Configuration
    uint16_t  ind_cfg;
    
  	uint16_t  ntf_cfg;
    
};


/// Parameters of the @ref  message
struct ayla_conn_enable_rsp
{
    /// connection index
    uint8_t conidx;
    ///status
    uint8_t status;
};

struct ayla_conn_seto_write_ind
{   
    uint8_t conidx; /// Connection index
    uint8_t length;
    uint8_t value[__ARRAY_EMPTY];
};

/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */

extern const struct kernel_state_handler ayla_conn_default_handler;
#endif // BLE_AYLA_CONN_SERVER


#endif /* _AYLA_CONN_TASK_H_ */

