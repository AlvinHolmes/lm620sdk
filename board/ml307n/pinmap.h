

#ifndef __DRV_PINMAP_H__
#define __DRV_PINMAP_H__

/************************************************************************************
 *                                 头文件定义
 ************************************************************************************/
#include "chip_pin_list.h"
#include "drv_pin.h"

/**
 * @addtogroup Pinmap
 */

/**@{*/

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define LPUART0_RX      PIN_RES(PIN_5)
#define LPUART0_TX      PIN_RES(PIN_6)
#define LPUART0_CTS     PIN_RES(PIN_7)
#define LPUART0_RTS     PIN_RES(PIN_8)
#define AON_GPIO_0      PIN_RES(PIN_9)
#define AON_GPIO_1      PIN_RES(PIN_10)
#define CLK_EXT_OUT     PIN_RES(PIN_11)
#define SWD0_SWCLK      PIN_RES(PIN_12)
#define SWD0_SWDIO      PIN_RES(PIN_13)
#define CLK_AU_MCLK     PIN_RES(PIN_14)
#define PD_GPIO_0       PIN_RES(PIN_21)
#define PD_GPIO_1       PIN_RES(PIN_22)
#define PD_GPIO_2       PIN_RES(PIN_23)
#define PD_GPIO_3       PIN_RES(PIN_24)
#define PD_GPIO_4       PIN_RES(PIN_25)
#define PD_GPIO_5       PIN_RES(PIN_26)
#define PD_GPIO_6       PIN_RES(PIN_27)
#define PD_GPIO_7       PIN_RES(PIN_28)
#define PD_GPIO_8       PIN_RES(PIN_29)
#define PD_GPIO_9       PIN_RES(PIN_30)
#define PD_GPIO_10      PIN_RES(PIN_31)
#define PD_GPIO_11      PIN_RES(PIN_32)
#define PD_GPIO_12      PIN_RES(PIN_33)
#define PD_GPIO_13      PIN_RES(PIN_34)
#define PD_GPIO_14      PIN_RES(PIN_35)
#define PD_GPIO_15      PIN_RES(PIN_36)
#define SIM0_RST        PIN_RES(PIN_37)
#define SIM0_CLK        PIN_RES(PIN_38)
#define SIM0_DATA       PIN_RES(PIN_39)
#define RF_CONTROL_0    PIN_RES(PIN_40)
#define RF_CONTROL_1    PIN_RES(PIN_41)
#define RF_CONTROL_2    PIN_RES(PIN_42)
#define RF_CONTROL_3    PIN_RES(PIN_43)
#define RF_CONTROL_4    PIN_RES(PIN_44)
#define RF_CONTROL_5    PIN_RES(PIN_45)
#define RF_CONTROL_6    PIN_RES(PIN_46)
#define RF_CONTROL_7    PIN_RES(PIN_47)
#define AON_GPIO_7      PIN_RES(PIN_48)
#define AON_GPIO_8      PIN_RES(PIN_49)
#define PD_GPIO_16      PIN_RES(PIN_50)
#define PD_GPIO_17      PIN_RES(PIN_51)
#define PD_GPIO_18      PIN_RES(PIN_52)
#define PD_GPIO_19      PIN_RES(PIN_53)

/**********功能标志位定义******** */
#define    PIN_CFG_DRV   (1 << 0)
#define    PIN_CFG_SEN   (1 << 1)
#define    PIN_CFG_PULL  (1 << 2)
#define    PIN_CFG_DIR   (1 << 3)
#define    PIN_CFG_RATE  (1 << 4)

typedef enum {
    SEN_DISABLE,
    SEN_ENABLE,
}PIN_SEN;

/***********核心结构体定义************ */
typedef struct {
    const struct PIN_Res*   pinNum;     /*引脚名称，与pinlist中ball_num对应*/
    uint8_t                 pinMux;     /*功能复用*/
    uint32_t                flags;      /*功能标志位*/
    PIN_DR                  pinDrv;     /*驱动能力*/
    PIN_SEN                 pinSen;     /*施密特触发*/
    PIN_PL                  pinPull;    /*上下拉配置*/
    PIN_IE                  pinDir;     /*输入输出方向*/
    PIN_SLEW_RATE           pinRate;    /*传输速度*/
    uint32_t                reserved[2];/*预留拓展空间*/

} PM_PINFUNC_T;


#endif