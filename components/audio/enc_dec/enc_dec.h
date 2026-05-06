#ifndef _ENC_DEC_H_
#define _ENC_DEC_H_

#include "stdint.h"

/**
 * @brief AMR 相关定义
 * 
 */
#define     AMR_WB_FILE_TAG_LEN             9       /** WB 文件头标识长度*/
#define     AMR_NB_FILE_TAG_LEN             6       /** NB 文件头标识长度*/
#define     AMR_NB_SAMPLERATE               8000    /** NB 采样率*/
#define     AMR_WB_SAMPLERATE               16000   /** WB 采样率*/
#define     AMR_NB_FRAME_SIZE               320     /** NB 帧大小*/
#define     AMR_WB_FRAME_SIZE               640     /** WB 帧大小*/
#define     AMR_BITS_PER_SAMPLE             16      /** 采样位数*/

/**
 * @brief 编解码器的类型
 * 
 */
typedef enum
{
    AMR_DEC,
    AMR_ENC,
    MP3_DEC,
    WAV_DEC,
    WAV_ENC,
    PCMAU_DEC,
    PCMAU_ENC,
    OPUS_DEC,
    OPUS_ENC,
}AudioEncDecType;

typedef struct 
{
    /** 编解码器的初始化，文件的打开，音频信息的获取*/
    void* (*init)(void *para);   
    /** 从数据源中获取一帧数据*/   
    int32_t (*get)(void *handle, uint8_t *out, uint32_t outSize);
    /** 编解码的过程 */
    int8_t (*proc)(void *handle, uint8_t *in, uint32_t inSize, uint8_t *out, uint32_t *outSize);
    /** 编解码器的关闭，资源释放*/
    void (*deinit)(void *handle);
}EncDecOps;

#endif
