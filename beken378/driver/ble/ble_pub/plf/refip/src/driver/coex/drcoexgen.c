/**
****************************************************************************************
*
* @file drcoexgen.c
*
* @brief Coexistence event generator functions
*
* Copyright (C) RivieraWaves 2009-2015
*
*
****************************************************************************************
*/

/**
****************************************************************************************
* @addtogroup COEXGEN
* @{
****************************************************************************************
*/

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>               // standard integer definition
#include "common_bt.h"                // common bt definitions
#include "rwip_config.h"        // stack configuration
#include "common_buf.h"               // buffer functions
#include "common_error.h"             // definition of errors
#include "drcoexgenevtreg.h"      // register definitions
#include "drcoexgen.h"            // functions definitions
#include "dbg.h"                  // debug definition

/*
 * FUNCTION DECLARATION
 ****************************************************************************************
 */

void DR_WmGenStart(uint16_t EventPhase)
{
    WARNING(("Start WiMAX Event Generator with Phase: %d", EventPhase));

    DR_BtCoexGenEvtRegs[WMGENPERIOD] = 5000; /* 5ms as period */
    DR_BtCoexGenEvtRegs[WMGENDCYCLE] = 126;  /* 2,5% duty cycle */
    DR_BtCoexGenEvtRegs[WMGENCNTL] = (EventPhase << WMGENDELAY_SFT) | WMGENEN | WMGENSYNCEN;
}

void DR_WlGenStart(uint32_t Period, uint32_t ActivePhase)
{
    WARNING(("Start WLAN Generator with Period: %d, Active Phase: %d", Period
                                                                        , ActivePhase));
    DR_BtCoexGenEvtRegs[WLGENPERIOD] = Period;
    DR_BtCoexGenEvtRegs[WLGENDCYCLE] = ActivePhase;
    DR_BtCoexGenEvtRegs[WMGENCNTL] |= WLGENEN|WLGENMODE; /* No phase applied */
}

void DR_WmGenStop(void)
{
    WARNING(("Stop WiMAX Event Generator"));

    DR_BtCoexGenEvtRegs[WMGENCNTL] &= ~WMGENEN;
}

void DR_WlGenStop(void)
{
    WARNING(("Stop WLAN Event Generator"));
    DR_BtCoexGenEvtRegs[WMGENCNTL] &= ~(WLGENEN|WLGENMODE);
}

/// @} COEXGEN
