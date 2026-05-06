#include "wav.h"
#include "os.h"

int8_t WAV_Check(const uint8_t *wavFile, uint32_t size)
{
    WavFileHeader *wavHeader = (WavFileHeader *)wavFile;

    if(wavFile == NULL || size < WAV_HEADER_SIZE)
        return -1;

    // osPrintf("chunkid: %x, format: %x, audio format: %x, samplerate: %d, databits:%d \r\n",
    //  wavHeader->ChunkID, wavHeader->Format, wavHeader->AudioFormat, wavHeader->SampleRate, wavHeader->BitsPerSample);

    if(wavHeader->ChunkID != ('R' + ('I' << 8) + ('F' << 16) + ('F' << 24)))
    {
        osPrintf("wav File RIFF not Match \r\n");
        return -1;
    }

    if(wavHeader->Format != ('W' + ('A' << 8) + ('V' << 16) + ('E' << 24)))
    {
        osPrintf("wav File WAVE not Match \r\n");
        return -1;
    }

    if(wavHeader->AudioFormat != 0x1)   // PCM
    {
        osPrintf("wav File Not PCM audio \r\n");
        return -1;
    }
     return 0;
}

int8_t WAV_GetBitsOfPerSample(const uint8_t *wavFile, uint32_t size, uint8_t *BitsPerSample)
{
    if(wavFile == NULL || size < WAV_HEADER_SIZE)
        return -1;

    WavFileHeader *wavHeader = (WavFileHeader *)wavFile;
    *BitsPerSample = wavHeader->BitsPerSample;
    return 0;
}

int8_t WAV_GetSampleRate(const uint8_t *wavFile, uint32_t size, uint32_t *sampleRate)
{
    if(wavFile == NULL || size < WAV_HEADER_SIZE)
        return -1;

    WavFileHeader *wavHeader = (WavFileHeader *)wavFile;
    *sampleRate = wavHeader->SampleRate;
    return 0;
}

int8_t WAV_GetChnls(const uint8_t *wavFile, uint32_t size, uint8_t *chnls)
{
    if(wavFile == NULL || size < WAV_HEADER_SIZE)
        return -1;

    WavFileHeader *wavHeader = (WavFileHeader *)wavFile;
    *chnls = (uint8_t)wavHeader->NumChannels;
    return 0;  
}

int32_t WAV_FindDataChunk(const uint8_t *data, uint32_t size)
{
    uint32_t data_chunk = ('d' << 24) + ('a' << 16) + ('t' << 8) + 'a';

    for(uint32_t i = 0; i < size; i ++)
    {
        if(data_chunk == ((data[i] << 24) + (data[i+1] << 16) + (data[i+2] << 8) + (data[i+3])))
        {
            return i + 8;
        }
    }

    return -1;
}

int8_t WAV_CreateHeader(const uint8_t *wavFile, uint32_t size, uint32_t sampleRate, uint16_t dataBits, uint16_t chnls)
{
    if(wavFile == NULL || size < WAV_HEADER_SIZE)
        return -1;
    WavFileHeader *wavHeader = (WavFileHeader *)wavFile;

    wavHeader->ChunkID = ('R' + ('I' << 8) + ('F' << 16) + ('F' << 24));
    wavHeader->ChunkSize = size - 8;
    wavHeader->Format = ('W' + ('A' << 8) + ('V' << 16) + ('E' << 24));
    wavHeader->SubChunk1ID = ('f' + ('m' << 8) + ('t' << 16) + (0x20 << 24));
    wavHeader->SubChunk1Size = 16;
    wavHeader->AudioFormat = 0x1;
    wavHeader->NumChannels = chnls;
    wavHeader->SampleRate = sampleRate;
    wavHeader->ByteRate = sampleRate * dataBits / 8 * chnls; 
    wavHeader->BlockAlign = dataBits / 8 * chnls;
    wavHeader->BitsPerSample = dataBits;
    wavHeader->SubChunk2ID = ('d' + ('a' << 8) + ('t' << 16) + ('a' << 24));
    wavHeader->SubChunk2Size = size - WAV_HEADER_SIZE;

    return 0;
}

