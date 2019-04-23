/**
 ****************************************************************************************
 *
 * @file diss_task.c
 *
 * @brief Device Information Service Task implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup DISSTASK
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_DIS_SERVER)
#include "gap.h"
#include "gattc_task.h"
#include "diss.h"
#include "diss_task.h"
#include "prf_utils.h"

#include "kernel_mem.h"

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref DISS_SET_VALUE_REQ message.
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int diss_set_value_req_handler(kernel_msg_id_t const msgid,
                                      struct diss_set_value_req const *param,
                                      kernel_task_id_t const dest_id,
                                      kernel_task_id_t const src_id)
{
    // Request status
    uint8_t status;
    // Characteristic Declaration attribute handle
    uint16_t handle;

    struct diss_env_tag* diss_env = PRF_ENV_GET(DISS, diss);
    struct diss_set_value_rsp* rsp;
    // Check Characteristic Code
    if (param->value < DIS_CHAR_MAX)
    {
        // Get Characteristic Declaration attribute handle
        handle = diss_value_to_handle(diss_env, param->value);

        // Check if the Characteristic exists in the database
        if (handle != ATT_INVALID_HDL)
        {
            // Check the value length
            status = diss_check_val_len(param->value, param->length);

            if (status == GAP_ERR_NO_ERROR)
            {
                // Check value in already present in service
                struct diss_val_elmt *val = (struct diss_val_elmt *) common_list_pick(&(diss_env->values));
                // loop until value found
                while(val != NULL)
                {
                    // if value already present, remove old one
                    if(val->value == param->value)
                    {
                        common_list_extract(&(diss_env->values), &(val->hdr), 0);
                        kernel_free(val);
                        break;
                    }
                    val = (struct diss_val_elmt *)val->hdr.next;
                }

                // allocate value data
                val = (struct diss_val_elmt *) kernel_malloc(sizeof(struct diss_val_elmt) + param->length, KERNEL_MEM_ATT_DB);
                val->value = param->value;
                val->length = param->length;
                memcpy(val->data, param->data, param->length);
                // insert value into the list
                common_list_push_back(&(diss_env->values), &(val->hdr));
            }
        }
        else
        {
            status = PRF_ERR_INEXISTENT_HDL;
        }
    }
    else
    {
        status = PRF_ERR_INVALID_PARAM;
    }

    // send response to application
    rsp = KERNEL_MSG_ALLOC(DISS_SET_VALUE_RSP, src_id, dest_id, diss_set_value_rsp);
    rsp->value = param->value;
    rsp->status = status;
    kernel_msg_send(rsp);


    return (KERNEL_MSG_CONSUMED);
}



/**
 ****************************************************************************************
 * @brief Handles reception of the read request from peer device
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_read_req_ind_handler(kernel_msg_id_t const msgid,
                                      struct gattc_read_req_ind const *param,
                                      kernel_task_id_t const dest_id,
                                      kernel_task_id_t const src_id)
{
    int msg_status = KERNEL_MSG_CONSUMED;
    kernel_state_t state = kernel_state_get(dest_id);

    if(state == DISS_IDLE)
    {
        struct diss_env_tag* diss_env = PRF_ENV_GET(DISS, diss);
        // retrieve value attribute
        uint8_t value = diss_handle_to_value(diss_env, param->handle);

        // Check Characteristic Code
        if (value < DIS_CHAR_MAX)
        {
            // Check value in already present in service
            struct diss_val_elmt *val = (struct diss_val_elmt *) common_list_pick(&(diss_env->values));
            // loop until value found
            while(val != NULL)
            {
                // value is present in service
                if(val->value == value)
                {
                    break;
                }
                val = (struct diss_val_elmt *)val->hdr.next;
            }

            if(val != NULL)
            {
                // Send value to peer device.
                struct gattc_read_cfm* cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, val->length);
                cfm->handle = param->handle;
                cfm->status = ATT_ERR_NO_ERROR;
                cfm->length = val->length;
                memcpy(cfm->value, val->data, val->length);
                kernel_msg_send(cfm);
            }
            else
            {
                // request value to application
                diss_env->req_val    = value;
                diss_env->req_conidx = KERNEL_IDX_GET(src_id);

                struct diss_value_req_ind* req_ind = KERNEL_MSG_ALLOC(DISS_VALUE_REQ_IND,
                        prf_dst_task_get(&(diss_env->prf_env), KERNEL_IDX_GET(src_id)),
                        dest_id, diss_value_req_ind);
                req_ind->value = value;
                kernel_msg_send(req_ind);

                // Put Service in a busy state
                kernel_state_set(dest_id, DISS_BUSY);
            }
        }
        else
        {
            // application error, value cannot be retrieved
            struct gattc_read_cfm* cfm = KERNEL_MSG_ALLOC(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm);
            cfm->handle = param->handle;
            cfm->status = ATT_ERR_APP_ERROR;
            kernel_msg_send(cfm);
        }
    }
    // postpone request if profile is in a busy state - required for multipoint
    else if(state == DISS_BUSY)
    {
        msg_status = KERNEL_MSG_SAVED;
    }

    return (msg_status);
}


/**
 ****************************************************************************************
 * @brief Handles reception of the value confirmation from application
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int diss_value_cfm_handler(kernel_msg_id_t const msgid,
                                      struct diss_value_cfm const *param,
                                      kernel_task_id_t const dest_id,
                                      kernel_task_id_t const src_id)
{
    kernel_state_t state = kernel_state_get(dest_id);

    if(state == DISS_BUSY)
    {
        struct diss_env_tag* diss_env = PRF_ENV_GET(DISS, diss);
        // retrieve value attribute
        uint16_t handle = diss_value_to_handle(diss_env, diss_env->req_val);

        // chack if application provide correct value
        if (diss_env->req_val == param->value)
        {
            // Send value to peer device.
            struct gattc_read_cfm* cfm = KERNEL_MSG_ALLOC_DYN(GATTC_READ_CFM, KERNEL_BUILD_ID(TASK_GATTC, diss_env->req_conidx), dest_id, gattc_read_cfm, param->length);
            cfm->handle = handle;
            cfm->status = ATT_ERR_NO_ERROR;
            cfm->length = param->length;
            memcpy(cfm->value, param->data, param->length);
            kernel_msg_send(cfm);
        }
        else
        {
            // application error, value provided by application is not the expected one
            struct gattc_read_cfm* cfm = KERNEL_MSG_ALLOC(GATTC_READ_CFM, KERNEL_BUILD_ID(TASK_GATTC, diss_env->req_conidx), dest_id, gattc_read_cfm);
            cfm->handle = handle;
            cfm->status = ATT_ERR_APP_ERROR;
            kernel_msg_send(cfm);
        }

        // return to idle state
        kernel_state_set(dest_id, DISS_IDLE);
    }
    // else ignore request if not in busy state

    return (KERNEL_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
const struct kernel_msg_handler diss_default_state[] =
{
    {DISS_SET_VALUE_REQ,      (kernel_msg_func_t)diss_set_value_req_handler},
    {GATTC_READ_REQ_IND,      (kernel_msg_func_t)gattc_read_req_ind_handler},
    {DISS_VALUE_CFM,          (kernel_msg_func_t)diss_value_cfm_handler},
};

///Specifies the message handlers that are common to all states.
const struct kernel_state_handler diss_default_handler = KERNEL_STATE_HANDLER(diss_default_state);

#endif //BLE_DIS_SERVER

/// @} DISSTASK
