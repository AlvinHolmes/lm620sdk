#ifndef G711_TABLE_H
#define G711_TABLE_H

// typedef enum
// {
//     PCM_ALAW,
//     PCM_UALW,
//     PCM_16
// }PCM_Type;

void pcm16_to_alaw(int length, const char *src_samples, char *dst_samples);
void pcm16_to_ulaw(int length, const char *src_samples, char *dst_samples);
void alaw_to_pcm16(int length, const char *src_samples, char *dst_samples);
void ulaw_to_pcm16(int length, const char *src_samples, char *dst_samples);

void pcm16_alaw_tableinit();
void pcm16_ulaw_tableinit();
void alaw_pcm16_tableinit();
void ulaw_pcm16_tableinit();
void pcm16_alaw_tabledeinit();
void pcm16_ulaw_tabledeinit();
void alaw_pcm16_tabledeinit();
void ulaw_pcm16_tabledeinit();

#endif // G711_TABLE_H