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
#include "oss_types.h"
#include "amt.h"
#include "platcfg.h"
#include "at_parser.h"
#include "app_pub.h"
#include "app_urc_monitor.h"
#include "at_client_api.h"
#include "drv_rtc.h"

/************************************************************************************
 *                                 外部函数声明
 ************************************************************************************/
static uint8_t APP_AutoConnectCid(void);
/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define APP_4G_CONNECT_CID  APP_AutoConnectCid()   //   以太网使用的CID
#define APP_4G_TASK_STACK_SIZE ((uint32_t) (1024*2))
#define APP_4G_TASK_PRIORITY   (osPriorityBelowNormal)
/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/

/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
static bool APP_AutoCfun1Flag(void)
{
    bool  auto_cfun1 = false;
    int ret;
    ret = PlatCfg_GetAutoCfun1Flag(&auto_cfun1);
    APP_PRINT_INFO("APP_AutoCfun1Flag [%u][%d]\r\n", auto_cfun1, ret);
    return auto_cfun1;
}

static uint8_t APP_AutoConnectCid(void)
{
    uint8_t  auto_connect_cid = 0;
    int ret;
    ret = PlatCfg_GetAutoConnectCid(&auto_connect_cid);
    APP_PRINT_INFO("APP_AutoConnectCid [%u][%d]\r\n", auto_connect_cid, ret);

    return auto_connect_cid;
}

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
#ifdef SVC_SUPPORT
#include "svc_api.h"
#include "svc_network.h"
#include "svc_sms.h"
#include "svc_call.h"
#include "svc_usim.h"
#ifdef APP_BIP_SUPPORT
#include "svc_usat.h"
#include "app_bip.h"
#endif

static void APP_4G_Task(void *argument)
{
    SVC_Event_Info event;
    uint16_t eventId = 0;
    int send = 0;

    APP_PRINT_INFO("APP_4G_Task running\r\n");
    osPrintf("APP_4G_Task running\r\n");

    // send cfun=1
    if(APP_AutoCfun1Flag())
    {
        svc_network_set_cfun(1);
    }

    while (1)
    {
        if (osOK != svc_event_recv(&event, osWaitForever))
        {
            APP_PRINT_INFO("APP_4G_Task svc_event_recv recv err\r\n");
            osThreadSleepRelaxed(osTickFromMs(1000), osWaitForever);
            continue;
        }

        eventId = SVC_EVENT_ID(event.event);

        switch(eventId)
        {
            case SVC_EVT_NETWORK_SET_CEREG_RESULT:
            {
                APP_PRINT_INFO("APP_4G_Task CEREG result:%d\r\n", event.param.int_value);
                //svc_network_set_cfun(1);
            }
            break;

            case SVC_EVT_NETWORK_SET_CFUN_RESULT:
            {
                APP_PRINT_INFO("APP_4G_Task CFUN result:%d\r\n", event.param.int_value);
            }
            break;

            case SVC_EVT_NETWORK_CEREG_IND:
            {
                SVC_NETWORK_CeregInfo *netstat = event.param.ptr_param;

                APP_PRINT_INFO("APP_4G_Task CEREG_IND stat[%d]tac[%s]ci[%s]act[%d]subact[%d]\r\n",
                    netstat->stat, netstat->tac, netstat->ci, netstat->act, netstat->subact);
            }
            break;

            case SVC_EVT_NETWORK_CGEV_IND:
            {
                SVC_NETWORK_CgevInfo *cgev = event.param.ptr_param;

                APP_PRINT_INFO("APP_4G_Task CGEV_IND stat[%d]cid[%d]\r\n", cgev->stat, cgev->cid);
                if(cgev->stat == 1 && cgev->cid == APP_4G_CONNECT_CID) //   网络附着成功
                {
                    send = svc_network_set_cgact(1,cgev->cid);
                    APP_PRINT_INFO("APP_4G_Task send cagct [%d]\r\n", send);
                }

                if(cgev->stat == 0 && cgev->cid == APP_4G_CONNECT_CID) //   网络断开
                {
                    //demo_http_stop();
                    //demo_mqtt_stop();
                }
            }
            break;

            case SVC_EVT_NETWORK_SET_CGACT_RESULT:
            {
                APP_PRINT_INFO("APP_4G_Task CGACT result:%d\r\n", event.param.int_value);
            }
            break;

            case SVC_EVT_NETWORK_SET_NETDEVCTL_RESULT:
            {
                APP_PRINT_INFO("APP_4G_Task NETDEVCTL result:%d\r\n", event.param.int_value);
            }
            break;

            case SVC_EVT_NETWORK_NETSTAT_IND:
            {
                SVC_NETWORK_NetstatInfo *stat = event.param.ptr_param;

                APP_PRINT_INFO("APP_4G_Task NETSTAT_IND stat[%d]cid[%d]\r\n", stat->stat, stat->cid);
                #ifndef USE_TOP_PPP //  PPP和RNDIS不能共存
                if(stat->stat == 1 && stat->cid == APP_4G_CONNECT_CID) //   lwip ready
                {
                    send = svc_network_set_netdevctl(1,stat->cid);
                    APP_PRINT_INFO("APP_4G_Task send netdevctl [%d]\r\n", send);
                }
                #endif
            }
            break;

            case SVC_EVT_NETWORK_NETDEVCTL_IND:
            {
                SVC_NETWORK_NetdevctlInfo *stat = event.param.ptr_param;

                APP_PRINT_INFO("APP_4G_Task NETDEVCTL_IND stat[%d]cid[%d]\r\n", stat->stat, stat->cid);
            }
            break;

            case SVC_EVT_NETWORK_IPV4_READY_IND:
            {
                APP_PRINT_INFO("APP_4G_Task ipv4 Ready ind\r\n");
                //demo_http_start();
                //demo_mqtt_start();
            }
            break;

            case SVC_EVT_NETWORK_IPV6_READY_IND:
            {
                APP_PRINT_INFO("APP_4G_Task ipv6 Ready ind\r\n");
            }
            break;

            case SVC_EVT_NETWORK_TIME_IND:
            {
                SVC_NETWORK_DateTime *time = event.param.ptr_param;

                APP_PRINT_INFO("APP_4G_Task time ind zone:%d, %04d/%02d/%02d,%2d:%02d:%02d\r\n", time->zone, time->year, time->mon, time->mday, time->hour, time->min, time->sec);
            }
            break;

            case SVC_EVT_NETWORK_QCSQ_IND:
            {
                SVC_NETWORK_QCSQInfo *qcsqinfo = event.param.ptr_param;
                APP_PRINT_INFO("APP_4G_Task signal rssi:%d rsrp:%d sinr:%d rsrq:%d\r\n", qcsqinfo->rssi, qcsqinfo->rsrp, qcsqinfo->sinr, qcsqinfo->rsrq);
            }
            break;

            case SVC_EVT_NETWORK_READ_CSQ_RESULT:
            {
                APP_PRINT_INFO("APP_4G_Task READ_CSQ rssi:%d\r\n", event.param.int_value);
            }
            break;

            case SVC_EVT_NETWORK_READ_COPS_RESULT:
            {
                SVC_NETWORK_CopsReadInfo *copsinfo = event.param.ptr_param;
                APP_PRINT_INFO("APP_4G_Task READ_COPS oper[%s]act[%u]\r\n", copsinfo->oper, copsinfo->act);
            }
            break;

            case SVC_EVT_SMS_NEWSMS_IND:
            {
                //app_new_sms_recv((SVC_SMS_SmsInd *)event.param.ptr_param);
                APP_PRINT_INFO("APP_4G_Task SVC_EVT_SMS_NEWSMS_IND\r\n");
            }
            break;

            case SVC_EVT_SMS_SEND_RESULT:
            {
                APP_PRINT_INFO("APP_4G_Task SMS send result:%d\r\n", event.param.int_value);
            }
            break;

            case SVC_EVT_SMS_CSCA_IND:
            {
                //app_csca_recv((SVC_SMS_CscaInd *)event.param.ptr_param);
                SVC_SMS_CscaInd *csca = (SVC_SMS_CscaInd *)event.param.ptr_param;
                APP_PRINT_INFO("APP_4G_Task CSCA_IND rlt[%d] sca[%s]tosca[%d]\r\n", csca->result, csca->sca, csca->tosca);
            }
            break;

            case SVC_EVT_SMS_CSCA_SET_RESULT:
            {
                APP_PRINT_INFO("APP_4G_Task CSCA result:%d\r\n", event.param.int_value);
            }
            break;

            case SVC_EVT_SMS_CMGD_SET_RESULT:
            {
                APP_PRINT_INFO("APP_4G_Task CMGD result:%d\r\n", event.param.int_value);
            }
            break;

            case SVC_EVT_USIM_CMUSLOT_IND:
            {
                SVC_USIM_Cmuslot *cmuslot = event.param.ptr_param;

                APP_PRINT_INFO("APP_4G_Task CMUSLOT_IND slot[%d]state[%d]\r\n", cmuslot->slot, cmuslot->slot_state);
            }
            break;

            case SVC_EVT_USIM_ICCID_IND:
            {
                SVC_USIM_IccidInfo *iccid = event.param.ptr_param;

                APP_PRINT_INFO("APP_4G_Task ICCID_IND [%s]\r\n", iccid->iccid);
            }
            break;

            case SVC_EVT_USIM_CMURDY_IND:
            {
                int32_t sim_status = event.param.int_value;
                APP_PRINT_INFO("APP_4G_Task CMURDY_IND usim ready result:%d\r\n", sim_status);
                // sim_status 值的意义，参考AT手册+CMURDY的initresult参数
                /*
                if(event.param.int_value == 30) //SIM卡初始化成功
                {
                    svc_sms_read_csca();
                }
                */
            }
            break;

            case SVC_EVT_NETWORK_IMSVMODE_IND:
            {
                // VOLTE注册结果
                APP_PRINT_INFO("APP_4G_Task IMSVMODE_IND volte ready result:%d\r\n", event.param.int_value);
            }
            break;

            case SVC_EVT_CALL_DSCI_IND:
            {
                SVC_CALL_DsciInfo *dsci = event.param.ptr_param;

                APP_PRINT_INFO("APP_4G_Task DSCI_IND id[%d]dir[%d]stat[%d]type[%d]mpty[%d]num[%s]numtype[%d]\r\n", dsci->id, dsci->dir, dsci->stat, dsci->type, dsci->mpty, dsci->number, dsci->num_type);
            }
            break;

            case SVC_EVT_CALL_CEND_IND:
            {
                SVC_CALL_CendInfo *cend = event.param.ptr_param;

                APP_PRINT_INFO("APP_4G_Task CEND_IND id[%d]stat[%d]cause[%d]\r\n", cend->id, cend->end_status, cend->cc_cause);
            }
            break;

            case SVC_EVT_CALL_SET_ATD_RESULT:
            {
                APP_PRINT_INFO("APP_4G_Task ATD result:%d\r\n", event.param.int_value);
            }
            break;

            case SVC_EVT_CALL_SET_ATA_RESULT:
            {
                APP_PRINT_INFO("APP_4G_Task ATA result:%d\r\n", event.param.int_value);
            }
            break;

            case SVC_EVT_CALL_SET_CHUP_RESULT:
            {
                APP_PRINT_INFO("APP_4G_Task CHUP result:%d\r\n", event.param.int_value);
            }
            break;

            case SVC_EVT_USAT_PROACTIVE_COMMAND_IND:
            {
                #ifdef APP_BIP_SUPPORT
                SVC_USAT_ProactiveCommandInfo *info = event.param.ptr_param;
                int ret;
                APP_BIP_Event_Info event = {0};
                APP_PRINT_INFO("app usat proactive command length[%d]\r\n", info->dataLen);
                event.eventID = APP_BIP_USAT_PROACTIVE_COMMAND;
                event.ptrParam = info;

                APP_BIP_Init();
                ret = APP_BIP_EventSend(event, 0);
                if(ret == osOK)
                {
                    continue; // 消息转发
                }
                else
                {
                    APP_PRINT_ERROR("app usat proactive command trsend error %d\r\n",ret);
                }
                #else
                APP_PRINT_ERROR("app usat not support\r\n");
                #endif
            }
            break;
            case SVC_EVT_USAT_END_IND:
            {
                #ifdef APP_BIP_SUPPORT
                int ret;
                APP_BIP_Event_Info event = {0};
                APP_PRINT_INFO("app usat end ind\r\n");
                event.eventID = APP_BIP_USAT_END;
                event.ptrParam = NULL;

                ret = APP_BIP_EventSend(event, 0);
                if(ret == osOK)
                {
                    continue; // 消息转发
                }
                else
                {
                    APP_PRINT_ERROR("app usat end ind trsend error %d\r\n",ret);
                }
                #else
                APP_PRINT_ERROR("app usat not support\r\n");
                #endif
            }
            break;
            case SVC_EVT_USAT_SET_TERMINAL_RESPONSE_RESULT:
            {
                APP_PRINT_INFO("app usat terminal response result:%d\r\n", event.param.int_value);
            }
            break;
            case SVC_EVT_USAT_SET_ENVELOPE_COMMAND_RESULT:
            {
                APP_PRINT_INFO("app usat envelope command result:%d\r\n", event.param.int_value);
            }
            break;

            default:
            {
                APP_PRINT_INFO("APP_4G_Task recv_mq not support:%d\r\n", eventId);
            }
            break;
        }

        svc_event_free(&event);
    }
    return;
}

/**
 ************************************************************************************
 * @brief           开机自动搜网和拨号
 *
 * @param[in]       none
 *
 * @return
 ************************************************************************************
*/
void APP_4G_Start(void)
{
    osThreadId_t app_service;
    osThreadAttr_t attr = {"app_4g", osThreadDetached, NULL, 0U, NULL, APP_4G_TASK_STACK_SIZE, APP_4G_TASK_PRIORITY, 0U, 0U};
    osPrintf("APP_4G_Start [%d]-cfun1[%u]-auto_cid[%u]\r\n", amt_IsAmtMode(), APP_AutoCfun1Flag(), APP_4G_CONNECT_CID);
    if(amt_IsAmtMode() == FALSE) //  非AMT版本才启动
    {
        app_service = osThreadNew(APP_4G_Task, NULL, &attr);
        OS_ASSERT(app_service != NULL);
        APP_PRINT_INFO("APP_4G_init");
        osPrintf("APP_4G_init\r\n");
    }
}

void APP_4G_Init(void)
{
    int32_t ret;

    service_init();
    ret = APP_URC_AddHandler(svc_urc_callback);
    APP_PRINT_INFO("APP_4G_Init Urc add[%d]\r\n", ret);
    osPrintf("APP_4G_Init Urc add[%d]\r\n", ret);
}

#else // SVC_SUPPORT
/**
 ************************************************************************************
 * @brief           主动上报处理
 *
 * @param[in]       none
 *
 * @return
 ************************************************************************************
*/
static void APP_4G_StartUrcHandle(const char *cmd, uint32_t cmdLen)
{
    int ret;

    APP_PRINT_INFO("APP_4G_StartUrc [%d][%s]\r\n", cmdLen, cmd);

    if(at_parser_is_at_start_with((const char *)cmd, cmdLen, "\r\n+NETSTAT: "))
    {
        at_errno_t at_rlt = AT_ERRNO_NO_ERROR;
        uint8_t cid, state;

        at_rlt = at_parser_matched(cmd, cmdLen, "\r\n+NETSTAT: ", "%hhu,%hhu\r\n", &cid, &state);
        APP_PRINT_INFO("APP_4G_StartUrc NETSTAT parser %d\r\n", at_rlt);
        #ifndef USE_TOP_PPP  //  PPP和RNDIS不能共存
        if(at_rlt == AT_ERRNO_NO_ERROR)
        {
            if(state == 1 && cid == APP_4G_CONNECT_CID)
            {
                char netdevctl[25] = {0};
                osSnprintf(netdevctl, sizeof(netdevctl), "AT+NETDEVCTL=1,%d\r\n", APP_4G_CONNECT_CID);
                ret = AT_Client_SendCommand(netdevctl, strlen(netdevctl), 0, NULL, NULL);
                APP_PRINT_INFO("APP_4G_StartUrc send [%d][%s]\r\n", ret, netdevctl);
                osPrintf("APP_4G_StartUrc send [%d][%s]\r\n", ret, netdevctl);
            }
        }
        #endif
    }

    return;
}

/**
 ************************************************************************************
 * @brief           开机自动搜网和拨号
 *
 * @param[in]       none
 *
 * @return
 ************************************************************************************
*/
void APP_4G_Start(void)
{
    osPrintf("APP_4G_Start [%d]-cfun1[%u]-auto_cid[%u]\r\n", amt_IsAmtMode(), APP_AutoCfun1Flag(), APP_4G_CONNECT_CID);
    if(amt_IsAmtMode() == FALSE)
    {
        int32_t ret;
        char *cfun1 = "AT+CFUN=1\r\n";
        if(APP_4G_CONNECT_CID != 0)
        {
            //   注册主动上报处理自动发送AT+NETDEVCTL
            ret = APP_URC_AddHandler(APP_4G_StartUrcHandle);
            APP_PRINT_INFO("APP_4G_Start Urc add[%d]\r\n", ret);
            osPrintf("APP_4G_Start Urc add[%d]\r\n", ret);
        }

        if(APP_AutoCfun1Flag())
        {
            ret = AT_Client_SendCommand(cfun1, strlen(cfun1), 0, NULL, NULL);
            APP_PRINT_INFO("APP_4G_Start Send CFUN1 [%d]\r\n", ret);
            osPrintf("APP_4G_Start Send CFUN1 [%d]\r\n", ret);
        }
    }
    return;
}

void APP_4G_Init(void)
{
    return;
}
#endif // end SVC_SUPPORT

