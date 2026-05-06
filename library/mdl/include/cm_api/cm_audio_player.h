/**
 * @file        cm_audio_player.h
 * @brief       Audio player接口
 * @copyright   Copyright © 2021 China Mobile IOT. All rights reserved.
 * @author      By WangPeng
 * @date        2023/07/27
 *
 * @defgroup audio_player
 * @ingroup audio_player
 * @{
 */

#ifndef __OPENCPU_AUDIO_PLAYER_H__
#define __OPENCPU_AUDIO_PLAYER_H__


/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "cm_audio_common.h"

/**
 * 放开宏定义，将对应格式解码器加入编译
 * 解码器占用较多代码空间
 */
// #define CM_AUDIO_PLAYER_WAV_ENABLE
// #define CM_AUDIO_PLAYER_MP3_ENABLE
// #define CM_AUDIO_PLAYER_FLAC_ENABLE
#define CM_AUDIO_PLAYER_AMR_ENABLE

/**
 * 放开宏定义，使用对应外设播放音频数据
 */
#define CM_AUDIO_PLAY_HW_DRIVER_PWM
#define CM_AUDIO_PLAY_HW_DRIVER_DAC
#define CM_AUDIO_PLAY_HW_DRIVER_CODEC

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/**线程句柄*/
typedef void *cmo_thread_id_t;

/**线程处理函数*/
typedef void (*cmo_thread_func_t)(void *param);

/****************************************************************************
 * Public Types
 ****************************************************************************/
typedef enum {
    CM_AUDIO_PLAY_FILE_STATE_IDLE,
    CM_AUDIO_PLAY_FILE_STATE_INIT,
    CM_AUDIO_PLAY_FILE_STATE_PLAY,
    CM_AUDIO_PLAY_FILE_STATE_PAUSE,
} cm_audio_play_file_state_e;
typedef enum {
    CM_AUDIO_PLAY_FILE_HW_DRIVER_START,
    CM_AUDIO_PLAY_FILE_HW_DRIVER_STOP,
} cm_audio_play_file_hw_driver_state_e;

/** 音频播放输出驱动类型 */
typedef enum
{
    CM_AUDIO_PLAY_DRIVER_CODEC = 1,                 /*!< CODEC输出方案 */
    CM_AUDIO_PLAY_DRIVER_DAC,                       /*!< DAC输出方案 */
    CM_AUDIO_PLAY_DRIVER_PWM,                       /*!< PWM输出方案 */
} cm_audio_play_driver_e;

/** 音频播放通道支持，不支持 */
typedef enum
{
    CM_AUDIO_PLAY_CHANNEL_RECEIVER = 1,         /*!< 听筒通道，不支持设置 */
    CM_AUDIO_PLAY_CHANNEL_HEADSET,              /*!< 耳机通道，不支持设置 */
    CM_AUDIO_PLAY_CHANNEL_SPEAKER,              /*!< 扬声器通道，不支持设置 */
#if 0
    CM_AUDIO_PLAY_CHANNEL_REMOTE,               /*!< 远端播放（需建立通话），暂不支持 */
#endif
} cm_audio_play_channel_e;

/** 音频播放回调的事件类型 */
typedef enum
{
    CM_AUDIO_PLAY_EVENT_FINISHED = 1,           /*!< 播放结束 */
#if 1
    CM_AUDIO_PLAY_EVENT_INTERRUPT,              /*!< 播放中断 */
#endif
} cm_audio_play_event_e;

/**
 *   流播放事件枚举
*/
typedef enum
{
    /* 被注释的目前还没有实现*/
    CM_AUDIO_TRACK_EVENT_NONE = 0,              /*!< dummy event */
    //CM_AUDIO_TRACK_EVENT_TICK,                  /*!< tick event, 20 ms typically */
    CM_AUDIO_TRACK_EVENT_STARTED,               /*!< start event, current track is involved in mixing */
    //CM_AUDIO_TRACK_EVENT_WAITING,               /*!< waiting event, current track is preempted by other track which has higher priority */
    //CM_AUDIO_TRACK_EVENT_RUNNING,               /*!< running event, current track is recovered from waiting */
    //CM_AUDIO_TRACK_EVENT_UNDERRUN,              /*!< underrun event, current track has played out all the pcm data */
    CM_AUDIO_TRACK_EVENT_OVERRUN,               /*!< overrun event, current track can not hold more pcm data */
    //CM_AUDIO_TRACK_EVENT_NEARLY_UNDERRUN,       /*!< fast up event, current track need more pcm data */
    //CM_AUDIO_TRACK_EVENT_NEARLY_OVERRUN,        /*!< slow down event, current track need less pcm data */
    CM_AUDIO_TRACK_EVENT_CLOSED,                /*!< close event, current track is closed */
    //CM_AUDIO_TRACK_EVENT_DRAINING,              /*!< draining event, current track is in draining */
    //CM_AUDIO_TRACK_EVENT_TERMINATED,            /*!< terminate event, current track is terminated in an unexpected way */
    //CM_AUDIO_TRACK_EVENT_CONSUMED,              /*!< consume event, current track has consumed vendor buffer */
    //CM_AUDIO_TRACK_EVENT_ENUM_32_BIT,           /*!< 32bit enum compiling enforcement */

    /* 下述事件与ASR底层AMR播放事件相对应，不支持*/
    CM_AMR_PLAYBACK_STATUS_ENDED = 100,         /*!< play out all for file or stream */
    CM_AMR_PLAYBACK_STATUS_STARTED,             /*!< indicate current playback is started */
    CM_AMR_PLAYBACK_STATUS_STREAM,
    CM_AMR_STREAM_STATUS_NEARLY_OVERRUN,        /*!< indicate amr data producer should slow down to avoid overrun */
    CM_AMR_STREAM_STATUS_NEARLY_UNDERRUN,       /*!< indicate amr data producer should fast up to avoid underrun */

    /* 下述事件与ASR底层MP3播放事件相对应，不支持*/
    CM_MP3_PLAYBACK_STATUS_ENDED = 200,         /*!< play out all for file or stream */
    CM_MP3_PLAYBACK_STATUS_STARTED,             /*!< indicate current playback is started */
    CM_MP3_PLAYBACK_STATUS_OPENED,              /*!< indicate output device is opened */
    CM_MP3_PLAYBACK_STATUS_FILE_READED,         /*!< reach file end */
    CM_MP3_PLAYBACK_STATUS_STREAM,
    CM_MP3_STREAM_STATUS_NEARLY_OVERRUN,        /*!< indicate mp3 data producer should slow down to avoid overrun */
    CM_MP3_STREAM_STATUS_NEARLY_UNDERRUN,       /*!< indicate mp3 data producer should fast up to avoid underrun */
} cm_audio_player_stream_event_e;

/****************************************************************************
 * Public Data
 ****************************************************************************/

/**
 * @addtogroup audio_player_impl 实现细节说明
 * @{
 *
 * @section player_singleton 单例模式
 * 播放器使用全局单例模式，同时只能有一个播放任务（文件或流）。
 * 当有播放任务正在进行时，调用 cm_audio_play_file() 或 stream_open() 会返回失败。
 *
 * @section player_thread 线程模型
 * - 播放操作在独立的线程中执行，栈大小 8KB，优先级 Normal
 *
 * @section player_state 状态机
 * - IDLE: 空闲状态，可开始新播放
 * - PLAYING: 播放中
 * - PAUSED: 暂停中
 * - STOPPING: 停止中（内部状态）
 *
 * @section player_buffer 流播放缓冲区
 * - 环形缓冲区大小：4096 * 8 = 32768 字节
 * - 缓冲区满时：写入部分数据并触发 OVERRUN 回调
 * - 缓冲区空时：播放线程等待 10ms 后重试
 * @}
 */
#define SINGLE_TRANSFER_SIZE  4096

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif
// cm_audio_play_file_state_e get_playFileCtx_state();
// cm_audio_play_file_hw_driver_state_e get_playFileCtx_hwState();

/**
*  @brief 流播放状态回调
*
*  @param [in] event 事件
*
*/
typedef void (*cm_audio_player_stream_cb)(cm_audio_player_stream_event_e event);

/**
 * @brief 播放回调函数
 *
 * @param [in] event 事件类型
 * @param [in] param  事件参数
 *
 * @details  须在播放API中传入
 *           通知事件为中断触发，不可阻塞，不可做耗时较长的操作，例如不可使用UART打印log
 */
typedef void (*cm_audio_play_callback_t)(cm_audio_play_event_e event, void *param);

#if 1 /* 过时的，仅保留用于兼容 */
/** 音频播放设置类型 */
typedef enum
{
    CM_AUDIO_PLAY_CFG_CHANNEL = 1,              /*!< 播放通道，支持范围参见cm_audio_play_channel_e枚举量，不支持设置 */
    CM_AUDIO_PLAY_CFG_VOLUME,                   /*!< 播放音量，支持范围参见0~100 */
} cm_audio_play_cfg_type_e;

/**
 * @brief 设置播放参数
 *
 * @param [in] type  设置参数类型
 * @param [in] value 设置参数数值
 *
 * @return
 *   =  0 - 成功 \n
 *   = -1 - 失败
 *
 * @details 音量调节范围为 0~100，自动转换为 10 的倍数，设置后立即生效并保存到 NVM \n
 *          音量值到实际音量的映射关系：\n
 *          音量值 | 音量等级 |
 *          ------|---------|
 *          0      | -       |
 *          10     | 0       |
 *          20     | 1       |
 *          30     | 2       |
 *          40     | 3       |
 *          50     | 4       |
 *          60     | 5       |
 *          70     | 6       |
 *          80     | 7       |
 *          90     | 8       |
 *          100    | 9       |
 *          实际音量等级见 NVM.
 *          不支持设置播放通道（模组仅一个播放通道，默认使用该通道，该通道外接扬声器/耳机/听筒由用户决定） \n
 *          使用音频功能时，相关PIN脚（PCM_CLK、PCM_SYNC、PCM_IN、PCM_OUT）不可做其他功能用途
 */
int32_t cm_audio_play_set_cfg(cm_audio_play_cfg_type_e type, void *value);
#endif /* #if 1 */

/**
 * @brief 读取播放参数
 *
 * @param [in]  type  读取参数类型
 * @param [out] value 读取参数数值
 *
 * @return
 *   =  0 - 成功 \n
 *   = -1 - 失败
 *
 * @details 读取当前音量值（0~100），返回值为 10 的倍数 \n
 *          不支持获取播放通道（模组仅一个播放通道，默认使用该通道，该通道外接扬声器/耳机/听筒由用户决定） \n
 *          使用音频功能时，相关PIN脚（PCM_CLK、PCM_SYNC、PCM_IN、PCM_OUT）不可做其他功能用途
 */
int32_t cm_audio_play_get_cfg(cm_audio_play_cfg_type_e type, void *value);

/**
 *  @brief 从文件系统播放音频文件
 *
 *  @param [in] path         文件路径/名称
 *  @param [in] format       播放格式
 *  @param [in] sample_param 播放音频PCM采样参数（format参数为CM_AUDIO_PLAY_FORMAT_PCM使用，其余情况传入NULL）
 *  @param [in] cb           音频播放回调函数（回调函数在音频处理线程中被执行）
 *  @param [in] cb_param     用户参数（与cm_audio_play_callback回调函数中param参数相对应）
 *
 *  @return
 *   =  0 - 成功 \n
 *   = -1 - 失败
 *
 *  @details 支持 pcm、mp3、wav、flac 格式
 *           通知事件为中断触发，不可阻塞，不可做耗时较长的操作
 *
 *           ML307N音频接口支持DAC接口输出音频模拟信号驱动扬声器方案（默认）
 *           和PWM接口输出音频模拟信号驱动扬声器方案
 *           和PCM接口输出音频数字信号至用户外挂音频设备由用户外挂音频设备执行播放方案
 *           PWM和DAC接口输出音频模拟信号驱动扬声器方案有如下特性：
 *           1.模组无内置CODEC芯片，播放声音的方式为PWM和DAC输出音频模拟信号波形驱动扬声器进行播放
 *           2.因模组无内置CODEC芯片，不支持音频参数校准 \n
 *           3.PWM和DAC驱动能力有限，无法驱动大功率的扬声器，若用户需要使用大功率扬声器，需自行选型PA芯片并进行软硬件开发 \n
 *           4.若用户引入PA芯片和PA电路，开启或关闭音频设备时电路可能会出现电压跳变导致爆破音 \n
 *           5.若出现爆破音且用户的硬件或者使用的PA芯片无法处理爆破音，请参考【ML307N音频开发指导】，从软件上进行规避 \n
 *
 *           PCM接口输出音频数字信号至用户外挂设备由用户外挂音频设备执行播放方案有如下特性： \n
 *           1.模组自身无音频播放能力，播放相关API执行效果为通过硬件PCM口将API接口传入的PCM数据（或MP3/WAV/FLAC解码后的PCM数据）发送至用户外挂音频设备 \n
 *           2.音频播放操作由用户外挂音频设备完成，音效校准、降噪等由用户外挂音频设备实现 \n
 *           3.如用户外挂音频设备需要驱动才可使用，可使用AudioHAL_AifBindCodec_CB()注册用户外挂音频设备开启/关闭操作的回调函数，底层在相应操作下自行调用用户注册的回调 \n
 *           4.cm_ex_codec.c中默认提供“PT8311 CODEC芯片+无DSP降噪芯片”方案，用户使用此方案仅需完成IIC从设备地址和PT8311 CODEC参数适配后即可使用 \n
 *           5.本方案音频播放能力为用户外挂音频设备提供，相关音效、降噪、校准等能力请咨询外挂音频设备厂商 \n
 *           6.用户应在完成外挂音频芯片驱动开发并确保音频芯片工作正常和具备播放功能下，方可使用此接口 \n
 *
 *  @note 实际行为：
 *  - 支持格式：PCM、AMRNB、AMRWB（WAV/MP3/FLAC 等当前仍返回错误）
 *  - sample_param 为 NULL 时使用默认值（16kHz, 16BIT, MONO）
 *  - 有播放任务进行时会返回失败（单例模式）
 *  - 创建独立线程处理播放，立即返回
 *  - 播放结束时触发 CM_AUDIO_PLAY_EVENT_FINISHED 回调
 *  - 路径最大长度 127 字符（含 '\0'）
 */
int32_t cm_audio_play_file(const char *path, cm_audio_play_format_e format, cm_audio_sample_param_t *sample_param, cm_audio_play_callback_t cb, void *cb_param);

/**
 *  @brief 暂停播放
 *
 *  @return
 *   =  0 - 成功 \n
 *   = -1 - 失败
 *
 *  @details 调用该接口到暂停播放存在一定延时
 *
 *  @note 实际行为：
 *  - 只能在 PLAYING 状态下调用，否则返回失败
 *  - 暂停通过事件标志通知播放线程
 *  - 暂停后需要调用 resume() 恢复
 */
int32_t cm_audio_player_pause(void);

/**
 *  @brief 继续播放
 *
 *  @return
 *   =  0 - 成功 \n
 *   = -1 - 失败
 *
 *  @note 实际行为：
 *  - 只能在 PAUSED 状态下调用，否则返回失败
 *  - 通过事件标志通知播放线程恢复
 */
int32_t cm_audio_player_resume(void);

/**
 *  @brief 停止播放
 *
 *  @return
 *   =  0 - 成功 \n
 *   = -1 - 失败
 *
 *  @details 调用该接口到停止播放存在一定延时
 *
 *  @note 实际行为：
 *  - 可用于停止文件播放或流播放
 *  - 在 IDLE 状态下调用返回失败
 *  - 流播放时会等待缓冲区播放完毕（最多 1 秒）
 *  - 等待播放线程退出（最多 5 秒）
 *  - 退出后清理 events/mutex/rb_pool 资源
 */
int32_t cm_audio_player_stop(void);

 /**
 *  @brief 注册流播放状态回调
 *
 *  @return void
 *
 *  @details 回调事件仅适用于cm_audio_player_stream_open()/cm_audio_player_stream_push()/cm_audio_player_stream_close()等接口 \n
 *           该接口应在调用上述接口前先注册 \n
 *
 *  @note 实际行为：
 *  - 可在 stream_open 之前或之后调用
 *  - 回调在播放线程中执行，不要阻塞
 *  - 支持事件：STARTED、CLOSED、OVERRUN
 */
void cm_audio_player_stream_cb_reg(cm_audio_player_stream_cb cb);

/**
 *  @brief 从管道/消息队列中播放音频（开启）
 *
 *  @param [in] format       播放格式
 *  @param [in] sample_param 播放音频PCM采样参数（format参数为CM_AUDIO_PLAY_FORMAT_PCM使用，其余情况传入NULL）
 *
 *  @return
 *   =  0 - 成功 \n
 *   = -1 - 失败
 *
 *  @details 支持 PCM、WAV、MP3、FLAC 格式 \n
 *
 *           ML307N音频接口支持DAC接口输出音频模拟信号驱动扬声器方案（默认）
 *           和PWM接口输出音频模拟信号驱动扬声器方案
 *           和PCM接口输出音频数字信号至用户外挂音频设备由用户外挂音频设备执行播放方案
 *           PWM和DAC接口输出音频模拟信号驱动扬声器方案有如下特性：
 *           1.模组无内置CODEC芯片，播放声音的方式为PWM和DAC输出音频模拟信号波形驱动扬声器进行播放
 *           2.因模组无内置CODEC芯片，不支持音频参数校准 \n
 *           3.PWM和DAC驱动能力有限，无法驱动大功率的扬声器，若用户需要使用大功率扬声器，需自行选型PA芯片并进行软硬件开发 \n
 *           4.若用户引入PA芯片和PA电路，开启或关闭音频设备时电路可能会出现电压跳变导致爆破音 \n
 *           5.若出现爆破音且用户的硬件或者使用的PA芯片无法处理爆破音，请参考【ML307N音频开发指导】，从软件上进行规避 \n
 *
 *           PCM接口输出音频数字信号至用户外挂设备由用户外挂音频设备执行播放方案有如下特性： \n
 *           1.模组自身无音频播放能力，播放相关API执行效果为通过硬件PCM口将API接口传入的PCM数据（或MP3/WAV/FLAC解码后的PCM数据）发送至用户外挂音频设备 \n
 *           2.音频播放操作由用户外挂音频设备完成，音效校准、降噪等由用户外挂音频设备实现 \n
 *           3.如用户外挂音频设备需要驱动才可使用，可使用AudioHAL_AifBindCodec_CB()注册用户外挂音频设备开启/关闭操作的回调函数，底层在相应操作下自行调用用户注册的回调 \n
 *           4.cm_ex_codec.c中默认提供“ES8311 CODEC芯片+无DSP降噪芯片”方案，用户使用此方案仅需完成IIC从设备地址和ES8311 CODEC参数适配后即可使用 \n
 *           5.本方案音频播放能力为用户外挂音频设备提供，相关音效、降噪、校准等能力请咨询外挂音频设备厂商 \n
 *           6.用户应在完成外挂音频芯片驱动开发并确保音频芯片工作正常和具备播放功能下，方可使用此接口 \n
 *
 *  @note 实际行为：
 *  - 实际仅支持 PCM 格式（其他格式返回错误）
 *  - 分配 32KB 环形缓冲区
 *  - 创建独立播放线程
 *  - 有播放任务进行时会返回失败
 *  - 启动后触发 STARTED 回调
 */
int32_t cm_audio_player_stream_open(cm_audio_play_format_e format, cm_audio_sample_param_t *sample_param);

/**
 *  @brief 往管道/消息队列中发送要播放的音频数据
 *
 *  @param [in] data 播放的数据
 *  @param [in] size 播放数据的长度
 *
 *  @return
 *   >= 0 - 实际写入的数据长度 \n
 *   = -1 - 失败
 *
 *  @details 支持PCM、WAV、MP3、FLAC格式 \n
 *
 *  @note 实际行为：
 *  - 写入环形缓冲区，使用互斥锁保护
 *  - 缓冲区满时写入部分数据并触发 OVERRUN 回调
 *  - 返回实际写入的字节数
 */
int32_t cm_audio_player_stream_push(uint8_t *data, uint32_t size);

/**
 *  @brief 从管道/消息队列中播放音频（关闭）
 *
 *  @note 实际行为：
 *  - 等待缓冲区播放完毕（最多 1 秒）
 *  - 等待播放线程退出（最多 5 秒）
 *  - 清理 events/mutex/rb_pool 资源
 */
void cm_audio_player_stream_close(void);

/**
 *  @brief 从管道/消息队列中播放音频（清除待播放数据及关闭）
 *
 *  @note 实际行为：
 *  - 立即清空环形缓冲区（不等待播放完毕）
 *  - 等待播放线程退出（最多 5 秒）
 *  - 清理 events/mutex/rb_pool 资源
 */
void cm_audio_player_stream_clear_close(void);

/**
 *  @brief 设置播放音量(0-100)
 *
 *  @param [in] vol  播放的音量（0-100），10的倍数（0,10,20 ... 100）
 *
 *  @return
 *   = 0 -  设置成功
 *   others - 失败
 *
 * @details 设置后立即生效并保存到 NVM
 */
int cm_audio_player_set_volume(int vol);

/**
 *  @brief 获取播放音量(10-100)
 *
 *  @param [out] vol  读取播放音量指针，10的倍数（0,10,20 ... 100）
 *
 *  @return
 *   = 0 -  读取成功
 *   others - 失败
 */
int cm_audio_player_get_volume(int* vol);

/**
 *  @brief 设置播放输出方案
 *
 *  @param [in] driver_type  支持CODEC/PWM/DAC
 *
 */
void cm_audio_player_set_driver(cm_audio_play_driver_e driver_type);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif

/** @}*/

