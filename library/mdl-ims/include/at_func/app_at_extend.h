/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file
 *
 * @brief
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-08-28     ict team          创建
 ************************************************************************************
 */

#ifndef __APP_AT_EXTEND_H__
#define __APP_AT_EXTEND_H__

#ifdef __cplusplus
extern "C" {
#endif
/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/
void APP_AT_Test_OK(uint8_t channelId);
void APP_AT_Set_OK(uint8_t channelId, uint8_t *paramIn, uint16_t paramLen);

void APP_AT_ATI_Exec(uint8_t channelId);

void APP_AT_GMI_Exec(uint8_t channelId);
void APP_AT_GMM_Exec(uint8_t channelId);
void APP_AT_GMR_Exec(uint8_t channelId);

void APP_AT_IGMR_Exec(uint8_t channelId);

void APP_AT_X_GSN_Exec(uint8_t channelId);

void APP_AT_X_CCID_Exec(uint8_t channelId);

void APP_AT_X_NWINFO_Exec(uint8_t channelId);

void APP_AT_X_ENG_Set(uint8_t channelId, uint8_t *paramIn, uint16_t paramLen);
void APP_AT_X_ENG_Test(uint8_t channelId);

void APP_AT_X_CELL_Read(uint8_t channelId);
void APP_AT_X_CELLEX_Set(uint8_t channelId, uint8_t *paramIn, uint16_t paramLen);

void APP_AT_X_URCCFG_Set(uint8_t channelId, uint8_t *paramIn, uint16_t paramLen);
void APP_AT_X_URCCFG_Test(uint8_t channelId);

void APP_AT_X_SCLK_Set(uint8_t channelId, uint8_t *paramIn, uint16_t paramLen);
void APP_AT_X_SCLK_Read(uint8_t channelId);
void APP_AT_X_SCLK_Test(uint8_t channelId);

void APP_AT_IPR_Set(uint8_t channelId, uint8_t *paramIn, uint16_t paramLen);
void APP_AT_IPR_Read(uint8_t channelId);
void APP_AT_IPR_Test(uint8_t channelId);

void APP_AT_CCLK_Set(uint8_t channelId, uint8_t *paramIn, uint16_t paramLen);
void APP_AT_CCLK_Read(uint8_t channelId);

void APP_AT_RAMDUMP_Exec(uint8_t channelId);
void APP_AT_RAMDUMP_Set(uint8_t channelId, uint8_t *paramIn, uint16_t paramLen);
void APP_AT_REBOOT_Set(uint8_t channelId, uint8_t *paramIn, uint16_t paramLen);
void APP_AT_REBOOT_Exec(uint8_t channelId);

void APP_AT_SHUTDOWN_Exec(uint8_t channelId);
void APP_AT_SHUTDOWN_Set(uint8_t channelId, uint8_t *paramIn, uint16_t paramLen);
void APP_AT_SHUTDOWN_Test(uint8_t channelId);
void APP_AT_XADC_Set(uint8_t channelId, uint8_t *paramIn, uint16_t paramLen);
void APP_AT_XADC_Test(uint8_t channelId);


void APP_AT_X_LTS_Set(uint8_t channelId, uint8_t *paramIn, uint16_t paramLen);
void APP_AT_X_LTS_Exec(uint8_t channelId);
void APP_AT_X_LTS_Test(uint8_t channelId);

#ifdef __cplusplus
}
#endif
#endif


