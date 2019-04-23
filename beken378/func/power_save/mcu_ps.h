#ifndef _MCU_PS_H_
#define _MCU_PS_H_

//#define MCU_PS_DEBUG

#ifdef MCU_PS_DEBUG
#define MCU_PS_PRT                 os_printf

#else
#define MCU_PS_PRT                 os_null_printf

#endif

typedef struct
{
    UINT32 first_tick;
    UINT32 prev_tick;
    UINT64 first_tsf;
} MCU_PS_TSF;
typedef struct
{
    UINT32 fclk_tick;
    UINT32 machw_tm;
} MCU_PS_MACHW_TM;

#endif

