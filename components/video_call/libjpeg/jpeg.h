
#ifndef __JPEG_H__
#define __JPEG_H__

#include "stdint.h"

int8_t JPEG_Compress(uint8_t *image, uint16_t imageH, uint16_t imageW, uint8_t *outBuf, uint64_t *outSize);

int8_t JPEG_DeCompress( uint8_t *inbuffer,  uint32_t insize, uint8_t *outbuffer, uint32_t outsize);

#endif

