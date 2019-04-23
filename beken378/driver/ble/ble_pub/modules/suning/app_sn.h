/**
 ****************************************************************************************
 *
 * @file app_sn.c
 *
 * @brief findt Application Module entry point
 *
 * @auth  gang.cheng
 *
 * @date  2018.09.17
 *
 * Copyright (C) Beken 2009-2018
 *
 *
 ****************************************************************************************
 */
#ifndef APP_SN_H_
#define APP_SN_H_
/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief SN Application Module entry point
 *
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_APP_SN)
#include <stdint.h>          // Standard Integer Definition
#include "ble_pub.h"
#include "sn_pub.h"
#include "kernel_task.h"         // Kernel Task Definition

/*
 * STRUCTURES DEFINITION
 ****************************************************************************************
 */

///  Application Module Environment Structure
struct app_sn_env_tag
{
    /// Connection handle
    uint8_t conidx;
    /// Current Ba
    uint16_t send_ntf_cfg;
    uint8_t allow_send_flag;

};
/*
 * GLOBAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */

///  Application environment
extern struct app_sn_env_tag app_sn_env;

/// Table of message handlers
extern const struct kernel_state_handler app_sn_table_handler;
/*
 * FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 *
 *  Application Functions
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize  Application Module
 ****************************************************************************************
 */
void app_sn_init(void);
/**
 ****************************************************************************************
 * @brief Add a Service instance in the DB
 ****************************************************************************************
 */
void app_sn_add_sn(void);
/**
 ****************************************************************************************
 * @brief Enable the  Service
 ****************************************************************************************
 */
void app_sn_enable_prf(uint8_t conidx);
/**
 ****************************************************************************************
 * @brief Send a Battery level value
 ****************************************************************************************
 */
void app_sn_5301_val(uint8_t len,uint8_t *buf,uint16_t seq_num);

#endif //(BLE_APP_SN)

#endif // APP_SN_H_
