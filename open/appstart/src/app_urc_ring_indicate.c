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
 * 2023-08-01     ict team          创建
  ************************************************************************************
  */

 /************************************************************************************
  *                                 头文件
  ************************************************************************************/
#include "os.h"
#include "drv_pin.h"
#include "at_api.h"
#include "app_at_cfg.h"
#include "app_pub.h"

/************************************************************************************
 *                                 外部函数声明
 ************************************************************************************/


/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
//#define URC_MAIN_RI_SUPPORT

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef enum {
    APP_URC_RI_STATUS_PULSE = 0, // 当前处于输出脉冲阶段
    APP_URC_RI_STATUS_DELAY,     // 当前处于延迟阶段
}APP_URC_RI_STATUS;

#define APP_URC_RI_SIGNAL_TYPE_INDICATE   (GPIO_LOW)        // RI指示信号电平
#define APP_URC_RI_SIGNAL_TYPE_IDLE       (GPIO_HIGH)       // RI空闲信号电平

#define APP_URC_MAIN_RI_PIN_RES           PIN_RES(PIN_25)   // RI信号芯片PIN脚 PD_GPIO_4
#define APP_URC_MAIN_RI_PIN_MUX_GPIO      (PIN_25_MUX_GPIO) // RI信号PIN脚复用成GPIO定义

typedef struct
{
    uint16_t pulseDuration;  // 5~2000 毫秒   默认120
    uint8_t  pulseCount;     // 脉冲个数 1~5 默认1
    uint32_t  urcDelay;       // 0~120 000 毫秒 默认0
} APP_URC_RI_Cfg;

typedef struct
{
    char* urc;              // URC数据内容
    uint32_t urcLen;        // URC数据长度
    channel_cb sendURCFunc; // 发送URC的函数
    osTimerId_t timer;      // RI使用的timer
    APP_URC_RI_Cfg cfg;     // RI配置参数
    uint8_t channelId;      // AT通道ID
    uint8_t sigleType;      // 最近输出的信号类型
    uint8_t status;         // 当前RI状态，pulse or urc delay
    uint8_t pulseCount;     // 已经发出的脉冲计数
} APP_URC_RI_Context;

/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/


/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
/* 设置RI信号 */
static void APP_URC_RI_SetSignal(uint8_t single)
{
    GPIO_Write(APP_URC_MAIN_RI_PIN_RES, single);
    GPIO_SetDir(APP_URC_MAIN_RI_PIN_RES, GPIO_OUTPUT);
    PIN_SetMux(APP_URC_MAIN_RI_PIN_RES, APP_URC_MAIN_RI_PIN_MUX_GPIO);
}

/* 获取URC分类 */
static uint8_t APP_URC_RI_GetURCType(const char *urc, const uint32_t urcLen)
{
    char* sms_urc[] =
    {
        "+CMT: ","+CMTI: "
    };

    char* ring_urc[] =
    {
        "RING","+CRING: "
    };

    uint32_t i = 0;

    if(urc != NULL)
    {
        //  跳过命令开头的\r或\n
        for(i = 0; i < urcLen; i++)
        {
            if (*urc != '\r' && *urc != '\n')
                break;
            else
                urc++;
        }

        for(i = 0; i < sizeof(ring_urc)/sizeof(ring_urc[0]); i++)
        {
            if(strncmp(urc, ring_urc[i], strlen(ring_urc[i])) == 0)
            {
                return APP_AT_URC_RI_CFGTYPE_RING;
            }
        }

        for(i = 0; i < sizeof(sms_urc)/sizeof(sms_urc[0]); i++)
        {
            if(strncmp(urc, sms_urc[i], strlen(sms_urc[i])) == 0)
            {
                return APP_AT_URC_RI_CFGTYPE_SMS;
            }
        }

        return APP_AT_URC_RI_CFGTYPE_OTHER;
    }

    return APP_AT_URC_RI_CFGTYPE_NONE;
}

static void APP_URC_TimerFunc(void *argument)
{
    APP_URC_RI_Context *context = (APP_URC_RI_Context *)argument;
    osStatus_t ret = osOK;

    //osPrintf("[%llu]ri timer status[%u] sigleType[%u] count[%u] [%s]\r\n", osTickGet(), context->status, context->sigleType, context->pulseCount, context->urc);
    if(context->status == APP_URC_RI_STATUS_DELAY)
    {
        APP_PRINT_INFO("RI send urc in TimerFunc delay [%llu][%s]\r\n", osTickGet(), context->urc);
        context->sendURCFunc(context->channelId, context->urc, context->urcLen);
        osTimerDelete(context->timer);
        osFree(context);
    }
    else
    {
        if(context->sigleType == APP_URC_RI_SIGNAL_TYPE_IDLE)
        {
            context->pulseCount++;
            context->sigleType = APP_URC_RI_SIGNAL_TYPE_INDICATE;
            APP_URC_RI_SetSignal(APP_URC_RI_SIGNAL_TYPE_INDICATE);

            ret = osTimerStart(context->timer, osTickFromMs(context->cfg.pulseDuration));
            //osPrintf("[%llu]RI timer start %d in TimerFunc IDLE [%s]\r\n", osTickGet(), ret, context->urc);
        }
        else
        {
            context->sigleType = APP_URC_RI_SIGNAL_TYPE_IDLE;
            APP_URC_RI_SetSignal(APP_URC_RI_SIGNAL_TYPE_IDLE);

            if(context->pulseCount < context->cfg.pulseCount)
            {
                ret = osTimerStart(context->timer, osTickFromMs(context->cfg.pulseDuration));
                //osPrintf("[%llu]RI timer start %d in TimerFunc Pulse [%s]\r\n", osTickGet(), ret, context->urc);
            }
            else if(context->cfg.urcDelay != 0)
            {
                context->status = APP_URC_RI_STATUS_DELAY;
                ret = osTimerStart(context->timer, osTickFromMs(context->cfg.urcDelay));
                //osPrintf("[%llu]RI timer start %d in TimerFunc delay [%s]\r\n", osTickGet(), ret, context->urc);
            }
            else
            {
                APP_PRINT_INFO("RI send urc in TimerFunc [%llu][%s]\r\n", osTickGet(), context->urc);
                context->sendURCFunc(context->channelId, context->urc, context->urcLen);
                osTimerDelete(context->timer);
                osFree(context);
            }
        }
    }

    (void)ret;
}
/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
/**
 ************************************************************************************
 * @brief            开机时初始化RI信号脚
 *
 * @param[in]       none
 *
 * @return none
 ************************************************************************************
*/
void APP_URC_RI_Initiate(void)
{
    #ifdef URC_MAIN_RI_SUPPORT
    APP_URC_RI_SetSignal(APP_URC_RI_SIGNAL_TYPE_IDLE);
    #endif
}

/**
 ************************************************************************************
 * @brief            启动RI功能
 *
 * @param[in]       channelId  AT通道
 * @param[in]       data  URC数据
 * @param[in]       dataLen  URC数据长度
 * @param[in]       sendURCFunc  发送URC的函数
 *
 * @return osOK : 成功 其他：失败
 ************************************************************************************
*/
osStatus_t APP_URC_RI_Start(uint8_t channelId, char* data, uint32_t dataLen, channel_cb sendURCFunc)
{
#ifdef URC_MAIN_RI_SUPPORT
    APP_URC_RI_Context *context = NULL;
    uint8_t urcType = APP_AT_URC_RI_CFGTYPE_NONE;
    osStatus_t ret = osOK;
    APP_AT_URC_RI_CfgPulse plusecfg = {0};

    urcType = APP_URC_RI_GetURCType(data, dataLen);
    if(urcType != APP_AT_URC_RI_CFGTYPE_NONE)
    {
        if( APP_URC_RI_GetPluseCfg(urcType, &plusecfg) == osOK)
        {
            context = osCalloc(1, sizeof(APP_URC_RI_Context));
            if(context == NULL)
                return osErrorNoMemory;

            if (plusecfg.riType == APP_AT_CFG_URC_RI_TYPE_PULSE)
            {
                context->timer = osTimerNew(APP_URC_TimerFunc, osTimerOnce, (void *)context, NULL);
                if(context->timer != NULL)
                {
                    //  准备启动timer，带好所有的参数
                    context->channelId = channelId;
                    context->urc = data;
                    context->urcLen = dataLen;
                    context->sendURCFunc = sendURCFunc;
                    context->status = APP_URC_RI_STATUS_PULSE;

                    context->cfg.pulseCount = plusecfg.pulseCount;
                    context->cfg.pulseDuration = plusecfg.pulseDuration;
                    context->cfg.urcDelay = APP_URC_RI_GetURCDelay();

                    context->pulseCount++;
                    context->sigleType = APP_URC_RI_SIGNAL_TYPE_INDICATE;
                    APP_URC_RI_SetSignal(APP_URC_RI_SIGNAL_TYPE_INDICATE);

                    ret = osTimerStart(context->timer, osTickFromMs(context->cfg.pulseDuration));
                    APP_PRINT_INFO("RI Start timer ret %d\r\n", ret);
                    if(ret == osOK)
                    {
                        return osOK;
                    }
                }
            }
        }
    }

    if(context != NULL)
    {
        if(context->timer != NULL)
        {
            osTimerDelete(context->timer);
        }
        osFree(context);
    }
    return osError;
#else
    return osErrorNoSys; // 不支持时，返回ERROR
#endif
}

#if 0
#include <nr_micro_shell.h>
int32_t main_ri_send_test(uint8_t channelId, char *buf, uint32_t bufLen)
{
    osPrintf("RI send [%llu][%d][%s]\r\n", osTickGet(), channelId, buf);
    return osOK;
}

void main_ri_test(char argc, char **argv)
{
    char* ri_test = "+RI_TEST";
    osStatus_t ret = osOK;

    APP_URC_RI_Initial();
    ret = APP_URC_RI_Start(55, ri_test, strlen(ri_test), main_ri_send_test);
    osPrintf("RI start [%llu][%d][%s]\r\n", osTickGet(), ret, ri_test);

    return;
}

NR_SHELL_CMD_EXPORT(main_ri, main_ri_test);
#endif

