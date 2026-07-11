#include "watchdog.h"
#include "ti_msp_dl_config.h"

void System_Watchdog_Init(void)
{
#ifdef WWDT0_INST
    DL_WWDT_setActiveWindow(WWDT0_INST, DL_WWDT_WINDOW0);
    DL_WWDT_setCoreHaltBehavior(WWDT0_INST, DL_WWDT_CORE_HALT_STOP);
#endif
}

void System_Watchdog_Feed(void)
{
#ifdef WWDT0_INST
    DL_WWDT_restart(WWDT0_INST);
#endif
}
