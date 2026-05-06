#ifndef __VIDEO_CALL_H__
#define __VIDEO_CALL_H__

#include "stdint.h"
#include "stddef.h"

#define     MAX_VIDEO_STR_LEN     64
typedef struct 
{
    char localUserId[MAX_VIDEO_STR_LEN];
    char remoteUserId[MAX_VIDEO_STR_LEN];
    char chnlId[MAX_VIDEO_STR_LEN];
}Video_CallInfo;

typedef enum
{
    VIDEO_SPEAKER    = 1,
    VIDEO_MICROPHONE = 2,
    VIDEO_CAMERA     = 4,
    VIDEO_AUDIO      = 3,
}VIDEO_DevType;

typedef enum
{
    VIDEO_DEV_CLOSE = 0,
    VIDEO_DEV_OPEN
}VIDEO_DevStat;


typedef enum
{
    VIDEO_CALL_TYPE_CLOSE = 0,
    VIDEO_CALL_TYPE_OPEN
}VIDEO_CallType;


int8_t Video_Open(char *remoteId);

int8_t Video_Start(void);

void Video_Close(void);

void Video_Ctrl(uint8_t devices, VIDEO_DevStat devStat);

int8_t Video_GetImage(uint8_t *image, uint32_t imageSize);

void Video_GetRecvImgInfo(uint16_t *imgH, uint16_t *imgW);

void Video_GetSendImgInfo(uint16_t *imgH, uint16_t *imgW);

int8_t Video_GetStatus(uint8_t devices);

Video_CallInfo *Video_GetCallInfo(void);

int8_t Video_GetType(void);

void Video_SetCallInfo(char *remoteId);
#endif
