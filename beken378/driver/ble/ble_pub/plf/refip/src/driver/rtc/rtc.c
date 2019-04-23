/**
****************************************************************************************
* @addtogroup RTC
* @ingroup beken corp
* @brief RTC
* @author Alen
*
* This is the driver block for RTC
* @{
****************************************************************************************
*/


#include <stdint.h>        // standard integer definition
#include <string.h>        // string manipulation
#include <stddef.h>        // standard definition
#include "BK3435_reg.h"
#include "rtc.h"


static void (*p_RTC_Int_Handler)(void) = NULL;



void rtc_init(RTC_DATE_DESC *p_RTC_date_desc)
{
    if (p_RTC_date_desc == NULL)
    {
        return;
    }

    ICU_RTC_CLK_PWD_CLEAR();

    REG_APB6_RTC_CFG  = (1 << BIT_RTC_ENABLE)
                    | (0 << BIT_RTC_CLEAR)
                    | (0 << BIT_RTC_ALARM_EN)
                    | (0 << BIT_RTC_ALARM_MODE);

    REG_APB6_RTC_UNIT = (0 << BIT_RTC_DIV)
                 | (31    << BIT_RTC_MSUNIT)
                 | (31999 << BIT_RTC_SUNIT);

    REG_APB6_RTC_SET  = ((p_RTC_date_desc->second   & 0x3F) << BIT_RTC_SECOND)
                 | ((p_RTC_date_desc->minute   & 0x3F) << BIT_RTC_MINUTE)
                 | ((p_RTC_date_desc->hour     & 0x1F) << BIT_RTC_HOUR  )
                 | ((p_RTC_date_desc->week_day & 0x07) << BIT_RTC_WEEK  );

	while (!(REG_APB6_RTC_CFG & (1 << BIT_RTC_ENABLE)));

}


void rtc_enable(void)
{
    REG_APB6_RTC_CFG |= (1 << BIT_RTC_ENABLE);
}

void rtc_disable(void)
{
    REG_APB6_RTC_CFG &= (~(1 << BIT_RTC_ENABLE));
}


void rtc_alarm_init(unsigned char ucMode, RTC_DATE_DESC *p_RTC_alarm_time, 
                    unsigned long ulMiiliSecond, void (*p_Int_Handler)(void))
{
    ICU_RTC_CLK_PWD_CLEAR();

    if (ucMode == 0x00)         // clock alarm mode
    {
        if (p_RTC_alarm_time == NULL)
        {
            return;
        }
        REG_APB6_RTC_ALM_TIME = ((p_RTC_alarm_time->second   & 0x3F) << BIT_RTC_ALARM_SECOND)
                          | ((p_RTC_alarm_time->minute   & 0x3F) << BIT_RTC_ALARM_MINUTE)
                          | ((p_RTC_alarm_time->hour     & 0x1F) << BIT_RTC_ALARM_HOUR);
    }
    else if (ucMode == 0x01)    // millisecond alarm mode
    {
        REG_APB6_RTC_ALM_TIME = (ulMiiliSecond & 0x3FF) << BIT_RTC_ALARM_MILLISEC;
    }
    else
    {
        return;
    }

    p_RTC_Int_Handler = p_Int_Handler;


    REG_APB6_RTC_CFG = (REG_APB6_RTC_CFG & 0x03)
                     | (1 << BIT_RTC_ENABLE)
                     | (1 << BIT_RTC_ALARM_EN)
                     | ((ucMode & 0x01) << BIT_RTC_ALARM_MODE);

    REG_APB6_RTC_UNIT = (0 << BIT_RTC_DIV)
                 | (31    << BIT_RTC_MSUNIT)
                 | (31999 << BIT_RTC_SUNIT);

    if (p_Int_Handler != NULL)
    {
		REG_AHB0_ICU_INT_ENABLE |= (0x1 << 12);
    }
}

void rtc_alarm_enable(void)
{
    REG_APB6_RTC_CFG |= (1 << BIT_RTC_ALARM_EN);
}

void rtc_alarm_disable(void)
{
    REG_APB6_RTC_CFG &= (~(1 << BIT_RTC_ALARM_EN));
}


void rtc_set_time(RTC_DATE_DESC *p_RTC_date_desc)
{
    if (p_RTC_date_desc == NULL)
    {
        return;
    }

    REG_APB6_RTC_SET = ((p_RTC_date_desc->second   & 0x3F) << BIT_RTC_SECOND)
                | ((p_RTC_date_desc->minute   & 0x3F) << BIT_RTC_MINUTE)
                | ((p_RTC_date_desc->hour     & 0x1F) << BIT_RTC_HOUR)
                | ((p_RTC_date_desc->week_day & 0x07) << BIT_RTC_WEEK);
}

void rtc_get_time(RTC_DATE_DESC *p_RTC_date_desc)
{
    unsigned long ulTime;

    if (p_RTC_date_desc == NULL)
    {
        return;
    }
    ulTime = REG_APB6_RTC_SET;
    p_RTC_date_desc->second   = (ulTime >> BIT_RTC_SECOND) & 0x3F;
    p_RTC_date_desc->minute   = (ulTime >> BIT_RTC_MINUTE) & 0x3F;
    p_RTC_date_desc->hour     = (ulTime >> BIT_RTC_HOUR  ) & 0x1F;
    p_RTC_date_desc->week_day = (ulTime >> BIT_RTC_WEEK  ) & 0x07;
}

void rtc_int_handler_clear(void)
{
    p_RTC_Int_Handler = NULL;
}


void rtc_isr(void)
{
    if (REG_APB6_RTC_ALM_FLAG & (0x1 << 0))
    {
        if (p_RTC_Int_Handler != NULL)
        {
            (void)p_RTC_Int_Handler();
        }
    }

    REG_APB6_RTC_ALM_FLAG = 0x1 << 0;
}




