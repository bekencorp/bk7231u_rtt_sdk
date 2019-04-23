#include <stdint.h>        // standard integer definition
#include <string.h>        // string manipulation
#include <stddef.h>        // standard definition
#include "BK3435_reg.h"
#include "wdt.h"


//每个单位250us，最大0xffff，约16s
#define WDT_COUNT           0x00001000UL



void wdt_feed(void)
{
    // Write WDT key: 0x5A firstly and 0xA5 secondly.
    REG_APB0_WDT_CFG = ((WDKEY_ENABLE1 << WDT_CONFIG_WDKEY_POSI)
                    | (WDT_COUNT     << WDT_CONFIG_PERIOD_POSI));
    REG_APB0_WDT_CFG = ((WDKEY_ENABLE2 << WDT_CONFIG_WDKEY_POSI)
                    | (WDT_COUNT     << WDT_CONFIG_PERIOD_POSI));
}

void  wdt_init(void)
{
	//close JTAG mode
    CLOSE_JTAG_MODE();                 

    ICU_WDT_CLK_PWD_CLEAR();

    // Write WDT key: 0x5A firstly and 0xA5 secondly.
    wdt_feed();
}


void  wdt_enable(void)
{
     wdt_feed();
}

void  wdt_disable(void)
{
    // Write WDT key: 0xDE firstly and 0xDA secondly.
    REG_APB0_WDT_CFG = (WDKEY_DISABLE1 << WDT_CONFIG_WDKEY_POSI);
    REG_APB0_WDT_CFG = (WDKEY_DISABLE2 << WDT_CONFIG_WDKEY_POSI);
}


