#ifndef APP_STATE_H
#define APP_STATE_H

#include "app_main.h"
#include <stdint.h>

void App_State_Init(void);
void App_State_Set(AppState_t state);
void App_State_Tick(uint32_t now_ms);

#endif /* APP_STATE_H */
