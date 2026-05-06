
#include "stdlib.h"
#include "os.h"
#include "image_convert.h"

int8_t JPEG_RGB565ToRGB888(uint8_t *rgb565Data, uint32_t rgb565Size, uint8_t *rgb888Data, uint32_t *rgb888Size)
{
    uint32_t rgb888index = 0;
    uint8_t r,g,b;
    uint16_t rgb565pix = 0;

    if(rgb565Data == NULL || rgb565Size == 0 || rgb888Data == NULL || (*rgb888Size) == 0 || rgb888Size == NULL)
    {
        osPrintf("JPEG_RGB565ToRGB888 para invalid \r\n");
        return -1;
    }

    for(uint32_t rgb565index = 0; rgb565index < rgb565Size; rgb565index += 2)
    {
        rgb565pix = (rgb565Data[rgb565index] << 8) | rgb565Data[rgb565index + 1];

        r = (rgb565pix & 0xF800) >> 8;
        g = (rgb565pix & 0x07e0) >> 3;
        b = (rgb565pix & 0x001f) << 3;

        r = r | ((r & 0x38) >> 3);
        g = g | ((g & 0x0c) >> 2);
        b = b | ((b & 0x38) >> 3);

        rgb888Data[rgb888index ++] = r;
        rgb888Data[rgb888index ++] = g;
        rgb888Data[rgb888index ++] = b;

        if(rgb565index > *rgb888Size)       // rgb888 buffer too small
        {
            osPrintf("rgb888 buffer too small \r\n");
            return -1;
        }
    }

    return 0;
}

int8_t JPEG_RGB888ToRGB565(uint8_t *rgb888Data, uint32_t rgb888Size, uint8_t *rgb565Data, uint32_t *rgb565Size)
{
    uint8_t r,g,b;
    uint16_t rgb565pix = 0;
    uint32_t rgb565Index = 0;

    if(rgb565Data == NULL || (*rgb565Size) == 0 || rgb888Data == NULL || rgb888Size == 0 || rgb565Size == NULL)
    {
        osPrintf("JPEG_RGB888ToRGB565 para invalid \r\n");
        return -1;
    }

    for(uint32_t i = 0; i < rgb888Size; i += 3)
    {
        r = rgb888Data[i];
        g = rgb888Data[i + 1];
        b = rgb888Data[i + 2];

        rgb565pix = (((r>>3) & 0x1f) << 11) | (((g >> 2) & 0x3f) << 5) | ((b >> 3) & 0x1f);

        rgb565Data[rgb565Index++] = rgb565pix & 0xFF;
        rgb565Data[rgb565Index++] = (rgb565pix >> 8) & 0xFF;

        if(rgb565Index > (*rgb565Size))
        {
            osPrintf("rgb565 buffer too small \r\n");
            return -1;
        }
    }

    return 0;
}


#define RANGE_LIMIT(x) (x > 255 ? 255 : (x < 0 ? 0 : x))
 
void YUV422ToRGB565(const void* inbuf, void* outbuf, int width, int height)
{
	int rows, cols;
	int y, u, v, r, g, b;
	unsigned char *yuv_buf;
	unsigned short *rgb_buf;
    unsigned short tmp_rgb = 0;
	int y_pos,u_pos,v_pos;
 
	yuv_buf = (unsigned char *)inbuf;
	rgb_buf = (unsigned short *)outbuf;
 
	y_pos = 0;
	u_pos = 1;
	v_pos = 3;
 
	for (rows = 0; rows < height; rows++) {
		for (cols = 0; cols < width; cols++) {
			y = yuv_buf[y_pos];
			u = yuv_buf[u_pos] - 128;
			v = yuv_buf[v_pos] - 128;
 
			r = RANGE_LIMIT(y + v + ((v * 103) >> 8));
			g = RANGE_LIMIT(y - ((u * 88) >> 8) - ((v * 183) >> 8));
			b = RANGE_LIMIT(y + u + ((u * 198) >> 8));
 
            tmp_rgb = (((r & 0xf8) << 8) | ((g & 0xfc) << 3) | ((b & 0xf8) >> 3));
			*rgb_buf++ = ((tmp_rgb>>8) & 0x00FF) + ((tmp_rgb << 8) & 0xFF00);
 
			y_pos += 2;
 
			if (cols & 0x01) {
				u_pos += 4;
				v_pos += 4;
			}
		}
	}
}

/**********************************************************************
* 函数名称： PicZoom
* 功能描述： 近邻取样插值方法缩放图片
*            注意该函数会分配内存来存放缩放后的图片,用完后要用free函数释放掉
*            "近邻取样插值"的原理请参考网友"lantianyu520"所著的"图像缩放算法"
* 输入参数：  ptOriginPic - 内含原始图片的象素数据
*             ptZoomPic    - 内含缩放后的图片的象素数据
* 输出参数： 无
* 返 回 值： 0 - 成功, 其他值 - 失败
***********************************************************************/
int ImageZoom(unsigned char *ptOriginPic_aucPixelDatas,unsigned int ptOriginPic_iWidth,unsigned int ptOriginPic_iHeight,unsigned char *ptZoomPic_aucPixelDatas,unsigned int ptZoomPic_iWidth,unsigned int ptZoomPic_iHeight)
{
	unsigned int ptOriginPic_iLineBytes=ptOriginPic_iWidth*2; //一行的字节数
	unsigned int ptZoomPic_iLineBytes=ptZoomPic_iWidth*2;  //一行的字节数

	unsigned int dwDstWidth=ptZoomPic_iWidth;
	unsigned int* pdwSrcXTable;
	unsigned int x;
	unsigned int y;
	unsigned int dwSrcY;
	unsigned char *pucDest;
	unsigned char *pucSrc;
	unsigned int dwPixelBytes=2; //像素字节
	pdwSrcXTable = (unsigned int *)osMalloc(sizeof(unsigned int) * dwDstWidth);
	if(NULL==pdwSrcXTable)
	{
		return -1;
	}

	for(x=0; x < dwDstWidth; x++)//生成表 pdwSrcXTable
	{
		pdwSrcXTable[x]=(x*ptOriginPic_iWidth/ptZoomPic_iWidth);
	}

	for(y=0; y < ptZoomPic_iHeight; y++)
	{
		dwSrcY=(y * ptOriginPic_iHeight/ptZoomPic_iHeight);

		pucDest=ptZoomPic_aucPixelDatas + y * ptZoomPic_iLineBytes;
		pucSrc=ptOriginPic_aucPixelDatas+dwSrcY * ptOriginPic_iLineBytes;

		for(x=0; x <dwDstWidth; x++)
		{
			memcpy(pucDest+x*dwPixelBytes,pucSrc+pdwSrcXTable[x]*dwPixelBytes,dwPixelBytes);
		}
	}

	osFree(pdwSrcXTable);
	return 0;
}