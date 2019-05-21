/**
 ****************************************************************************************
 *
 * @file app_ayla_gen.h
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
#ifndef APP_AYLA_GEN_H_
#define APP_AYLA_GEN_H_
/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief AYLA_GEN Application Module entry point
 *
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_APP_AYLA_GEN)
#include <stdint.h>          // Standard Integer Definition
#include "ble_pub.h"
#include "ayla_gen_pub.h"
#include "kernel_task.h"         // Kernel Task Definition

/*
 * STRUCTURES DEFINITION
 ****************************************************************************************
 */

///  Application Module Environment Structure
struct app_ayla_gen_env_tag
{
    /// Connection handle
    uint8_t conidx;
};
/*
 * GLOBAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */

///  Application environment
extern struct app_ayla_gen_env_tag app_ayla_gen_env;

/// Table of message handlers
extern const struct kernel_state_handler app_ayla_gen_table_handler;
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
void app_ayla_gen_init(void);
/**
 ****************************************************************************************
 * @brief Add a Service instance in the DB
 ****************************************************************************************
 */
void app_ayla_gen_add_gen(void);
/**
 ****************************************************************************************
 * @brief Enable the  Service
 ****************************************************************************************
 */
void app_ayla_gen_enable_prf(uint8_t conidx);
/**
 ****************************************************************************************
 * @brief Send a Battery level value
 ****************************************************************************************
 */

#endif //(BLE_APP_AYLA_GEN)

#endif // APP_AYLA_GEN_H_
