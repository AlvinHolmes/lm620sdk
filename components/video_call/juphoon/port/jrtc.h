#ifndef __JRTC_H__
#define __JRTC_H__
#ifdef __cplusplus
extern "C" {
#endif
/** 产品概述
 * 实现 IoT 设备在 Juphoon 多人通话中的音视频和消息互通功能:
 * - 会话名(频道 channelId)作为全局唯一的屏幕共享视频
 * - 音频接收至多一路, 不区别来源(即全局混音流)
 * - 视频接收至多一路, 但可以切换来源
 * - 暂不支持消息发送
 *
 * 最简流程
 * - jrtc_config(...); //全局配置
 * - jc = jrtc_open(...); //加入通话,不阻塞, 成功后有 on_user_joined 事件
 * - 在 UI 线程, 更新视频或预览, 参考如下的`渲染流程'
 * - jrtc_set_video(jc, ...);  //请求切换视频源, 成功后有 on_video_changed 事件
 * - jrtc_set_status(jc, ...); //改变自身或他人状态, 成功后有 on_user_changed 事件
 * - jrtc_close(jc, JRTC_EBYE);//阻塞销毁所有资源
 */

/** 当前的版本号 */
#define JRTC_VERSION   "2.0.0.1"

/** 错误码 */
enum jrtc_error
{
    JRTC_ENIL   = 0,  /* unknown error */
    JRTC_EPERM  = 1,  /* Operation not permitted */
    JRTC_ENONET = 64, /* Machine is not on the network */
    JRTC_EPROTO = 71, /* Protocol error */
    JRTC_EMSGSIZE = 90,  /* Message too long */
    JRTC_ECONNRESET=104, /* Connection reset by peer */
    JRTC_ETIMEDOUT =101, /* Connection timed out */
    JRTC_ECONNREFUSED=111,/* Connection refused or closed */

    JRTC_EOFFLINE = 128, /* Peer offline or over */
    JRTC_EDECLINE = 129, /* Peer decline the call */
    JRTC_EINVOKETIMEOUT = 130, /* IoTUnit invoke timeout */
    JRTC_ECALLSERVERERROR = 131, /* Call server error */
    JRTC_EROOMNOTFND = 132, /* Room or callee not found */
    JRTC_ECALLTIMEOUT = 133, /* Call timeout */
    JRTC_ENOTREG = 134, /* device id not registered */
    JRTC_ELOGIN = 135, /* device id login failed */
    JRTC_ETOKEN_NOTFOUND = 136, /* secret not found */
    JRTC_ETOKEN_INV = 137, /* token invalid */
    JRTC_ETOKEN_EXPIRE = 138, /* token expire */
    JRTC_ECODECINVALID = 139, /* Codec params invalid */

    JRTC_EAUDIOCALL = 252, /* external audio call, don't close audio deivce */
    JRTC_EAUDIO=253,  /* Audio device error */
    JRTC_EVIDEO=254,  /* Video device error */

    JRTC_EBYE = 255,  /* Success, Closed by peer leave */
};

/** 图像数据格式 */
enum jrtc_dtype
{
    JRTC_I420 = 0, /*< 图像字节数=W*H*3/2 */
    JRTC_NV12 = 1, /*< 图像字节数=W*H*3/2 */
    JRTC_UYVY = 2, /*< 图像字节数=W*H*2 */
    JRTC_JPEG = 3, /*< 保持兼容性 */
    JRTC_NV16 = 4, /*< 图像字节数=W*H*2 */

    JRTC_RGB565 = 64,

    JRTC_H264 = 96,
    JRTC_H263 = 97,
};

/** 视频流中的图像
 * 各平台的图像格式如下:
 * |   软件系统   |   平台   |  采集预览格式  |  接收后渲染格式
 * | ------------ | -------- | ---------------| -----------------------
 * | ASR Crane    | 3603     | JRTC_NV12      | JRTC_I420, JRTC_RGB565, JRTC_JPEG [^1]
 * | Unisoc Mocor | UIS8910  | JRTC_UYVY      | JRTC_UYVY, JRTC_JPEG [^1]
 * | Unisoc Mocor | UMS9117  | JRTC_NV16      | JRTC_RGB565,JRTC_JPEG [^1]
 * | Unisoc Mocor | ANTISW3  | JRTC_H264      | [^2]
 * | 乐鑫 ESP-IDF | ESP32-S3 | JRTC_JPEG      | [^3]
 *
 * [^1]: 可通过 video->format = JRTC_JPEG 设置
 * [^2]: 解码后直接到屏幕上显示
 * [^3]: 单向通话,不支持接收
 *
 * 渲染流程:
 * jrtc_image_t* img 代表传入的 video 或 camera
 * - SDK 内部自动更新图像:
 *      if (img->get == img->put) {
 *          '更新图像内容: 即对img中的data, width, height, format 赋值'
 *          img->put++;//增加写计数
 *      }
 * - 界面UI线程(App 界面层)负责绘制图像:
 *      if (img->get != img->put) {
 *          '界面绘制操作: 即使用img中的data, 更新界面控件'
 *          img->get = img->put;//增加读计数
 *      }
 * - 界面绘制操作, 请参考各自 IoT 平台的官方文档
 */
struct jrtc_image_t
{/* 可强转为 jrtc_photo_t 使用 */
    void* data;
    unsigned bytes;
    unsigned short width, height;
    char format;/*< enum jrtc_dtype */
    char fps; /* 配置/实际帧速率 */
    char reserve[2];
    unsigned short kbps;
    volatile char put, get;
};

/** 相对某个起始地址的数据片 */
struct jrtc_slice_t
{
    unsigned    offset:16;
    unsigned    length:16;
    unsigned    reserve:16;
};

/** 发送者 */
#define JRTC_ROLE_SENDER    0x2
/** 演示者, 其事件将被广播 */
#define JRTC_ROLE_PLAYER    0x4
/** 唯一的拥有者 */
#define JRTC_ROLE_OWNER     0x8
/* 保留低8位, 高位允许按位扩展自定义角色 */

/** 通话时, 视频是否被转发 */
#define JRTC_STATUS_FORWARD_VIDEO    0x1
/** 通话时, 音频是否被转发 */
#define JRTC_STATUS_FORWARD_AUDIO    0x2
/** 本地, 视频设备是否可发送 */
#define JRTC_STATUS_VIDEO   0x4
/** 本地, 音频设备是否可发送 */
#define JRTC_STATUS_AUDIO   0x8
/** 本地, 音视频设备的是否可发送 */
#define JRTC_STATUS_MEDIA   (JRTC_STATUS_AUDIO|JRTC_STATUS_VIDEO)
/** 最终是否处于发送音频状态 */
#define JRTC_SENDING_AUDIO  (JRTC_STATUS_AUDIO|JRTC_STATUS_FORWARD_AUDIO)
/** 最终是否处于发送视频状态 */
#define JRTC_SENDING_VIDEO  (JRTC_STATUS_VIDEO|JRTC_STATUS_FORWARD_VIDEO)
/* 保留低8位,高位允许按位扩展自定义状态 */

/** 透明的会话句柄 */
struct jrtc_t;

/** 会话中的事件处理函数集合
 * - 会话中的用户都有两个数值属性: role 和 status
 *   @see JRTC_ROLE_*, JRTC_STATUS_*
 * - 事件回调发生在内部线程中, 不允许处理耗时操作
 *   应该立即发送到主线程(UI或业务线程) 处理
 * - 通过 jrtc_set_ptr/jrtc_get_ptr 可关联界面的自定义值
 * - 若 NULL, 则不处理对应事件
 * - 通常将用户信息保存于数组中, 使用序号作为索引值[1,255], 查找效率最高.
 *   因此允许 uid 映射为数值型的 index( 索引值 )
 * - 用户的index 由 on_user_joined/on_user_changed 的返回值指定
 *   或者手工调用 jrtc_set_index 申请修改
 * - 约定索引值 0 作为保留值表示未知或不修改, 负数表示删除
 * - 定时请求批量的音量或网络状态后, 服务器将返回
 *   批量的index与对应的音量值或状态值
 */
struct jrtc_handler_t
{
    /** 用户的加入事件
     * 当 uid 为自身时, 说明加入成功, 后续将打开并独占媒体设备, 此时界面必须释放设备
     * 不保证加入的次序, 有可能存在其他人早于自身加入成功的情况
     *
     * @param[in] uid       用户ID
     * @param[in] role      用户角色
     * @param[in] status    用户状态
     * @retval 请求绑定该用户的 index, 若 0 或负数则忽略
     */
    int (*on_user_joined) (struct jrtc_t* jc, char uid[64], unsigned role, unsigned status);

    /** 用户的更新事件
     * 有可能存在其他人加入并更新早于自身加入成功的情况
     *
     * @param[in] uid       用户ID
     * @param[in] index     该用户索引值
     * @param[in] role      用户角色
     * @param[in] status    用户状态
     * @retval 请求重绑定该用户的 newindex, 若 0 或与 index 相等则忽略
     */
    int (*on_user_changed) (struct jrtc_t* jc, char uid[64], int index, unsigned role, unsigned status);

    /** 用户的离线事件
     * 当 uid 为自身时,说明已离开会话, 无须调用 jrtc_leave
     * 内部可以保证离开后, 没有后续任何事件
     * 对应的index也将自动删除
     *
     * @param[in] uid       用户ID
     * @param[in] index     该用户索引值
     * @param[in] reason    离线原因
     */
    void (*on_user_offline) (struct jrtc_t* jc, char uid[64], int index, enum jrtc_error reason);

    /** 用户的消息事件
     *
     * @param[in] uid       消息来源的用户ID
     * @param[in] index     该用户索引值
     * @param[in] msg       多个消息块. 起始地址为uid, 第i个消息地址 = (uid + msg[i].offset)
     * @param[in] num       消息个数
     */
    void (*on_user_message) (struct jrtc_t* jc, char uid[64], int index, struct jrtc_slice_t msg[], unsigned num);

    /** 视频首帧事件
     * 会话中用户的 status 包含JRTC_STATUS_VIDEO, 说明他有发送视频
     * 使用 jrtc_set_video 订阅该远端用户后, 收到首帧后就会有该事件
     *
     * @param[in] vid       视频ID. 可以是用户视频 uid 或 共享屏幕 channelId
     *                      空表示已停止接收视频
     * @param[in] index     该用户索引值
     */
    void (*on_video_changed) (struct jrtc_t* jc, char vid[64], int index);

    /** 共享者的改变事件
     * 会话内全局只有一个共享屏幕, 被所有用户抢占使用.
     * 因此每个用户至多上传两路视频流。
     * 共享屏幕ID是 channelId
     *
     * @param[in] uid       用户ID, 共享者.
     *                      空表示不存在共享
     * @param[in] index     该用户索引值
     */
    void (*on_share_changed) (struct jrtc_t* jc, char uid[64], int index);

    /** 音量事件
     * 首次收到音频数据时也上报该事件, 且num=0,index_volume=NULL
     * 其他情况下需要定时主动调用 jrtc_request_volume 请求才有,
     * 且只上报绑定过索引值的用户音量
     *
     * @param[in] num 后续的音量个数
     * @param[in] index_volume 索引与音量的数组,音量范围[0,100]
     */
    void (*on_audio_volume) (struct jrtc_t* jc, unsigned num, unsigned char index_volume[][2]);

    /** 会话内各成员的网络状态
     * 需要定时主动调用 jrtc_request_netstate 请求才有,
     * 且只上报绑定过索引值的用户网络状态
     * 网络状态范围[0,5], 0是未知, 1是最差, 5是最好
     * 定义为只影响本地接收质量的状态:
     * - 其他成员的上行发送状态
     * - 本地自身的下行接收状态
     *
     * @param[in] num 后续的状态个数
     * @param[in] index_netstate 索引与网络状态的数组,
     */
    void (*on_user_netstate) (struct jrtc_t* jc, unsigned num, unsigned char index_netstate[][2]);
};

/** 全局配置, 只需调用一次。 内部将复制所有参数。
 *
 * @param[in] appKey        当前APP特有的标识, 固定长度24, 需向Juphoon申请获得
 * @param[in] license       自定义的设备许可标识, 要求在appKey内唯一, 长度不超64, 且需要向Juphoon 登记并激活
 * @param[in] aesKey        通讯密钥, 必须与 appKey 匹配, 固定长度16, 需向Juphoon申请或注册
 * @param[in] tokens        用于鉴权的凭证, 长度不超128
 * @param[in] handler       统一的事件处理回调集合, 若 NULL, 则不处理事件
 *
 * @note appKey, license, aesKey, tokens 都不能包含JSON需转义的字符
 */
void jrtc_config (const char appKey[24], const char license[64],
    const char aesKey[16], const char tokens[128],  const struct jrtc_handler_t *handler);

/** 可选的会话参数 */
struct jrtc_options_t
{
    /** 初始的角色集合, 默认 0xe */
    unsigned role;
    /** 初始的状态集合, 默认 0xf */
    unsigned status;
    /** 初始自动订阅的视频源,  默认 channelId(全局共享), 也可指定 uid(用户视频) */
    const char* vid;
};

/** 分配通话资源并启动网络线程
 * 若加入多人通话, 成功后将收到 on_user_joined 事件
 *
 * @param[in] channelId     会话名(频道名)
 * @param[in] uid           自定义的用户名, 要求在会话内唯一
 * @param[in,out] video     期望接收视频的建议值. 若 NULL, 则不接收视频
 * @param[in,out] camera    期望发送视频的建议值. 若 NULL, 则不发送视频
 * @param[in] options       其他可选参数. 若 NULL, 则使用默认值
 *
 * @retval 会话句柄
 *
 * @note uid 和 channelId 不能相等
 * - 最终的实际收发参数, 都由服务器协商决定.
 * - 若界面上, 检测到 video 和 camera 中的 width,height 都为0,
 *   说明协商后不支持视频, 自动转为音频通话
 * - video, camera 指向的 jrtc_image_t 结构
 *   同时用于界面渲染和内部更新, 因此会话中都必须一起保持内存有效,
 */
struct jrtc_t* jrtc_open (const char channelId[64], const char uid[64],
    struct jrtc_image_t* video, struct jrtc_image_t* camera, struct jrtc_options_t* options);

/** 释放通话资源,并等待网络线程结束.
 *
 * @param[in] err   挂断原因, 参考jrtc_leave
 */
void jrtc_close (struct jrtc_t* jc, enum jrtc_error err);

/** 返回最近的出错值 */
enum jrtc_error jrtc_error  (const struct jrtc_t *jc);

/** 通话状态
 *             jrtc_alloc              jrtc_schedule                   jrtc_schedule
 *             创建实例                发送请求                        收到应答
 * JRTC_CLOSED ----------> JRTC_OPENED ------------------> JRTC_JOINING -------------> JRTC_JOINED(on_user_joined)
 *       ^                                                                            | 
 *       | jrtc_schedule                                                              | jrtc_schedule
 *       | 销毁实例        jrtc_leave 或 内部出错                                     | 打开媒体设备
 * JRTC_LEAVING <---------------------------+------------- JRTC_TALKING <-------------+
 *                                          |                  ^         收到媒体数据(首次音频 on_audio_volume)
 *                                          |  jrtc_schedule   |
 *                                          +------------------+
 */
enum jrtc_state
{
    JRTC_CLOSED=0,/*< 初始状态 */
    JRTC_LEAVING, /*< jrtc_leave 或内部出错后的状态 */
    JRTC_OPENED,  /*< 调用jrtc_join 后的未初始状态 */
    JRTC_JOINING, /*< 加入中的状态 */
    JRTC_JOINED,  /*< 成功加入会话, 将打开音视频设备 */
    JRTC_TALKING, /*< 收到媒体后的状态 */
};

/** 返回通话状态 */
enum jrtc_state jrtc_state (const struct jrtc_t *jc);

/** 主动离开/挂断, 不会阻塞, 也没有后续任何事件
 * 注意: 这里只改变状态, 销毁将在jrtc_schedule()中自动处理
 *
 * @param[in] err   主动挂断为 JRTC_EBYE
 *                  但当语音电话中断时, 须设置为JRTC_EAUDIOCALL,
 *                  其他未知情况,可设置为 JRTC_ENIL(不修改挂断原因)
 * @retval 返回通话状态
 */
enum jrtc_state jrtc_leave(struct jrtc_t* jc, enum jrtc_error err);

/** 请求视频源
 * 默认初始值由 jrtc_options_t:vid 指定
 * 一旦设置后, 共享者改变或用户重新加入导致的视频源失效, 内部都会自动请求
 * 请求后收到首帧图像时将触发 on_video_changed 事件
 *
 * @param[in] vid           视频源. 可以是用户视频 uid 或 共享屏幕 channelId, 空则不接收视频
 * @param[in] index         同时绑定该用户的索引值, 范围[1, 255], 负数表示删除, 0 则不修改
 *
 * @note 未连接服务或 vid 非法时调用无效。 其他情况均阻塞式发送请求,
 *  最终由服务决定是否可以切换视频源
 */
void jrtc_set_video(struct jrtc_t* jc, const char vid[64], int index);

/** 设置用户的状态
 * 状态变化后将收到 on_user_changed 事件
 *
 * @param[in] uid           用户ID
 * @param[in] mask          修改的位掩码
 * @param[in] status        设置的状态集合
 *
 * @note 只修改由mask指定的位, 不影响其他位。
 *  即按如下计算
 *    用户状态 = (用户状态 & ~mask) | (status & mask)
 *
 *  允许 uid 为空或未连接服务时强制设置自身状态且不广播, 其他情况均阻塞式发送请求,
 *  最终由服务广播实际的状态值
 */
void jrtc_set_status (struct jrtc_t* jc, const char uid[64], unsigned mask, unsigned status);

/** 请求设置用户的角色
 * 状态变化后将收到 on_user_changed 事件
 *
 * @param[in] uid           用户ID
 * @param[in] mask          修改的位掩码
 * @param[in] role          设置的角色集合
 *
 * @note 只修改由mask指定的位, 不影响其他位。
 *  即按如下计算
 *    用户角色 = (用户角色 & ~mask) | (role & mask)
 *
 *  未连接服务或 uid 非法时调用无效。 其他情况均阻塞式发送请求,
 *  最终由服务广播实际的角色值
 */
void jrtc_set_role (struct jrtc_t* jc, const char uid[64], unsigned mask, unsigned role);

/** 请求绑定用户的索引值
 * 尽量避免使用该函数, 而使用 on_user_joined/on_user_changed 的返回值绑定索引值
 * 因为请求 (jrtc_set_index/jrtc_set_role/jrtc_set_status/jrtc_set_video)
 * 每次只能发送一个,连续多次时可能阻塞, 而且index不同可能导致其他异步问题。
 *
 * @param[in] uid           用户ID
 * @param[in] index         索引值, 范围[1, 255], 负数表示删除, 0 则忽略
 */
void jrtc_set_index(struct jrtc_t*jc, const char uid[64], int index);

/** 全局动态的批量数据类型 */
enum jrtc_batch
{
    JRTC_AUDIO_VOLUME  = 1,
    JRTC_USER_NETSTATE = 2,
};
/** 请求会话中的动态批量数据
 *
 * @param[in] types         jrtc_batch 类型集合
 */
void jrtc_request_batch(struct jrtc_t* jc, int types);
/** 请求用户音量变化, 将触发 on_audio_volume 回调 */
#define jrtc_request_volume(jc)   jrtc_request_batch(jc, JRTC_AUDIO_VOLUME)
/** 请求用户网络状态, 将触发 on_user_netstate 回调 */
#define jrtc_request_netstate(jc) jrtc_request_batch(jc, JRTC_USER_NETSTATE)

/** 保存外部指针, 返回上次值 */
void* jrtc_set_ptr (struct jrtc_t* jc, void* ptr);
/** 返回保存的指针,初始值为NULL */
void* jrtc_get_ptr (struct jrtc_t* jc);

/** 镜头采集的图像 */
struct jrtc_photo_t
{
    void* data;
    unsigned bytes;
    unsigned short width,height;
    char format;/*< enum jrtc_dtype 采集预览格式 */
    char reserve[3];
};

/** 动态设置前后镜头
 *
 * @param[in] camera 0为后镜头,1是前镜头
 * @param[in] effect 编码前图像的处理回调
 *                   禁止改变图像尺寸和格式.
 *                   可设置为 NULL
 * @code
 *   static char* imgbuf = NULL;
 *   //只镜像发送图像
 *   jrtc_set_camera(jc, 1, [](struct jrtc_photo_t* frame){
 *      if (!imgbuf) imgbuf = new char[frame->bytes];
 *
 *      assert (frame->format == JRTC_NV12 && "根据平台选择转换函数");
 *      jrtc_nv12_mirror(frame, frame->width, frame->height, imgbuf);
 *      frame->data = imgbuf;
 *   });
 *   //只镜像预览图像
 *   jrtc_set_camera(jc, 1, [](struct jrtc_photo_t* frame){
 *      auto preview = frame->data;
 *      if (!imgbuf) imgbuf = new char[frame->bytes];
 *      frame->data = memcpy(imgbuf, preview, frame->bytes);
 *
 *      assert (frame->format == JRTC_NV12 && "根据平台选择转换函数");
 *      jrtc_nv12_mirror(frame, frame->width, frame->height, preview);
 *   });
 *   //同时镜像预览和发送图像
 *   jrtc_set_camera(jc, 1, [](struct jrtc_photo_t* frame){
 *      auto preview = frame->data;
 *      if (!imgbuf) imgbuf = new char[frame->bytes];
 *      frame->data = memcpy(imgbuf, preview, frame->bytes);
 *
 *      assert (frame->format == JRTC_NV12 && "根据平台选择转换函数");
 *      jrtc_nv12_mirror(frame, frame->width, frame->height, preview);
 *      frame->data = preview;
 *   });
 * @endcode
 */
void jrtc_set_camera (struct jrtc_t* jc, int camera,
    void (*effect) (struct jrtc_photo_t* frame));

/*******************************************************************************
 * 各平台相关的特殊全局设置, 通话前直接赋值                                    *
 *******************************************************************************/
/**展锐 Mocor 平台 需要设置PDP流程后获得的网络 ID */
extern unsigned _jrtc_net_id;
/**展锐 Mocor 平台的镜头角度,即采集图像需要顺时针旋转该值,才能正立. 默认90 */
extern unsigned _jrtc_cam_angle;
/** quectel 平台的全局拨号信息地址, 网络就绪后, 按如下代码获取并设置 
 * @code
 * static struct ql_data_call_info info = {0};
 * if (ql_get_data_call_info(1, 0, &info) == 0)
 *      _jrtc_net_info = &info;
 * @endcode
 */
extern void* _jrtc_net_info;

/*******************************************************************************
 * 为了保持暂时兼容, 即将废弃的接口, 新项目上不要使用                          *
 *******************************************************************************/
/** 通话中的媒体设备
 * @deprecated 该枚举和相关函数都将废弃, 不要使用
 */
enum jrtc_device
{
    JRTC_SPEAKER    = 1,
    JRTC_MICROPHONE = 2,
    JRTC_CAMERA     = 4,
    JRTC_AUDIO      = 3,
};
/** 开启发送状态
 * @param[in] devices jrtc_device 的集合
 * @deprecated 将废弃, 请直接使用 jrtc_set_status. 对应代码:
 * @code
 *  int status = 0;
 *  if (devices & JRTC_SPEAKER)    MME_MutePlayout(0);
 *  if (devices & JRTC_MICROPHONE) status |= JRTC_STATUS_AUDIO;
 *  if (devices & JRTC_CAMERA)     status |= JRTC_STATUS_VIDEO;
 *  jrtc_set_status(jc, NULL, status, status);
 * @endcode
 */
void jrtc_activate (struct jrtc_t* jc, int devices);

/** 关闭发送状态
 * @param[in] devices jrtc_device 的集合
 * @deprecated 将废弃, 请直接使用 jrtc_set_status. 对应代码:
 * @code
 *  int status = 0;
 *  if (devices & JRTC_SPEAKER)    MME_MutePlayout(1);
 *  if (devices & JRTC_MICROPHONE) status |= JRTC_STATUS_AUDIO;
 *  if (devices & JRTC_CAMERA)     status |= JRTC_STATUS_VIDEO;
 *  jrtc_set_status(jc, NULL, status, ~status);
 * @endcode
 */
void jrtc_deactivate (struct jrtc_t* jc, int devices);

#ifdef __cplusplus
}
#endif
#endif
