/**
 ****************************************************************************************
 *
 * @file timer.c
 *
 * @brief TIMER driver
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup TIMER
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "timer.h"        // timer definition

#include "kernel_event.h"
#include "reg_timer.h"

/*
 * MACRO DEFINITIONS
 ****************************************************************************************
 */

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
void timer_init(void)
{
}



void timer_set_timeout(uint32_t to)
{
    tmr_gross_setf(to);
}

uint32_t timer_get_time(void)
{
    return tmr_current_time_get();
}

void timer_enable(bool enable)
{
    tmr_enable_setf(enable);
}


/**
 * used to check if timeout occurs.
 */
void timer_isr(void)
{
    if(tmr_expire_getf())
    {
        // Set kernel event for deferred handling
        kernel_event_set(KERNEL_EVENT_KERNEL_TIMER);
    }
}

/// @} TIMER
