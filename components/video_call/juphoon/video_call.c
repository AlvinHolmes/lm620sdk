
#include "os.h"
#include "stddef.h"
#include "stdint.h"
#include "stdlib.h"
#include "drv_rtc.h"
#include "jrtc.h"
#include "jrtc0.h"
#include "video_call.h"

#include "mbedtls/md.h"
#include "mbedtls/md_internal.h"
#include "mbedtls/base64.h"

#ifdef USE_SVC
#include "svc_settings.h"
#include "svc_video_call.h"
#endif

#ifdef USE_WATCH_SERVER_PROTOCOL
#include "watch_protocol_main.h"
#endif

#define TEST_VIDEO

#define VIDEO_DEBUG_PRINTF
#ifdef  VIDEO_DEBUG_PRINTF
    #define VIDEO_PRINTF        osPrintf
#else
    #define VIDEO_PRINTF(...)
#endif 

#define     TOKEN_MAX_LEN       128

const char *appKey = "ee022fc8a44a960262395096";    // AppKey
const char *AppSecret = "4xv6kz3ix2s8mf0vbyrt";
const char *license = "imei-961580018912383757";    // license key
const char *aesKey = "4K112Ygy7o3Q1857";            // 通讯密钥
const char *testServerUrl="rtos.jrtc.juphooncloud.com:2199";     // 测试服务器          // SDK 2.0 对接
//const char *testServerUrl = "cn.iot.justalkcloud.com:2198;119.3.204.221:2198";      // SDK 1.0 对接
const char *devName = "ict-dev";
const char *chnlId = "123456789";

#ifdef USE_SVC
static char *appKey = NULL;            // AppKey
static char *AppSecret = NULL;
static char *license = NULL;           // license key
static char *aesKey = NULL;            // 通讯密钥
static char *testServerUrl = NULL;     // 测试服务器
static char *devName = NULL;
#endif

#ifdef TEST_VIDEO
#include "drv_display.h"

static osCompletion g_dispCmpl = {0};
static DispDevice_t *g_dispDev = NULL;
static DispOps_t *g_dispOps = NULL;
static osThreadId_t g_videoUITask = NULL;
static uint8_t *g_ImageBuf = NULL;
#endif

static struct jrtc_handler_t g_rtcCallBackHandler = {0};
static struct jrtc_t *g_jc = NULL;
static uint32_t g_joinIndex = 0;
static uint32_t g_remoteUserStatus = 0xff;

static Video_CallInfo *g_CallInfo = NULL;
static char *g_token = NULL;
static struct jrtc_image_t g_recvVideoCfg = {0};
static struct jrtc_image_t g_sendVideoCfg = {0};

static int8_t g_VideoCallType = VIDEO_CALL_TYPE_CLOSE;

int8_t Video_GetType(void)
{
    return g_VideoCallType;
}

Video_CallInfo *Video_GetCallInfo(void)
{
    return g_CallInfo;
}

void Video_SetCallInfo(char *remoteId)
{
    if(!g_CallInfo)
        g_CallInfo = (Video_CallInfo *)osMalloc(sizeof(Video_CallInfo));
    
    strcpy(g_CallInfo->remoteUserId, remoteId);

    uint8_t len = sizeof(g_CallInfo->localUserId);
    memset(g_CallInfo->localUserId, 0, len);
#ifdef USE_WATCH_SERVER_PROTOCOL
    Watch_VideoGetUserId(g_CallInfo->localUserId, &len);
#endif
}

void Video_GetRecvImgInfo(uint16_t *imgH, uint16_t *imgW)
{
    *imgH = g_recvVideoCfg.height;
    *imgW = g_recvVideoCfg.width;
}

void Video_GetSendImgInfo(uint16_t *imgH, uint16_t *imgW)
{
    *imgH = g_sendVideoCfg.height;
    *imgW = g_sendVideoCfg.width;
}

static int on_user_joined (struct jrtc_t* jc, char uid[64], unsigned role, unsigned status)
{
    VIDEO_PRINTF("on_user_joined uid: %s, role: %d, status: %d \r\n", uid, role, status);
    g_joinIndex ++;
    VIDEO_PRINTF("g_joinIndex: %d \r\n", g_joinIndex);

#ifdef USE_WATCH_SERVER_PROTOCOL
    if(strcmp(uid, g_CallInfo->localUserId) == 0)
#endif
    {
    #ifdef USE_SVC
        svc_video_call_event_accept();
    #endif
        jrtc_set_video(g_jc, uid, g_joinIndex);
    }
    return g_joinIndex;
}

static int on_user_changed (struct jrtc_t* jc, char uid[64], int index, unsigned role, unsigned status)
{
    VIDEO_PRINTF("user changed uid: %s, index: %d, role: %d, status: %d\r\n", uid, index, role, status);

    if(strcmp(uid, g_CallInfo->remoteUserId) == 0)
        g_remoteUserStatus = status;
        
    return 0;
}

static void on_user_offline (struct jrtc_t* jc, char uid[64], int index, enum jrtc_error reason)
{
#ifdef USE_SVC
    svc_video_call_event_end();
#endif
    VIDEO_PRINTF("on_user_offline uid: %s, index: %d, reason: %d \r\n", uid, index, reason);
}

static void on_user_message (struct jrtc_t* jc, char uid[64], int index, struct jrtc_slice_t msg[], unsigned num)
{
    VIDEO_PRINTF("user message uid: %s index: %d\r\n", uid, index);
    // uint64_t baseaddr = *(uint64_t *)uid;

    // for(uint32_t i = 0; i <= num; i++)
    // {
    //     VIDEO_PRINTF("message[%d]: %s \r\n", i, (uint8_t *)(baseaddr + msg[i].offset));
    // }
}

static void on_video_changed (struct jrtc_t* jc, char vid[64], int index)
{
    VIDEO_PRINTF("on_video_changed vid: %s, index: %d \r\n", vid, index);
}

static void on_share_changed (struct jrtc_t* jc, char uid[64], int index)
{
    VIDEO_PRINTF("on_share_changed uid: %s, index: %d \r\n", uid, index);
}

static void on_audio_volume (struct jrtc_t* jc, unsigned num, unsigned char index_volume[][2])
{
    if(index_volume == NULL && num == 0)
    {
        VIDEO_PRINTF("on_audio_volume, first get audio data \r\n");

    }
    else
    {
        for(uint32_t i = 0; i < num; i++)
        {
            VIDEO_PRINTF("index_volume: %x, %x \r\n",index_volume[i][0],index_volume[i][1]);
        }
    }
}

static void on_user_netstate (struct jrtc_t* jc, unsigned num, unsigned char index_netstate[][2])
{
    for(uint32_t i = 0; i < num; i++)
    {
        VIDEO_PRINTF("index_netstate: %x, %x \r\n",index_netstate[i][0],index_netstate[i][1]);
    }
}

static uint8_t byte1(uint8_t input, uint8_t salt)
{
    return (input ^ salt);
}

static uint32_t byte4(int32_t input, uint8_t salt)
{
    uint32_t salt32 = (salt << 24) + (salt << 16) + (salt << 8) + salt;
    uint32_t result = (salt32 ^ input);
    return result;
}

#define POLY_CRC32 0xEDB88320
static uint32_t crc32(const char *data, size_t length) {
    uint32_t crc = 0xFFFFFFFF; // 初始值
    size_t i, j;

    for (i = 0; i < length; ++i) {
        crc ^= (uint32_t)data[i]; // 与当前字节异或
        for (j = 0; j < 8; ++j) {
            if (crc & 1) { // 如果最低位是1
                crc = (crc >> 1) ^ POLY_CRC32; // 右移一位，并与多项式异或
            } else {
                crc >>= 1; // 否则只右移一位
            }
        }
    }
    // 返回补码形式的CRC值
    return ~crc;
}

static void Video_GenToken(const char *AppKey, const char *AppSecret, const char *userId, const char *chnlId, char *Token)
{
    char *tmpArray = osMalloc(TOKEN_MAX_LEN);
    char *Payload = osMalloc(TOKEN_MAX_LEN);
    char *Signature = osMalloc(TOKEN_MAX_LEN);
    char *AccountID = osMalloc(TOKEN_MAX_LEN);
    uint8_t index = 0;
    uint32_t tmpVal = 0;
    uint8_t tokenIndex = 0;

    memset(tmpArray, 0 , TOKEN_MAX_LEN);
    memset(Payload, 0, TOKEN_MAX_LEN);
    memset(Signature, 0, TOKEN_MAX_LEN);
    memset(AccountID, 0, TOKEN_MAX_LEN);

    strcpy(AccountID, userId);
    strcpy(&AccountID[strlen(userId)], ":");
    strcpy(&AccountID[strlen(userId) + 1], chnlId);

/////PayLoad
    //<随机生成的数，范围 1-254>
    uint8_t Salt = (rand() % 253) + 1;  
    //<过期时间的UNIX时间戳，默认是当前时间 + 24小时>
    RTC_Time Timestamp =  0; 
    Timestamp = RTC_GetTime();

    Timestamp += 24 * 3600;
    //salt
    tmpArray[index++] = byte1(Salt, 0);
    //Timestamp
    tmpVal = byte4(Timestamp, Salt);
    tmpArray[index++] = (uint8_t)(tmpVal >> 24);
    tmpArray[index++] = (uint8_t)(tmpVal >> 16);
    tmpArray[index++] = (uint8_t)(tmpVal >> 8);
    tmpArray[index++] = (uint8_t)(tmpVal);
    //appkey
    tmpVal = byte4(crc32(AppKey, strlen(AppKey)), Salt);
    tmpArray[index++] = (uint8_t)(tmpVal >> 24);
    tmpArray[index++] = (uint8_t)(tmpVal >> 16);
    tmpArray[index++] = (uint8_t)(tmpVal >> 8);
    tmpArray[index++] = (uint8_t)(tmpVal);
    //accountID
    tmpArray[index++] = byte1(strlen(AccountID), Salt);
    for(uint8_t i = 0; i < strlen(AccountID); i++)
        tmpArray[index++] = byte1(AccountID[i], Salt);
    
    size_t PayloadLen = 0; 
    mbedtls_base64_encode((unsigned char *)Payload, 128, &PayloadLen, (unsigned char *)tmpArray, index);

////Generate Signature
    int ret = 0;
    mbedtls_md_context_t hmac_ctx;
    mbedtls_md_init(&hmac_ctx);
    const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (md_info == OS_NULL)
    {
        osPrintf("Step-3-1: ERROR can not support md_type[%u]", MBEDTLS_MD_SHA256);
        goto TOKEN_EXIT;
    }

    ret = mbedtls_md_setup(&hmac_ctx, md_info, 1);
    if (ret != 0)
    {
        osPrintf("Step-3-2: ERR mbedtls_md_setup ret[%d] \r\n", ret);
    }

    ret = mbedtls_md_hmac_starts(&hmac_ctx, (const unsigned char *)AppSecret, strlen(AppSecret));
    if (ret != 0)
    {
        osPrintf("Step-3-3: ERR mbedtls_md_hmac_starts ret[%d] \r\n", ret);
    }

    ret = mbedtls_md_hmac_update(&hmac_ctx, (const unsigned char *)Payload, PayloadLen);
    if (ret != 0)
    {
        osPrintf("Step-3-4: ERR mbedtls_md_hmac_update ret[%d] \r\n", ret);
    }

    char hmac_sign_buf[64] = {0};
    ret= mbedtls_md_hmac_finish(&hmac_ctx, (unsigned char *)hmac_sign_buf);
    osPrintf("Step3 do hmac_sha1: mbedtls_md_hmac_finish ret[%d]hmac_sign_buf[%u] \r\n", ret, strlen(hmac_sign_buf));
    mbedtls_md_free(&hmac_ctx);

    size_t SignatureLen = 0;
    mbedtls_base64_encode((unsigned char *)Signature, 128, &SignatureLen, (unsigned char *)hmac_sign_buf, strlen(hmac_sign_buf));

//// Merge
    /////Ver 01
    Token[tokenIndex++] = '0';
    Token[tokenIndex++] = '1';

    for(uint8_t i = 0; i < 6; i++)
        Token[tokenIndex++] = AppSecret[i];

    strcpy(&Token[tokenIndex], Signature);
    tokenIndex += SignatureLen;

    memcpy(&Token[tokenIndex], Payload, PayloadLen);

TOKEN_EXIT:
    if(tmpArray)
        osFree(tmpArray);
    if(Payload)
        osFree(Payload);
    if(Signature)
        osFree(Signature);
    if(AccountID)
        osFree(AccountID);
}

#ifdef TEST_VIDEO
static void Video_DispCallBack(void *userData)
{
    if(userData) {
        osComplete(userData);
    }
}

static void Video_UITask(void *para)
{
    VIDEO_PRINTF("Video UI task \r\n");
    g_dispOps->endianConvert(g_dispDev, true);

    uint32_t id = g_dispOps->id(g_dispDev);
    VIDEO_PRINTF("get disp id = 0x%08x", id);
    while (1)
    {
        struct jrtc_image_t *recvImg = &g_recvVideoCfg;

        if(jrtc_state(g_jc) >= JRTC_JOINED)
        {
            if(recvImg->get != recvImg->put)
            {
                recvImg->get = recvImg->put;

                memcpy(g_ImageBuf, recvImg->data, recvImg->bytes);

                g_dispOps->flush(g_dispDev, 0, g_dispDev->horRes - 1, 0, g_dispDev->verRes - 1, g_ImageBuf);
                osWaitForCompletion(&g_dispCmpl, osWaitForever);
            }
        }
        osThreadMsSleep(20);
    } 
}
#endif

int8_t Video_Open(char *remoteId)
{
    if(!remoteId)
    {
        osPrintf("remoteId is NULL \r\n");
        return -1;
    }

    if(!g_CallInfo)
        g_CallInfo = (Video_CallInfo *)osMalloc(sizeof(Video_CallInfo));

    if(!g_CallInfo)
    {
        osPrintf("g_CallInfo malloc failed \r\n");
        return -1;
    }

    g_token = (char *)osMalloc(TOKEN_MAX_LEN);
    if(!g_token)
    {
        osPrintf("g_token malloc failed \r\n");
        return -1;
    }

    memset(g_token, 0, TOKEN_MAX_LEN);

    memset(g_CallInfo, 0, sizeof(Video_CallInfo));
    
    // char *chnlId = NULL;
    // chnlId = svc_video_contact_search_chnl(remoteId);
    // OS_ASSERT(chnlId);
#ifdef USE_SVC
    if(!testServerUrl)
    {
        testServerUrl = osMalloc(MAX_VIDEO_STR_LEN);
        OS_ASSERT(testServerUrl);
    }

    if(!AppSecret)
    {
        AppSecret = osMalloc(MAX_VIDEO_STR_LEN);
        OS_ASSERT(AppSecret);
    }

    if(!license)
    {
        license = osMalloc(MAX_VIDEO_STR_LEN);
        OS_ASSERT(license);
    }

    if(!aesKey)
    {
        aesKey = osMalloc(MAX_VIDEO_STR_LEN);
        OS_ASSERT(aesKey);
    }

    if(!appKey)
    {
        appKey = osMalloc(MAX_VIDEO_STR_LEN);
        OS_ASSERT(appKey);
    }

    if(!devName)
    {
        devName = osMalloc(MAX_VIDEO_STR_LEN);
        OS_ASSERT(devName);
    }
#endif

    g_joinIndex = 0;
#ifdef USE_SVC
    SvcSet_Get(SETTING_VIDEO_URL, testServerUrl, MAX_VIDEO_STR_LEN);
    SvcSet_Get(SETTING_VIDEO_SECRET, AppSecret, MAX_VIDEO_STR_LEN);
    SvcSet_Get(SETTING_VIDEO_LICENSE, license, MAX_VIDEO_STR_LEN);
    SvcSet_Get(SETTING_VIDEO_AESKEY, aesKey, MAX_VIDEO_STR_LEN);
    SvcSet_Get(SETTING_VIDEO_APPKEY, appKey, MAX_VIDEO_STR_LEN);
    SvcSet_Get(SETTING_VIDEO_DEV_NAME, devName, MAX_VIDEO_STR_LEN);
#endif  
    strcpy(g_CallInfo->remoteUserId, remoteId);
#ifdef USE_WATCH_SERVER_PROTOCOL
    uint8_t len = sizeof(g_CallInfo->localUserId);
    memset(g_CallInfo->localUserId, 0, len);
    Watch_VideoGetUserId(g_CallInfo->localUserId, &len);

    if(Watch_VideoIsCallIn())
    {
        char *roomid = Watch_VideoGetRoomId();
        memcpy(g_CallInfo->chnlId, roomid, strlen(roomid));
    }
    else
    {
        time_t time_stamp = 0;
        get_local_time_stamp(&time_stamp);
        os_snprintf(g_CallInfo->chnlId, MAX_VIDEO_STR_LEN, "%d-%d\r\n", time_stamp, os_tick_get());
        Watch_VideoSetRoomId(g_CallInfo->chnlId, strlen(g_CallInfo->chnlId));
    }

#else
    strcpy(g_CallInfo->localUserId, devName);
    strcpy(g_CallInfo->chnlId, chnlId);
 //osPrintf("get video para: %s, %s, %s, %s, %s \r\n", testServerUrl, AppSecret, license, aesKey, appKey);
    Video_GenToken(appKey, AppSecret, g_CallInfo->localUserId, g_CallInfo->chnlId, g_token);
#endif

    g_rtcCallBackHandler.on_user_joined = on_user_joined;
    g_rtcCallBackHandler.on_user_changed = on_user_changed;
    g_rtcCallBackHandler.on_user_offline = on_user_offline;
    g_rtcCallBackHandler.on_user_message = on_user_message;
    g_rtcCallBackHandler.on_video_changed = on_video_changed;
    g_rtcCallBackHandler.on_share_changed = on_share_changed;
    g_rtcCallBackHandler.on_audio_volume = on_audio_volume;
    g_rtcCallBackHandler.on_user_netstate = on_user_netstate;
    g_VideoCallType = VIDEO_CALL_TYPE_OPEN;

    return 0;
}

int8_t Video_Start(void)
{
    jrtc_config_server(testServerUrl);
#ifdef USE_WATCH_SERVER_PROTOCOL
    jrtc_config(appKey, g_CallInfo->chnlId, aesKey, "", &g_rtcCallBackHandler);
#else
    jrtc_config(appKey, license, aesKey, g_token, &g_rtcCallBackHandler);
#endif

    g_recvVideoCfg.fps = 15;
    g_sendVideoCfg.fps = 15;
    g_sendVideoCfg.kbps = 2000;
    g_recvVideoCfg.kbps = 2000;
    g_sendVideoCfg.height = 128;
    g_sendVideoCfg.width = 128;
    g_recvVideoCfg.height = 128;
    g_recvVideoCfg.width = 128;
#ifdef USE_WATCH_SERVER_PROTOCOL
    g_jc = jrtc_open(g_CallInfo->remoteUserId, g_CallInfo->localUserId, &g_recvVideoCfg, &g_sendVideoCfg, NULL);
#else
    g_jc = jrtc_open(g_CallInfo->chnlId, g_CallInfo->localUserId, &g_recvVideoCfg, &g_sendVideoCfg, NULL);
#endif
    if(!g_jc)
    {
        //osFree(g_CallInfo);
        osPrintf("jrtc_open failed \r\n");
        return -1;
    }

#ifdef TEST_VIDEO
    osInitCompletion(&g_dispCmpl);
    g_dispDev = DispST7789V_GetDevice();
    g_dispOps = DispST7789V_GetOps();
    g_dispDev->horRes = g_recvVideoCfg.height;
    g_dispDev->verRes = g_recvVideoCfg.width;
    g_dispOps->init(g_dispDev, Video_DispCallBack, &g_dispCmpl);

    g_ImageBuf = osMallocAlign(g_recvVideoCfg.height * g_recvVideoCfg.width * 2, OS_CACHE_LINE_SZ);

    osThreadAttr_t attr = {"ui-v", osThreadDetached, NULL, 0U, NULL, 4096, osPriorityISR - 20, 0U, 0U};
    g_videoUITask = osThreadNew(Video_UITask, OS_NULL, &attr);
    OS_ASSERT(g_videoUITask);
#endif

    return 0;
}

void Video_Close(void)
{
#ifdef TEST_VIDEO
    if(g_videoUITask)
    {
        osThreadDetach(g_videoUITask);
        g_videoUITask = NULL;
    }

    if(g_ImageBuf)
    {
        osFree(g_ImageBuf);
        g_ImageBuf = NULL;
    }
    g_dispOps->deinit(g_dispDev);
#endif
    if(g_jc)
        jrtc_leave(g_jc, JRTC_EBYE);
    
    osTick_t start_tick = osTickGet();
    struct osThread* task_handle = OS_NULL;

    while(1)
    {
        osList_t    *itor;
        osCriticalEnter();
        bool isFind = false;
        osList_t* threadListHead =  osThreadGetList();
        osListForEach(itor, threadListHead)
        {
            task_handle = osListEntry(itor, struct osThread, resource_node);
            if(strcmp("jrtc",task_handle->name) == 0)
            {
                isFind = true;
            }
        }
        osCriticalExit();

        if(!isFind)  // 自主退出
        {
            //osPrintf("jrtc task quit \r\n");
            break;
        }

        if((osTickGet() - start_tick) >= osTickFromMillisecond(10000))    // 10s 超时， 未退出，强制回收
        {
            osThreadDetach(task_handle);
            //osPrintf("jrtc task destroy \r\n");
            break;
        }

        osThreadMsSleep(100);
    }
        
    g_jc = NULL;

    // if(g_CallInfo)
    // {
    //     osFree(g_CallInfo);
    //     g_CallInfo = NULL;
    // }  

    if(g_token)
    {
        osFree(g_token);
        g_token = NULL;
    }  
#ifdef USE_SVC
   if(testServerUrl)
    {
        osFree(testServerUrl);
        testServerUrl = NULL;
    }

    if(AppSecret)
    {
        osFree(AppSecret);
        AppSecret = NULL;
    }

    if(license)
    {
        osFree(license);
        license = NULL;
    }

    if(aesKey)
    {
        osFree(aesKey);
        aesKey = NULL;
    }

    if(appKey)
    {
        osFree(appKey);
        appKey = NULL;
    }

    if(devName)
    {
        osFree(devName);
        devName = NULL;
    }
#endif
    g_remoteUserStatus = 0xff;

    g_VideoCallType = VIDEO_CALL_TYPE_CLOSE;
}

void Video_Ctrl(uint8_t devices, VIDEO_DevStat devStat)
{
    if(devStat == VIDEO_DEV_OPEN)
    {
        if(devices & VIDEO_CAMERA)
            jrtc_set_status(g_jc, g_CallInfo->localUserId, JRTC_STATUS_VIDEO, JRTC_STATUS_VIDEO);
        else if(devices & VIDEO_MICROPHONE)
            jrtc_set_status(g_jc, g_CallInfo->localUserId, JRTC_STATUS_AUDIO, JRTC_STATUS_AUDIO);
    }
    else
    {
        if(devices & VIDEO_CAMERA)
            jrtc_set_status(g_jc, g_CallInfo->localUserId, JRTC_STATUS_VIDEO, 0);
        else if(devices & VIDEO_MICROPHONE)
            jrtc_set_status(g_jc, g_CallInfo->localUserId, JRTC_STATUS_AUDIO, 0);
    }
}

int8_t Video_GetImage(uint8_t *image, uint32_t imageSize)
{
    if(!image || imageSize != g_recvVideoCfg.bytes)
    {
       // osPrintf("Video_GetImage para error: %p, %d %d\r\n", image, imageSize,  g_recvVideoCfg.bytes);
        return -1;
    }

    if(jrtc_state(g_jc) >= JRTC_JOINED)
    {
        if(g_recvVideoCfg.get != g_recvVideoCfg.put)
        {
            g_recvVideoCfg.get = g_recvVideoCfg.put;
            memcpy(image, g_recvVideoCfg.data, imageSize);

            return 0;
        }
    }

    return -1;    
}

int8_t Video_GetStatus(uint8_t devices)
{
    if(devices & VIDEO_CAMERA)
    {
        return (g_remoteUserStatus & JRTC_STATUS_VIDEO);
    }

    return 0;
}


#include "nr_micro_shell.h"
static void video_call(char argc, char **argv)
{
    Video_Open("13621702319");
    Video_Start();

  //  osThreadMsSleep(10000);
   // Video_Close();
}
NR_SHELL_CMD_EXPORT(video_call, video_call);
