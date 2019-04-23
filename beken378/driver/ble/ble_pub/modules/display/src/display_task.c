/**
 ****************************************************************************************
 *
 * @file display_task.c
 *
 * @brief This file contains definitions related to the Display module
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup DISPLAY
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (DISPLAY_SUPPORT)

#include "display.h"
#include "display_env.h"
#include "display_task.h"
#include "kernel_msg.h"          // kernel message defines
#include "kernel_task.h"         // kernel task defines
#if DISPLAY_PERIODIC
#include "kernel_timer.h"        // kernel timer defines
#endif // DISPLAY_PERIODIC


/*
 * DEFINES
 ****************************************************************************************
 */

/// Maximum number of instances of the DISPLAY task
#define DISPLAY_IDX_MAX 1

#if DISPLAY_PERIODIC
/// Display Screen Refreshment period (in 10ms unit)
#define DISPLAY_SCREEN_REFRESH_PERIOD      120
#endif // DISPLAY_PERIODIC



/*
 * MESSAGE HANDLERS DECLARATION
 ****************************************************************************************
 */

#if DISPLAY_PERIODIC
KERNEL_MSG_HANDLER(display_screen_to, void);
#endif // DISPLAY_PERIODIC


/*
 * TASK DESCRIPTOR DEFINITIONS
 ****************************************************************************************
 */

/// Specifies the message handlers that are common to all states.
static const struct kernel_msg_handler display_default_state[] =
{
    #if DISPLAY_PERIODIC
    {DISPLAY_SCREEN_TO, (kernel_msg_func_t)display_screen_to_handler},
    #endif // DISPLAY_PERIODIC

};

/// Specifies the message handlers that are common to all states.
static const struct kernel_state_handler display_default_handler = KERNEL_STATE_HANDLER(display_default_state);

/// Defines the placeholder for the states of all the task instances.
static kernel_state_t display_state[DISPLAY_IDX_MAX];


/// DISPLAY task descriptor
const struct kernel_task_desc TASK_DESC_DISPLAY = {NULL, &display_default_handler, display_state, DISPLAY_STATE_MAX, DISPLAY_IDX_MAX};


/*
 * MESSAGE HANDLERS DEFINITION
 ****************************************************************************************
 */


#if DISPLAY_PERIODIC
/**
 ****************************************************************************************
 * @brief Handles the timer for display periodic mode
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int display_screen_to_handler(kernel_msg_id_t const msgid,
                        void const *param,
                        kernel_task_id_t const dest_id,
                        kernel_task_id_t const src_id)
{

    switch(display_env.disp_mode)
    {
        case DISPLAY_MODE_PERIODIC_ON:
        {
            // Display the next screen
            display_switch(DISPLAY_DIR_DOWN);

            // Program the next Screen switch
            kernel_timer_set(DISPLAY_SCREEN_TO, TASK_DISPLAY, DISPLAY_SCREEN_REFRESH_PERIOD);
        }
        break;

        default:
        {
            // Should not happen
        }
        break;
    }

    return (KERNEL_MSG_CONSUMED);
}
#endif // DISPLAY_PERIODIC


#endif //DISPLAY_SUPPORT

/// @} DISPLAY
