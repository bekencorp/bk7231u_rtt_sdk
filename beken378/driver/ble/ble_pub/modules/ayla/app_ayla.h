/**
 ****************************************************************************************
 *
 * @file app_ayla.h
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
#ifndef APP_AYLA_H_
#define APP_AYLA_H_
/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief AYLA Application Module entry point
 *
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_APP_AYLA)
#include <stdint.h>          // Standard Integer Definition
#include "ble_pub.h"
#include "ayla_pub.h"
#include "kernel_task.h"         // Kernel Task Definition

/*
 * STRUCTURES DEFINITION
 ****************************************************************************************
 */

///  Application Module Environment Structure
struct app_ayla_env_tag
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
extern struct app_ayla_env_tag app_ayla_env;

/// Table of message handlers
extern const struct kernel_state_handler app_ayla_table_handler;
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
void app_ayla_init(void);
/**
 ****************************************************************************************
 * @brief Add a Service instance in the DB
 ****************************************************************************************
 */
void app_ayla_add_ayla(void);
/**
 ****************************************************************************************
 * @brief Enable the  Service
 ****************************************************************************************
 */
void app_ayla_enable_prf(uint8_t conidx);
/**
 ****************************************************************************************
 * @brief Send a Battery level value
 ****************************************************************************************
 */
void app_ayla_ffa1_val(uint8_t len,uint8_t *buf,uint16_t seq_num);

#endif //(BLE_APP_AYLA)

#endif // APP_AYLA_H_
