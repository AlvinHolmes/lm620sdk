#include "string.h"
#include "os.h"
#include "audio_debug.h"
#include "audio_local.h"
#include "drv_i2s.h"
#include "audio_codec.h"

#include "at_api.h"

#include "ymodem.h"

#include "wav.h"

#ifdef OS_USING_PM
#include <psm_common.h>
#endif

#define AUDIO_DEBUG_PRINT

#ifdef AUDIO_DEBUG_PRINT
    #define PRINT_INFO osPrintf
#else
    #define PRINT_INFO(...)
#endif

typedef struct 
{
    AudioDbgCmdType cmd;
    int8_t (*func)(uint8_t *paraIn, uint8_t lenIn, uint8_t *paraOut, uint8_t *lenOut);
}CmdFunc;

typedef struct 
{
    uint8_t para;
    int8_t (*get_func)(uint8_t *paraOut, uint8_t *lenOut); 
    int8_t (*set_func)(uint8_t *paraIn, uint8_t lenIn); 
}ParaFunc;

static ParaFunc g_ParaFunclist[] = 
{
    {ADC_DIGITAL_GAIN, Codec_GetAdcDigitGain, Codec_SetAdcDigitGain},
    {DAC_DIGITAL_GAIN, Codec_GetDacDigitGain, Codec_SetDacDigitGain},
    {ADC_PGA,          Codec_GetAdcPgaGain,   Codec_SetAdcPgaGain},
    {MIC_GAIN,         Codec_GetMicGain,      Codec_SetMicGain},
    {NR_GATE,          Codec_GetNrGate,       Codec_SetNrGate},
    {ADC_EQ,           Codec_GetAdcEQ,        Codec_SetAdcEQ},
    {DAC_EQ,           Codec_GetDacEQ,        Codec_SetDacEQ},
    {SIDE_TONE,        Codec_GetSideTone,     Codec_SetSideTone},
    {DRC,              Codec_GetDrc,          Codec_SetDrc},
    {ALC,              Codec_GetAlc,          Codec_SetAlc},
    {CODEC_ID,         Codec_GetCodecID,      NULL},
    {MCU_ID,           Codec_GetMcuID,        NULL},
    {VOLUME_LEVEL,     Codec_GetVolumeLevelPara,  Codec_SetVolumeLevelPara}
};

typedef struct 
{
    T_YModemFile     audioPlayFile;
    T_YModemFile     audioRecordFile;
    AudioFileType    recordFileType; 
    AudioFileType    playFileType;
}AudioFileHandle;
static AudioFileHandle     g_audioFile = {0};

typedef struct 
{
    uint32_t        sampleRate;
    uint8_t         dataBits;
    uint8_t         chnlNum;
    uint16_t        frameSize;
}AudioCtrlHandle;
static AudioCtrlHandle g_audioCtrlHandle = {.sampleRate = 16000,.dataBits = 16};

static void Audio_YmodemInit(uint8_t transType);

static int8_t Audio_Init(void)
{
    // static bool initFlag = false;
    // I2S_BusCfg busCfg;
    // I2S_TimingCfg i2sTimingCfg;

    // if(initFlag)
    // {
    //     return CODEC_OK;
    // }
    // initFlag = true;

    // i2sTimingCfg.chnl = I2S_LEFT_CHNL;
    // i2sTimingCfg.dataBits = g_audioCtrlHandle.dataBits;
    // i2sTimingCfg.dataCycle = DATA_32_BITS;
    // busCfg.workMode = I2S_PLAY_MODE | I2S_RECORD_MODE;
    // busCfg.timing = TIMING_I2S;
    // busCfg.frameSize = 1024;
    // busCfg.timingCfg = &i2sTimingCfg;
    // busCfg.sampleRate = g_audioCtrlHandle.sampleRate;
    // busCfg.fs = I2S_64FS;

    // if(I2S_Initialize(&busCfg, NULL, NULL) < 0)
    // {
    //     PRINT_INFO("i2s init failed \r\n");
    //     return CODEC_ERR;
    // }

    //codec config
    Codec_Initialize(g_audioCtrlHandle.sampleRate, g_audioCtrlHandle.dataBits);
    return CODEC_OK;
}

static void Audio_DumpArray(uint8_t *array, uint8_t len)
{
    for(uint8_t i = 0; i < len; i++)
    {
        PRINT_INFO("0x%x ", array[i]);
    }
    PRINT_INFO("\r\n");
}

static int8_t Audio_GetPara(uint8_t *paraIn, uint8_t lenIn, uint8_t *paraOut, uint8_t *lenOut)
{
    int8_t ret = 0;
    uint8_t i = 0;

    if(lenIn != 1)
        return CODEC_ERR_PARAMETER;

    for(i = 0; i < sizeof(g_ParaFunclist)/sizeof(ParaFunc); i++)
    {
        if(g_ParaFunclist[i].para == paraIn[0])
        {
            ret = g_ParaFunclist[i].get_func(paraOut, lenOut);
            PRINT_INFO("\r\nAudio Get Para: %x ",paraIn[0]);
            Audio_DumpArray(paraOut, *lenOut);
            break;
        }
    }

    if(i >= sizeof(g_ParaFunclist)/sizeof(ParaFunc))
    {
        ret = CODEC_ERR_UNSUPPORTED;
        PRINT_INFO("unsupport para get !!\r\n");
    }
    
    return ret;
}

static int8_t Audio_SetPara(uint8_t *paraIn, uint8_t lenIn, uint8_t *paraOut, uint8_t *lenOut)
{
    int8_t ret = 0;
    uint8_t i = 0;

    if(lenIn < 2)
        return CODEC_ERR_PARAMETER;
    PRINT_INFO("\r\nAudio Set Para: ");
    Audio_DumpArray(paraIn, lenIn);
    for(i = 0; i < sizeof(g_ParaFunclist)/sizeof(ParaFunc); i++)
    {
        if(g_ParaFunclist[i].para == paraIn[0])
        {
            ret = g_ParaFunclist[i].set_func(paraIn + 1, lenIn - 1);
            break;
        }
    }

    if(i >= sizeof(g_ParaFunclist)/sizeof(ParaFunc))
    {
        ret = CODEC_ERR_UNSUPPORTED;
        PRINT_INFO("unsupport para set !!\r\n");
    }
        
    return ret;
}

static int8_t Audio_WriteFile(uint8_t *paraIn, uint8_t lenIn, uint8_t *paraOut, uint8_t *lenOut)
{
    uint32_t FileSize =0;
    uint8_t FileType = 0;
    if(lenIn != 5)
        return CODEC_ERR_PARAMETER;

    FileSize = (paraIn[0] << 24) + (paraIn[1] << 16) + (paraIn[2] << 8) + paraIn[3];
    FileType = paraIn[4];

    if(FileType > PCM_RAW_DATA)
    {
        PRINT_INFO("File type not support !\r\n");
        return CODEC_ERR_PARAMETER;
    }

    g_audioFile.audioPlayFile.size = FileSize;
    g_audioFile.playFileType = FileType;
    memset(g_audioFile.audioPlayFile.name, 0, FILE_NAME_LENGTH);

    //stop play     // stop record
    //AudioLocal_Close(AUDIO_PLAY_MODE | AUDIO_RECORD_MODE);

    if(g_audioFile.audioPlayFile.buf)
    {
        osFree(g_audioFile.audioPlayFile.buf);
        g_audioFile.audioPlayFile.buf = NULL;
    }
    g_audioFile.audioPlayFile.buf  = (uint8_t *)osMalloc(g_audioFile.audioPlayFile.size);
    OS_ASSERT(g_audioFile.audioPlayFile.buf);

    PRINT_INFO("write file, size: 0x%x, type:%d \r\n", g_audioFile.audioPlayFile.size, g_audioFile.playFileType);

    Audio_YmodemInit(START_WRITE_FILE);
    return CODEC_OK;
}

static int8_t Audio_ReadFile(uint8_t *paraIn, uint8_t lenIn, uint8_t *paraOut, uint8_t *lenOut)
{ 
    if(!g_audioFile.audioRecordFile.buf)
    {
        PRINT_INFO("no file read \r\n");
        return CODEC_ERR_EMPTY;
    }
    Audio_YmodemInit(START_READ_FILE);
    return CODEC_OK;
}

static void Audio_Cb(uint32_t event, void *para)
{
    if(event == RECORD_OVER_CB_EVENT)
    {
        PRINT_INFO("record over \r\n");
        memset(g_audioFile.audioRecordFile.name, 0, FILE_NAME_LENGTH);
        strcpy(g_audioFile.audioRecordFile.name, "record.wav");
        AT_SendUnsolicited(0, "\r\n+AUDIODBG:5\r\n", strlen("\r\n+AUDIODBG:5\r\n"));

        // // 录音播放
        // if(!g_audioFile.audioPlayFile.buf)
        // {
        //     g_audioFile.audioPlayFile.buf = (uint8_t *)osMalloc(g_audioFile.audioRecordFile.size);
        //     OS_ASSERT(g_audioFile.audioPlayFile.buf);
        // }

        // g_audioFile.playFileType = g_audioFile.recordFileType;
        // memcpy(g_audioFile.audioPlayFile.buf, g_audioFile.audioRecordFile.buf, g_audioFile.audioRecordFile.size);
    }
}

//static AudioMonitorHandle *g_dbgAudioHandle = NULL;
static int8_t Audio_PlayFile(uint8_t *paraIn, uint8_t lenIn, uint8_t *paraOut, uint8_t *lenOut)
{
    if(!g_audioFile.audioPlayFile.buf)
    {
        PRINT_INFO("no file play !\r\n");
        return -1;
    }

    static AudioInfo info;

    info.ctrl = AUDIO_PLAY_MODE;
    info.audioType = g_audioFile.playFileType;
    info.isLoop = true;
    info.data = g_audioFile.audioPlayFile.buf;
    info.size = g_audioFile.audioPlayFile.size;

    if(lenIn == 1 && paraIn[0] == 1)    // 播放并录音
    {
        if(g_audioFile.audioRecordFile.buf)
        {
            osFree(g_audioFile.audioRecordFile.buf);
            g_audioFile.audioRecordFile.buf = NULL;
        }

        g_audioFile.audioRecordFile.size = g_audioFile.audioPlayFile.size;
        g_audioFile.audioRecordFile.buf  = (uint8_t *)osMalloc(g_audioFile.audioRecordFile.size);
        OS_ASSERT(g_audioFile.audioRecordFile.buf);
        g_audioFile.recordFileType = WAV_FILE_DATA;
        
        info.recordData = g_audioFile.audioRecordFile.buf;
        info.recordSize = g_audioFile.audioRecordFile.size;
        info.ctrl = AUDIO_PLAY_MODE | AUDIO_RECORD_MODE;

        Audio_GetWavFileInfo(info.data, info.size, &info.dataBits, &info.sampleRate, &info.chnls);
    }

    info.cb = Audio_Cb;
    return AudioLocal_Open(&info);
}

static int8_t Audio_StopPlayFile(uint8_t *paraIn, uint8_t lenIn, uint8_t *paraOut, uint8_t *lenOut)
{
    AudioLocal_Close(AUDIO_PLAY_MODE | AUDIO_RECORD_MODE);
    PRINT_INFO("stop play !! \r\n");
    return CODEC_OK;
}

static int8_t Audio_RecordFile(uint8_t *paraIn, uint8_t lenIn, uint8_t *paraOut, uint8_t *lenOut)
{
    uint16_t recordTime = 0;
    uint32_t recordSize = 0;
    uint32_t sampleRate = 0;
    uint8_t dataBits = 0;

    if(lenIn != 7)
    {
        PRINT_INFO("RecordFile arg len invalid: %d\r\n", lenIn);
        return CODEC_ERR_PARAMETER;
    }

    recordTime = (paraIn[0] << 8) + paraIn[1];
    sampleRate = (paraIn[2] << 24) + (paraIn[3] << 16) + (paraIn[4] << 8) + (paraIn[5]);
    dataBits = paraIn[6];

    if(Codec_CheckSampleRate(sampleRate) < 0)
    {
        PRINT_INFO("RecordFile sample rate invalid: %d\r\n", sampleRate);
        return CODEC_ERR_PARAMETER;
    }
        
    if((dataBits % 8) != 0 || dataBits == 0)
    {
        PRINT_INFO("RecordFile databits invalid: %d \r\n", dataBits);
        return CODEC_ERR_PARAMETER;
    }

    g_audioCtrlHandle.sampleRate = sampleRate;
    g_audioCtrlHandle.dataBits = dataBits;
    recordSize = recordTime * g_audioCtrlHandle.sampleRate * (g_audioCtrlHandle.dataBits / 8);

    PRINT_INFO("recordTime:%d s, recordSize: %d bytes, record samplerate: %d Hz, dataBits: %d bits\r\n", 
                recordTime, recordSize, sampleRate, dataBits);

    AudioLocal_Close(AUDIO_RECORD_MODE | AUDIO_PLAY_MODE);

    if(g_audioFile.audioRecordFile.buf)
    {
        osFree(g_audioFile.audioRecordFile.buf);
        g_audioFile.audioRecordFile.buf = NULL;
    }

    g_audioFile.audioRecordFile.size = recordSize + WAV_HEADER_SIZE;
    g_audioFile.audioRecordFile.buf  = (uint8_t *)osMalloc(g_audioFile.audioRecordFile.size);
    OS_ASSERT(g_audioFile.audioRecordFile.buf);
    g_audioFile.recordFileType = WAV_FILE_DATA;

    static AudioInfo info;

    info.ctrl = AUDIO_RECORD_MODE;
    info.recordData = g_audioFile.audioRecordFile.buf;
    info.recordSize= g_audioFile.audioRecordFile.size;
    info.dataBits = g_audioCtrlHandle.dataBits;
    info.sampleRate = g_audioCtrlHandle.sampleRate;
    info.chnls = 1;
    info.audioType = g_audioFile.recordFileType;
    info.cb = Audio_Cb;
    if(AudioLocal_Open(&info) < 0)
        return -1;

    return CODEC_OK;
}

static CmdFunc g_CtrlFunclist[] = 
{
    {GET_AUDIO_PARA,        Audio_GetPara},
    {SET_AUDIO_PARA,        Audio_SetPara},
    {START_WRITE_FILE,      Audio_WriteFile},
    {START_READ_FILE,       Audio_ReadFile},
    {AUDIO_PLAY,            Audio_PlayFile},
    {AUDIO_STOP,            Audio_StopPlayFile},
    {AUDIO_RECORD,          Audio_RecordFile}
};

int8_t Audio_DbgCtrl(uint8_t *paraIn, uint8_t lenIn, uint8_t *paraOut, uint8_t *lenOut)
{
    int8_t ret = 0;
    uint8_t i = 0;
    static bool psmlock = false;

    if(!psmlock)
    {
        psmlock = true;
    #ifdef OS_USING_PM
        PSM_AP_OFF = 1;
        PSM_CP_OFF = 1;
    #endif
    }

    Audio_Init();

    for(i = 0; i < sizeof(g_CtrlFunclist)/sizeof(CmdFunc); i++)
    {
        if( g_CtrlFunclist[i].cmd == paraIn[0])
        {
            ret = g_CtrlFunclist[i].func(paraIn + 1, lenIn - 1, paraOut, lenOut);
            break;
        }
    }

    if(i >= sizeof(g_CtrlFunclist)/sizeof(CmdFunc))
    {
        ret = CODEC_ERR_PARAMETER;
        PRINT_INFO("control cmd not support !!\r\n");
    }

    return ret;
}

/*****************************Ymodem 相关*******************************************************/
typedef struct 
{
    osThreadId_t *taskHandle;
    uint8_t      atChnlId;
    osSem_t      writeOverSem;
    osSem_t      readOverSem;
    uint8_t      *transBuf;
    uint32_t      transWrtCnt;
    uint32_t      transRdCnt;
    uint8_t       transType;        //START_READ_FILE / START_WRITE_FILE
}AudioTransferHandle;

static AudioTransferHandle  *g_audioTransferHandle = NULL;

extern F_ConGetc g_Getc;
extern F_ConPutc g_Putc;
extern F_ConPutBytes g_PutBytes;
#ifdef ICT_PSM
static WakeLock audiodbg_wakelock = {0};
#endif

#define         MAX_TRANS1K_BUF_SIZE           (PACKET_1K_SIZE + PACKET_OVERHEAD)
#define         MAX_TRANS128_BUF_SIZE          (PACKET_SIZE + PACKET_OVERHEAD)

int32_t Audio_AtPassThruCb(uint8_t ch_id, uint8_t eventId, char *data_p, uint32_t len_data)
{
    uint16_t dataIndex = 0;
    
    if(eventId != AT_CHANNEL_DATAMODE_EVENT_RX)
        return 0;
       
    if((NULL == data_p) || (0 == len_data))
    {
        OS_ASSERT(0);
    }
    // for(int i = 0; i < len_data; i++)
    // {
    //     osPrintf("%x ", data_p[i]);
    // }
    // osPrintf("\r\n");

    while(len_data != 0)
    {
        if(len_data > (MAX_TRANS1K_BUF_SIZE - g_audioTransferHandle->transWrtCnt))
        {
            memcpy(&g_audioTransferHandle->transBuf[g_audioTransferHandle->transWrtCnt], 
                    &data_p[dataIndex], 
                    MAX_TRANS1K_BUF_SIZE - g_audioTransferHandle->transWrtCnt);
            
            len_data -= (MAX_TRANS1K_BUF_SIZE - g_audioTransferHandle->transWrtCnt);
            g_audioTransferHandle->transWrtCnt = MAX_TRANS1K_BUF_SIZE;
            dataIndex += MAX_TRANS1K_BUF_SIZE - g_audioTransferHandle->transWrtCnt;
        }
        else
        {
            memcpy(&g_audioTransferHandle->transBuf[g_audioTransferHandle->transWrtCnt], 
                    &data_p[dataIndex], 
                    len_data);
            g_audioTransferHandle->transWrtCnt += len_data;
            len_data = 0;
        }

        osSemaphoreRelease(g_audioTransferHandle->writeOverSem);
        osSemaphoreAcquire(g_audioTransferHandle->readOverSem, 300); 
    }

    return 1;
}

void Audio_SetAtPassThruMode(uint8_t ch_id, uint8_t cmd)
{
    if(cmd == START_WRITE_FILE || cmd == START_READ_FILE)
    {   
        g_audioTransferHandle->atChnlId = ch_id;
        osPrintf("audio set at in pass thru \r\n");

        AT_ChannelSetDataMode(ch_id, Audio_AtPassThruCb);
#ifdef ICT_PSM
        PSM_WakeLock(&audiodbg_wakelock);
#endif
    } 
}

static void Audio_YmodemPutBytes(uint8_t *bytes, uint32_t bytesLen, uint32_t timeOut)
{
    AT_SendData(g_audioTransferHandle->atChnlId, (char *)bytes, bytesLen);
}

static void Audio_YmodemPutc(uint8_t c)
{
    AT_SendData(g_audioTransferHandle->atChnlId, (char *)&c, 1);
}

static int32_t Audio_YodemGetc(uint32_t timeout)
{
    uint32_t timeOut_tick = osTickFromMillisecond(timeout);
    uint8_t byteData = 0;
    static bool isRecvSem = false;

    while(true)
    {
        if(!isRecvSem)
        {
            if(osSemaphoreAcquire(g_audioTransferHandle->writeOverSem, timeOut_tick) != osOK)
                return -1;

            isRecvSem = true;
        }

        if(g_audioTransferHandle->transWrtCnt != g_audioTransferHandle->transRdCnt)
        {
            byteData = g_audioTransferHandle->transBuf[g_audioTransferHandle->transRdCnt++] & 0xff;
            return byteData;
        }
        else
        {
            g_audioTransferHandle->transRdCnt = 0;
            g_audioTransferHandle->transWrtCnt = 0;
            isRecvSem = false;

            osSemaphoreRelease(g_audioTransferHandle->readOverSem);
            continue;
        }
    }

    return -1;
}

static void Audio_YmodemTask(void *para)
{
    g_Getc = Audio_YodemGetc;
    g_Putc = Audio_YmodemPutc;
    g_PutBytes = Audio_YmodemPutBytes;
    osThreadMsSleep(100);
    if(g_audioTransferHandle->transType == START_WRITE_FILE)
        ymodem_receive(&g_audioFile.audioPlayFile, 1);
    else
        ymodem_send(&g_audioFile.audioRecordFile, 1, 1);
    //退出透传
    osSemaphoreRelease(g_audioTransferHandle->readOverSem);
    osThreadMsSleep(100);
    AT_ChannelClearDataMode(g_audioTransferHandle->atChnlId);

    PRINT_INFO("audio-Ymodem trans over !!!\r\n");
    AT_SendUnsolicited(g_audioTransferHandle->atChnlId, "\r\n+AUDIODBG:7\r\n", strlen("\r\n+AUDIODBG:7\r\n"));
    //释放资源
    if(g_audioTransferHandle->writeOverSem)
    {
        osSemaphoreDetach(g_audioTransferHandle->writeOverSem);
        osFree(g_audioTransferHandle->writeOverSem);
        g_audioTransferHandle->writeOverSem = NULL;
    }

    if(g_audioTransferHandle->readOverSem)
    {
        osSemaphoreDetach(g_audioTransferHandle->readOverSem);
        osFree(g_audioTransferHandle->readOverSem);
        g_audioTransferHandle->readOverSem= NULL;
    }

    if(g_audioTransferHandle->transBuf)
    {
        osFree(g_audioTransferHandle->transBuf);
        g_audioTransferHandle->transBuf = NULL;
    }

    if(g_audioTransferHandle)
    {
        osFree(g_audioTransferHandle);
        g_audioTransferHandle = NULL;
    }
#ifdef ICT_PSM
    PSM_WakeUnlock(&audiodbg_wakelock);
#endif
}

static void Audio_YmodemInit(uint8_t transType)
{
#ifdef ICT_PSM
    static bool wakeLockInitFlag = false;

    if(!wakeLockInitFlag)
    {
        PSM_WakelockInit(&audiodbg_wakelock, PSM_DEEP_SLEEP);
        wakeLockInitFlag = true;
    }   
#endif

    if(!g_audioTransferHandle)
    {
        g_audioTransferHandle = (AudioTransferHandle *)osMalloc(sizeof(AudioTransferHandle));
        memset(g_audioTransferHandle, 0, sizeof(AudioTransferHandle));
        OS_ASSERT(g_audioTransferHandle);
    }   

    if(!g_audioTransferHandle->transBuf)
    {
        g_audioTransferHandle->transBuf = (uint8_t *)osMalloc(MAX_TRANS1K_BUF_SIZE);
        OS_ASSERT(g_audioTransferHandle->transBuf);
    }

    if(!g_audioTransferHandle->writeOverSem)
    {
        g_audioTransferHandle->writeOverSem = osMalloc(sizeof(struct osSemaphore));
        OS_ASSERT(g_audioTransferHandle->writeOverSem);
        osSemaphoreInit(g_audioTransferHandle->writeOverSem, 0, 1, OS_IPC_FLAG_FIFO);
    }

    if(!g_audioTransferHandle->readOverSem)
    {
        g_audioTransferHandle->readOverSem = osMalloc(sizeof(struct osSemaphore));
        OS_ASSERT(g_audioTransferHandle->readOverSem);
        osSemaphoreInit(g_audioTransferHandle->readOverSem, 0, 1, OS_IPC_FLAG_FIFO);
    }

    g_audioTransferHandle->transType = transType;
    if(!g_audioTransferHandle->taskHandle)
    {
        osThreadAttr_t attr = {"audio-y", osThreadDetached, NULL, 0U, NULL, 4*1024, 5, 0U, 0U};
        g_audioTransferHandle->taskHandle = osThreadNew((osThreadFunc_t)Audio_YmodemTask, NULL, &attr);
        OS_ASSERT(g_audioTransferHandle->taskHandle);
    }
}
