#include "app_shared.h"

static AppContext_t s_app;

AppContext_t *App_Shared_Get(void)
{
    return &s_app;
}

void App_Shared_SetState(AppState_t state)
{
    s_app.state = state;
    s_app.state_id = (uint8_t)state;
}
