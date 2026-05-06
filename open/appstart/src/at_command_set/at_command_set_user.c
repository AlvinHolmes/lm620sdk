/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file
 *
 * @brief 用户AT命令集
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
#include "stdint.h"

#include "os.h"
#include "at_api.h"
#include "at_parser.h"
#include "app_pub.h"
#include "app_urc_monitor.h"

#include "app_at_pub.h"
#if defined(AT_CMDSET_M)
    #ifdef AT_SOCKET
    #include "app_at_socket.h"
    #endif
    #ifdef AT_MQTT
    #include "app_at_mqtt.h"
    #endif
#endif // AT_CMDSET_M

#if defined(AT_CMDSET_Q)
    #ifdef AT_SOCKET
    #include "app_at_socket_qi.h"
    #endif
    #ifdef AT_MQTT
    #include "app_at_mqtt_qi.h"
    #endif
    #ifdef AT_SSL
    #include "app_at_ssl.h"
    #endif
    #ifdef AT_HTTP
    #include "app_at_http.h"
    #endif
    #ifdef AT_VFS
    #include "app_at_vfs.h"
    #endif
    #ifdef AT_FOTA
    #include "app_at_fota.h"
    #endif
#endif // AT_CMDSET_Q

#if defined(AT_CMDSET_I)
    #ifdef AT_SOCKET
    #include "app_at_socket_qi.h"
    #endif
    #ifdef AT_MQTT
    #include "app_at_mqtt_qi.h"
    #endif
    #ifdef AT_SSL
    #include "app_at_ssl.h"
    #endif
    #ifdef AT_HTTP
    #include "app_at_http.h"
    #endif
    #ifdef AT_VFS
    #include "app_at_vfs.h"
    #endif
    #ifdef AT_FOTA
    #include "app_at_fota.h"
    #endif
#endif // AT_CMDSET_I

#include "app_at_extend.h"
#include "app_at_pdp.h"
#include "app_at_cfg.h"
#include "app_at_ping.h"
#include "app_at_sntp.h"
#include "app_at_name_space.h"

/************************************************************************************
 *                                 外部函数声明
 ************************************************************************************/

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/

/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
static void APP_AT_BootHis_Exec(uint8_t channelId);

/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/
AT_CMD_TABLE_SECTION_USER  const AT_CommandItem g_atCmdTable_User[] =
{
#if defined(AT_CMDSET_M)
#ifdef AT_SOCKET
    {"+MIPCFG",             AT_SocketMipCfgSet,         OS_NULL,                    AT_SocketMipCfgTest,        OS_NULL},
    {"+MIPTKA",             AT_SocketMipTkaSet,         AT_SocketMipTkaRead,        AT_SocketMipTkaTest,        OS_NULL},
    {"+MIPOPEN",            AT_SocketMipOpenSet,        OS_NULL,                    AT_SocketMipOpenTest,       OS_NULL},
    {"+MIPCLOSE",           AT_SocketMipCloseSet,       OS_NULL,                    AT_SocketMipCloseTest,      OS_NULL},
    {"+MIPSEND",            AT_SocketMipSendSet,        OS_NULL,                    AT_SocketMipSendTest,       OS_NULL},
    {"+MIPRD",              AT_SocketMipReadSet,        OS_NULL,                    AT_SocketMipReadTest,       OS_NULL},
    {"+MIPMODE",            AT_SocketMipModeSet,        OS_NULL,                    AT_SocketMipModeTest,       OS_NULL},
    {"+MIPSTATE",           AT_SocketMipStateSet,       AT_SocketMipStateRead,      OS_NULL,                    OS_NULL},
    {"+MIPSACK",            AT_SocketMipSackSet,        OS_NULL,                    AT_SocketMipSackTest,       OS_NULL},
//{"+MDNSCFG",     OS_NULL,          OS_NULL,                  OS_NULL,        OS_NULL},
    //{"+MDNSGIP",     OS_NULL,          OS_NULL,                  OS_NULL,        OS_NULL},
    //{"+MNTP",     OS_NULL,          OS_NULL,                  OS_NULL,        OS_NULL},
    {"+MPING",              AT_MpingSet,                OS_NULL,                    AT_MpingTest,               OS_NULL},
#endif
#ifdef AT_MQTT
    /*MQTT*/
    {"+MQTTCFG",            AT_MqttCfgSet,              OS_NULL,                    AT_MqttCfgTest,             OS_NULL},
    {"+MQTTCONN",           AT_MqtConnSet,              OS_NULL,                    AT_MqttConnTest,            OS_NULL},
    {"+MQTTSUB",            AT_MqttSubSet,              OS_NULL,                    OS_NULL,                    OS_NULL},
    {"+MQTTUNSUB",          AT_MqttUnsubSet,            OS_NULL,                    OS_NULL,                    OS_NULL},
    {"+MQTTPUB",            AT_MqttPubSet,              OS_NULL,                    OS_NULL,                    OS_NULL},
    {"+MQTTREAD",           AT_MqttReadSet,             OS_NULL,                    OS_NULL,                    OS_NULL},
    {"+MQTTSTATE",          AT_MqttSateSet,             OS_NULL,                    OS_NULL,                    OS_NULL},
    {"+MQTTDISC",           AT_MqttDiscSet,             OS_NULL,                    OS_NULL,                    OS_NULL},
#endif
#endif //AT_CMDSET_M

#if defined(AT_CMDSET_Q)
#ifdef AT_SOCKET
    {AT_SOCKET_CFG,         AT_SocketQiCfgSet,          OS_NULL,                    AT_SocketQiCfgTest,         OS_NULL},
    {AT_SOCKET_OPEN,        AT_SocketQiOpenSet,         OS_NULL,                    AT_SocketQiOpenTest,        OS_NULL},
    {AT_SOCKET_CLOSE,       AT_SocketQiCloseSet,        OS_NULL,                    AT_SocketQiCloseTest,       OS_NULL},
    {AT_SOCKET_STATE,       AT_SocketQiStateSet,        AT_SocketQiStateRead,       AT_SocketQiStateTest,       AT_SocketQiStateExec},
    {AT_SOCKET_SEND,        AT_SocketQiSendSet,         OS_NULL,                    AT_SocketQiSendTest,        OS_NULL},
    {AT_SOCKET_READ,        AT_SocketQiReadSet,         OS_NULL,                    AT_SocketQiReadTest,        OS_NULL},
    {AT_SOCKET_SENDEX,      AT_SocketQiSendExSet,       OS_NULL,                    AT_SocketQiSendExTest,      OS_NULL},
    {AT_SOCKET_WTMD,        AT_SocketQiModeSet,         OS_NULL,                    AT_SocketQiModeTest,        OS_NULL},
    {AT_SOCKET_SDE,         AT_SocketQiSendEchoSet,     AT_SocketQiSendEchoRead,    AT_SocketQiSendEchoTest,    OS_NULL},
    {AT_SOCKET_GETERROR,    OS_NULL,                    OS_NULL,                    AT_SocketQiGetErrorTest,    AT_SocketQiGetErrorExec},
    {AT_X_PING,             AT_QpingSet,                OS_NULL,                    AT_QpingTest,               OS_NULL},
    {AT_X_NTP,              AT_INtpSet,                 AT_INtpRead,                AT_INtpTest,                OS_NULL},
    {AT_X_IDNSGIP,          APP_AT_X_IDNSGIP_Set,       OS_NULL,                    APP_AT_X_IDNSGIP_Test,      OS_NULL},
    {AT_X_IDNSCFG,          APP_AT_X_IDNSCFG_Set,       OS_NULL,                    APP_AT_X_IDNSCFG_Test,      OS_NULL},
    {AT_X_ICSGP,            APP_AT_X_ICSGP_Set,         OS_NULL,                    APP_AT_X_ICSGP_Test,        OS_NULL},
    {AT_X_IACT,             APP_AT_X_IACT_Set,          APP_AT_X_IACT_Read,         APP_AT_X_IACT_Test,         OS_NULL},
    {AT_X_IDEACT,           APP_AT_X_IDEACT_Set,        OS_NULL,                    APP_AT_X_IDEACT_Test,       OS_NULL},

#endif
#ifdef AT_MQTT
    {AT_MQTT_CFG,           AT_MqttQiCfgSet,            OS_NULL,                    AT_MqttQiCfgTest,           OS_NULL},
    {AT_MQTT_OPEN,          AT_MqttQiOpenSet,           AT_MqttQiOpenRead,          AT_MqttQiOpenTest,          OS_NULL},
    {AT_MQTT_CLOSE,         AT_MqttQiCloseSet,          OS_NULL,                    AT_MqttQiCloseTest,         OS_NULL},
    {AT_MQTT_CONN,          AT_MqttQiConnSet,           AT_MqttQiConnRead,          AT_MqttQiConnTest,          OS_NULL},
    {AT_MQTT_DISC,          AT_MqttQiDiscSet,           OS_NULL,                    AT_MqttQiDiscTest,          OS_NULL},
    {AT_MQTT_SUB,           AT_MqttQiSubSet,            OS_NULL,                    AT_MqttQiSubTest,           OS_NULL},
    {AT_MQTT_UNS,           AT_MqttQiUnsSet,            OS_NULL,                    AT_MqttQiUnsTest,           OS_NULL},
    {AT_MQTT_PUBEX,         AT_MqttQiPubexSet,          OS_NULL,                    AT_MqttQiPubexTest,         OS_NULL},
    {AT_MQTT_PUB,           AT_MqttQiPubSet,            OS_NULL,                    AT_MqttQiPubTest,           OS_NULL},
    {AT_MQTT_RECV,          AT_MqttQiRecvSet,           AT_MqttQiRecvRead,          AT_MqttQiRecvTest,          OS_NULL},
#endif
#ifdef AT_HTTP
    /*HTTP*/
    {AT_HTTP_CFG,           AT_MhttpCfgSet,             AT_MhttpCfgRead,            AT_MhttpCfgTest,            OS_NULL},
    {AT_HTTP_URL,           AT_MhttpUrlSet,             AT_MhttpUrlRead,            AT_MhttpUrlTest,            OS_NULL},
    {AT_HTTP_GET,           AT_MhttpGetSet,             OS_NULL,                    AT_MhttpGetTest,            AT_MhttpGetExec},
    {AT_HTTP_GETEX,         AT_MhttpGetExSet,           OS_NULL,                    AT_MhttpGetExTest,          OS_NULL},
    {AT_HTTP_POST,          AT_MhttpPostSet,            OS_NULL,                    AT_MhttpPostTest,           OS_NULL},
    {AT_HTTP_POSTFILE,      AT_MhttpPostFileSet,        OS_NULL,                    AT_MhttpPostFileTest,       OS_NULL},
    {AT_HTTP_PUT,           AT_MhttpPutSet,             OS_NULL,                    AT_MhttpPutTest,            OS_NULL},
    {AT_HTTP_PUTFILE,       AT_MhttpPutFileSet,         OS_NULL,                    AT_MhttpPutFileTest,        OS_NULL},
    {AT_HTTP_READ,          AT_MhttpReadSet,            OS_NULL,                    AT_MhttpReadTest,           AT_MhttpReadExec},
    {AT_HTTP_READFILE,      AT_MhttpReadFileSet,        OS_NULL,                    AT_MhttpReadFileTest,       OS_NULL},
    {AT_HTTP_CFGEX,         AT_MhttpCfgExSet,           OS_NULL,                    AT_MhttpCfgExTest,          OS_NULL},
    {AT_HTTP_SEND,          AT_MhttpSendSet,            OS_NULL,                    AT_MhttpSendTest,           AT_MhttpSendExec},
    {AT_HTTP_STOP,          OS_NULL,                    OS_NULL,                    AT_MhttpStopTest,           AT_MhttpStopExec},
#endif
#ifdef AT_SSL
    {AT_SSL_CFG,            AT_SslCfgSet,               OS_NULL,                    AT_SslCfgTest,              OS_NULL},
    {AT_SSL_OPEN,           AT_SslOpenSet,              OS_NULL,                    AT_SslOpenTest,             OS_NULL},
    {AT_SSL_SEND,           AT_SslSendSet,              OS_NULL,                    AT_SslSendTest,             OS_NULL},
    {AT_SSL_RECV,           AT_SslRecvSet,              OS_NULL,                    AT_SslRecvTest,             OS_NULL},
    {AT_SSL_CLOSE,          AT_SslCloseSet,             OS_NULL,                    AT_SslCloseTest,            OS_NULL},
    {AT_SSL_STATE,          AT_SslStateSet,             OS_NULL,                    AT_SslStateTest,            AT_SslStateExec},
#endif
#ifdef AT_VFS
    /* File System */
    {AT_DISKINFO,           AT_VFSDiskInfoSet,          OS_NULL,                    APP_AT_Test_OK,             AT_VFSDiskInfoExec},
    {AT_LISTFILE,           AT_VFSListFileSet,          OS_NULL,                    APP_AT_Test_OK,             AT_VFSListFileExec},
    {AT_DELETEFILE,         AT_VFSDeleteFileSet,        OS_NULL,                    AT_VFSDeleteFileTest,       OS_NULL},
    {AT_UPLOADFILE,         AT_VFSUploadFileSet,        AT_VFSUploadFileRead,       AT_VFSUploadFileTest,       OS_NULL},
    {AT_DWLOADFILE,         AT_VFSDownloadFileSet,      AT_VFSDownloadFileRead,     AT_VFSDownloadFileTest,     OS_NULL},
    {AT_OPENFILE,           AT_VFSOpenFileSet,          AT_VFSOpenFileRead,         AT_VFSOpenFileTest,         OS_NULL},
    {AT_READFILE,           AT_VFSReadFileSet,          OS_NULL,                    AT_VFSReadFileTest,         OS_NULL},
    {AT_WRITEFILE,          AT_VFSWriteFileSet,         OS_NULL,                    AT_VFSWriteFileTest,        OS_NULL},
    {AT_SEEKFILE,           AT_VFSSeekFileSet,          OS_NULL,                    AT_VFSSeekFileTest,         OS_NULL},
    {AT_FILEPOS,            AT_VFSOffsetFileSet,        OS_NULL,                    AT_VFSOffsetFileTest,       OS_NULL},
    {AT_TUCATFILE,          AT_VFSTruncateFileSet,      OS_NULL,                    AT_VFSTruncateFileTest,     OS_NULL},
    {AT_CLOSEFILE,          AT_VFSCloseFileSet,         OS_NULL,                    AT_VFSCloseFileTest,        OS_NULL},
    {AT_MOVEFILE,           AT_VFSMoveFileSet,          OS_NULL,                    AT_VFSMoveFileTest,         OS_NULL},
    {AT_FLUSHFILE,          AT_VFSFlushFileSet,         OS_NULL,                    AT_VFSFlushFileTest,        OS_NULL},
    {AT_CHANGEDIR,          AT_VFSChangeDirSet,         OS_NULL,                    AT_VFSChangeDirTest,        OS_NULL},
    {AT_MAKEDIR,            AT_VFSMakeDirSet,           OS_NULL,                    AT_VFSMakeDirTest,          OS_NULL},
    {AT_CURRDIR,            AT_VFSPwdSet,               OS_NULL,                    AT_VFSPwdTest,              AT_VFSPwdExec},
#endif
#ifdef AT_FOTA
    {AT_QFOTADL,            AT_FOTASet,                 AT_FOTARead,                APP_AT_Test_OK,             OS_NULL},
#endif
    {AT_X_GSN,              OS_NULL,                    OS_NULL,                    APP_AT_Test_OK,             APP_AT_X_GSN_Exec},
    {AT_X_CCID,             OS_NULL,                    OS_NULL,                    APP_AT_Test_OK,             APP_AT_X_CCID_Exec},
    {AT_X_NWINFO,           OS_NULL,                    OS_NULL,                    APP_AT_Test_OK,             APP_AT_X_NWINFO_Exec},
    {AT_X_ENG,              APP_AT_X_ENG_Set,           OS_NULL,                    APP_AT_X_ENG_Test,          OS_NULL},
    {AT_X_CELL,             OS_NULL,                    APP_AT_X_CELL_Read,         APP_AT_Test_OK,             OS_NULL},
    {AT_X_CELLEX,           APP_AT_X_CELLEX_Set,        OS_NULL,                    APP_AT_Test_OK,             OS_NULL},
    {AT_X_SCLK,             APP_AT_X_SCLK_Set,          APP_AT_X_SCLK_Read,         APP_AT_X_SCLK_Test,         OS_NULL},
    {AT_X_URCCFG,           APP_AT_X_URCCFG_Set,        OS_NULL,                    APP_AT_X_URCCFG_Test,       OS_NULL},
    {AT_X_CFG,              APP_AT_X_CFG_Set,           OS_NULL,                    APP_AT_X_CFG_Test,          OS_NULL},
    {AT_X_POWD,             APP_AT_SHUTDOWN_Set,        OS_NULL,                    APP_AT_SHUTDOWN_Test,       APP_AT_SHUTDOWN_Exec},
    {AT_X_LTS,              APP_AT_X_LTS_Set,           OS_NULL,                    APP_AT_X_LTS_Test,          APP_AT_X_LTS_Exec},
#endif // AT_CMDSET_Q

#if defined(AT_CMDSET_I)
#ifdef AT_SOCKET
    {AT_SOCKET_CFG,         AT_SocketQiCfgSet,          OS_NULL,                    AT_SocketQiCfgTest,         OS_NULL},
    {AT_SOCKET_OPEN,        AT_SocketQiOpenSet,         OS_NULL,                    AT_SocketQiOpenTest,        OS_NULL},
    {AT_SOCKET_CLOSE,       AT_SocketQiCloseSet,        OS_NULL,                    AT_SocketQiCloseTest,       OS_NULL},
    {AT_SOCKET_STATE,       AT_SocketQiStateSet,        AT_SocketQiStateRead,       AT_SocketQiStateTest,       AT_SocketQiStateExec},
    {AT_SOCKET_SEND,        AT_SocketQiSendSet,         OS_NULL,                    AT_SocketQiSendTest,        OS_NULL},
    {AT_SOCKET_READ,        AT_SocketQiReadSet,         OS_NULL,                    AT_SocketQiReadTest,        OS_NULL},
    {AT_SOCKET_SENDEX,      AT_SocketQiSendExSet,       OS_NULL,                    AT_SocketQiSendExTest,      OS_NULL},
    {AT_SOCKET_WTMD,        AT_SocketQiModeSet,         OS_NULL,                    AT_SocketQiModeTest,        OS_NULL},
    {AT_SOCKET_SDE,         AT_SocketQiSendEchoSet,     AT_SocketQiSendEchoRead,    AT_SocketQiSendEchoTest,    OS_NULL},
    {AT_SOCKET_GETERROR,    OS_NULL,                    OS_NULL,                    AT_SocketQiGetErrorTest,    AT_SocketQiGetErrorExec},
    {AT_X_PING,             AT_QpingSet,                OS_NULL,                    AT_QpingTest,               OS_NULL},
    {AT_X_NTP,              AT_INtpSet,                 AT_INtpRead,                AT_INtpTest,                OS_NULL},
    {AT_X_IDNSGIP,          APP_AT_X_IDNSGIP_Set,       OS_NULL,                    APP_AT_X_IDNSGIP_Test,      OS_NULL},
    {AT_X_IDNSCFG,          APP_AT_X_IDNSCFG_Set,       OS_NULL,                    APP_AT_X_IDNSCFG_Test,      OS_NULL},
    {AT_X_ICSGP,            APP_AT_X_ICSGP_Set,         OS_NULL,                    APP_AT_X_ICSGP_Test,        OS_NULL},
    {AT_X_IACT,             APP_AT_X_IACT_Set,          APP_AT_X_IACT_Read,         APP_AT_X_IACT_Test,         OS_NULL},
    {AT_X_IDEACT,           APP_AT_X_IDEACT_Set,        OS_NULL,                    APP_AT_X_IDEACT_Test,       OS_NULL},

#endif
#ifdef AT_MQTT
    {AT_MQTT_CFG,           AT_MqttQiCfgSet,            OS_NULL,                    AT_MqttQiCfgTest,           OS_NULL},
    {AT_MQTT_OPEN,          AT_MqttQiOpenSet,           AT_MqttQiOpenRead,          AT_MqttQiOpenTest,          OS_NULL},
    {AT_MQTT_CLOSE,         AT_MqttQiCloseSet,          OS_NULL,                    AT_MqttQiCloseTest,         OS_NULL},
    {AT_MQTT_CONN,          AT_MqttQiConnSet,           AT_MqttQiConnRead,          AT_MqttQiConnTest,          OS_NULL},
    {AT_MQTT_DISC,          AT_MqttQiDiscSet,           OS_NULL,                    AT_MqttQiDiscTest,          OS_NULL},
    {AT_MQTT_SUB,           AT_MqttQiSubSet,            OS_NULL,                    AT_MqttQiSubTest,           OS_NULL},
    {AT_MQTT_UNS,           AT_MqttQiUnsSet,            OS_NULL,                    AT_MqttQiUnsTest,           OS_NULL},
    {AT_MQTT_PUBEX,         AT_MqttQiPubexSet,          OS_NULL,                    AT_MqttQiPubexTest,         OS_NULL},
    {AT_MQTT_PUB,           AT_MqttQiPubSet,            OS_NULL,                    AT_MqttQiPubTest,           OS_NULL},
    {AT_MQTT_RECV,          AT_MqttQiRecvSet,           AT_MqttQiRecvRead,          AT_MqttQiRecvTest,          OS_NULL},
#endif
#ifdef AT_HTTP
    /*HTTP*/
    {AT_HTTP_CFG,           AT_MhttpCfgSet,             AT_MhttpCfgRead,            AT_MhttpCfgTest,            OS_NULL},
    {AT_HTTP_URL,           AT_MhttpUrlSet,             AT_MhttpUrlRead,            AT_MhttpUrlTest,            OS_NULL},
    {AT_HTTP_GET,           AT_MhttpGetSet,             OS_NULL,                    AT_MhttpGetTest,            AT_MhttpGetExec},
    {AT_HTTP_GETEX,         AT_MhttpGetExSet,           OS_NULL,                    AT_MhttpGetExTest,          OS_NULL},
    {AT_HTTP_POST,          AT_MhttpPostSet,            OS_NULL,                    AT_MhttpPostTest,           OS_NULL},
    {AT_HTTP_POSTFILE,      AT_MhttpPostFileSet,        OS_NULL,                    AT_MhttpPostFileTest,       OS_NULL},
    {AT_HTTP_PUT,           AT_MhttpPutSet,             OS_NULL,                    AT_MhttpPutTest,            OS_NULL},
    {AT_HTTP_PUTFILE,       AT_MhttpPutFileSet,         OS_NULL,                    AT_MhttpPutFileTest,        OS_NULL},
    {AT_HTTP_READ,          AT_MhttpReadSet,            OS_NULL,                    AT_MhttpReadTest,           AT_MhttpReadExec},
    {AT_HTTP_READFILE,      AT_MhttpReadFileSet,        OS_NULL,                    AT_MhttpReadFileTest,       OS_NULL},
    {AT_HTTP_CFGEX,         AT_MhttpCfgExSet,           OS_NULL,                    AT_MhttpCfgExTest,          OS_NULL},
    {AT_HTTP_SEND,          AT_MhttpSendSet,            OS_NULL,                    AT_MhttpSendTest,           AT_MhttpSendExec},
    {AT_HTTP_STOP,          OS_NULL,                    OS_NULL,                    AT_MhttpStopTest,           AT_MhttpStopExec},
#endif
#ifdef AT_SSL
    {AT_SSL_CFG,            AT_SslCfgSet,               OS_NULL,                    AT_SslCfgTest,              OS_NULL},
    {AT_SSL_OPEN,           AT_SslOpenSet,              OS_NULL,                    AT_SslOpenTest,             OS_NULL},
    {AT_SSL_SEND,           AT_SslSendSet,              OS_NULL,                    AT_SslSendTest,             OS_NULL},
    {AT_SSL_RECV,           AT_SslRecvSet,              OS_NULL,                    AT_SslRecvTest,             OS_NULL},
    {AT_SSL_CLOSE,          AT_SslCloseSet,             OS_NULL,                    AT_SslCloseTest,            OS_NULL},
    {AT_SSL_STATE,          AT_SslStateSet,             OS_NULL,                    AT_SslStateTest,            AT_SslStateExec},
#endif
#ifdef AT_VFS
    /* File System */
    {AT_DISKINFO,           AT_VFSDiskInfoSet,          OS_NULL,                    APP_AT_Test_OK,             AT_VFSDiskInfoExec},
    {AT_LISTFILE,           AT_VFSListFileSet,          OS_NULL,                    APP_AT_Test_OK,             AT_VFSListFileExec},
    {AT_DELETEFILE,         AT_VFSDeleteFileSet,        OS_NULL,                    AT_VFSDeleteFileTest,       OS_NULL},
    {AT_UPLOADFILE,         AT_VFSUploadFileSet,        AT_VFSUploadFileRead,       AT_VFSUploadFileTest,       OS_NULL},
    {AT_DWLOADFILE,         AT_VFSDownloadFileSet,      AT_VFSDownloadFileRead,     AT_VFSDownloadFileTest,     OS_NULL},
    {AT_OPENFILE,           AT_VFSOpenFileSet,          AT_VFSOpenFileRead,         AT_VFSOpenFileTest,         OS_NULL},
    {AT_READFILE,           AT_VFSReadFileSet,          OS_NULL,                    AT_VFSReadFileTest,         OS_NULL},
    {AT_WRITEFILE,          AT_VFSWriteFileSet,         OS_NULL,                    AT_VFSWriteFileTest,        OS_NULL},
    {AT_SEEKFILE,           AT_VFSSeekFileSet,          OS_NULL,                    AT_VFSSeekFileTest,         OS_NULL},
    {AT_FILEPOS,            AT_VFSOffsetFileSet,        OS_NULL,                    AT_VFSOffsetFileTest,       OS_NULL},
    {AT_TUCATFILE,          AT_VFSTruncateFileSet,      OS_NULL,                    AT_VFSTruncateFileTest,     OS_NULL},
    {AT_CLOSEFILE,          AT_VFSCloseFileSet,         OS_NULL,                    AT_VFSCloseFileTest,        OS_NULL},
    {AT_MOVEFILE,           AT_VFSMoveFileSet,          OS_NULL,                    AT_VFSMoveFileTest,         OS_NULL},
    {AT_FLUSHFILE,          AT_VFSFlushFileSet,         OS_NULL,                    AT_VFSFlushFileTest,        OS_NULL},
    {AT_CHANGEDIR,          AT_VFSChangeDirSet,         OS_NULL,                    AT_VFSChangeDirTest,        OS_NULL},
    {AT_MAKEDIR,            AT_VFSMakeDirSet,           OS_NULL,                    AT_VFSMakeDirTest,          OS_NULL},
    {AT_CURRDIR,            AT_VFSPwdSet,               OS_NULL,                    AT_VFSPwdTest,              AT_VFSPwdExec},
#endif
#ifdef AT_FOTA
    {AT_QFOTADL,            AT_FOTASet,                 AT_FOTARead,                APP_AT_Test_OK,             OS_NULL},
#endif
    #if 1 //纯Q命令
    {AT_X_GSN,              OS_NULL,                    OS_NULL,                    APP_AT_Test_OK,             APP_AT_X_GSN_Exec},
    {AT_X_CCID,             OS_NULL,                    OS_NULL,                    APP_AT_Test_OK,             APP_AT_X_CCID_Exec},
    {AT_X_NWINFO,           OS_NULL,                    OS_NULL,                    APP_AT_Test_OK,             APP_AT_X_NWINFO_Exec},
    {AT_X_ENG,              APP_AT_X_ENG_Set,           OS_NULL,                    APP_AT_X_ENG_Test,          OS_NULL},
    {AT_X_CELL,             OS_NULL,                    APP_AT_X_CELL_Read,         APP_AT_Test_OK,            OS_NULL},
    {AT_X_CELLEX,           APP_AT_X_CELLEX_Set,        OS_NULL,                    APP_AT_Test_OK,             OS_NULL},
    //{AT_X_SCLK,             APP_AT_X_SCLK_Set,          APP_AT_X_SCLK_Read,         APP_AT_X_SCLK_Test,         OS_NULL},
    {AT_X_URCCFG,           APP_AT_X_URCCFG_Set,        OS_NULL,                    APP_AT_X_URCCFG_Test,       OS_NULL},
    {AT_X_CFG,              APP_AT_X_CFG_Set,           OS_NULL,                    APP_AT_X_CFG_Test,          OS_NULL},
    #endif
    {AT_X_POWD,             APP_AT_SHUTDOWN_Set,        OS_NULL,                    APP_AT_SHUTDOWN_Test,       APP_AT_SHUTDOWN_Exec},
    {AT_X_LTS,              APP_AT_X_LTS_Set,           OS_NULL,                    APP_AT_X_LTS_Test,          APP_AT_X_LTS_Exec},
#endif // AT_CMDSET_I

/***    通用AT命令 Begin        ***/
    {"I",                   OS_NULL,                    OS_NULL,                    OS_NULL,                    APP_AT_ATI_Exec},
    {"+GMR",                OS_NULL,                    OS_NULL,                    APP_AT_Test_OK,             APP_AT_GMR_Exec},
    {"+CGMR",               OS_NULL,                    OS_NULL,                    APP_AT_Test_OK,             APP_AT_GMR_Exec},
    {"+GMI",                OS_NULL,                    OS_NULL,                    APP_AT_Test_OK,             APP_AT_GMI_Exec},
    {"+CGMI",               OS_NULL,                    OS_NULL,                    APP_AT_Test_OK,             APP_AT_GMI_Exec},
    {"+GMM",                OS_NULL,                    OS_NULL,                    APP_AT_Test_OK,             APP_AT_GMM_Exec},
    {"+CGMM",               OS_NULL,                    OS_NULL,                    APP_AT_Test_OK,             APP_AT_GMM_Exec},
    {"+IGMR",               OS_NULL,                    OS_NULL,                    APP_AT_Test_OK,             APP_AT_IGMR_Exec},

    {"+IPR",                APP_AT_IPR_Set,             APP_AT_IPR_Read,            APP_AT_IPR_Test,            OS_NULL},
    {"+CCLK",               APP_AT_CCLK_Set,            APP_AT_CCLK_Read,           APP_AT_Test_OK,             OS_NULL},
    {"+RAMDUMP",            APP_AT_RAMDUMP_Set,         OS_NULL,                    APP_AT_Test_OK,             APP_AT_RAMDUMP_Exec},
    {"+REBOOT",             APP_AT_REBOOT_Set,          OS_NULL,                    APP_AT_Test_OK,             APP_AT_REBOOT_Exec},
    {"+IADC",               APP_AT_XADC_Set,            OS_NULL,                    APP_AT_XADC_Test,           OS_NULL},
    {"+BOOTHIS",            OS_NULL,                    OS_NULL,                    APP_AT_Test_OK,             APP_AT_BootHis_Exec},
/***    通用AT命令 End        ***/
};

/************************************************************************************
 *                                 外部函数定义
 ************************************************************************************/

/**
 ************************************************************************************
 * @brief           初始化AT命令功能集
 *
 * @param[in]       void
 *
 * @return          none
 ************************************************************************************
*/
void APP_AT_Function_Init(void)
{
#if defined(AT_SOCKET) || defined(AT_HTTP) || defined(AT_MQTT) || defined(AT_SSL)
        int ret;
        ret = APP_URC_AddHandler(APP_AT_UrcCallback);
        OS_ASSERT(ret == osOK);
#endif
#if defined(AT_CMDSET_M)
    #if defined(AT_SOCKET)
        AT_SocketInit();
    #endif
    #if defined(AT_MQTT)
        APP_MqttInit();
    #endif
#endif  //AT_CMDSET_M

#if defined(AT_CMDSET_Q)
    #if defined(AT_SOCKET)
        AT_SocketQiInit();
    #endif
    #if defined(AT_HTTP)
        AT_HttpInit();
    #endif
    #if defined(AT_MQTT)
        APP_MqttQiInit();
    #endif
    #if defined(AT_SSL)
        AT_SslInit();
    #endif
#endif  //AT_CMDSET_Q

#if defined(AT_CMDSET_I)
    #if defined(AT_SOCKET)
        AT_SocketQiInit();
    #endif
    #if defined(AT_HTTP)
        AT_HttpInit();
    #endif
    #if defined(AT_MQTT)
        APP_MqttQiInit();
    #endif
    #if defined(AT_SSL)
        AT_SslInit();
    #endif
#endif  //AT_CMDSET_I

}

extern int32_t APP_GetBootHistory(char* buf, uint32_t len);
static void APP_AT_BootHis_Exec(uint8_t channelId)
{
    char buf[64+1];
    char at_rsp[80] = {0};
    const char *rsp_format = "\r\n%s\r\n";

    if (APP_GetBootHistory(buf, sizeof(buf)) == 0) {
        APP_AT_SNPRINTF(at_rsp, sizeof(at_rsp), rsp_format, buf);
        AT_SendResponse(channelId, at_rsp, strlen(at_rsp));
        AT_SendResponseOK(channelId);
    } else {
        AT_SendResponseError(channelId);
    }
}


