/**
 * @file example_main.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-10-17
 *
 * SPDX-FileCopyrightText: 2025 深圳市天工聚创科技有限公司
 * SPDX-License-Identifier: Apache-2.0
 *
 */

/* ==================== [Includes] ========================================== */

#include <os.h>
#include "test_data.h"

#include <os.h>
#include <drv_pin.h>

/* ==================== [Defines] =========================================== */

/* ==================== [Typedefs] ========================================== */

/* ==================== [Static Prototypes] ================================= */

/* ==================== [Static Variables] ================================== */

/* ==================== [Macros] ============================================ */

/* ==================== [Global Functions] ================================== */

int main(void)
{
    osPrintf("hello world");
    
    /* audio 电源 */
    PIN_SetMux(PIN_RES(PIN_7), PIN_7_MUX_GPIO); /* AON_GPIO_4 */
    GPIO_SetDir(PIN_RES(PIN_7), GPIO_OUTPUT);
    GPIO_Write(PIN_RES(PIN_7), GPIO_HIGH);

    /* PA 使能 */
    PIN_SetMux(PIN_RES(PIN_49), PIN_49_MUX_GPIO); /* AON_GPIO_8 */
    GPIO_SetDir(PIN_RES(PIN_49), GPIO_OUTPUT);
    GPIO_Write(PIN_RES(PIN_49), GPIO_HIGH);

    osDelay(100);

    test_audio_play();
    return 0;   
}

/* ==================== [Static Functions] ================================== */
