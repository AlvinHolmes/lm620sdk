
#include <stdio.h>

#include "os.h"
#include "audio_dev.h"

#include "audio_local.h"
#include "audio_monitor.h"
#include "riscv_general.h"

#define AUDIO_LOCAL_DEBUG_PRINTF
#ifdef  AUDIO_LOCAL_DEBUG_PRINTF
    #define AUDIO_LOCAL_PRINTF        osPrintf
#else
    #define AUDIO_LOCAL_PRINTF(...)
#endif 

#define     AUDIO_FILE_READ_MAX_SIZE    AUDIO_DEFAULT_FRAME_SIZE

static osSem_t stopPlaySem = NULL;
static osSem_t playQuitSem = NULL;
static osThreadId_t audioPlayTaskHandle = NULL;

static osSem_t stopRecordSem = NULL;
static osSem_t recordQuitSem = NULL;
static osThreadId_t audioRecordTaskHandle = NULL;

// #define         AUDIO_CAL_CYCLE

#ifdef AUDIO_CAL_CYCLE
#define         MAX_CYCLE_CNT       100
static uint32_t g_cycles[MAX_CYCLE_CNT] = {0};
static uint32_t g_cycleIndex = 0;
#endif


typedef struct 
{
    EncDecOps *ops;
    AudioEncDecType type;
    osSlist_t node;
}AudioEncDecNode;

static osSlist_t *g_AudioEncDecList = NULL;
void Audio_EncDecRegister(EncDecOps *ops, AudioEncDecType type)
{
    if(!g_AudioEncDecList)
    {
        g_AudioEncDecList = osMalloc(sizeof(osSlist_t));
        OS_ASSERT(g_AudioEncDecList);
        g_AudioEncDecList->next = NULL;
    }

    AudioEncDecNode *EncDecNode = osMalloc(sizeof(AudioEncDecNode));
    OS_ASSERT(EncDecNode);
    EncDecNode->ops = ops;
    EncDecNode->type = type;
    
    osSlistInsert(g_AudioEncDecList, &EncDecNode->node);
}

EncDecOps *Audio_FindEncDecOps(AudioEncDecType type)
{
    struct osSlistNode *tmpNode = NULL;
    AudioEncDecNode *EncDecNode = NULL;

    osSlistForEach(tmpNode, g_AudioEncDecList)
    {
         EncDecNode = osListEntry(tmpNode, AudioEncDecNode, node);
         if(EncDecNode->type == type)
         {
             return EncDecNode->ops;
         }
    }

    return NULL;
}

static void _PlayTask(void *para)
{
    LocalAudioCfg *cfg = (LocalAudioCfg *)para;
    AUDIO_LOCAL_PRINTF("start play ... \r\n");
    EncDecOps *EncDecOps = NULL;
    uint8_t *pcmData = NULL;
    uint8_t *encodeData = NULL;
    bool forceStop = false;

    if(!g_AudioEncDecList)
    {
        osPrintf("No AudioEncDec \r\n");
        return ;
    }

    AudioEncDecType type = 0;    
    if(cfg->audioType == AMR_FILE_DATA || cfg->audioType == AMR_FILE)
        type = AMR_DEC;
    else if(cfg->audioType == MP3_FILE_DATA || cfg->audioType == MP3_FILE)
        type = MP3_DEC;
    else if(cfg->audioType == WAV_FILE_DATA || cfg->audioType == WAV_FILE)
        type = WAV_DEC;
    else if(cfg->audioType == OPUS_FILE_DATA || cfg->audioType == OPUS_FILE)
        type = OPUS_DEC;
    else
       osPrintf("UnSpport audio type : %d \r\n", cfg->audioType);

    EncDecOps = Audio_FindEncDecOps(type);
    
    if(!EncDecOps)
    {
        osPrintf("find encdec ops failed : %d \r\n", cfg->audioType);
        return;
    }

    void *EncDecHandle = EncDecOps->init((void *)cfg);
    if(!EncDecHandle)
    {
        osPrintf("EncDec Init Failed \r\n");
        return;
    }

    AudioDevCfg devCfg = {0};
    devCfg.chnls = cfg->chnls;
    devCfg.sampleRate = cfg->sampleRate;
    devCfg.dataBits = cfg->dataBits;
    devCfg.frameSize = cfg->frameSize;
    devCfg.dev = cfg->dev;
    devCfg.ctrl = cfg->ctrl;

    if(AudioDev_Init(&devCfg) != DRV_OK)
        goto AUDIO_PLAY_EXIT;

    pcmData = osMalloc(cfg->frameSize);
    OS_ASSERT(pcmData);

    encodeData = osMalloc(cfg->frameSize);
    OS_ASSERT(encodeData);

    int8_t devWrtRet = DRV_OK;
    uint32_t pcmSize = 0;

#ifdef AUDIO_CAL_CYCLE 
    g_cycleIndex = 0;
#endif

    while(true)
    {
        if(osSemaphoreAcquire(stopPlaySem, osNoWait) == osOK)
        {
            AUDIO_LOCAL_PRINTF("sem stop wait ok \r\n");
            forceStop = true;
            break;
        }

        if(devWrtRet == DRV_OK)
        {
            int32_t getSize = EncDecOps->get(EncDecHandle, encodeData, cfg->frameSize);
            if(getSize <= 0)
                break;
            
            pcmSize = cfg->frameSize;

        #ifdef AUDIO_CAL_CYCLE 
            unsigned long start_cycle =  (unsigned long)(__RV_CSR_READ(CSR_MCYCLE));
        #endif

            if(EncDecOps->proc(EncDecHandle, encodeData, getSize, pcmData, &pcmSize) < 0)
            {
                continue;
            } 
        
        #ifdef AUDIO_CAL_CYCLE     
            unsigned long end_cycle =  (unsigned long)(__RV_CSR_READ(CSR_MCYCLE));
            if(g_cycleIndex < MAX_CYCLE_CNT)
                g_cycles[g_cycleIndex ++] = (end_cycle - start_cycle) / 416;
        #endif

        }

        devWrtRet = AudioDev_Write(devCfg.dev, pcmData, pcmSize, osNoWait);
        if(devWrtRet == DRV_ERR_BUSY)
            osThreadMsSleep(10);
    }

AUDIO_PLAY_EXIT:
#ifdef AUDIO_CAL_CYCLE
    uint32_t sumCycle = 0;
    for(uint16_t i = 0; i < g_cycleIndex; i++)
    {
        sumCycle += g_cycles[i];
        osPrintf("%d: %d us\r\n", i, g_cycles[i]);
    }

    osPrintf("average :%d us \r\n", sumCycle / g_cycleIndex);

#endif

    if(EncDecHandle)
        EncDecOps->deinit(EncDecHandle);

    if(pcmData)
        osFree(pcmData);
 
    if(encodeData)
        osFree(encodeData);

    AudioDev_Deinit(&devCfg, forceStop);

    if(cfg->sysCb)
        cfg->sysCb(PLAY_OVER_CB_EVENT, NULL);
    if(cfg->cb)
        cfg->cb(PLAY_OVER_CB_EVENT, NULL);

    audioPlayTaskHandle = NULL;
    osSemaphoreRelease(playQuitSem);
    AUDIO_LOCAL_PRINTF("audio local play quit \r\n");
}

static void _RecordTask(void *para)
{
    LocalAudioCfg *cfg = (LocalAudioCfg *)para;
    AUDIO_LOCAL_PRINTF("start record ... \r\n");
    EncDecOps *EncDecOps = NULL;
    uint8_t *pcmData = NULL;
    uint8_t *encodeData = NULL;

    if(!g_AudioEncDecList)
    {
        osPrintf("No AudioEncDec \r\n");
        return ;
    }

    AudioEncDecType type = 0;    
    if(cfg->audioType == AMR_FILE_DATA)
        type = AMR_ENC;
    if(cfg->audioType == WAV_FILE_DATA)
        type = WAV_ENC;
    else
       osPrintf("UnSpport audio type : %d \r\n", cfg->audioType);

    EncDecOps = Audio_FindEncDecOps(type);
    
    if(!EncDecOps)
    {
        osPrintf("find encdec ops failed : %d \r\n", cfg->audioType);
        return;
    }

    void *EncDecHandle = EncDecOps->init((void *)cfg);
    if(!EncDecHandle)
    {
        osPrintf("EncDec Init Failed \r\n");
        return;
    }

    AudioDevCfg devCfg = {0};
    devCfg.chnls = cfg->chnls;
    devCfg.sampleRate = cfg->sampleRate;
    devCfg.dataBits = cfg->dataBits;
    devCfg.frameSize = cfg->frameSize;
    devCfg.dev = cfg->dev;
    devCfg.ctrl = cfg->ctrl;

    if(AudioDev_Init(&devCfg) != DRV_OK)
        goto AUDIO_RECORD_EXIT;

    pcmData = osMalloc(cfg->frameSize);
    OS_ASSERT(pcmData);

    encodeData = osMalloc(cfg->frameSize);
    OS_ASSERT(encodeData);

    uint32_t encodeSize = 0;
#ifdef AUDIO_CAL_CYCLE 
    g_cycleIndex = 0;
#endif
    while(true)
    {
        if(osSemaphoreAcquire(stopRecordSem, osNoWait) == osOK)
            break;

        //audio read
        if(AudioDev_Read(devCfg.dev, pcmData, devCfg.frameSize, osNoWait) < 0)
        {
            osThreadMsSleep(10);
            continue;
        }

        encodeSize = devCfg.frameSize;
        
        #ifdef AUDIO_CAL_CYCLE 
            unsigned long start_cycle =  (unsigned long)(__RV_CSR_READ(CSR_MCYCLE));
        #endif

        if(EncDecOps->proc(EncDecHandle, pcmData, devCfg.frameSize, encodeData, &encodeSize) < 0)
        {
            continue;
        }

        #ifdef AUDIO_CAL_CYCLE     
            unsigned long end_cycle =  (unsigned long)(__RV_CSR_READ(CSR_MCYCLE));
            if(g_cycleIndex < MAX_CYCLE_CNT)
                g_cycles[g_cycleIndex ++] = (end_cycle - start_cycle) / 416;
        #endif

        //store
        if((cfg->recordIndex + encodeSize) < cfg->recordSize)
        {
            memcpy(&cfg->recordData[cfg->recordIndex], encodeData, encodeSize);
            cfg->recordIndex += encodeSize;
        }
        else
            break;
    }

AUDIO_RECORD_EXIT:

#ifdef AUDIO_CAL_CYCLE
    uint32_t sumCycle = 0;
    for(uint16_t i = 0; i < g_cycleIndex; i++)
    {
        sumCycle += g_cycles[i];
        osPrintf("%d: %d us\r\n", i, g_cycles[i]);
    }

    osPrintf("average :%d us \r\n", sumCycle / g_cycleIndex);

#endif

    cfg->recordSize = cfg->recordIndex;

    if(EncDecHandle)
        EncDecOps->deinit(EncDecHandle);

    if(pcmData)
        osFree(pcmData);
 
    if(encodeData)
        osFree(encodeData);

    AudioDev_Deinit(&devCfg, true);

    if(!(cfg->ctrl & AUDIO_PLAY_MODE))
    {
        if(cfg->sysCb)
            cfg->sysCb(RECORD_OVER_CB_EVENT, NULL);
    }

    if(cfg->cb)
        cfg->cb(RECORD_OVER_CB_EVENT, NULL);

    audioRecordTaskHandle = NULL;
    osSemaphoreRelease(recordQuitSem);

    AUDIO_LOCAL_PRINTF("audio local record quit \r\n");
}

static LocalAudioCfg *g_recordAudioCfg = NULL;
static LocalAudioCfg *g_userRecordAudioCfg = NULL;
static LocalAudioCfg *g_playAudioCfg = NULL;

int8_t AudioLocal_Open(void * cfg)
{
    if(!cfg)
    {
        osPrintf("para error \r\n");
        return -1;
    }
    LocalAudioCfg *audioCfg = (LocalAudioCfg *)cfg;

    if(audioCfg->ctrl & AUDIO_PLAY_MODE )
    {
        if(!g_playAudioCfg)
        {
            g_playAudioCfg = osMalloc(sizeof(LocalAudioCfg));
            OS_ASSERT(g_playAudioCfg);
        }
        memcpy(g_playAudioCfg, audioCfg, sizeof(LocalAudioCfg));

        if(stopPlaySem)
        {
            osSemaphoreDetach(stopPlaySem);
            osFree(stopPlaySem);
            stopPlaySem = NULL;
        }

        if(playQuitSem)
        {
            osSemaphoreDetach(playQuitSem);
            osFree(playQuitSem);
            playQuitSem = NULL; 
        }

        stopPlaySem = osMalloc(sizeof(struct osSemaphore));
        OS_ASSERT(stopPlaySem);
        osSemaphoreInit(stopPlaySem, 0, 1, OS_IPC_FLAG_FIFO);
 
        playQuitSem = osMalloc(sizeof(struct osSemaphore));
        OS_ASSERT(playQuitSem);
        osSemaphoreInit(playQuitSem, 0, 1, OS_IPC_FLAG_FIFO);

        //open task
        osThreadAttr_t attr = {"audio-p", osThreadDetached, NULL, 0U, NULL, 4*1024, 5, 0U, 0U};
        audioPlayTaskHandle = osThreadNew((osThreadFunc_t)_PlayTask, g_playAudioCfg, &attr);
        OS_ASSERT(audioPlayTaskHandle);
    }
    
    if(audioCfg->ctrl & AUDIO_RECORD_MODE)
    {
        if(!g_recordAudioCfg)
        {
            g_recordAudioCfg = osMalloc(sizeof(LocalAudioCfg));
            OS_ASSERT(g_recordAudioCfg);
        }
        memcpy(g_recordAudioCfg, audioCfg, sizeof(LocalAudioCfg));
        g_userRecordAudioCfg = audioCfg;

        if(stopRecordSem)
        {
            osSemaphoreDetach(stopRecordSem);
            osFree(stopRecordSem);
            stopRecordSem = NULL;
        }

        if(recordQuitSem)
        {
            osSemaphoreDetach(recordQuitSem);
            osFree(recordQuitSem);
            recordQuitSem = NULL; 
        }
        stopRecordSem = osMalloc(sizeof(struct osSemaphore));
        OS_ASSERT(stopRecordSem);
        osSemaphoreInit(stopRecordSem, 0, 1, OS_IPC_FLAG_FIFO);
 
        recordQuitSem = osMalloc(sizeof(struct osSemaphore));
        OS_ASSERT(recordQuitSem);
        osSemaphoreInit(recordQuitSem, 0, 1, OS_IPC_FLAG_FIFO);

        osThreadAttr_t attr = {"audio-r", osThreadDetached, NULL, 0U, NULL, 12*1024, 7, 0U, 0U};
        audioRecordTaskHandle = osThreadNew((osThreadFunc_t)_RecordTask, g_recordAudioCfg, &attr);
        OS_ASSERT(audioRecordTaskHandle);
    }

    return 0;
}

int8_t AudioLocal_Close(uint8_t ctrl)
{
    AUDIO_LOCAL_PRINTF("audio local close %d \r\n", ctrl);
    if(ctrl & AUDIO_PLAY_MODE)
    {
        if(audioPlayTaskHandle)
        {
            if(stopPlaySem)
            {
                osSemaphoreRelease(stopPlaySem);
                osSemaphoreAcquire(playQuitSem, osWaitForever);
            }
        }

        if(g_playAudioCfg)
        {
            osFree(g_playAudioCfg);
            g_playAudioCfg = NULL;
        }
    }

    if(ctrl & AUDIO_RECORD_MODE)
    {
        if(audioRecordTaskHandle)
        {
            if(stopRecordSem)
            {
                osSemaphoreRelease(stopRecordSem);
                osSemaphoreAcquire(recordQuitSem, osWaitForever);
            }
        }

        if(g_userRecordAudioCfg && g_recordAudioCfg)
        {
            g_userRecordAudioCfg->recordSize = g_recordAudioCfg->recordSize;
        }

        if(g_recordAudioCfg)
        {
            osFree(g_recordAudioCfg);
            g_recordAudioCfg = NULL;
        }
    }

    return 0;
}