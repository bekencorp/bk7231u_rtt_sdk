/**
 ****************************************************************************************
 *
 * @file sn_task.h
 *
 * @brief Header file - SN Service Server Role Task.
 *
 * Copyright (C) RivieraWaves 2009-2018
 *
 *
 ****************************************************************************************
 */


#ifndef _SN_TASK_H_
#define _SN_TASK_H_


#include "rwprf_config.h"
#if (BLE_SN_SERVER)
#include <stdint.h>
#include "rwip_task.h" // Task definitions
#include "ble_compiler.h"
/*
 * DEFINES
 ****************************************************************************************
 */

///Maximum number of SN Server task instances
#define SN_IDX_MAX     0x01
///Maximal number of SN that can be added in the DB

#define  SN_CHAR_DATA_LEN  128

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Possible states of the MS task
enum sn_state
{
    /// Idle state
    SN_IDLE,
    /// busy state
    SN_BUSY,
    /// Number of defined states.
    SN_STATE_MAX
};

/// Messages for FFF0 Server
enum sn_msg_id
{
	/// Start the 5300 Server - at connection used to restore bond data
	SN_ENABLE_REQ  =  TASK_FIRST_MSG(TASK_ID_SN),

	/// Confirmation of the SN Server start
	SN_ENABLE_RSP,

	SN_5302_WRITE_IND,
	///  Level Value Update Request
	SN_5301_VALUE_UPD_REQ,
	/// Inform APP if  Level value has been notified or not
	SN_5301_VALUE_UPD_RSP,
	/// Inform APP that  send l Notification Configuration has been changed - use to update bond data
	SN_5301_VALUE_NTF_CFG_IND,

	SN_GATTC_CMP_EVT
};

/// Features Flag Masks
enum sn_features
{
    /// 5301 Level Characteristic doesn't support notifications
    SN_5301_LVL_NTF_NOT_SUP,
    /// 5301 Level Characteristic support notifications
    SN_5301_LVL_NTF_SUP = 1,
};
/*
 * APIs Structures
 ****************************************************************************************
 */

/// Parameters for the database creation
struct sn_db_cfg
{
    /// Number of 5300 to add
	uint8_t sn_nb;

	uint16_t cfg_flag;
	/// Features of each 5300 instance
	uint16_t features;
};

/// Parameters of the @ref  message
struct sn_enable_req
{
    /// connection index
    uint8_t  conidx;
    ///  Configuration
    uint16_t  ind_cfg;
    
};


/// Parameters of the @ref  message
struct sn_enable_rsp
{
    /// connection index
    uint8_t conidx;
    ///status
    uint8_t status;
};

///Parameters of the @ref  message
struct sn_5301_value_upd_req
{
    /// BAS instance
    uint8_t conidx;
	
	uint8_t length;
	
	uint16_t seq_num;
    ///  Level
    uint8_t value[__ARRAY_EMPTY];
};

///Parameters of the @ref  message
struct sn_5301_value_upd_rsp
{
	 uint8_t conidx;
    ///status
    uint8_t status;
};

///Parameters of the @ref  message
struct sn_5301_value_ntf_cfg_ind
{
    /// connection index
    uint8_t  conidx;
    ///Notification Configuration
    uint16_t  ntf_cfg;
};


/// Parameters of the @ref SNS_5302_WRITER_REQ_IND message
struct sn_5302_write_ind
{
   
    uint8_t conidx; /// Connection index
		uint8_t length;
	  uint8_t value[__ARRAY_EMPTY];
};






/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */

extern const struct kernel_state_handler sn_default_handler;
#endif // BLE_SN_SERVER


#endif /* _SN_TASK_H_ */

