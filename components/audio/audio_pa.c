#include "os.h"
#include "audio_pa.h"
#include "drv_pin.h"

#ifdef AUDIO_PA_AW87390
#include "aw87390.h"
#endif

static AudioPaCfg* g_PaCfg = NULL;

void PA_SetCfg(AudioPaCfg* cfg)
{
    g_PaCfg = cfg;
}

void PA_Enable(void)
{
#ifdef AUDIO_PA_AW87390
    OS_ASSERT(g_PaCfg != NULL);
    aw87390_init(g_PaCfg->busNo);
#else
    if(g_PaCfg != NULL && g_PaCfg->res)
    {
        PIN_SetMux(g_PaCfg->res, 0);
        GPIO_SetDir(g_PaCfg->res, GPIO_OUTPUT);
        GPIO_Write(g_PaCfg->res, g_PaCfg->activeLevel);
    }
#endif
}

void PA_Disable(void)
{
#ifdef AUDIO_PA_AW87390
    OS_ASSERT(g_PaCfg != NULL);
    aw87390_deinit(g_PaCfg->busNo);
#else
    if(g_PaCfg != NULL && g_PaCfg->res)
    {
        PIN_SetMux(g_PaCfg->res, 0);
        GPIO_SetDir(g_PaCfg->res, GPIO_OUTPUT);
        GPIO_Write(g_PaCfg->res, (g_PaCfg->activeLevel == GPIO_HIGH)? GPIO_LOW : GPIO_HIGH);
    }
#endif
}
