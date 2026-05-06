#ifndef __RES_TREE_H__
#define __RES_TREE_H__

#include <os.h>
#include <drv_common.h>
#include <stdint.h>
#include <drv_i2c.h>
#include <drv_soft_i2c.h>
#include <drv_pin.h>

/**
 * @addtogroup ResTree
 */

/**@{*/

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef enum {
    EXT_LDO_1V8 = 0,
    EXT_LDO_2V8,
    EXT_LDO_3V3,
    EXT_LDO_NUM
}EXT_LDO_VAL;

typedef struct {
    void        *res;
    uint8_t     mux;
}EXTLDO_Pin_t;

typedef struct {
    PIN_MultiMux_t *ldo_1v8;
    PIN_MultiMux_t *ldo_2v8;
    PIN_MultiMux_t *ldo_3v3;
}EXTLDO_PinRes_t;

void EXTLDO_PinRegister(PIN_MultiMux_t *ldo_1v8, PIN_MultiMux_t *ldo_2v8, PIN_MultiMux_t *ldo_3v3);
void EXTLDO_Acquire(EXT_LDO_VAL val);
void EXTLDO_Release(EXT_LDO_VAL val);


#endif
/** @} */