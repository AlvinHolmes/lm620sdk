/**
 * @file        cm_gpio.h
 * @brief       GPIO接口
 * @copyright   Copyright © 2021 China Mobile IOT. All rights reserved.
 * @author      By ZHANGXW
 * @date        2021/03/09
 *
 * @defgroup gpio gpio
 * @ingroup PI
 * @{
 */

#ifndef __CM_GPIO_H__
#define __CM_GPIO_H__

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Types
 ****************************************************************************/

typedef enum{
    CM_GPIO_CHIP_PIN_2 = 2,         /*!< keyon */
    CM_GPIO_CHIP_PIN_3 = 3,         /*!< wakeup */
    CM_GPIO_CHIP_PIN_5 = 5,         /*!< LPUART0_RX / AON_GPIO_2 */
    CM_GPIO_CHIP_PIN_6,             /*!< LPUART0_TX / AON_GPIO_3 */
    CM_GPIO_CHIP_PIN_7,             /*!< LPUART0_CTS / AON_GPIO_4 */
    CM_GPIO_CHIP_PIN_8,             /*!< LPUART0_RTS / AON_GPIO_5 */
    CM_GPIO_CHIP_PIN_9,             /*!< AON_GPIO_0 */
    CM_GPIO_CHIP_PIN_10,            /*!< AON_GPIO_1 */
    CM_GPIO_CHIP_PIN_11,            /*!< CLK_EXT_OUT / AON_GPIO_6 */
    CM_GPIO_CHIP_PIN_12,            /*!< SWD0_SWCLK / PD_GPIO_24 */
    CM_GPIO_CHIP_PIN_13,            /*!< SWD0_SWDIO / PD_GPIO_25 */
    CM_GPIO_CHIP_PIN_14,            /*!< CLK_AU_MCLK / PD_GPIO_26 */
    CM_GPIO_CHIP_PIN_21 = 21,       /*!< PD_GPIO_0 */
    CM_GPIO_CHIP_PIN_22,            /*!< PD_GPIO_1 */
    CM_GPIO_CHIP_PIN_23,            /*!< PD_GPIO_2 */
    CM_GPIO_CHIP_PIN_24,            /*!< PD_GPIO_3 */
    CM_GPIO_CHIP_PIN_25,            /*!< PD_GPIO_4 */
    CM_GPIO_CHIP_PIN_26,            /*!< PD_GPIO_5 */
    CM_GPIO_CHIP_PIN_27,            /*!< PD_GPIO_6 */
    CM_GPIO_CHIP_PIN_28,            /*!< PD_GPIO_7 */
    CM_GPIO_CHIP_PIN_29,            /*!< PD_GPIO_8 */
    CM_GPIO_CHIP_PIN_30,            /*!< PD_GPIO_9 */
    CM_GPIO_CHIP_PIN_31,            /*!< PD_GPIO_10 */
    CM_GPIO_CHIP_PIN_32,            /*!< PD_GPIO_11 */
    CM_GPIO_CHIP_PIN_33,            /*!< PD_GPIO_12 */
    CM_GPIO_CHIP_PIN_34,            /*!< PD_GPIO_13 */
    CM_GPIO_CHIP_PIN_35,            /*!< PD_GPIO_14 */
    CM_GPIO_CHIP_PIN_36,            /*!< PD_GPIO_15 */
    CM_GPIO_CHIP_PIN_37,            /*!< SIM0_RST / PD_GPIO_29 */
    CM_GPIO_CHIP_PIN_38,            /*!< SIM0_CLK / PD_GPIO_30 */
    CM_GPIO_CHIP_PIN_39,            /*!< SIM0_DATA / PD_GPIO_31 */
    CM_GPIO_CHIP_PIN_40,            /*!< RF_CONTROL_0 / PD_GPIO_16 */
    CM_GPIO_CHIP_PIN_41,            /*!< RF_CONTROL_1 / PD_GPIO_17 */
    CM_GPIO_CHIP_PIN_42,            /*!< RF_CONTROL_2 / PD_GPIO_18 */
    CM_GPIO_CHIP_PIN_43,            /*!< RF_CONTROL_3 / PD_GPIO_19 */
    CM_GPIO_CHIP_PIN_44,            /*!< RF_CONTROL_4 / PD_GPIO_20 */
    CM_GPIO_CHIP_PIN_45,            /*!< RF_CONTROL_5 / PD_GPIO_21 */
    CM_GPIO_CHIP_PIN_46,            /*!< RF_CONTROL_6 / PD_GPIO_22 */
    CM_GPIO_CHIP_PIN_47,            /*!< RF_CONTROL_7 / PD_GPIO_23 */
    CM_GPIO_CHIP_PIN_48,            /*!< AON_GPIO_7 */
    CM_GPIO_CHIP_PIN_49,            /*!< AON_GPIO_8 */
    CM_GPIO_CHIP_PIN_50,            /*!< PD_GPIO_16 */
    CM_GPIO_CHIP_PIN_51,            /*!< PD_GPIO_17 */
    CM_GPIO_CHIP_PIN_52,            /*!< PD_GPIO_18 */
    CM_GPIO_CHIP_PIN_53,            /*!< PD_GPIO_19 */
    CM_GPIO_CHIP_PIN_MAX
} cm_gpio_chip_pin_e;

#define CM_GPIO_KEYON       CM_GPIO_CHIP_PIN_2
#define CM_GPIO_WAKEUP      CM_GPIO_CHIP_PIN_3
#define CM_GPIO_AON_0       CM_GPIO_CHIP_PIN_9
#define CM_GPIO_AON_1       CM_GPIO_CHIP_PIN_10
#define CM_GPIO_AON_2       CM_GPIO_CHIP_PIN_5
#define CM_GPIO_AON_3       CM_GPIO_CHIP_PIN_6
#define CM_GPIO_AON_4       CM_GPIO_CHIP_PIN_7
#define CM_GPIO_AON_5       CM_GPIO_CHIP_PIN_8
#define CM_GPIO_AON_6       CM_GPIO_CHIP_PIN_11
#define CM_GPIO_AON_7       CM_GPIO_CHIP_PIN_48
#define CM_GPIO_AON_8       CM_GPIO_CHIP_PIN_49
#define CM_GPIO_PD_0        CM_GPIO_CHIP_PIN_21
#define CM_GPIO_PD_1        CM_GPIO_CHIP_PIN_22
#define CM_GPIO_PD_2        CM_GPIO_CHIP_PIN_23
#define CM_GPIO_PD_3        CM_GPIO_CHIP_PIN_24
#define CM_GPIO_PD_4        CM_GPIO_CHIP_PIN_25
#define CM_GPIO_PD_5        CM_GPIO_CHIP_PIN_26
#define CM_GPIO_PD_6        CM_GPIO_CHIP_PIN_27
#define CM_GPIO_PD_7        CM_GPIO_CHIP_PIN_28
#define CM_GPIO_PD_8        CM_GPIO_CHIP_PIN_29
#define CM_GPIO_PD_9        CM_GPIO_CHIP_PIN_30
#define CM_GPIO_PD_10       CM_GPIO_CHIP_PIN_31
#define CM_GPIO_PD_11       CM_GPIO_CHIP_PIN_32
#define CM_GPIO_PD_12       CM_GPIO_CHIP_PIN_33
#define CM_GPIO_PD_13       CM_GPIO_CHIP_PIN_34
#define CM_GPIO_PD_14       CM_GPIO_CHIP_PIN_35
#define CM_GPIO_PD_15       CM_GPIO_CHIP_PIN_36
#define CM_GPIO_PD_16       CM_GPIO_CHIP_PIN_50
#define CM_GPIO_PD_17       CM_GPIO_CHIP_PIN_51
#define CM_GPIO_PD_18       CM_GPIO_CHIP_PIN_52
#define CM_GPIO_PD_19       CM_GPIO_CHIP_PIN_53
#define CM_GPIO_PD_20       CM_GPIO_CHIP_PIN_44
#define CM_GPIO_PD_21       CM_GPIO_CHIP_PIN_45
#define CM_GPIO_PD_22       CM_GPIO_CHIP_PIN_46
#define CM_GPIO_PD_23       CM_GPIO_CHIP_PIN_47
#define CM_GPIO_PD_24       CM_GPIO_CHIP_PIN_12
#define CM_GPIO_PD_25       CM_GPIO_CHIP_PIN_13
#define CM_GPIO_PD_26       CM_GPIO_CHIP_PIN_14
#define CM_GPIO_PD_29       CM_GPIO_CHIP_PIN_37
#define CM_GPIO_PD_30       CM_GPIO_CHIP_PIN_38
#define CM_GPIO_PD_31       CM_GPIO_CHIP_PIN_39

typedef enum{
    CM_GPIO_NUM_0 = CM_GPIO_CHIP_PIN_21,        // CM_IOMUX_PIN_16          // PD_GPIO_0
    CM_GPIO_NUM_1 = CM_GPIO_CHIP_PIN_5,         // CM_IOMUX_PIN_17          // LPUART0_RX
    CM_GPIO_NUM_2 = CM_GPIO_CHIP_PIN_6,         // CM_IOMUX_PIN_18          // LPUART0_TX
    CM_GPIO_NUM_3 = CM_GPIO_CHIP_PIN_25,        // CM_IOMUX_PIN_20          // PD_GPIO_4
    CM_GPIO_NUM_4 = CM_GPIO_CHIP_PIN_51,        // CM_IOMUX_PIN_21          // PD_GPIO_17
    CM_GPIO_NUM_5 = CM_GPIO_CHIP_PIN_7,         // CM_IOMUX_PIN_22          // LPUART0_CTS
    CM_GPIO_NUM_6 = CM_GPIO_CHIP_PIN_8,         // CM_IOMUX_PIN_23          // LPUART0_RTS
    CM_GPIO_NUM_7 = CM_GPIO_CHIP_PIN_48,        // CM_IOMUX_PIN_25          // AON_GPIO_7
    CM_GPIO_NUM_8 = CM_GPIO_CHIP_PIN_27,        // CM_IOMUX_PIN_28          // PD_GPIO_6
    CM_GPIO_NUM_9 = CM_GPIO_CHIP_PIN_28,        // CM_IOMUX_PIN_29          // PD_GPIO_7
    CM_GPIO_NUM_10 = CM_GPIO_CHIP_PIN_12,       // CM_IOMUX_PIN_38          // SWD0_SWCLK
    CM_GPIO_NUM_11 = CM_GPIO_CHIP_PIN_13,       // CM_IOMUX_PIN_39          // SWD0_SWDIO
    CM_GPIO_NUM_12 = CM_GPIO_CHIP_PIN_49,       // CM_IOMUX_PIN_49          // AON_GPIO_8
    CM_GPIO_NUM_13 = CM_GPIO_CHIP_PIN_24,       // CM_IOMUX_PIN_54          // PD_GPIO_3
    CM_GPIO_NUM_14 = CM_GPIO_CHIP_PIN_23,       // CM_IOMUX_PIN_55          // PD_GPIO_2
    CM_GPIO_NUM_15 = CM_GPIO_CHIP_PIN_22,       // CM_IOMUX_PIN_56          // PD_GPIO_1
    CM_GPIO_NUM_16 = CM_GPIO_CHIP_PIN_14,       // CM_IOMUX_PIN_57          // CLK_AU_MCLK
    CM_GPIO_NUM_17 = CM_GPIO_CHIP_PIN_50,       // CM_IOMUX_PIN_58          // PD_GPIO_16
    CM_GPIO_NUM_18 = CM_GPIO_CHIP_PIN_30,       // CM_IOMUX_PIN_62          // PD_GPIO_9
    CM_GPIO_NUM_19 = CM_GPIO_CHIP_PIN_29,       // CM_IOMUX_PIN_63          // PD_GPIO_8
    CM_GPIO_NUM_20 = CM_GPIO_CHIP_PIN_31,       // CM_IOMUX_PIN_64          // PD_GPIO_10
    CM_GPIO_NUM_21 = CM_GPIO_CHIP_PIN_9,        // CM_IOMUX_PIN_74          // AON_GPIO_0
    CM_GPIO_NUM_22 = CM_GPIO_CHIP_PIN_26,       // CM_IOMUX_PIN_75          // PD_GPIO_5
    CM_GPIO_NUM_23 = CM_GPIO_CHIP_PIN_34,       // CM_IOMUX_PIN_76          // PD_GPIO_13
    CM_GPIO_NUM_24 = CM_GPIO_CHIP_PIN_36,       // CM_IOMUX_PIN_77          // PD_GPIO_15
    CM_GPIO_NUM_25 = CM_GPIO_CHIP_PIN_10,       // CM_IOMUX_PIN_79          // AON_GPIO_1
    CM_GPIO_NUM_26 = CM_GPIO_CHIP_PIN_11,       // CM_IOMUX_PIN_80          // CLK_EXT_OUT
    CM_GPIO_NUM_27 = CM_GPIO_CHIP_PIN_32,       // CM_IOMUX_PIN_81          // PD_GPIO_11
    CM_GPIO_NUM_28 = CM_GPIO_CHIP_PIN_35,       // CM_IOMUX_PIN_86          // PD_GPIO_14
    CM_GPIO_NUM_29 = CM_GPIO_CHIP_PIN_33,       // CM_IOMUX_PIN_87          // PD_GPIO_12
    // CM_GPIO_NUM_MAX
}cm_gpio_num_e;

/* *GPIO工作模式 */
typedef enum
{
    CM_GPIO_MODE_NUM/*!<不可用 工作模式数量*/
} cm_gpio_mode_e;

/** I/O方向 */
typedef enum{
    CM_GPIO_DIRECTION_INPUT = 0,
    CM_GPIO_DIRECTION_OUTPUT,
}cm_gpio_direction_e;

/** 上/下拉 */
typedef enum{
    CM_GPIO_PULL_NONE,
    CM_GPIO_PULL_DOWN,
    CM_GPIO_PULL_UP,
}cm_gpio_pull_e;

/** 边沿+电平触发 */
typedef enum{
    CM_GPIO_IT_EDGE_RISING,
    CM_GPIO_IT_EDGE_FALLING,
    CM_GPIO_IT_EDGE_BOTH,
    CM_GPIO_IT_LEVEL_HIGH,/*!<不支持*/
    CM_GPIO_IT_LEVEL_LOW,/*!<不支持*/
}cm_gpio_interrupt_e;

/** 高低电平 */
typedef enum{
    CM_GPIO_LEVEL_LOW,
    CM_GPIO_LEVEL_HIGH,
}cm_gpio_level_e;

/** 命令码 */
typedef enum{
    CM_GPIO_CMD_SET_PULL,                  /*!< 上下拉设置命令码*/
    CM_GPIO_CMD_GET_PULL,                  /*!< 上下拉获取命令码*/
    CM_GPIO_CMD_SET_LEVEL,                 /*!< 驱动能力设置命令码*/
    CM_GPIO_CMD_GET_LEVEL,                 /*!< 驱动能力获取命令码*/
    CM_GPIO_CMD_SET_DIRECTION,             /*!< 输入输出设置命令码*/
    CM_GPIO_CMD_GET_DIRECTION,             /*!< 输入输出获取命令码*/
} cm_gpio_cmd_e;

/** 配置 */
typedef struct{
    cm_gpio_mode_e mode;/*!< 不支持*/
    cm_gpio_direction_e direction;
    cm_gpio_pull_e pull;
} cm_gpio_cfg_t;

/****************************************************************************
 * Public Data
 ****************************************************************************/


/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
***********************GPIO使用注意事项*************************************
* 1、GPIO引脚和SD卡控制引脚所用PIN脚存在复用关系，若用户使用此PIN脚进行
*    GPIO功能开发，请阅知cm_sd.h中【SD卡使用注意事项】
* 2、GPIO引脚和SD卡控制引脚复用关系见《资源综述》
***************************************************************************/

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/**
 *  @brief 初始化GPIO
 *
 *  @param [in] gpio_num GPIO号
 *  @param [in] cfg 配置
 *
 *  @return
 *    = 0 - 成功 \n
 *    < 0 - 失败, 返回值为错误码
 *
 *  @details 初始化之前一定要先设置引脚复用
 */
int32_t cm_gpio_init(cm_gpio_num_e gpio_num, cm_gpio_cfg_t *cfg);

/**
 *  @brief 去初始化
 *
 *  @param [in] gpio_num GPIO号
 *
 *  @return
 *    = 0 - 成功 \n
 *    < 0 - 失败, 返回值为错误码
 *
 *  @details More details
 */
int32_t cm_gpio_deinit(cm_gpio_num_e gpio_num);

/**
 *  @brief 设置输出电平
 *
 *  @param [in] gpio_num GPIO号
 *  @param [in] level 输出电平
 *
 *  @return
 *    = 0 - 成功 \n
 *    < 0 - 失败, 返回值为错误码
 *
 *  @details More details
 */
int32_t cm_gpio_set_level(cm_gpio_num_e gpio_num, cm_gpio_level_e level);

/**
 *  @brief 读取输入电平
 *
 *  @param [in] gpio_num GPIO号
 *  @param [out] level 输入电平
 *
 *  @return
 *    = 0 - 成功 \n
 *    < 0 - 失败, 返回值为错误码
 *
 *  @details More details
 */
int32_t cm_gpio_get_level(cm_gpio_num_e gpio_num, cm_gpio_level_e *level);

/**
 *  @brief 设置上/下拉
 *
 *  @param [in] gpio_num GPIO号
 *  @param [in] type 上下拉配置
 *
 *  @return
 *    = 0 - 成功 \n
 *    < 0 - 失败, 返回值为错误码
 *
 *  @details More details
 */
int32_t cmo_gpio_set_pull(cm_gpio_num_e gpio_num, cm_gpio_pull_e type);
static inline int32_t cm_gpio_set_pull(cm_gpio_num_e gpio_num, cm_gpio_pull_e type)
{
    return cmo_gpio_set_pull(gpio_num, type);
}

/**
 *  @brief 读取上/下拉
 *
 *  @param [in] gpio_num GPIO号
 *  @param [out] type 上下拉配置
 *
 *  @return
 *    = 0 - 成功 \n
 *    < 0 - 失败, 返回值为错误码
 *
 *  @details More details
 */
int32_t cmo_gpio_get_pull(cm_gpio_num_e gpio_num, cm_gpio_pull_e *type);
static inline int32_t cm_gpio_get_pull(cm_gpio_num_e gpio_num, cm_gpio_pull_e *type)
{
    return cmo_gpio_get_pull(gpio_num, type);
}

/**
 *  @brief I/O方向模式配置
 *
 *  @param [in] gpio_num GPIO号
 *  @param [in] dir 输入/输出
 *
 *  @return
 *    = 0 - 成功 \n
 *    < 0 - 失败, 返回值为错误码
 *
 *  @details More details
 */
int32_t cm_gpio_set_direction(cm_gpio_num_e gpio_num, cm_gpio_direction_e dir);

/**
 *  @brief 获取I/O方向模式配置
 *
 *  @param [in] gpio_num GPIO号
 *  @param [out] dir 输入/输出
 *
 *  @return
 *    = 0 - 成功 \n
 *    < 0 - 失败, 返回值为错误码
 *
 *  @details More details
 */
int32_t cm_gpio_get_direction(cm_gpio_num_e gpio_num, cm_gpio_direction_e *dir);

/**
 *  @brief 注册中断回调函数
 *
 *  @param [in] gpio_num GPIO号
 *  @param [in] interrupt_cb 中断回调函数
 *
 *  @return
 *    = 0 - 成功 \n
 *    < 0 - 失败, 返回值为错误码
 *
 *  @details 中断函数中不要处理耗时任务和打印等重进入函数
 */
int32_t cm_gpio_interrupt_register(cm_gpio_num_e gpio_num, void *interrupt_cb);

/**
 *  @brief 使能中断
 *
 *  @param [in] gpio_num GPIO号
 *  @param [in] intr_mode 中断触发方式
 *
 *  @return
 *    = 0 - 成功 \n
 *    < 0 - 失败, 返回值为错误码
 *
 *  @details 当前可使用 edge rising / falling / both；level high / low 不建议使用。
 */
int32_t cm_gpio_interrupt_enable(cm_gpio_num_e gpio_num, cm_gpio_interrupt_e intr_mode);

/**
 *  @brief 失能中断
 *
 *  @param [in] gpio_num GPIO号
 *
 *  @return
 *    = 0 - 成功 \n
 *    < 0 - 失败, 返回值为错误码
 *
 *  @details More details
 */
int32_t cm_gpio_interrupt_disable(cm_gpio_num_e gpio_num);

/**
 *  @brief GPIO控制
 *
 *  @param [in] gpio_num GPIO号
 *  @param [in] cmd 命令码
 *  @param [in/out] arg 命令
 *
 *  @return
 *    = 0 - 成功 \n
 *    < 0 - 失败, 返回值为错误码
 *
 *  @details 根据不同的cmd值（命令码），arg参数类型须对应配置。\n
 *  cmd = \ref GPIO_CMD_SET_PULL, arg传入\ref cm_gpio_pull_e *               \n
 *  cmd = \ref GPIO_CMD_GET_PULL, arg传入\ref cm_gpio_pull_e *               \n
 *  cmd = \ref GPIO_CMD_SET_LEVEL, arg传入\ref cm_gpio_level_e *             \n
 *  cmd = \ref GPIO_CMD_GET_LEVEL, arg传入\ref cm_gpio_level_e *             \n
 *  cmd = \ref GPIO_CMD_SET_DIRECTION, arg传入\ref cm_gpio_direction_e *     \n
 *  cmd = \ref GPIO_CMD_GET_DIRECTION, arg传入\ref cm_gpio_direction_e *     \n
 */
int32_t cm_gpio_ioctl(cm_gpio_num_e gpio_num, int32_t cmd, void *arg);

#undef EXTERN
#ifdef __cplusplus
}
#endif


#endif /* __CM_GPIO_H__ */

/** @}*/
