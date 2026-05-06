/**********************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 *  distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License  for the specific language governing permissions and limitations under the License.
 *
 ***********************************************************************************************************/

/******************************************************************************
* Referenced head files
*****************************************************************************/

#ifndef __CMDM_API_H__
#define __CMDM_API_H__

#ifdef __cplusplus
extern "C" {
#endif

//#include <oneos_config.h>
#include <stdint.h>
//#include "os_mq.h"
#include <os.h>


/******************************************************************************
* Definitions
******************************************************************************/
typedef enum
{
    ENUM_RESOURCE_IMSI1 = 0,
    ENUM_RESOURCE_IMSI2,
    ENUM_RESOURCE_SN,
    ENUM_RESOURCE_MAC,
    ENUM_RESOURCE_ROM,
    ENUM_RESOURCE_RAM,
    ENUM_RESOURCE_CPU,
    ENUM_RESOURCE_SYSVERSION,
    ENUM_RESOURCE_SOFTWAREVER,
    ENUM_RESOURCE_SOFTWARENAME,
    ENUM_RESOURCE_VOLTE,
    ENUM_RESOURCE_NETTYPE,
    ENUM_RESOURCE_PHONENUMBER,
    ENUM_RESOURCE_BATTERYCAPACITY,
    ENUM_RESOURCE_SCREENSIZE,
    ENUM_RESOURCE_NETWORKSTATUS,
    ENUM_RESOURCE_WEARINGSTATUS,
    ENUM_RESOURCE_ROUTERMAC,
    ENUM_RESOURCE_BLUETOOTHMAC,
    ENUM_RESOURCE_GPU,
    ENUM_RESOURCE_BOARD,
    ENUM_RESOURCE_RESOLUTION,
    MAX_RESOURCE_CNT
} enum_resource_id;

typedef struct
{
    uint8_t msg_id;
    uint8_t event_id;
}cmdm_msg_t;

#define CMDM_USER_MSG_ID_INVALID                        0
#define CMDM_USER_MSG_ID_STOPPED                        1

#define CMDM_USER_MSG_EVENT_INVALID                     0
#define CMDM_USER_MSG_EVENT_USER_STOPPED                1
#define CMDM_USER_MSG_EVENT_MSG_QUEUE_ERROR             2
#define CMDM_USER_MSG_EVENT_ENDPOINT_ERROR              3
#define CMDM_USER_MSG_EVENT_CREATE_CLIENT_ERROR         4


#define CMDM_NO_ERROR                       (0)
#define CMDM_RUNNING_STATE_ERROR            (-1)
#define CMDM_MQ_ERROR                       (-2)
#define CMDM_MUTEX_ERROR                    (-3)
#define CMDM_MALLOC_ERROR                   (-4)
#define CMDM_TASK_ERROR                     (-5)
#define CMDM_SET_CONFIG_ERROR               (-6)

#ifndef os_malloc
#define os_malloc             osMalloc
#endif

#ifndef os_free
#define os_free               osFree
#endif

typedef void os_task_t;
typedef void os_mq_t;
typedef void os_mutex_t;

#define OS_WAIT_FOREVER         osWaitForever

/******************************************************************************
 * Function declares
 *****************************************************************************/
/**
 ************************************************************************************
 * @brief           设置设备信息
 *
 * @param[in]       brand : 品牌名称 ， 字符串
 * @param[in]       model : 型号名称  ，字符串
 * @param[in]       template_id : 模板ID， 字符串
 *
 * @return 0:success
 * @note none
 ************************************************************************************
*/
int cmdm_set_device_config(char* brand, char* model, char* template_id);
/**
 ************************************************************************************
 * @brief           获取设备信息
 *
 * @param[out]       brand : 品牌名称 ， 字符串
 * @param[in]        brand_len : 指示brand缓存大小
 * @param[out]       model : 型号名称  ，字符串
 * @param[in]        model_len : 指示model缓存大小
 * @param[out]       template_id : 模板ID， 字符串
 * @param[in]        id_len : 指示template_id缓存大小
 *
 * @return none
 * @note none
 ************************************************************************************
*/
void cmdm_get_device_config(char* brand, uint8_t brand_len, char* model, uint8_t model_len, char* template_id, uint8_t id_len);
/**
 ************************************************************************************
 * @brief           设置规则信息
 *
 * @param[in]       report_time : 最大上报频率，单位分钟
 * @param[in]       report_num : 最大上报频率次数
 * @param[in]       retry_interval : 异常重试间隔
 * @param[in]       retry_num : 异常重试次数
 *
 * @return 0:success
 * @note report_time：默认1440分钟 report_num：默认8 retry_interval：默认10 retry_num：默认1
 ************************************************************************************
*/
int cmdm_set_rule_config(uint16_t report_time, uint16_t report_num, uint16_t retry_interval, uint16_t retry_num);
/**
 ************************************************************************************
 * @brief           获取规则信息
 *
 * @param[out]       report_time : 最大上报频率，单位分钟
 * @param[out]       report_num : 最大上报频率次数
 * @param[out]       retry_interval : 异常重试间隔
 * @param[out]       retry_num : 异常重试次数
 *
 * @return none
 * @note none
 ************************************************************************************
*/
void cmdm_get_rule_config(uint16_t* report_time, uint16_t* report_num, uint16_t* retry_interval, uint16_t* retry_num);
/**
 ************************************************************************************
 * @brief           设置地址信息
 *
 * @param[in]       addr : 服务器地址
 * @param[in]       port : 服务器端口
 *
 * @return 0:success
 * @note 默认地址: coap://m.fxltsbl.com  默认端口：5683
 ************************************************************************************
*/
int cmdm_set_addr_config(char* addr, uint16_t port);
/**
 ************************************************************************************
 * @brief           获取地址信息
 *
 * @param[out]       addr : 服务器地址
 * @param[in]        addr_len : 指示addr缓存大小
 * @param[out]       port : 服务器端口
 *
 * @return none
 * @note none
 ************************************************************************************
*/
void cmdm_get_addr_config(char* addr, uint8_t addr_len, uint16_t* port);

/**
 ************************************************************************************
 * @brief           设置资源
 *
 * @param[in]       id : 资源ID，见 enum_resource_id 定义
 * @param[in]       resource : 资源值，字符串
 *
 * @return 0:success
 * @note
 ************************************************************************************
*/
int cmdm_set_resource_config(enum_resource_id id, char* resource);

/**
 ************************************************************************************
 * @brief           启动CMDM
 *
 * @param[in]       id : 资源ID，见 enum_resource_id 定义
 * @param[out]      resource : 存放id对应的值
 * @param[in]       resource_len ：指示resource内存大小
 *
 * @return none
 * @note
 ************************************************************************************
*/
void cmdm_get_resource_config(enum_resource_id id, char* resource, uint8_t resource_len);

/**
 ************************************************************************************
 * @brief           启动CMDM
 *
 * @param[in]       heart_beat：心跳频率，单位分钟
 * @param[in]       app_key ： 平台注册的appKey
 * @param[in]       password : 平台注册的password
 * @param[in]       user_mq :  系统消息队列，用于任务间通信，用 os_mq_create 创建
 *
 * @return 0:success
 * @note
 ************************************************************************************
*/
int cmdm_start(uint16_t heart_beat, char* app_key, char* password, void* user_mq);
/**
 ************************************************************************************
 * @brief           结束CMDM
 *
 * @param[in]       none
 *
 * @return 0:success
 * @note
 ************************************************************************************
*/
int cmdm_stop(void);



#ifdef __cplusplus
}
#endif
#endif /* __CMDM_API_H__*/

