
#include "os.h"
#include "os_hw.h"

#include "amr.h"
#include "interf_enc.h"
#include "interf_dec.h"
#include "enc_if.h"
#include "dec_if.h"

osMutex_t  dec_ctx_mutex[MAX_AMR_CTX] = {NULL};
osMutex_t  enc_ctx_mutex[MAX_AMR_CTX] = {NULL};
struct ict_amr *g_amr = NULL;

int ict_amr_pre_init(void)
{
    if(!dec_ctx_mutex[AMR_NB_CTX])
    {
        dec_ctx_mutex[AMR_NB_CTX] = osMalloc(sizeof(struct osMutex ));
        OS_ASSERT(dec_ctx_mutex[AMR_NB_CTX]);
        if (osMutexInit(dec_ctx_mutex[AMR_NB_CTX], OS_IPC_FLAG_FIFO) < 0)
            OS_ASSERT(0);
    }

    if(!dec_ctx_mutex[AMR_WB_CTX])
    {
        dec_ctx_mutex[AMR_WB_CTX] = osMalloc(sizeof(struct osMutex ));
        OS_ASSERT(dec_ctx_mutex[AMR_WB_CTX]);
        if (osMutexInit(dec_ctx_mutex[AMR_WB_CTX], OS_IPC_FLAG_FIFO) < 0)
            OS_ASSERT(0);
    }

    if(!enc_ctx_mutex[AMR_NB_CTX])
    {
        enc_ctx_mutex[AMR_NB_CTX] = osMalloc(sizeof(struct osMutex ));
        OS_ASSERT(enc_ctx_mutex[AMR_NB_CTX]);
        if (osMutexInit(enc_ctx_mutex[AMR_NB_CTX], OS_IPC_FLAG_FIFO) < 0)
            OS_ASSERT(0);
    }

    if(!enc_ctx_mutex[AMR_WB_CTX])
    {
        enc_ctx_mutex[AMR_WB_CTX] = osMalloc(sizeof(struct osMutex ));
        OS_ASSERT(enc_ctx_mutex[AMR_WB_CTX]);
        if (osMutexInit(enc_ctx_mutex[AMR_WB_CTX], OS_IPC_FLAG_FIFO) < 0)
            OS_ASSERT(0);
    }

    if(!g_amr)
    {
        g_amr = osMalloc(sizeof(struct ict_amr));
        OS_ASSERT(g_amr);

        g_amr->cfg = osMalloc(sizeof(struct ict_amr_cfg));
        OS_ASSERT(g_amr->cfg);

        g_amr->cfg->dec_codec_ctx[AMR_WB_CTX] = NULL;
        g_amr->cfg->enc_codec_ctx[AMR_WB_CTX] = NULL;
        g_amr->cfg->dec_codec_ctx[AMR_NB_CTX] = NULL;
        g_amr->cfg->enc_codec_ctx[AMR_NB_CTX] = NULL;
    }

    osPrintf("amr pre init ok \r\n");
    return 0;
}
INIT_APP_EXPORT(ict_amr_pre_init, OS_INIT_SUBLEVEL_4);

static int8_t ict_amr_dec_init(AMR_CtxId ctxId)
{
    osMutexAcquire(dec_ctx_mutex[ctxId], osWaitForever);
    if(g_amr->cfg->dec_codec_ctx[ctxId])
    {
        osMutexRelease(dec_ctx_mutex[ctxId]);
        return 0;
    }

    g_amr->cfg->dec_codec_ctx[ctxId] = osMalloc(sizeof(T_Mmp_AmrCodecContext));
    OS_ASSERT(g_amr->cfg->dec_codec_ctx[ctxId]);
    memset((uint8_t*)g_amr->cfg->dec_codec_ctx[ctxId], 0, sizeof(T_Mmp_AmrCodecContext));  

    if(g_amr->cfg->dec_codec_ctx[ctxId]->In || g_amr->cfg->dec_codec_ctx[ctxId]->Out)
    {
        OS_ASSERT(0);
    }

    g_amr->cfg->dec_codec_ctx[ctxId]->In = osMalloc(64);
    OS_ASSERT(g_amr->cfg->dec_codec_ctx[ctxId]->In);
    
    g_amr->cfg->dec_codec_ctx[ctxId]->Out = osMalloc(640);
    OS_ASSERT(g_amr->cfg->dec_codec_ctx[ctxId]->Out);
    
    osMutexRelease(dec_ctx_mutex[ctxId]);

    return 0;
}

static int8_t ict_amr_enc_init(AMR_CtxId ctxId)
{
    osMutexAcquire(enc_ctx_mutex[ctxId], osWaitForever);

    if(g_amr->cfg->enc_codec_ctx[ctxId])
    {
        osMutexRelease(enc_ctx_mutex[ctxId]);
        return 0;
    }
    
    g_amr->cfg->enc_codec_ctx[ctxId] = osMalloc(sizeof(T_Mmp_AmrCodecContext));
    OS_ASSERT(g_amr->cfg->enc_codec_ctx[ctxId]);
    
    memset((uint8_t*)g_amr->cfg->enc_codec_ctx[ctxId], 0, sizeof(T_Mmp_AmrCodecContext));   

    if(g_amr->cfg->enc_codec_ctx[ctxId]->In || g_amr->cfg->enc_codec_ctx[ctxId]->Out)
    {
        OS_ASSERT(0);
    }
    
    g_amr->cfg->enc_codec_ctx[ctxId]->In = osMalloc(640);
    OS_ASSERT(g_amr->cfg->enc_codec_ctx[ctxId]->In);
    
    g_amr->cfg->enc_codec_ctx[ctxId]->Out = osMalloc(64);  
    OS_ASSERT(g_amr->cfg->enc_codec_ctx[ctxId]->Out);

    osMutexRelease(enc_ctx_mutex[ctxId]);
    return 0;
}

static int8_t ict_amr_dec_deinit(AMR_CtxId ctxId)
{
    osMutexAcquire(dec_ctx_mutex[ctxId], osWaitForever);

    if(g_amr->cfg->dec_codec_ctx[ctxId]->In)
        osFree(g_amr->cfg->dec_codec_ctx[ctxId]->In);

    if(g_amr->cfg->dec_codec_ctx[ctxId]->Out)    
        osFree(g_amr->cfg->dec_codec_ctx[ctxId]->Out);

    if(g_amr->cfg->dec_codec_ctx[ctxId])
        osFree(g_amr->cfg->dec_codec_ctx[ctxId]);

    g_amr->cfg->dec_codec_ctx[ctxId]->In = OS_NULL;
    g_amr->cfg->dec_codec_ctx[ctxId]->Out = OS_NULL;
    g_amr->cfg->dec_codec_ctx[ctxId] = OS_NULL;

    osMutexRelease(dec_ctx_mutex[ctxId]);    

    return 0;
}

static int8_t ict_amr_enc_deinit(AMR_CtxId ctxId)
{
    osMutexAcquire(enc_ctx_mutex[ctxId], osWaitForever);

    if(g_amr->cfg->enc_codec_ctx[ctxId]->In)
        osFree(g_amr->cfg->enc_codec_ctx[ctxId]->In);

    if(g_amr->cfg->enc_codec_ctx[ctxId]->Out)
        osFree(g_amr->cfg->enc_codec_ctx[ctxId]->Out);

    if(g_amr->cfg->enc_codec_ctx[ctxId])
        osFree(g_amr->cfg->enc_codec_ctx[ctxId]);

    g_amr->cfg->enc_codec_ctx[ctxId]->In = OS_NULL;
    g_amr->cfg->enc_codec_ctx[ctxId]->Out = OS_NULL;
    g_amr->cfg->enc_codec_ctx[ctxId] = OS_NULL;

    osMutexRelease(enc_ctx_mutex[ctxId]);
    return 0;
}

int8_t ict_amr_dec_open(AMR_CtxId ctxId, T_Mmp_CodecType codecType)
{
    if(ctxId >= MAX_AMR_CTX || codecType > ZMMP_CODEC_AMR_WB_IETF)
    {
        //osPrintf("ict_amr_dec_open para error %d, %d \r\n", ctxId, codecType);
        return -1;
    }

    ict_amr_dec_init(ctxId);

    osMutexAcquire(dec_ctx_mutex[ctxId], osWaitForever);
    if(g_amr->cfg->dec_codec_ctx[ctxId] == OS_NULL)
    {
        osMutexRelease(dec_ctx_mutex[ctxId]);
        //osPrintf("dec_codec_ctx[%d] is NULL \r\n", ctxId);
        return -1;
    }

    if(g_amr->cfg->dec_codec_ctx[ctxId]->pCodecState)
    {
        osMutexRelease(dec_ctx_mutex[ctxId]);
        //osPrintf("amr dec %d open already \r\n", ctxId);
        return 0;
    }
        
    g_amr->cfg->dec_codec_ctx[ctxId]->codecType = codecType;
    g_amr->cfg->dec_codec_ctx[ctxId]->pCodecState = OS_NULL;
    g_amr->cfg->dec_codec_ctx[ctxId]->isWb = OS_FALSE;

    if( codecType >= ZMMP_CODEC_AMR_NB_IF1  &&  codecType <= ZMMP_CODEC_AMR_NB_IETF )
    {
        g_amr->cfg->dec_codec_ctx[ctxId]->handle.decHandle.decInit = Decoder_Interface_init;
        g_amr->cfg->dec_codec_ctx[ctxId]->handle.decHandle.decExit = Decoder_Interface_exit;
        g_amr->cfg->dec_codec_ctx[ctxId]->handle.decHandle.decProc = Decoder_Interface_Decode;
    }
    else
    {
        g_amr->cfg->dec_codec_ctx[ctxId]->handle.decHandle.decInit = D_IF_init;
        g_amr->cfg->dec_codec_ctx[ctxId]->handle.decHandle.decExit = D_IF_exit;
        g_amr->cfg->dec_codec_ctx[ctxId]->handle.decHandle.decProc = D_IF_decode;
        g_amr->cfg->dec_codec_ctx[ctxId]->isWb = OS_TRUE;
    }

    g_amr->cfg->dec_codec_ctx[ctxId]->pCodecState = g_amr->cfg->dec_codec_ctx[ctxId]->handle.decHandle.decInit();
    if(g_amr->cfg->dec_codec_ctx[ctxId]->pCodecState == OS_NULL)
    {
        osMutexRelease(dec_ctx_mutex[ctxId]);
        //osPrintf("amr %d dec failed \r\n", ctxId);
        return -1;
    }
    osMutexRelease(dec_ctx_mutex[ctxId]);

    osPrintf("amr %d dec open ok \r\n", ctxId);
    return 0;
}

int8_t ict_amr_enc_open(AMR_CtxId ctxId, T_DrvVoice_EncOpenArg *open_arg)
{
    if(ctxId >= MAX_AMR_CTX || !open_arg)
    {
        //osPrintf("ict_amr_enc_open para error %d, %d\r\n", ctxId, (uint32_t)open_arg);
        return -1;
    }

    ict_amr_enc_init(ctxId);

    osMutexAcquire(enc_ctx_mutex[ctxId], osWaitForever);

    if(g_amr->cfg->enc_codec_ctx[ctxId] == OS_NULL)
    {
        osMutexRelease(enc_ctx_mutex[ctxId]);
        //osPrintf("amr enc %d ctx is NULL \r\n", ctxId);
        return -1;
    }

    if(g_amr->cfg->enc_codec_ctx[ctxId]->pCodecState)
    {
        osMutexRelease(enc_ctx_mutex[ctxId]);
        //osPrintf("amr %d enc already \r\n", ctxId);
        return 0;
    }
        
    g_amr->cfg->enc_codec_ctx[ctxId]->codecType = open_arg->codecType;
    g_amr->cfg->enc_codec_ctx[ctxId]->pCodecState = OS_NULL;
    g_amr->cfg->enc_codec_ctx[ctxId]->isDtxEnable = open_arg->isDtxEnable;
    g_amr->cfg->enc_codec_ctx[ctxId]->isWb = OS_FALSE;

    if(open_arg->codecType >= ZMMP_CODEC_AMR_NB_IF1  &&  open_arg->codecType <= ZMMP_CODEC_AMR_NB_IETF )
    {
        g_amr->cfg->enc_codec_ctx[ctxId]->handle.encHandle.encInit = Encoder_Interface_init;
        g_amr->cfg->enc_codec_ctx[ctxId]->handle.encHandle.encExit = Encoder_Interface_exit;
        g_amr->cfg->enc_codec_ctx[ctxId]->handle.encHandle.encProc = Encoder_Interface_Encode;
    }
    else
    {
        g_amr->cfg->enc_codec_ctx[ctxId]->handle.encHandle.encInit = E_IF_init;
        g_amr->cfg->enc_codec_ctx[ctxId]->handle.encHandle.encExit = E_IF_exit;
        g_amr->cfg->enc_codec_ctx[ctxId]->handle.encHandle.encProc = E_IF_encode;
        g_amr->cfg->enc_codec_ctx[ctxId]->isWb = OS_TRUE;
    }

    g_amr->cfg->enc_codec_ctx[ctxId]->pCodecState = g_amr->cfg->enc_codec_ctx[ctxId]->handle.encHandle.encInit(g_amr->cfg->enc_codec_ctx[ctxId]->isDtxEnable);
    if(g_amr->cfg->enc_codec_ctx[ctxId]->pCodecState == OS_NULL)
    {
        osMutexRelease(enc_ctx_mutex[ctxId]);
        //osPrintf("amr %d enc open failed \r\n", ctxId);
        return -2;
    }
    osMutexRelease(enc_ctx_mutex[ctxId]);

    osPrintf("amr %d enc open ok \r\n", ctxId);
    return 0;
}

//#define     AMR_TEST_CYCLE

#ifdef AMR_TEST_CYCLE
#include "os.h"
#include "riscv_general.h"
uint64_t start_cycle = 0;
uint64_t end_cycle = 0;

#define     AMR_TEST_TOTAL_CYCLES        100
uint32_t decCycle[AMR_TEST_TOTAL_CYCLES] = {0};
uint32_t decCycleIndex = 0;

uint32_t encCycle[AMR_TEST_TOTAL_CYCLES] = {0};
uint32_t encCycleIndex = 0;

static uint32_t decMaxcycle = 0;
static uint32_t encMaxcycle = 0;
#endif

int8_t ict_amr_dec_proc(AMR_CtxId ctxId, T_DrvVoice_AmrDecodeArg *dec_arg)
{
    int bfi = 0;
#ifdef AMR_TEST_CYCLE
    start_cycle = (unsigned long)(__RV_CSR_READ(CSR_MCYCLE));
#endif
    if(dec_arg == OS_NULL || ctxId >= MAX_AMR_CTX)
    {
        //osPrintf("ict_amr_dec_proc para error \r\n");
        return -1;
    }

    osMutexAcquire(dec_ctx_mutex[ctxId], osWaitForever);
    if(!g_amr->cfg->dec_codec_ctx[ctxId]->pCodecState)
    {
        osMutexRelease(dec_ctx_mutex[ctxId]);
        return -2;
    }  

    dec_arg->pcmFrameSize = 320;
    memcpy((uint8_t *)g_amr->cfg->dec_codec_ctx[ctxId]->In, (uint8_t *)dec_arg->pAmrDec, dec_arg->amrdecsize);

    if(g_amr->cfg->dec_codec_ctx[ctxId]->isWb)
    {
        dec_arg->pcmFrameSize = 640;
        bfi = 0;
    }
    else
        bfi = g_amr->cfg->dec_codec_ctx[ctxId]->codecType;

    g_amr->cfg->dec_codec_ctx[ctxId]->handle.decHandle.decProc(g_amr->cfg->dec_codec_ctx[ctxId]->pCodecState, 
                                                                    (uint8_t *)g_amr->cfg->dec_codec_ctx[ctxId]->In, 
                                                                    (int16_t *)g_amr->cfg->dec_codec_ctx[ctxId]->Out,
                                                                    bfi
                                                                    );
    memcpy((uint8_t *)dec_arg->pPcmDec, (uint8_t *)g_amr->cfg->dec_codec_ctx[ctxId]->Out, dec_arg->pcmFrameSize );
    osMutexRelease(dec_ctx_mutex[ctxId]);


    //test
#ifdef AMR_TEST_CYCLE
    end_cycle = (unsigned long)(__RV_CSR_READ(CSR_MCYCLE));
    decCycle[decCycleIndex] =  (uint32_t)(end_cycle - start_cycle);

    if(decCycleIndex == (AMR_TEST_TOTAL_CYCLES -1))
    {
        uint32_t i = 0;
        uint64_t sum = 0;

        for(i = 0; i < AMR_TEST_TOTAL_CYCLES; i++)
        {
            sum += decCycle[i];
        }

        if(decMaxcycle < (uint32_t)(sum / AMR_TEST_TOTAL_CYCLES))
        {
            decMaxcycle = (uint32_t)(sum / AMR_TEST_TOTAL_CYCLES);
            osPrintf("d:%d \r\n", decMaxcycle);
        }
    }

    decCycleIndex = (decCycleIndex + 1) % AMR_TEST_TOTAL_CYCLES;
#endif

    return 0;
}


int8_t ict_amr_enc_proc(AMR_CtxId ctxId, T_DrvVoice_AmrEncodeArg *enc_arg)
{
    int8_t size = -1;
#ifdef AMR_TEST_CYCLE
    start_cycle = (unsigned long)(__RV_CSR_READ(CSR_MCYCLE));
#endif
    if(enc_arg == OS_NULL || ctxId >= MAX_AMR_CTX)
    {
        //osPrintf("ict_amr_enc_proc para error %d, %d \r\n",(uint32_t)enc_arg, ctxId);
        return -1;
    }

    osMutexAcquire(enc_ctx_mutex[ctxId], osWaitForever);
    if(!g_amr->cfg->enc_codec_ctx[ctxId]->pCodecState)
    {
        osMutexRelease(enc_ctx_mutex[ctxId]);
        //osPrintf("amr enc proc error, amr %d not open \r\n", ctxId);
        return -2;
    }

    memcpy((uint8_t *)g_amr->cfg->enc_codec_ctx[ctxId]->In, (uint8_t *)enc_arg->pPcmEnc, enc_arg->pcmencsize );
    size = g_amr->cfg->enc_codec_ctx[ctxId]->handle.encHandle.encProc(g_amr->cfg->enc_codec_ctx[ctxId]->pCodecState,
                                                        enc_arg->mode,
                                                        (int16_t *)g_amr->cfg->enc_codec_ctx[ctxId]->In,
                                                        (uint8_t *)g_amr->cfg->enc_codec_ctx[ctxId]->Out,
                                                        g_amr->cfg->enc_codec_ctx[ctxId]->codecType
                                                        );
    if(size <= 0)
    {
        osMutexRelease(enc_ctx_mutex[ctxId]);
        //osPrintf("amr %d enc failed, %d \r\n", ctxId, size);
        return -3;
    }

    memcpy(enc_arg->pAmrEnc, (uint8_t *)g_amr->cfg->enc_codec_ctx[ctxId]->Out, size);
    enc_arg->amrFrameSize = size;
    osMutexRelease(enc_ctx_mutex[ctxId]);

#ifdef AMR_TEST_CYCLE
    end_cycle = (unsigned long)(__RV_CSR_READ(CSR_MCYCLE));
    encCycle[encCycleIndex] =  (uint32_t)(end_cycle - start_cycle);

    if(encCycleIndex == (AMR_TEST_TOTAL_CYCLES -1))
    {
        uint32_t i = 0;
        uint64_t sum = 0;

        for(i = 0; i < AMR_TEST_TOTAL_CYCLES; i++)
        {
            sum += encCycle[i];
        }

        if(encMaxcycle < (uint32_t)(sum / AMR_TEST_TOTAL_CYCLES))
        {
            encMaxcycle = (uint32_t)(sum / AMR_TEST_TOTAL_CYCLES);
            osPrintf("e:%d \r\n", encMaxcycle);
        }
            
    }

    encCycleIndex = (encCycleIndex + 1) % AMR_TEST_TOTAL_CYCLES;
#endif
    return 0;
}

int8_t ict_amr_enc_close(AMR_CtxId ctxId)
{
    if(ctxId >= MAX_AMR_CTX)
    {
       // osPrintf("ict_amr_enc_close para error %d \r\n", ctxId);
        return -1;
    }

    if(!g_amr->cfg->enc_codec_ctx[ctxId])
    {
       // osPrintf("amr %d enc ctx is NULL \r\n", ctxId);
        return 0;
    }


    osMutexAcquire(enc_ctx_mutex[ctxId], osWaitForever);
    if(g_amr->cfg->enc_codec_ctx[ctxId]->pCodecState)
        g_amr->cfg->enc_codec_ctx[ctxId]->handle.encHandle.encExit(g_amr->cfg->enc_codec_ctx[ctxId]->pCodecState);
    g_amr->cfg->enc_codec_ctx[ctxId]->pCodecState = NULL;
    osMutexRelease(enc_ctx_mutex[ctxId]);

    ict_amr_enc_deinit(ctxId);

    osPrintf("amr %d enc close \r\n", ctxId);

#ifdef AMR_TEST_CYCLE
    encMaxcycle = 0;
    decMaxcycle = 0;
    memset(decCycle, 0, AMR_TEST_TOTAL_CYCLES *4);
    memset(encCycle, 0, AMR_TEST_TOTAL_CYCLES*4);
#endif
    return 0;
}

int8_t ict_amr_dec_close(AMR_CtxId ctxId)
{
    if(ctxId >= MAX_AMR_CTX)
    {
       // osPrintf("ict_amr_dec_close para error %d \r\n", ctxId);
        return -1;
    }

    if(!g_amr->cfg->dec_codec_ctx[ctxId])
    {
        //osPrintf("amr %d dec ctx is NULL \r\n", ctxId);
        return 0;
    }

    osMutexAcquire(dec_ctx_mutex[ctxId], osWaitForever);
    if(g_amr->cfg->dec_codec_ctx[ctxId]->pCodecState)
        g_amr->cfg->dec_codec_ctx[ctxId]->handle.decHandle.decExit(g_amr->cfg->dec_codec_ctx[ctxId]->pCodecState);
    g_amr->cfg->dec_codec_ctx[ctxId]->pCodecState = NULL;
    osMutexRelease(dec_ctx_mutex[ctxId]);

    ict_amr_dec_deinit(ctxId);

    osPrintf("amr %d dec close \r\n", ctxId);
    return 0;
}

