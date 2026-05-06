#ifndef __AUDIO_DBG__
#define __AUDIO_DBG__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    GET_AUDIO_PARA,
    SET_AUDIO_PARA,
    START_WRITE_FILE,
    START_READ_FILE,
    AUDIO_PLAY,
    AUDIO_STOP,
    AUDIO_RECORD
}AudioDbgCmdType;

typedef struct
{
    AudioDbgCmdType cmd;
    uint8_t *data;  // len data 
}AudioDbgPara;

int8_t Audio_DbgCtrl(uint8_t *paraIn, uint8_t lenIn, uint8_t *paraOut, uint8_t *lenOut);
void Audio_SetAtPassThruMode(uint8_t ch_id, uint8_t cmd);

#endif
