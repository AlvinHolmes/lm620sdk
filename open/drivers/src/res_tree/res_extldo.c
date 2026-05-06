#include <res_tree.h>
#include <os_list.h>
#include <drv_pin.h>
#include "res_tree_core.h"
#include <stdlib.h>
#if defined(OS_USING_PM)
#include <psm_wakelock.h>
#endif
/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
static void EXTLDO_Enable(EXT_LDO_VAL ldo, bool en);
static void EXTLDO_Enable_1V8(bool enable);
static void EXTLDO_Enable_2V8(bool enable);
static void EXTLDO_Enable_3V3(bool enable);

/************************************************************************************
 *                                 全局变量
 ************************************************************************************/
static EXTLDO_PinRes_t g_pinRes = {0};

#if defined(OS_USING_PM)
static WakeLock g_extLdoWakelock = {0};
static uint8_t g_lockRefs = 0;
#endif

// ldo叶子集合
static ResTreeLeaf_t g_extLdoLeafs[EXT_LDO_NUM] = {
#if RES_TREE_NAME_LEN
    {.func = EXTLDO_Enable_1V8, .refs = 0, .name = "ldo_1v8"},
    {.func = EXTLDO_Enable_2V8, .refs = 0, .name = "ldo_2v8"},
    {.func = EXTLDO_Enable_3V3, .refs = 0, .name = "ldo_3v3"},
#else
    {.func = EXTLDO_Enable_1V8, .refs = 0},
    {.func = EXTLDO_Enable_2V8, .refs = 0},
    {.func = EXTLDO_Enable_3V3, .refs = 0},
#endif
};

static ResTree_BodyLeaf g_ldoLeafs = {
#if RES_TREE_NAME_LEN
    .name = "LDO_BODY",
#endif
    .type = RES_TREE_TYPE_LEAF,
    .leafs = g_extLdoLeafs,
    .depth = EXT_LDO_NUM,
};

// ldo节点
static ResTree_t g_extLdoTree = {
    .parent = NULL,
    .child = NULL,
    .body = (void *)&g_ldoLeafs,
#if RES_TREE_NAME_LEN
    .name = "LDO_TOP"
#endif
};

static void EXTLDO_Enable_1V8(bool enable)
{
    EXTLDO_Enable(EXT_LDO_1V8, enable);
    RES_TREE_DEBUG("ldo 1v8 enable[%d]", enable);
}

static void EXTLDO_Enable_2V8(bool enable)
{
    EXTLDO_Enable(EXT_LDO_2V8, enable);
    RES_TREE_DEBUG("ldo 2v8 enable[%d]", enable);
}

static void EXTLDO_Enable_3V3(bool enable)
{
    EXTLDO_Enable(EXT_LDO_3V3, enable);
    RES_TREE_DEBUG("ldo 3v3 enable[%d]", enable);
}

static void EXTLDO_Enable(EXT_LDO_VAL ldo, bool en)
{
    GPIO_LEVEL level = en ? GPIO_HIGH : GPIO_LOW;

    PIN_MultiMux_t *multiRes = NULL;

    switch (ldo) {
        case EXT_LDO_1V8:
            multiRes = g_pinRes.ldo_1v8;
            break;

        case EXT_LDO_2V8:
            multiRes = g_pinRes.ldo_2v8;
            break;

        case EXT_LDO_3V3:
            multiRes = g_pinRes.ldo_3v3;
            break;
        
        default:
            return;
    }

    if(!multiRes) {
        return;
    }

    GPIO_Write(multiRes->res, level);
    PIN_SetMux(multiRes->res, multiRes->gpioMux);
    GPIO_SetDir(multiRes->res, GPIO_OUTPUT);

#if defined(OS_USING_PM)
    if(en) {
        if(g_lockRefs == 0) {
            PSM_WakeLock(&g_extLdoWakelock);
            RES_TREE_DEBUG("ldo psm lock");
        }
        g_lockRefs++;
    }
    else {
        g_lockRefs--;
        if(g_lockRefs == 0) {
            RES_TREE_DEBUG("ldo psm unlock");
            PSM_WakeUnlock(&g_extLdoWakelock);
        }
    }
#endif
}

void EXTLDO_PinRegister(PIN_MultiMux_t *ldo_1v8, PIN_MultiMux_t *ldo_2v8, PIN_MultiMux_t *ldo_3v3)
{
    g_pinRes.ldo_1v8 = ldo_1v8;
    g_pinRes.ldo_2v8 = ldo_2v8;
    g_pinRes.ldo_3v3 = ldo_3v3;

#if defined(OS_USING_PM)
    PSM_WakelockInit(&g_extLdoWakelock, PSM_DEEP_SLEEP);
#endif
}

void EXTLDO_Init(void)
{
    // 加入节点
}

void EXTLDO_Acquire(EXT_LDO_VAL val)
{
    ResTree_LeafAcquire(&g_extLdoTree, &g_extLdoLeafs[val]);
}

void EXTLDO_Release(EXT_LDO_VAL val)
{
    ResTree_LeafRelease(&g_extLdoTree, &g_extLdoLeafs[val]);
}


#if 1

#include <nr_micro_shell.h>

static void test_ldo_tree(char argc, char **argv)
{
    if(argc != 3) {
        return;
    }
    uint8_t ldo = (uint8_t)strtoul(argv[1], NULL, 0);
    uint8_t en = (uint8_t)strtoul(argv[2], NULL, 0);

    if(ldo >= EXT_LDO_NUM) {
        return;
    }
    if(en != 0 && en != 1) {
        return;
    }

    if(en) {
        EXTLDO_Acquire(ldo);
    }
    else {
        EXTLDO_Release(ldo);
    }
}
NR_SHELL_CMD_EXPORT(ldo, test_ldo_tree);




#endif






