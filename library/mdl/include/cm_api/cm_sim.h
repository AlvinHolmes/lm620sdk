/**
 * @file        cm_sim.h
 * @brief       SIM接口
 * @copyright   Copyright © 2021 China Mobile IOT. All rights reserved.
 * @author      By WangPeng
 * @date        2021/10/27
 *
 * @defgroup sim sim
 * @ingroup PHONE
 * @{
 */

#ifndef __CM_SIM_H__
#define __CM_SIM_H__

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define CM_IMSI_LEN       16 /*!< IMSI存储长度 */

#if 1 /* 扩展的 */
/* SIM 卡槽定义 */
#define CM_SIM_SLOT_0      (0) /*!< SIM 卡槽 0 */
#define CM_SIM_SLOT_1      (1) /*!< SIM 卡槽 1 */

/* SIM 状态定义 */
#define CM_SIM_STATUS_TIMEOUT       (-3) /*!< 获取 SIM 状态超时 */
#define CM_SIM_STATUS_NOT_INSERTED (-2) /*!< SIM 卡未插入 */
#define CM_SIM_STATUS_UNKNOW       (-1) /*!< SIM 状态未知 */
#define CM_SIM_STATUS_READY        (0)  /*!< SIM 卡就绪 */
#define CM_SIM_STATUS_SIMPIN       (1)  /*!< 需要 SIM PIN */
#define CM_SIM_STATUS_SIMPUK       (2)  /*!< 需要 SIM PUK */
#endif /* 扩展的 */

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/


/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif


/**
 * @brief 获取设备IMSI
 * 
 * @param [out] imsi 存储IMSI，长度16字节，申请内存后传入
 * @return  
 *   = 0  - 成功 \n
 *   < 0  - 失败, 返回值为错误码.
 *  
 * @details More details
 * 
 */
int32_t cm_sim_get_imsi(char* imsi);

/**
 * @brief 获取设备ICCID
 * 
 * @param [out] iccid 存储ICCID，长度21字节，申请内存后传入
 * @return  
 *   = 0  - 成功 \n
 *   < 0  - 失败, 返回值为错误码.
 *  
 * @details More details
 * 
 */
int32_t cm_sim_get_iccid(char* iccid);

#if 1 /* 扩展的 */
/**
 * @brief 获取 SIM 卡状态
 *
 * @param [out] status SIM 状态，参考 CM_SIM_STATUS_xxx 宏定义
 * @return
 *   = 0  - 成功 \n
 *   < 0  - 失败, 返回值为错误码.
 *
 * @details More details
 *
 */
int32_t cm_sim_get_status(int32_t *status);

/**
 * @brief 设置当前使用的 SIM 卡槽
 *
 * @param [in] slot SIM 卡槽，参考 CM_SIM_SLOT_xxx 宏定义
 * @return
 *   = 0  - 成功 \n
 *   < 0  - 失败, 返回值为错误码.
 *
 * @details More details
 *
 */
int32_t cm_sim_set_slot(uint8_t slot);

/**
 * @brief 获取当前使用的 SIM 卡槽
 *
 * @param [out] slot SIM 卡槽，参考 CM_SIM_SLOT_xxx 宏定义
 * @return
 *   = 0  - 成功 \n
 *   < 0  - 失败, 返回值为错误码.
 *
 * @details More details
 *
 */
int32_t cm_sim_get_slot(uint8_t *slot);
#endif /* 扩展的 */

#undef EXTERN
#ifdef __cplusplus
}
#endif


#endif /* __CM_SIM_H__ */

