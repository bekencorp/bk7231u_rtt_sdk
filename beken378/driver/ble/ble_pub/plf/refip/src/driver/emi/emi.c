/**
 ****************************************************************************************
 *
 * @file emi.c
 *
 * @brief  Definition of the EMI initialization API.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup EMI
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>         // standard integer definition
#include <stddef.h>         // standard defines
#include <string.h>         // string manipulation
#include "boot.h"           // boot function
#include "emi.h"            // emi

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
typedef void (*VOID_FUNC_PTR_VOID)(void);


/*
 * DEFINES
 ****************************************************************************************
 */
/// Base address of the Flash
#define FLASH_BASE_ADDR         0x00000000

/// Base address of the RAM
#define RAM_BASE_ADDR           0x00400000

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void emi_init(void)
{
	unsigned long *p=(unsigned long *)0x814000;
	int i;
	for(i=0;i<1000;i++)
	{
		*p++ =  0;					
	}
}
/// @} EMI
