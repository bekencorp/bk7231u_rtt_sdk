/**
 ****************************************************************************************
 *
 * @file ayla_task.h
 *
 * @brief Header file - AYLA Service Server Role Task.
 *
 * Copyright (C) RivieraWaves 2009-2018
 *
 *
 ****************************************************************************************
 */


#ifndef _AYLA_TASK_H_
#define _AYLA_TASK_H_


#include "rwprf_config.h"
#if (BLE_AYLA_SERVER)
#include <stdint.h>
#include "rwip_task.h" // Task definitions
#include "ble_compiler.h"
/*
 * DEFINES
 ****************************************************************************************
 */

///Maximum number of AYLA Server task instances
#define AYLA_IDX_MAX     0x01
///Maximal number of AYLA that can be added in the DB

#define  AYLA_CHAR_DATA_LEN  128

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Possible states of the AYLA task
enum ayla_state
{
    /// Idle state
    AYLA_IDLE,
    /// busy state
    AYLA_BUSY,
    /// Number of defined states.
    AYLA_STATE_MAX
};

/// Messages for FFA0 Server
enum ayla_msg_id
{
	/// Start the FFA0 Server - at connection used to restore bond data
	AYLA_ENABLE_REQ  =  TASK_FIRST_MSG(TASK_ID_AYLA),

	/// Confirmation of the AYLA Server start
	AYLA_ENABLE_RSP,

	AYLA_FFA1_WRITE_IND,
	///  Level Value Update Request
	AYLA_FFA1_VALUE_UPD_REQ,
	/// Inform APP if  Level value has been notified or not
	AYLA_FFA1_VALUE_UPD_RSP,
	/// Inform APP that  send l Notification Configuration has been changed - use to update bond data
	AYLA_FFA1_VALUE_NTF_CFG_IND,

	AYLA_GATTC_CMP_EVT
};

/// Features Flag Masks
enum ayla_features
{
    /// FFA1 Level Characteristic doesn't support notifications
    AYLA_FFA1_LVL_NTF_NOT_SUP,
    /// FFA1 Level Characteristic support notifications
    AYLA_FFA1_LVL_NTF_SUP = 1,
};
/*
 * APIs Structures
 ****************************************************************************************
 */

/// Parameters for the database creation
struct ayla_db_cfg
{
    /// Number of FFA0 to add
	uint8_t ayla_nb;

	uint16_t cfg_flag;
	/// Features of each FFA0 instance
	uint16_t features;
};

/// Parameters of the @ref  message
struct ayla_enable_req
{
    /// connection index
    uint8_t  conidx;
    ///  Configuration
    uint16_t  ind_cfg;
    
};


/// Parameters of the @ref  message
struct ayla_enable_rsp
{
    /// connection index
    uint8_t conidx;
    ///status
    uint8_t status;
};

///Parameters of the @ref  message
struct ayla_ffa1_value_upd_req
{
    /// BAS instance
    uint8_t conidx;
	
	uint8_t length;
	
	uint16_t seq_num;
    ///  Level
    uint8_t value[__ARRAY_EMPTY];
};

///Parameters of the @ref  message
struct ayla_ffa1_value_upd_rsp
{
	 uint8_t conidx;
    ///status
    uint8_t status;
};

///Parameters of the @ref  message
struct ayla_ffa1_value_ntf_cfg_ind
{
    /// connection index
    uint8_t  conidx;
    ///Notification Configuration
    uint16_t  ntf_cfg;
};


/// Parameters of the @ref AYLAS_FFA1_WRITER_REQ_IND message
struct ayla_ffa1_write_ind
{
   
    uint8_t conidx; /// Connection index
		uint8_t length;
	  uint8_t value[__ARRAY_EMPTY];
};






/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */

extern const struct kernel_state_handler ayla_default_handler;
#endif // BLE_AYLA_SERVER


#endif /* _AYLA_TASK_H_ */

