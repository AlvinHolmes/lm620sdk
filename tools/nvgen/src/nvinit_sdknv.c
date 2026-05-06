/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        nvinit_sdknv.c
 *
 * @brief       SDK 相关 NV   默认参数实现.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-05-25     ict team          创建
 ************************************************************************************
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "nv_init_defparam.h"
#include "dirfile.h"
#include "version_config.h"


/**
 *  NV写入版本号
 */
int NVGEN_InitVersion(NV_Default *nvDefault)
{
    NV_PubConfig *nv = &nvDefault->pubConfig;

    //versions
    memset(&nv->devInfo, 0x0, sizeof(nv->devInfo));
    strcpy(nv->devInfo.softVersion, SW_VERSION);
    strcpy(nv->devInfo.hardVersion, HW_VERSION);
    strcpy(nv->devInfo.prodInfo,    PRODUCT_NAME);
    strcpy(nv->devInfo.mfrInfo,     MANUFACTURE_NAME);

    return 0;
}


#ifdef BUILD_IN_SDK

NVGEN_NV_DEFINIT(customCfg, NVGEN_custom_init, 3);

int NVGEN_custom_init(NV_Default *nvDefault)
{
    FILE    *fp;
    int     ret;
    /* 读取 sdk NV    */
    fp = fopen(sdknv_path, "rb");
    if (fp != NULL) {
        size_t readCount;
        fseek(fp, 0, SEEK_SET);
        readCount = fread(nvDefault->ModemArea, 1, NV_ITEM_SIZE(ModemArea) + NV_ITEM_SIZE(pubConfigArea), fp);
        if (readCount != NV_ITEM_SIZE(ModemArea) + NV_ITEM_SIZE(pubConfigArea)) {
            NVGENLOG("sdknv bin size error: %d != %d", readCount, NV_ITEM_SIZE(ModemArea) + NV_ITEM_SIZE(pubConfigArea));
            ret = -1;
        } else {
            NV_CustomConfigDefault(&nvDefault->customCfg);
            NVGEN_InitVersion(nvDefault);
            ret = 0;
        }
        fclose(fp);
    } else {
        NVGENLOG("Open sdknv bin fail");
        ret = -1;
    }
    return ret;
}

#else

#include <nvparam_customcfg.h>

NVGEN_NV_DEFINIT(flag, NVGEN_custom_init, 3);

int NVGEN_custom_init(NV_Default *nvDefault)
{
    NV_CustomConfigDefault(nvDefault->customCfgArea);
    NVGEN_InitVersion(nvDefault);
    return 0;
}

#endif


