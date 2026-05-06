#ifndef __JRTC_0_H__
#define __JRTC_0_H__
#include "jrtc.h"
#ifdef __cplusplus
extern "C" {
#endif
/** 重置服务器地址
 * 注意:
 * - 手工设置新地址,通常只用于测试
 * - srvURL 为空时, 恢复为内置的默认服务器地址
 * - 在 jrtc_alloc 前设置才能生效
 *
 * @param[in] srvURL 服务器地址 
 */
void jrtc_config_server (const char srvURL[128]);

/** 分配资源
 * 参数与jrtc_open 相同, 但没有线程驱动, 需要不断调用 jrtc_schedule 手工更新
 */
struct jrtc_t* jrtc_alloc (const char channelId[64], const char uid[64],
    struct jrtc_image_t* video, struct jrtc_image_t* camera, struct jrtc_options_t* options);

/** 单步更新通话
 * 要求实时性处理, 务必在独立线程中持续更新, 
 * 当返回 JRTC_CLOSED 时, 可以退出该线程.
 * 例如:
 *  启动网络线程( [jc] {
 *      while (jrtc_schedule(jc) != JRTC_CLOSED);
 *  }
 */
enum jrtc_state jrtc_schedule (struct jrtc_t* jc);



enum jrtc_netstat
{
    JRTC_NET_NORMAL  = 0, /* network is normal or unknown */
    JRTC_LOCAL_POOR  = 1,
    JRTC_REMOTE_POOR = 2,
};
/** 估计网络状态 */
enum jrtc_netstat jrtc_netstat(struct jrtc_t* jc);

/** 镜像
 * @param[in] dst_w 目标宽度, 会自动裁剪,但不缩放
 * @param[in] dst_h 目标高度, 负数时会先上下翻转(等价于旋转180度)
 * @param[out] dst  目标内存
 */
void jrtc_i420_mirror(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);
void jrtc_nv12_mirror(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);
void jrtc_uyvy_mirror(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);
void jrtc_nv16_mirror(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);

/** 顺时针转90度
 * @param[in] dst_w 目标宽度, 会自动裁剪,但不缩放
 * @param[in] dst_h 目标高度, 负数时会先上下翻转(等价于转90后水平镜像)
 * @param[out] dst  目标内存
 */
void jrtc_i420_rotate90(const struct jrtc_photo_t* src, int dst_w, int dst_h,  char* dst);
void jrtc_nv12_rotate90(const struct jrtc_photo_t* src, int dst_w, int dst_h,  char* dst);
void jrtc_uyvy_rotate90(const struct jrtc_photo_t* src, int dst_w, int dst_h,  char* dst);
void jrtc_nv16_rotate90(const struct jrtc_photo_t* src, int dst_w, int dst_h,  char* dst);

/** 顺时针转270度
 * @param[in] dst_w 目标宽度, 会自动裁剪,但不缩放
 * @param[in] dst_h 目标高度, 负数时会先上下翻转(等价于转270后水平镜像)
 * @param[out] dst  目标内存
 */
void jrtc_i420_rotate270(const struct jrtc_photo_t* src, int dst_w, int dst_h,  char* dst);
void jrtc_nv12_rotate270(const struct jrtc_photo_t* src, int dst_w, int dst_h,  char* dst);
void jrtc_uyvy_rotate270(const struct jrtc_photo_t* src, int dst_w, int dst_h,  char* dst);
void jrtc_nv16_rotate270(const struct jrtc_photo_t* src, int dst_w, int dst_h,  char* dst);

/** 裁剪
 * @param[in] dst_w 目标宽度, 会自动裁剪,但不缩放
 * @param[in] dst_h 目标高度, 负数时会先上下翻转
 * @param[out] dst  目标内存
 */
void jrtc_i420_copy(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);
void jrtc_nv12_copy(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);
void jrtc_uyvy_copy(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);
void jrtc_nv16_copy(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);

/** 转换为 RGB565
 * @param[in] dst_w 目标宽度, 会自动裁剪,但不缩放
 * @param[in] dst_h 目标高度, 负数时会先上下翻转
 * @param[out] dst  目标内存, 输出长度为 dst_w*dst_h*sizeof(short)
 */
void jrtc_i420_torgb565(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);
void jrtc_nv12_torgb565(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);
void jrtc_uyvy_torgb565(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);
void jrtc_nv16_torgb565(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);

/** 可优化配置的参数 @{ */
extern unsigned JRTC_SESSION_TIMEOUT; /*< 建立通话总的超时阀值,默认 300 秒*/
extern unsigned JRTC_REPLY_TIMEOUT; /*< 建立通话中等待服务器应答的超时阀值,默认120 秒 */
extern unsigned JRTC_MEDIA_TIMEOUT; /*< 接收媒体数据的超时阀值, 默认 20 秒 */
extern unsigned JRTC_MAX_RTP_SIZE; /*< 发送的最大 RTP 包阀值, 默认1200,不能大于1500 */
extern unsigned JRTC_AUDIO_RED_LOSS;/*< 开启音频冗余编码的丢包率阀值, 默认 10 % */
extern unsigned JRTC_VIDEO_FEC_LOSS; /*< 开启视频冗余编码的丢包率阀值, 默认 5 % */
extern unsigned JRTC_VIDEO_FEC_MAX_RTT;/*< 开启视频冗余编码的RTT上限, 默认 3000 ms, 0 则强制关闭FEC */
extern unsigned JRTC_VIDEO_FEC_MAX_BW_RATIO; /*< 开启视频冗余编码的额外的码率占比, 默认 80 %, 0 则强制关闭FEC */
extern unsigned JRTC_VIDEO_INIT_GOP; /*< 初始默认的视频关键帧间隔, 默认2000 ms */
extern unsigned JRTC_TASK_STACK_KB; /*< 内部线程的栈大小,与平台相关 */
extern unsigned JRTC_TASK_PRIORITY; /*< 内部线程的优先级,与平台相关 */
/** @} */

/** 可辅助调试的全局值 @{ */
extern char  _jrtc_errstr[256]; /* 最近错误描述 */
/* 以下是当前全局配置值 */
extern char _jrtc_appKey[25];
extern char _jrtc_srvURL[129];
extern char _jrtc_tokens[129];
extern char _jrtc_aesKey[17];
extern char _jrtc_license[65];
extern struct jrtc_handler_t _jrtc_handler; /*< 事件处理的回调地址 */
/** @} */

extern unsigned jc_time(void);
#ifdef __cplusplus
}
#endif
#endif
