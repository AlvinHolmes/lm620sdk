#ifndef __RTP_H__
#define __RTP_H__
#ifdef __cplusplus
extern "C" {
#endif
/** 返回RTP包的头部长度 */
int rtp_header_length(const void* rtp, int rtpLen);
unsigned short rtp_unpack_uint16(const void* buf);
unsigned rtp_unpack_uint24(const void* buf);
unsigned rtp_unpack_uint32(const void* buf);
void rtp_pack_uint16(void* buf, unsigned short val);
void rtp_pack_uint24(void* buf, unsigned val);
void rtp_pack_uint32(void* buf, unsigned val);
struct jrtc_photo_t;
enum RTP_PT
{
    RTP_IDR     = 1, /*< video key frame */
    RTP_FIRST   = 2, /*< first packet of the frame */
    RTP_ANNEXB  = 4, /*< AnnexB format stream */
    RTP_PARAM   = 8, /*< param packet */
    RTP_NRI     = 0xF0, /*< nal_ref_idc */
};
/* 多个连续的媒体RTP包 */
struct rtp_buf_t
{
    void* buf;
    unsigned len[48];
    unsigned size;
    unsigned types;/* 载荷的特征,RTP_PT 的集合 */
    void (*effect)(struct jrtc_photo_t* image);/* 编码前的图像特效 */
};

struct rtp_file_t;
typedef unsigned (*rtp_file_fn)(void* buf, unsigned len, struct rtp_file_t* rf);
struct rtp_file_t
{
    void* stream; /*< 保存文件句柄 */
    rtp_file_fn filefn;/**< IO 函数 */
    unsigned startMs;
};
/** 初始化RTP文件的保存
 * @param[in] fn 写入函数
 * @return 成功返回 0
 * 例如:
 * @code
 * static unsigned write_rtp(void* buf, unsigned len, struct rtp_file_t* rf)
 * {
 *  if (!rf->stream) return 0;
 *  return fwrite(buf, 1, len, (FILE*)rf->stream);
 * }
 * struct rtp_file_t rf;
 * rf.stream = fopen('xx.rtp', 'wb+')
 * rtp_dump_init(&rf, write_rtp, nowMs);
 * @endcode
 */
int rtp_dump_init(struct rtp_file_t* rf, rtp_file_fn fn, unsigned nowMs);
/** 依次写入每个RTP包
 * @param[in] nowMs 收到RTP包时的本地时间截
 */
int rtp_dump_packet(struct rtp_file_t* rf, void* rtp, int rtpLen, unsigned nowMs);

/** 初始化RTP文件的读取
 * @param[in] fn 读取函数
 * @return 成功返回 0
 * 例如:
 * @code
 * static unsigned read_rtp(void* buf, unsigned len, struct rtp_file_t* rf)
 * {
 *  if (!rf->stream) return 0;
 *  return fread(buf, 1, len, (FILE*)rf->stream);
 * }
 * struct rtp_file_t rf;
 * rf.stream = fopen('xx.rtp', 'rb')
 * rtp_read_init(&rf, read_rtp, nowMs);
 * @endcode
 */
int rtp_read_init(struct rtp_file_t* rf, rtp_file_fn fn, unsigned nowMs);
/** 依次读取每个RTP包
 * @param[out] nowMs 收到RTP包的本地时间截
 */
int rtp_read_packet(struct rtp_file_t* rf, void* rtp, int rtpBufferSize, unsigned* nowMs);

/** 将AMR打包到RTP中
 * @param[in] bAlignment 是否对齐模式
 * @param[in] iFrmCnt    AMR的帧数
 * @code
 *  int MME_ReadAudioRTP(void* rtp, int rtpBufferSize, int rtpHeaderLength, unsigned timeoutMs) {
 *      uint8_t *amr[]; int len[];//已编码的AMR数据
 *      int rtpLen = rtp_pack_amr_init(rtp, rtpHeaderLength, true, true, iFrmCnt);
 *      assert(rtpLen == 0);
 *      for (int i=0; i<iFrmCnt; ++i) {
 *          rtpLen = rtp_pack_amr(rtp, rtpBufferSize, amr[i], len[i]);
 *      }
 *      return rtpLen;//RTP的实际长度
 *  }
 * @endcode
 */
int rtp_pack_amr_init(void* rtp, int rtpHeaderLength, char isAMRWB, char bAlignment, char iFrmCnt);

/** 成功返回整个RTP包长度, 反之-1 */ 
int rtp_pack_amr(void* rtp, int rtpBufferSize, const void* amr, int armLen);

/** 从RTP解包AMR
 * @param[in] bAlignment 是否对齐模式,必须与上面的AMR打包一致
 * @return 返回iFrmCnt 帧数
 * @code
 *  const int iFrmCnt = rtp_unpack_amr_init(rtp, rtpBufferSize, true, true);
 *  for (int i=0; i<iFrmCnt; ++i) {
 *      char amr[64];
 *      int len = rtp_unpack_amr(rtp, rtpBufferSize, amr);
 *  }
 * @endcode
 */
int rtp_unpack_amr_init(void* rtp, int rtpBufferSize, char isAMRWB, char bAlignment);
int rtp_unpack_amr(void* rtp, int rtpBufferSize, char amr[64]);

/** NetEQ 抗抖动队列
 * @param[in] codecName 解码器名称,目前只支持PCMU,PCMA,AMR
 * @param[in] sampleRate 采样频率  
 * @param[in] payloadType
 */
struct rtp_neteq_t* rtp_neteq_alloc(const char* codecName, int sampleRate, unsigned char payloadType);
void                rtp_neteq_free(struct rtp_neteq_t* neteq);

/** 收到RTP包,直接放入队列
 * @param[in] ms 收到RTP时的本地时间截
 */
int                 rtp_neteq_write(struct rtp_neteq_t* neteq, void* rtp, int rtpLen, unsigned ms);

/** 读取PCM数据用于实时播放
 * @param[out]  sampleNum 采样个数. 等于字节数一半
 */
int                 rtp_neteq_read(struct rtp_neteq_t* neteq, void* pcm_uint16, int* sampleNum);

/** 将一帧数据就地分割成多个连续的RTP包
 *
 * - 调用前: rtp.buf 和 rtp.len[0] 是媒体数据, rtp.size 是缓冲区容量,
 *           [rtp.buf - rtpHeaderLength, rtp.buf + rtp.size) 都是有效缓冲区
 *
 * - 调用后: rtp.buf 是连续的RTP包的首地址, rtp.size 是连续的RTP包的总长度
 *           rtp.len[] 是每个RTP长度, 除末尾外的RTP包长度都是 rtpPacketLength
 *
 * @param[in,out] rtp 媒体数据,成功后将是连接的RTP包
 * @param[in] rtpPacketLength 除末尾外包的定长
 * @param[in] rtpHeaderLength 每个包的头部定长
 *
 * @return 成功返回RTP包个数
 *
 * @code
 * int MME_ReadVideoRTP(struct rtp_buf_t* rtp, int rtpBufferSize, int rtpHeaderLength, MMEImage*image, unsigned timeoutMs) {
 *      assert(rtp->size == 0);
 *      rtp->buf = h264;//编码数据
 *      rtp->len[0] = h264_len;///编码长度
 *      rtp->size = buflen;//有效缓冲区为[h264 - rtpHeaderLength, h264 + buflen)
 *      int num = rtp_split_frame(rtp, rtpBufferSize, rtpHeaderLength);
 *      ....
 *      return num;
 * }
 * @endcode
 */
int rtp_split_frame(struct rtp_buf_t* rtp, int rtpPacketLength, int rtpHeaderLength);

#ifdef __cplusplus
}
#endif

#endif // __RTP_H__
