#ifndef APP_MAIN_H
#define APP_MAIN_H

#include <stdint.h>

typedef enum {
    APP_STATE_IDLE = 0,
    APP_STATE_RUN  = 1,
    APP_STATE_FAULT = 2,
} AppState_t;

void App_Init(void);
AppState_t App_GetState(void);

#endif /* APP_MAIN_H */
