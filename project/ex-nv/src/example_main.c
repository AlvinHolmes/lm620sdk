/*************************************************************************************
* 版权所有 (C) 2023, 南京创芯慧联技术有限公司
* 保留所有权利。
*
* @file main.c
*
* @brief  main函数入口文件.
*
* @revision
*
* 日期           作者               修改内容
* 2023-07-31   ICT Team        创建
************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <nvm.h>

int main(void)
{
#ifdef BUILD_IN_SDK
    uint16_t cfg1;

    NVM_Read(NV_ITEM_ADDR(customCfg.demoCfg.cfg1), (uint8_t *)&cfg1, sizeof(cfg1));
    cfg1 = ~cfg1;
    NVM_Write(NV_ITEM_ADDR(customCfg.demoCfg.cfg1), (uint8_t *)&cfg1, sizeof(cfg1));
#endif

    return 0;
}

