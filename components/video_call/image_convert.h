
#ifndef __IMAGE_CONVERT_H__
#define __IMAGE_CONVERT_H__

#include "stdint.h"

int8_t JPEG_RGB565ToRGB888(uint8_t *rgb565Data, uint32_t rgb565Size, uint8_t *rgb888Data, uint32_t *rgb888Size);

int8_t JPEG_RGB888ToRGB565(uint8_t *rgb888Data, uint32_t rgb888Size, uint8_t *rgb565Data, uint32_t *rgb565Size);

void YUV422ToRGB565(const void* inbuf, void* outbuf, int width, int height);

int ImageZoom(unsigned char *ptOriginPic_aucPixelDatas,unsigned int ptOriginPic_iWidth,unsigned int ptOriginPic_iHeight,unsigned char *ptZoomPic_aucPixelDatas,unsigned int ptZoomPic_iWidth,unsigned int ptZoomPic_iHeight);
#endif

