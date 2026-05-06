#include "os.h"
#include "ogg_user.h"
#include "opus_user.h"
#include "stdlib.h"

// #define OGG_DEBUG_PRINTF
#ifdef  OGG_DEBUG_PRINTF
    #define OGG_PRINTF        osPrintf
#else
    #define OGG_PRINTF(...)
#endif 

int8_t OGG_Unpack(OggHandle *ogg_handle, uint8_t *inData, uint32_t inSize, uint8_t *outData, uint32_t *outSize)
{
    if(inData == NULL || inSize == 0 || ogg_handle == NULL || outData == NULL || (*outSize) == 0 )
        return -1;

    if(!ogg_handle->initFlag)
    {
        ogg_sync_init(&ogg_handle->sync_state);
        ogg_handle->packetCnt = 0;
        ogg_handle->pageCnt = 0;
        ogg_handle->initFlag = true;
        OGG_PRINTF("new ogg decode \r\n");
    }

    int32_t outDataSize = 0;

    char *pBuf = ogg_sync_buffer(&ogg_handle->sync_state, inSize);
    OS_ASSERT(pBuf);
    memcpy(pBuf, inData, inSize);
    ogg_sync_wrote(&ogg_handle->sync_state, inSize);

    while(true)
    {
        int32_t ret = ogg_sync_pageout(&ogg_handle->sync_state, &ogg_handle->page);
        if(ret < 0)
        {
            OGG_PRINTF("page parser failed : %d \r\n", ret);
            goto OGG_UNPACK_END;
        }
        else if(ret == 0)
        {
            OGG_PRINTF("page need more inData \r\n");
            goto OGG_UNPACK_END;
        }

        ogg_handle->pageCnt ++;

        if(ogg_handle->pageCnt == 1)
        {
            ogg_stream_init(&ogg_handle->stream_state, ogg_page_serialno(&ogg_handle->page));
        }

        ogg_stream_pagein(&ogg_handle->stream_state, &ogg_handle->page);

        while(1)
        {
            ret = ogg_stream_packetout(&ogg_handle->stream_state, &ogg_handle->packet);
            if(ret <= 0)
            {
                OGG_PRINTF("stream parser failed %d\r\n", ret);
                break;
            }

            ogg_handle->packetCnt ++;
            OGG_PRINTF("ogg_handle->packetCnt %d \r\n", ogg_handle->packetCnt);
             OGG_PRINTF("packet bytes: %d \r\n", ogg_handle->packet.bytes);
            char head[16] = {0};
            if( ogg_handle->packetCnt == 1)
            {
                memcpy(head, ogg_handle->packet.packet, 8);
            #ifdef OGG_DEBUG_PRINTF
                OGG_PRINTF("head:%s \r\n", head);
                OGG_PRINTF("Version: %d \r\n", ogg_handle->packet.packet[8]);
                OGG_PRINTF("Chnls: %d \r\n", ogg_handle->packet.packet[9]);
                OGG_PRINTF("Pre-Skip: %d \r\n", ogg_handle->packet.packet[10] + (ogg_handle->packet.packet[11] << 8) );
                uint32_t sampleRate = ogg_handle->packet.packet[12] + (ogg_handle->packet.packet[13] << 8) +
                                      (ogg_handle->packet.packet[14] << 16) + (ogg_handle->packet.packet[15] << 24);
                OGG_PRINTF("sampleRate: %d \r\n", sampleRate);
            #endif

                ogg_handle->sampleRate = ogg_handle->packet.packet[12] + (ogg_handle->packet.packet[13] << 8) +
                                      (ogg_handle->packet.packet[14] << 16) + (ogg_handle->packet.packet[15] << 24);
                ogg_handle->dataBits = 16;
                ogg_handle->chnls = ogg_handle->packet.packet[9];
            }
            else if(ogg_handle->packetCnt == 2)
            {
                memcpy(head, ogg_handle->packet.packet, 8);
                OGG_PRINTF("packet:%d: %s \r\n",ogg_handle->packetCnt, head);
            }
            else
            {
                outData[outDataSize++] = (ogg_handle->packet.bytes & 0x00FF);
                outData[outDataSize++] = ((ogg_handle->packet.bytes >> 8) & 0x00ff);
                memcpy(&outData[outDataSize], ogg_handle->packet.packet, ogg_handle->packet.bytes);
                outDataSize += ogg_handle->packet.bytes;

                // uint32_t decodeBufSize = (*outSize) - outDataSize;
                // if( 0 == Opus_Decode((OpusDecoder *)OpusDecHandle, ogg_handle->packet.packet, ogg_handle->packet.bytes, &outData[outDataSize], &decodeBufSize))
                // {
                //     outDataSize += decodeBufSize * 2;
                // }
                
                OGG_PRINTF("packet: %d, audio %d \r\n", ogg_handle->packetCnt, ogg_handle->packet.bytes);
            }
        }

        if(ogg_page_eos(&ogg_handle->page))
        {
            OGG_PRINTF("end of page \r\n");
            break;
        }
    }

OGG_UNPACK_END:
    // ogg_stream_clear(&ogg_handle->stream_state);
    // ogg_sync_clear(&ogg_handle->sync_state);
    *outSize = outDataSize;
    return 0;
}

int8_t OGG_UnpackEnd(OggHandle *ogg_handle)
{
    if(ogg_handle == NULL)
    {
        return -1;
    }

    ogg_stream_clear(&ogg_handle->stream_state);
    ogg_sync_clear(&ogg_handle->sync_state);
    memset(ogg_handle, 0, sizeof(OggHandle));

    return 0;
}

static void OGG_PackHead(OggHandle *ogg_handle, uint32_t sampleRate, uint8_t chnls)
{
    srand((int)osTickGet());
    ogg_stream_init(&ogg_handle->stream_state, rand());

    uint8_t *header = osMalloc(127);
    memset(header, 0, 127);
    uint8_t *comment = osMalloc(127);
    memset(comment, 0, 127);
    uint32_t header_len = 0;
    uint32_t comment_len = 0;

    uint8_t *ptr = header;
    memcpy(ptr, "OpusHead", 8);
    ptr += 8;
    *ptr++ = 1;
    *ptr++ = chnls; // chnl
    *ptr ++ = 0x80;
    *ptr ++ = 0x0F;

    *(uint32_t *)ptr = sampleRate;
    ptr += 4;
    *ptr++ = 0;
    *ptr++ = 0;
    *ptr++ = 0;
    header_len = ptr - header;

    ptr = comment;
    memcpy(ptr, "OpusTags", 8);
    ptr += 8;
    *(uint32_t *)ptr = 6;
    ptr += 4;
    memcpy(ptr , "vendor", 6);
    ptr += 6;
    *(uint32_t *)ptr = 0;
    ptr += 4;
    *(uint32_t *)ptr = 0;
    ptr += 4;
    comment_len = ptr - comment;

    ogg_handle->packet.b_o_s = 1;
    ogg_handle->packet.e_o_s = 0;
    ogg_handle->packet.granulepos = 0;
    ogg_handle->packet.packetno = 0;
    ogg_handle->packet.bytes = header_len;
    ogg_handle->packet.packet = header;
    ogg_stream_packetin(&ogg_handle->stream_state, &ogg_handle->packet);

    ogg_handle->packet.b_o_s = 0;
    ogg_handle->packet.e_o_s = 0;
    ogg_handle->packet.packetno = 1;
    ogg_handle->packet.bytes = comment_len;
    ogg_handle->packet.packet = comment;
    ogg_stream_packetin(&ogg_handle->stream_state, &ogg_handle->packet);

    osFree(comment);
    osFree(header);
}

int8_t OGG_Pack(OggHandle *ogg_handle, OggAudioInfo *info, uint8_t *outData, uint32_t *outSize, bool isEnd)
{
    uint32_t outDataSize = 0;
    if(!ogg_handle->initFlag)
    {
        ogg_handle->initFlag = true;
        OGG_PackHead(ogg_handle, info->sampleRate, info->chnls);

        while(ogg_stream_flush(&ogg_handle->stream_state, &ogg_handle->page) != 0)
        {
            memcpy(&outData[outDataSize], ogg_handle->page.header, ogg_handle->page.header_len);
            outDataSize += ogg_handle->page.header_len;
            memcpy(&outData[outDataSize], ogg_handle->page.body, ogg_handle->page.body_len);
            outDataSize += ogg_handle->page.body_len;
        }

        ogg_handle->packetCnt = 2;

        *outSize = outDataSize;
    }
    
    ogg_handle->packet.b_o_s = 0;
    ogg_handle->packet.e_o_s = 0;
    if(isEnd)
        ogg_handle->packet.e_o_s = 1;
    ogg_handle->packet.packetno = (ogg_handle->packetCnt + 2);
    ogg_handle->packet.granulepos = ogg_handle->packet.packetno * info->sampleSize;
    ogg_handle->packet.bytes = info->audioDataSize;
    ogg_handle->packet.packet = info->audioData;
    ogg_handle->packetCnt ++;
    ogg_stream_packetin(&ogg_handle->stream_state, &ogg_handle->packet);

    while(ogg_stream_pageout(&ogg_handle->stream_state, &ogg_handle->page) != 0)
    {
        memcpy(&outData[outDataSize], ogg_handle->page.header, ogg_handle->page.header_len);
        outDataSize += ogg_handle->page.header_len;
        memcpy(&outData[outDataSize], ogg_handle->page.body, ogg_handle->page.body_len);
        outDataSize += ogg_handle->page.body_len;
    }

    if(isEnd)
    {
        while(ogg_stream_flush(&ogg_handle->stream_state, &ogg_handle->page) != 0)
        {
            memcpy(&outData[outDataSize], ogg_handle->page.header, ogg_handle->page.header_len);
            outDataSize += ogg_handle->page.header_len;
            memcpy(&outData[outDataSize], ogg_handle->page.body, ogg_handle->page.body_len);
            outDataSize += ogg_handle->page.body_len;
        }

        ogg_stream_clear(&ogg_handle->stream_state);
    }

    *outSize = outDataSize;
    return 0;
}
