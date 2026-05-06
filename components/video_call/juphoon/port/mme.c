#include "mme.h"
#include "rtp.h"
#include "jrtc.h"

#ifdef USE_PCM_U_A
#include "g711/g711_table.h"
#endif

#ifdef USE_LIBJPEG_TURBO
#include "jpeg.h"
#endif

#include "os.h"

// #include "audio_monitor.h"
#include "audio_voice.h"
#include "drv_i2s.h"
#include "drv_capture.h"
#include "video_call.h"
#include "image_convert.h"

//#define  MME_AUDIO_DUMP
#define  MME_AUDIO_DUMP_SIZE      (1024 * 1024)

#ifdef MME_AUDIO_DUMP
#include "audio_debug.h"
#endif

#define MME_DEBUG_PRINTF
#ifdef  MME_DEBUG_PRINTF
    #define MME_PRINTF        osPrintf
#else
    #define MME_PRINTF(...)
#endif 

#define         RTP_PCMU                    (0)
#define         RTP_PCMA                    (8)

#define         FRAME_TIME                  20          //20ms

#define         JPEG_HEADER_BYTE0           0xFF
#define         JPEG_HEADER_BYTE1           0xD8

// audio write buffer
#define     MME_MAX_AUDIO_BUF_NUM       32
typedef struct 
{
    uint8_t *bufPtr[MME_MAX_AUDIO_BUF_NUM];
    uint16_t bufSize;
    uint8_t writeIndex;
    uint8_t readIndex;
    uint8_t count;
}Audio_RingBuf;

static Audio_RingBuf *g_AudioRingBuf = NULL;
static osThreadId_t g_AudioWriteTask = NULL;

static void AudioWriteTask(void *para)
{
    while(1)
    {
        if(g_AudioRingBuf->count > 0)
        {
            Voice_WriteOneFrame(g_AudioRingBuf->bufPtr[g_AudioRingBuf->readIndex], VOICE_NB_FRAME_SIZE, osWaitForever);
            g_AudioRingBuf->readIndex = (g_AudioRingBuf->readIndex + 1) % MME_MAX_AUDIO_BUF_NUM;
            g_AudioRingBuf->count --;
        }
        else
        {
            osThreadMsSleep(20);
        }
    }
}


void MME_AudioBufInit(void)
{
    if(!g_AudioRingBuf)
    {
        g_AudioRingBuf = osMalloc(sizeof(Audio_RingBuf));
        OS_ASSERT(g_AudioRingBuf);

        memset(g_AudioRingBuf, 0 , sizeof(Audio_RingBuf));

        for(uint8_t i = 0; i < MME_MAX_AUDIO_BUF_NUM; i++)
        {
            if( !g_AudioRingBuf->bufPtr[i] )
            {
                g_AudioRingBuf->bufPtr[i] = osMalloc(VOICE_NB_FRAME_SIZE);
                OS_ASSERT(g_AudioRingBuf->bufPtr[i]);
            }    
        }
    }

    if(!g_AudioWriteTask)
    {
        osThreadAttr_t attr = {"audio-w", osThreadDetached, NULL, 0U, NULL, 4096, osPriorityNormal3, 0U, 0U};
        g_AudioWriteTask = osThreadNew(AudioWriteTask, NULL, &attr);
    }
}

void MME_AudioBufDeInit(void)
{
    for(uint8_t i = 0; i < MME_MAX_AUDIO_BUF_NUM; i++)
    {
        if(g_AudioRingBuf->bufPtr[i])
        {
            osFree(g_AudioRingBuf->bufPtr[i]);
            g_AudioRingBuf->bufPtr[i] = NULL;
        }
    }

    if(g_AudioRingBuf)
    {
        osFree(g_AudioRingBuf);
        g_AudioRingBuf = NULL;
    }
}


const char* MME_AudioCodecName(void)
{
    return "PCMA/8000";
}

static bool MME_StartAudioFlag = false;
// static void MME_AudioCallBack(uint32_t event)
// {
//     if(event == SCHEDULE_QUIT_EVENT)
//     {
// #ifdef USE_SVC
//         svc_video_call_api_close();
// #endif
//         MME_AudioHandle = NULL;
//     }
// }

int MME_StartAudio(int payloadType, int sendCodecIndex, int recvCodecIndex)
{
    switch (payloadType)
    {
    case RTP_PCMA:
#ifdef USE_PCM_U_A
        pcm16_alaw_tableinit();
        alaw_pcm16_tableinit();
#endif
        break;
    default:
        MME_PRINTF("not support payloadType : %d \r\n", payloadType);
        return -1;
        break;
    }

    // MME_AudioHandle = Audio_Request(VIDEO_VOICE_CALL_AUDIO_PRIO, VOICE_CALL, MME_AudioCallBack);
    // if(MME_AudioHandle == NULL)
    // {
    //     MME_PRINTF("Audio Request failed \r\n");
    //     return -1;
    // }

    // VoiceCfg cfg = {0};
    // cfg.sampleBits =DATA_16_BITS;
    // cfg.framSize = VOICE_NB_FRAME_SIZE;
    // cfg.sampleRate = VOICE_NB_SAMPLE_RATE;

    // MME_AudioHandle->open(&cfg);
    // MME_AudioHandle->start(VOICE_PLAY_MODE | VOICE_RECORD_MODE);
    Voice_Open(DATA_16_BITS, VOICE_NB_FRAME_SIZE, VOICE_NB_SAMPLE_RATE);
    Voice_WriteStart();

    MME_StartAudioFlag = true;
    MME_AudioBufInit();

    MME_PRINTF("MME_StartAudio \r\n");
#ifdef  MME_AUDIO_DUMP
    Audio_DumpInit(MME_AUDIO_DUMP_SIZE, VOICE_NB_SAMPLE_RATE, DATA_16_BITS);
#endif
    return 0;
}

int MME_ReadAudioRTP(void* rtp, int rtpBufferSize, int rtpHeaderLength, unsigned timeoutMs)
{
    static uint32_t _packetSequenceNumber = 0;

    if(!MME_StartAudioFlag)
        return 0;

    if(rtp == NULL || rtpBufferSize == 0)
    {
        return 0;
    }
    
    if(rtpBufferSize < (rtpHeaderLength + (VOICE_NB_FRAME_SIZE >> 1)))
    {
        MME_PRINTF("read audio rtp buffer too small %d, %d\r\n",rtpBufferSize, rtpHeaderLength);
        return 0;    
    }

    // 音频帧获取
    uint8_t *pcmFrame = NULL;
    pcmFrame = osMalloc(VOICE_NB_FRAME_SIZE);
    OS_ASSERT(pcmFrame);

    memset(pcmFrame, 0, VOICE_NB_FRAME_SIZE);
    if(Voice_ReadOneFrame(pcmFrame, VOICE_NB_FRAME_SIZE, osNoWait) < 0)
    {
        osFree(pcmFrame);
        return 0;
    }

    _packetSequenceNumber ++;
    if(_packetSequenceNumber > 65535)
        _packetSequenceNumber = 0;
    
    // 编码
    uint8_t *encodeFrame = NULL;
    encodeFrame = osMalloc(VOICE_NB_FRAME_SIZE);
    OS_ASSERT(encodeFrame);
    #ifdef USE_PCM_U_A
        pcm16_to_alaw((int)VOICE_NB_FRAME_SIZE, (char *)pcmFrame, (char *)encodeFrame);
    #endif

    //RTP 组包
    unsigned int ts = FRAME_TIME * (VOICE_NB_SAMPLE_RATE / 1000) * _packetSequenceNumber;
    unsigned char* p = (unsigned char*)rtp;
    p[4] = (unsigned char)(ts >> 24);
    p[5] = (unsigned char)(ts >> 16);
    p[6] = (unsigned char)(ts >> 8);
    p[7] = (unsigned char)(ts);

    memcpy(&p[rtpHeaderLength], encodeFrame, VOICE_NB_FRAME_SIZE >> 1);

    osFree(pcmFrame);
    osFree(encodeFrame);

    return (rtpHeaderLength + (VOICE_NB_FRAME_SIZE >> 1));
}

int MME_WriteAudioRTP(void*rtp, int rtpHeaderLength, int rtpLen)
{
    uint16_t encodeFrameSize = 0;
    uint8_t *encodeFrame = NULL;
    uint8_t *pcmFrame = NULL;

    if(!MME_StartAudioFlag)
        return 0;
        
    if(rtp == NULL)
    {
        MME_PRINTF("write rtp is null \r\n");
        return -1;
    }

    if(rtpLen < rtpHeaderLength)
    {
        MME_PRINTF("write audio rtp len too small %d,%d\r\n", rtpLen, rtpHeaderLength);
        return -1;
    }

    // 解码
    uint8_t* p = (uint8_t*)rtp;

    encodeFrameSize = rtpLen - rtpHeaderLength;
    encodeFrame = (uint8_t *)&p[rtpHeaderLength];
    pcmFrame = osMalloc(VOICE_NB_FRAME_SIZE);
    OS_ASSERT(pcmFrame);
#ifdef USE_PCM_U_A
    alaw_to_pcm16(encodeFrameSize, (char *)encodeFrame, (char *)pcmFrame);
#endif

#ifdef  MME_AUDIO_DUMP
    Audio_DumpStore(pcmFrame, VOICE_NB_FRAME_SIZE);
#endif

    memcpy(g_AudioRingBuf->bufPtr[g_AudioRingBuf->writeIndex], pcmFrame, VOICE_NB_FRAME_SIZE);
    g_AudioRingBuf->writeIndex = (g_AudioRingBuf->writeIndex + 1) % MME_MAX_AUDIO_BUF_NUM;
    g_AudioRingBuf->count ++;

    if(g_AudioRingBuf->count == MME_MAX_AUDIO_BUF_NUM)
    {
        osPrintf(" g_AudioRingBuf over flow ####### \r\n");
        g_AudioRingBuf->count = 0;
    }
    osFree(pcmFrame);

    return 0;
}

void MME_MutePlayout(char is_mute)
{
    if(!MME_StartAudioFlag)
        return;

    if(is_mute)
    {
        Voice_WriteStop();
    }
    else
    {
        Voice_WriteStart();
    }
}

void MME_OnAudioCall(void)
{
    MME_PRINTF("MME_OnAudioCall \r\n");
}

void MME_StopAudio(void)
{
    if(!MME_StartAudioFlag)
        return;
        
    if(g_AudioWriteTask)
    {
        osThreadDetach(g_AudioWriteTask);
        g_AudioWriteTask = NULL;
    }

    MME_AudioBufDeInit();
    Voice_WriteStop();
    Voice_Close();
#ifdef USE_PCM_U_A
    pcm16_alaw_tabledeinit();
    alaw_pcm16_tabledeinit();
#endif
    MME_StartAudioFlag = false;
    MME_PRINTF("MME_StopAudio \r\n");
}

const char* MME_VideoCodecName(void)
{
    return "JPEG";
}

static uint8_t *g_readImagebuf = NULL;
static uint8_t *g_zoomImageBuf = NULL;
static uint8_t *g_readEncodebuf = NULL;
static uint8_t *g_writeImageReadybuf = NULL;
static uint8_t *g_writeImagebuf = NULL;
static uint8_t *g_writeDecodebuf = NULL;
// static CameraImageInfo g_ImageInfo = {0};
static uint64_t encodeSize = 0;
static uint32_t g_VideoReadRtpHeaderLen = 0;
static uint32_t g_VideoWriteLen = 0;


/**** 压缩 和 解压缩*/
static osSemaphoreId_t g_JpegDecodeStartSem = NULL;
static osSemaphoreId_t g_JpegDecodeFinshSem = NULL;
static osSemaphoreId_t g_JpegEncodeStartSem = NULL;
static osSemaphoreId_t g_JpegEncodeFinshSem = NULL;

static osSemaphoreId_t g_JpegEncodeTaskQuitSem = NULL;
static bool g_JpegEncodeTaskStopFlag = false;
static osSemaphoreId_t g_JpegDecodeTaskQuitSem = NULL;
static bool g_JpegDecodeTaskStopFlag = false;

static osThreadId_t g_JpegDecodeTask = NULL;
static osThreadId_t g_JpegEncodeTask = NULL;

static CaptureDevice_t *g_captureDev = NULL;

static void JpegDecodeTask(void *para)
{
    uint16_t imgH, imgW;
    Video_GetRecvImgInfo(&imgH, &imgW);
    while (1)
    {
        osSemaphoreAcquire(g_JpegDecodeStartSem, osWaitForever);

        if(g_JpegDecodeTaskStopFlag)
        {
            g_JpegDecodeTaskStopFlag = false;
            break;
        }
#ifdef USE_LIBJPEG_TURBO
        if(JPEG_DeCompress(g_writeImageReadybuf, g_VideoWriteLen, g_writeDecodebuf, imgH * imgW * 2) < 0)
        {
            MME_PRINTF("JPEG_DeCompress failed\r\n");
        }
#endif
        osSemaphoreRelease(g_JpegDecodeFinshSem);
    } 

    osSemaphoreRelease(g_JpegDecodeTaskQuitSem);
}

static void JpegEncodeTask(void *para)
{
    uint16_t imgH, imgW;
    Video_GetSendImgInfo(&imgH, &imgW);

    while (1)
    {
        osSemaphoreAcquire(g_JpegEncodeStartSem, osWaitForever);
        if(g_JpegEncodeTaskStopFlag)
        {
            g_JpegEncodeTaskStopFlag = false;
            break;
        }
        while(1)
        {
            g_readImagebuf = g_captureDev->capture(g_captureDev);
            if(!g_readImagebuf)
            {
                osThreadMsSleep(10);
            }
            else
            {
                break;
            }
        }

        ImageZoom(g_readImagebuf, g_captureDev->verRes,  g_captureDev->horRes, g_zoomImageBuf, imgH, imgW); 
        //memcpy(g_zoomImageBuf, g_readImagebuf, imgH *imgW *2);
        g_captureDev->release(g_captureDev, g_readImagebuf);

        encodeSize = imgH * imgW;
#ifdef USE_LIBJPEG_TURBO
        JPEG_Compress(g_zoomImageBuf, imgH, imgW, &g_readEncodebuf[g_VideoReadRtpHeaderLen], &encodeSize);
#endif
        osSemaphoreRelease(g_JpegEncodeFinshSem);
    }

    osSemaphoreRelease(g_JpegEncodeTaskQuitSem);
}

static void JpegProcInit(void)
{
    if(g_JpegDecodeStartSem == NULL)
    {
        g_JpegDecodeStartSem = osMalloc(sizeof(struct osSemaphore));
        osSemaphoreInit(g_JpegDecodeStartSem,  0, 1, OS_IPC_FLAG_FIFO);
    }

    if(g_JpegDecodeFinshSem == NULL)
    {
        g_JpegDecodeFinshSem = osMalloc(sizeof(struct osSemaphore));
        osSemaphoreInit(g_JpegDecodeFinshSem,  0, 1, OS_IPC_FLAG_FIFO);
    }

    if(g_JpegEncodeStartSem == NULL)
    {
        g_JpegEncodeStartSem = osMalloc(sizeof(struct osSemaphore));
        osSemaphoreInit(g_JpegEncodeStartSem,  0, 1, OS_IPC_FLAG_FIFO);
    }

    if(g_JpegEncodeFinshSem == NULL)
    {
        g_JpegEncodeFinshSem = osMalloc(sizeof(struct osSemaphore));
        osSemaphoreInit(g_JpegEncodeFinshSem,  1, 1, OS_IPC_FLAG_FIFO);
    }

    if(g_JpegEncodeTaskQuitSem == NULL)
    {
        g_JpegEncodeTaskQuitSem = osMalloc(sizeof(struct osSemaphore));
        osSemaphoreInit(g_JpegEncodeTaskQuitSem,  0, 1, OS_IPC_FLAG_FIFO);
    }

    if(g_JpegDecodeTaskQuitSem == NULL)
    {
        g_JpegDecodeTaskQuitSem = osMalloc(sizeof(struct osSemaphore));
        osSemaphoreInit(g_JpegDecodeTaskQuitSem,  0, 1, OS_IPC_FLAG_FIFO);
    }

    if(g_JpegDecodeTask == NULL)
    {

        osThreadAttr_t attr = {"jpg-d", osThreadDetached, NULL, 0U, NULL, 8096, osPriorityNormal3, 0U, 0U};
        g_JpegDecodeTask = osThreadNew(JpegDecodeTask, NULL, &attr);
        OS_ASSERT(g_JpegDecodeTask);
        g_JpegDecodeTaskStopFlag = false;
    }

    if(g_JpegEncodeTask == NULL)
    {
        osThreadAttr_t attr = {"jpg-e", osThreadDetached, NULL, 0U, NULL, 4096, osPriorityNormal3, 0U, 0U};
        g_JpegEncodeTask = osThreadNew(JpegEncodeTask, NULL, &attr);

        OS_ASSERT(g_JpegEncodeTask);
        g_JpegEncodeTaskStopFlag = false;
    }
}

static void JpefProcDeinit(void)
{
    if(g_JpegEncodeTask)
    {
        g_JpegEncodeTaskStopFlag = true;
        osSemaphoreRelease(g_JpegEncodeStartSem);
        osSemaphoreAcquire(g_JpegEncodeTaskQuitSem, osWaitForever);
        g_JpegEncodeTask = NULL;
    }

    if(g_JpegDecodeTask)
    {
        g_JpegDecodeTaskStopFlag = true;
        osSemaphoreRelease(g_JpegDecodeStartSem);
        osSemaphoreAcquire(g_JpegDecodeTaskQuitSem, osWaitForever);
        g_JpegDecodeTask = NULL;
    }

    if(g_JpegDecodeStartSem)
    {
        osSemaphoreDetach(g_JpegDecodeStartSem);
        osFree(g_JpegDecodeStartSem);
        g_JpegDecodeStartSem = NULL; 
    }

    if(g_JpegDecodeFinshSem)
    {
        osSemaphoreDetach(g_JpegDecodeFinshSem);
        osFree(g_JpegDecodeFinshSem);
        g_JpegDecodeFinshSem = NULL; 
    }

    if(g_JpegEncodeStartSem)
    {
        osSemaphoreDetach(g_JpegEncodeStartSem);
        osFree(g_JpegEncodeStartSem);
        g_JpegEncodeStartSem = NULL; 
    }

    if(g_JpegEncodeFinshSem)
    {
        osSemaphoreDetach(g_JpegEncodeFinshSem);
        osFree(g_JpegEncodeFinshSem);
        g_JpegEncodeFinshSem = NULL; 
    }

    if(g_JpegDecodeTaskQuitSem)
    {
        osSemaphoreDetach(g_JpegDecodeTaskQuitSem);
        osFree(g_JpegDecodeTaskQuitSem);
        g_JpegDecodeTaskQuitSem = NULL;
    }
    
    if(g_JpegEncodeTaskQuitSem)
    {
        osSemaphoreDetach(g_JpegEncodeTaskQuitSem);
        osFree(g_JpegEncodeTaskQuitSem);
        g_JpegEncodeTaskQuitSem = NULL;
    }
}


int MME_StartVideo(MMEVideoParam* param)
{
    MME_PRINTF("%s \r\n",__FUNCTION__);
    if(param->sendCodecIndex != 0 || param->recvCodecIndex)
    {
        MME_PRINTF("CodecIndex not support : %d %d\r\n", param->sendCodecIndex, param->recvCodecIndex);
        return -1;
    }
    g_captureDev = CamGC032A_GetDevice();
    if(g_captureDev == NULL) {
        return -1;
    }

    //Video_GetSendImgInfo(&g_captureDev->horRes, &g_captureDev->verRes);
    g_captureDev->horRes = 240;
    g_captureDev->verRes = 240;
    
    if(0 != g_captureDev->init(g_captureDev, NULL, NULL))
    {
        return -1;
    }
    g_captureDev->start(g_captureDev);
    //读使用内存
    g_readImagebuf = osMallocAlign(g_captureDev->horRes * g_captureDev->verRes *2, OS_CACHE_LINE_SZ);
    OS_ASSERT(g_readImagebuf);
    g_zoomImageBuf = osMallocAlign(g_captureDev->horRes * g_captureDev->verRes *2, OS_CACHE_LINE_SZ);
    OS_ASSERT(g_zoomImageBuf);
    g_readEncodebuf = osMallocAlign(g_captureDev->horRes * g_captureDev->verRes, OS_CACHE_LINE_SZ);
    OS_ASSERT(g_readEncodebuf);

    //写使用内存
    uint16_t imgH, imgW;
    Video_GetRecvImgInfo(&imgH, &imgW);

    g_writeDecodebuf =osMallocAlign(imgH * imgW *2, OS_CACHE_LINE_SZ);
    OS_ASSERT(g_writeDecodebuf);
    g_writeImagebuf = osMallocAlign(imgH * imgW, OS_CACHE_LINE_SZ);
    OS_ASSERT(g_writeImagebuf);
    g_writeImageReadybuf = osMallocAlign(imgH * imgW, OS_CACHE_LINE_SZ);
    OS_ASSERT(g_writeImageReadybuf);

    JpegProcInit();
    return 0;
}

int MME_ReadVideoRTP(struct rtp_buf_t* rtp, int rtpBufferSize, int rtpHeaderLength, MMEImage*image, unsigned timeoutMs)
{
    // static bool decodeFinsh = false;
    if(rtp == NULL || rtpBufferSize == 0)
        return 0;

    if(rtp->size == 0)
    {
        g_readImagebuf = g_captureDev->capture(g_captureDev);
        if(!g_readImagebuf)
        {
            return 0;
        }
#ifdef USE_LIBJPEG_TURBO
        JPEG_Compress(g_readImagebuf, g_captureDev->verRes, g_captureDev->horRes, &g_readEncodebuf[rtpHeaderLength + 2], &encodeSize);
#endif
        if(image)
        {
            image->data = g_readImagebuf;
            image->bytes = g_captureDev->verRes * g_captureDev->horRes * 2;
            image->format = JRTC_RGB565;
            image->height = g_captureDev->verRes;
            image->width = g_captureDev->horRes; 
        }

        rtp->buf = &g_readEncodebuf[rtpHeaderLength + 2];   
        rtp->len[0] = encodeSize;
        rtp->size = encodeSize + rtpHeaderLength + 2;
        int num = rtp_split_frame(rtp, rtpBufferSize, rtpHeaderLength + 2);
        char *p = rtp->buf;
        for (int i=0; i< num; p += rtp->len[i++]) 
        {
            if(i == 0)
            {
                p[rtpHeaderLength+0] = JPEG_HEADER_BYTE0; // JPEG
                p[rtpHeaderLength+1] = JPEG_HEADER_BYTE1;
            }

            p[1] = 0;
        }
        p[1] = 0x80; 

        return num;
    }
    else if(rtp->size == rtpBufferSize)
    {
        static uint8_t *pBuf = NULL;

        if (timeoutMs || image)
        {
            if(osSemaphoreAcquire(g_JpegEncodeFinshSem, osNoWait) != 0)
                return 0;

            g_VideoReadRtpHeaderLen = rtpHeaderLength;
            pBuf = &g_readEncodebuf[rtpHeaderLength];
#if 0
            if(image)
            {
                image->data = g_readImagebuf;
                image->bytes = g_captureDev->verRes * g_captureDev->horRes * 2;
                image->format = JRTC_RGB565;
                image->height = g_captureDev->verRes;
                image->width = g_captureDev->horRes; 
            }
#endif
            osSemaphoreRelease(g_JpegEncodeStartSem);
        }

        rtp->buf = pBuf - rtpHeaderLength;

        const int payload = rtpBufferSize - rtpHeaderLength;

        if (encodeSize <= payload) 
        {
            ((uint8_t *)rtp->buf)[1] = 0x80; //结尾的Mark位
            return rtpHeaderLength + encodeSize;
        } 
        else 
        {
            ((uint8_t *)rtp->buf)[1] = 0;
            pBuf += payload;
            encodeSize -=  payload;
            return rtpBufferSize;
        }
    }
    else
    {
        MME_PRINTF("error: rtp.size: %d, rtpBufferSize: %d \r\n", rtp->size, rtpBufferSize);
        return -1;
    }
}

int MME_WriteVideoRTP(void* rtp, int rtpHeaderLength, int rtpLen, MMEImage* image)
{
    uint8_t *pBuf = NULL;
    static uint32_t imgIndex = 0;
    uint16_t imgH, imgW;

    if(rtp == NULL || rtpLen <= rtpHeaderLength)
    {
         MME_PRINTF("input para error \r\n");
        return -1;
    }

    Video_GetRecvImgInfo(&imgH, &imgW);

    uint8_t *p = (uint8_t *)rtp;
    pBuf = &p[rtpHeaderLength];
    if(pBuf[0] == JPEG_HEADER_BYTE0 && pBuf[1] == JPEG_HEADER_BYTE1)
    {
        if(imgIndex > 0)
        {
            memcpy(g_writeImageReadybuf, g_writeImagebuf, imgIndex);
            g_VideoWriteLen = imgIndex;
            osSemaphoreRelease(g_JpegDecodeStartSem);
        } 
        imgIndex = 0;
    }
    memcpy(&g_writeImagebuf[imgIndex], pBuf, rtpLen - rtpHeaderLength);
    imgIndex += (rtpLen - rtpHeaderLength);

    if(image != NULL && (osSemaphoreAcquire(g_JpegDecodeFinshSem, osNoWait) == 0))
    {
        image->data = g_writeDecodebuf;
        image->bytes = imgH * imgW * 2;
        image->format = JRTC_RGB565;
        image->height = imgH;
        image->width = imgW;
    }

    return 0;
}

void MME_StopVideo(void)
{ 
    MME_PRINTF("%s \r\n",__FUNCTION__);

    JpefProcDeinit();

   if(g_writeDecodebuf) 
   {
        osFree(g_writeDecodebuf);
        g_writeDecodebuf = NULL;
   }

   if(g_readEncodebuf)
   {
        osFree(g_readEncodebuf);
        g_readEncodebuf = NULL;
   }

    if(g_readImagebuf)
    {
        osFree(g_readImagebuf);
        g_readImagebuf = NULL;
    }
    
    if(g_zoomImageBuf)
    {
        osFree(g_zoomImageBuf);
        g_zoomImageBuf = NULL;
    }

    if(g_writeImageReadybuf)
    {
        osFree(g_writeImageReadybuf);
        g_writeImageReadybuf = NULL;
    }

    if(g_writeImagebuf)
    {
        osFree(g_writeImagebuf);
        g_writeImagebuf = NULL;
    }

    if(g_captureDev)
    {
        g_captureDev->stop(g_captureDev);
        g_captureDev->deinit(g_captureDev);
    }     
}

int MME_ResetVideo(unsigned char sendFps, unsigned short sendGopMs, unsigned short sendKbps)
{
     MME_PRINTF("%s \r\n",__FUNCTION__);

    if(g_captureDev)
    {
        g_captureDev->stop(g_captureDev);
        g_captureDev->deinit(g_captureDev);
        osThreadMsSleep(500);
        g_captureDev->init(g_captureDev, NULL, NULL);
        g_captureDev->start(g_captureDev);
    } 

    return 0;
}

void MME_OnVideoStats(unsigned nowMs, char sendLoss, unsigned sendKbps, unsigned rttMs, unsigned recvJitterMs, unsigned recvFPS)
{
    MME_PRINTF("nowMs: %d, sendLoss %d, sendKbps: %d,  rttMs: %d, recvJitterMs:%d, recvFPS:%d\r\n",
                  nowMs, sendLoss, sendKbps, rttMs, recvJitterMs, recvFPS);
}

int MME_SetCamera(int face)
{
     MME_PRINTF("%s \r\n",__FUNCTION__);
    return 0;
}
