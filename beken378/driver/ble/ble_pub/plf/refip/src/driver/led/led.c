/**
 ****************************************************************************************
 *
 * @file led.c
 *
 * @brief LED initialisation and behaviour functions.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup LED
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>          // standard definition
#include "ble_compiler.h"        // for inline functions
#include "led.h"             // led definitions

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void led_init(void)
{
    led_reset(1);
    led_reset(2);
    led_reset(3);
    led_reset(4);
    led_reset(5);
    led_reset(6);
    led_reset(7);
}

void led_set_all(uint32_t value)
{
}

/// @} LED
