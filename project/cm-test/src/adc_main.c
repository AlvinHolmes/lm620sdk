/**
 * @file example_main.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-12-25
 *
 * SPDX-FileCopyrightText: 2025 深圳市天工聚创科技有限公司
 * SPDX-License-Identifier: Apache-2.0
 *
 */

/****************************************************************************
* Included Files
****************************************************************************/

#include <stdlib.h>
#include "cm_os.h"
#include "cm_adc.h"
#include "cm_sys.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
****************************************************************************/

int main(void)
{
    int32_t ret;
    int32_t voltage;

    cm_printf("CM ADC test starts\n");

    /* 第一次需要等待一段时间让 adc 模块初始化 */
    voltage = 0;
    ret = cm_adc_read(CM_ADC_0, &voltage);
    osDelay(100);
    
    voltage = 0;
    ret = cm_adc_read(CM_ADC_0, &voltage);
    cm_printf("ADC_0 read: ret=%d, voltage=%dmV\n", ret, voltage);

    voltage = 0;
    ret = cm_adc_read(CM_ADC_1, &voltage);
    cm_printf("ADC_1 read: ret=%d, voltage=%dmV\n", ret, voltage);

    voltage = 0;
    ret = cm_adc_read(0, &voltage);
    cm_printf("ADC read invalid dev 0: ret=%d, voltage=%dmV\n", ret, voltage);

    voltage = 0;
    ret = cm_adc_read(CM_ADC_NUM_MAX, &voltage);
    cm_printf("ADC read invalid dev CM_ADC_NUM_MAX: ret=%d, voltage=%dmV\n", ret, voltage);

    voltage = 0;
    ret = cm_adc_vbat_read((uint32_t *)&voltage);
    if (ret != 0) {
        cm_printf("VBAT read failed: ret=%d\n", ret);
    } else {
        cm_printf("VBAT read success: voltage=%dmV\n", voltage);
    }

    cm_printf("CM ADC test ends\n");
    return 0;
}
