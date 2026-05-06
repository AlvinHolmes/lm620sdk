
#include "os.h"
#include "image_convert.h"
#include "libjpeg-turbo-3.0.1/jpeglib.h"
#include "libjpeg-turbo-3.0.1/jerror.h"
#include <setjmp.h>

#define JPEG_DEBUG_PRINT

#ifdef JPEG_DEBUG_PRINT
    #define JPEG_PRINT osPrintf
#else
    #define JPEG_PRINT(...)
#endif

int8_t JPEG_Compress(uint8_t *image, uint16_t imageH, uint16_t imageW, uint8_t *outBuf, uint64_t *outSize)
{
    struct  jpeg_compress_struct *cinfoPtr = NULL;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    int row_stride;

    // RGB 565 to RGB 888
    uint32_t rgb888Size = imageH * imageW * 3;
    uint8_t *rgb888Buf = osMalloc(rgb888Size);
    OS_ASSERT(rgb888Buf);

    if(JPEG_RGB565ToRGB888(image, imageH * imageW * 2, rgb888Buf, &rgb888Size) != 0)
    {
        osFree(rgb888Buf);
        return -1;
    }
    cinfoPtr = osMalloc(sizeof(struct  jpeg_compress_struct));
    OS_ASSERT(cinfoPtr);

    cinfoPtr->err = jpeg_std_error(&jerr);
    jpeg_create_compress(cinfoPtr);
        
    jpeg_mem_dest(cinfoPtr,  &outBuf, (unsigned long *)outSize);

    cinfoPtr->image_width = imageW;
    cinfoPtr->image_height = imageH;
    cinfoPtr->input_components = 3;
    cinfoPtr->in_color_space = JCS_RGB;
    cinfoPtr->data_precision = 8;

    jpeg_set_defaults(cinfoPtr);
    jpeg_set_quality(cinfoPtr, 80, TRUE);
    jpeg_start_compress(cinfoPtr, TRUE);

    row_stride = imageW * 3;

    while (cinfoPtr->next_scanline < cinfoPtr->image_height) 
    {
        row_pointer[0] = &rgb888Buf[cinfoPtr->next_scanline * row_stride];
        (void)jpeg_write_scanlines(cinfoPtr, row_pointer, 1);
    }

    jpeg_finish_compress(cinfoPtr);
    jpeg_destroy_compress(cinfoPtr);

    osFree(rgb888Buf);
    osFree(cinfoPtr);

    return 0;
}


struct my_error_mgr {
  struct jpeg_error_mgr pub;    /* "public" fields */

  jmp_buf setjmp_buffer;        /* for return to caller */
};

typedef struct my_error_mgr *my_error_ptr;

METHODDEF(void)
my_error_exit(j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr)cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

int8_t JPEG_DeCompress( uint8_t *inbuffer,  uint32_t insize, uint8_t *outbuffer, uint32_t outsize)
{
    struct my_error_mgr jerr;
    JSAMPARRAY buffer = NULL;
    uint32_t write_index = 0;
    uint32_t row_stride;
    struct jpeg_decompress_struct *cinfoPtr = NULL;
    uint32_t rgb565Size = 0;

    cinfoPtr = osMalloc(sizeof(struct jpeg_decompress_struct));
    OS_ASSERT(cinfoPtr);

    cinfoPtr->err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    if (setjmp(jerr.setjmp_buffer)) 
    {
        JPEG_PRINT("error decompress \r\n");
        jpeg_destroy_decompress(cinfoPtr);
        return -1;
    }
 
    jpeg_create_decompress(cinfoPtr);
    jpeg_mem_src(cinfoPtr, inbuffer, insize);
    (void)jpeg_read_header(cinfoPtr, TRUE);
    (void)jpeg_start_decompress(cinfoPtr);
    row_stride = cinfoPtr->output_width * cinfoPtr->output_components;
    rgb565Size = cinfoPtr->output_width * 2;

    buffer = (*cinfoPtr->mem->alloc_sarray)((j_common_ptr)cinfoPtr, JPOOL_IMAGE, row_stride, 1);

    while (cinfoPtr->output_scanline < cinfoPtr->output_height) 
    {
        (void)jpeg_read_scanlines(cinfoPtr, buffer, 1);

        JPEG_RGB888ToRGB565(buffer[0], row_stride, &outbuffer[write_index], &rgb565Size);
        write_index += rgb565Size;
        if(write_index > outsize)
        {
            JPEG_PRINT("decompress out buffer too small \r\n");
            break;
        }
    }

    (void)jpeg_finish_decompress(cinfoPtr);
    jpeg_destroy_decompress(cinfoPtr);

    osFree(cinfoPtr);

    return 0;
}

#if 0
int8_t JPEG_DeCompress_File( const char *filename,uint8_t *outbuffer, uint32_t outsize)
{
    
    uint32_t fd = 0;
	uint32_t ret = 0;
	uint32_t write_index = 0;
    uint32_t row_stride;
	uint32_t file_size;
	char  *file_buf = NULL;
	char *file_path = filename;
    uint32_t rgb565Size = 0;
 
	fd = vfs_open(file_path, O_RDONLY);
	if(fd <0 ) {
		vfs_close(fd);

		JPEG_PRINT("open decompress file :%s error \r\n",file_path);
		return -1;
                
     }
	//JPEG_PRINT("open decompress file :%s  \r\n",file_path);
	file_size = vfs_lseek(fd,0,SEEK_END);
	vfs_lseek(fd,0,SEEK_SET);
    file_buf = osMalloc(file_size);
	ret = vfs_read(fd, file_buf, file_size);
	if(ret < 0){
		
 		JPEG_PRINT("read file errors \n");
        osFree(file_buf);
        vfs_close(fd);
		return ret;
	
	}else {
		
		//JPEG_PRINT("read file %d bytes \n",ret);
		
	}
	
	if(JPEG_DeCompress(file_buf,file_size,outbuffer,outsize)){
		
		JPEG_PRINT("decompress failed \r\n");
        osFree(file_buf);
        vfs_close(fd);
		return -1;
	}
	osFree(file_buf);
    vfs_close(fd);

    return 0;
}
#endif
