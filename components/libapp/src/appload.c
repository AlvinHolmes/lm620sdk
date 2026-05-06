/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        appload.c
 *
 * @brief       实现静态加载接口
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-05-11     ICT Team          创建
 ************************************************************************************
 */
/************************************************************************************
 *                                 头文件定义
 ************************************************************************************/
#include "appload.h"
#include <drv_common.h>
#include <drv_lp.h>
#include <drv_flash.h>
#include <md5.h>

#define DBG_TAG    "APPS"
#define DBG_LVL    DBG_WARNING
#include <os_log.h>          // must after of DBG_ENABLE or some other options

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define APP_IMAGE_HEAD              "APPS"

extern int app_main(void *param);
/************************************************************************************
 *                                 结构变量定义
 ************************************************************************************/
extern uint32_t __app_ram_start;
extern uint32_t __app_ram_end;

typedef void (*app_entry_func_t)(void * param);

typedef struct {
    int8_t      symb[4];            ///< Symbol of image's head.
    uint32_t    addr_store;               ///store addr of app[flash].
    uint32_t    addr_load;               //run addr of app[psram].
    uint32_t    size;               ///< Size of image in bytes including head.
    uint32_t    resv;               ///< Resv of image.
}APP_ImageHead;

/**
 * @brief Image tail.
 */
typedef struct {
    uint8_t     md5[16];            ///< MD5 of image data excluding tail.
}APP_ImageTail;

typedef struct
{
    app_entry_func_t  entry_addr;
    void *arg;
} APP_ImageEntry;

APP_ImageEntry s_threadParam = {0};

/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/

static int32_t prvReadflash(const char* name, uint32_t offset, uint32_t len, uint8_t *buf)
{
    PART_Handle *part = PART_Find(name);
    if (part && (uint32_t)PART_Read(part, offset, buf, len) == len)
        return 0;
    return -1;
}

static void appimg_thread_entry(void  *param)
{
    APP_ImageEntry *imgEntry = (APP_ImageEntry *)param;
    app_entry_func_t appfunc = NULL;

    LOG_D("run main entry: 0x%08X\r\n", imgEntry->entry_addr);
    appfunc = (app_entry_func_t)(imgEntry->entry_addr);
    if (appfunc)
       (appfunc)(imgEntry->arg);

    return ;
}

static osStatus_t   appimg_run(void *param)
{
    if (param) {
        /* exec this module */
        osThread_t tid;
        osThreadAttr_t attr = {"appImg", osThreadDetached | osThreadManuStart, NULL, 0U, NULL, APP_THREAD_STACK_SIZE, APP_THREAD_PRIORITY, 0U, 0U};

        tid = osThreadNew(appimg_thread_entry, param, &attr);
        if (tid != OS_NULL) {
            osThreadStart(tid);
            return osOK;
        }
    }
    return osError;
}

static uint8_t *appimg_load(const char* part, uint32_t offset)
{
    APP_ImageHead  head;
    APP_ImageTail  tail;
    uint8_t         md5[16];
    MD5_CTX         md5ctx;

    size_t          length;
    uint8_t         *module_ptr = OS_NULL;
#if 0
    if(__malloc_end > __app_ram_start)
    {
        LOG_D("app image load addr err\r\n");
        goto __exit;
    }
#endif
    if (prvReadflash(part, offset, sizeof(head), (uint8_t*)&head)) {
        LOG_D("Load image head from fail\r\n");
        goto __exit;
    }

    if (memcmp(head.symb, APP_IMAGE_HEAD, sizeof(head.symb)) ||
        head.size <= (sizeof(APP_ImageHead) + sizeof(APP_ImageTail)))
    {
        //(head.addr_load != __app_ram_start)) {
        LOG_W("appimg error magic\r\n");
        goto __exit;
    }

    LOG_D("HEAD:__app_ram_start[%x],__app_ram_end[%x]\r\n",&__app_ram_start,&__app_ram_end);

    MD5Init(&md5ctx);
    if (prvReadflash(part, offset + head.size - sizeof(APP_ImageTail),
        sizeof(APP_ImageTail), (uint8_t*)&tail)) {
        LOG_D("Load image tail from fail\r\n");
        goto __exit;
    }
    MD5Update(&md5ctx, (unsigned char*)&head, sizeof(APP_ImageHead));

    //module_ptr = (uint8_t *)head.addr_load;
    module_ptr = (uint8_t *)&__app_ram_start;
    memset(module_ptr,0,(uint32_t)&__app_ram_end - (uint32_t)&__app_ram_start);
    LOG_D("appimg_load: memset size[%d]\r\n",(uint32_t)&__app_ram_end - (uint32_t)&__app_ram_start);

    length = head.size - sizeof(APP_ImageHead) - sizeof(APP_ImageTail);
    if (prvReadflash(part, offset + sizeof(head), length, module_ptr)) {
        LOG_D("Load image data from fail\r\n");
        goto __exit;
    }
    MD5Update(&md5ctx, (unsigned char*)module_ptr, length);
    MD5Final(md5, &md5ctx);

    if (memcmp(md5, tail.md5, sizeof(md5))) {
        LOG_W("Check image MD5 fail\r\n");
        goto __exit;
    }

    /* deal with cache */
    osDCacheCleanAndInvalidRange(module_ptr, length);
    osICacheInvalidRange(module_ptr, length);

    return module_ptr;

__exit:
    return NULL;
}


/************************************************************************************
 *                                 外部函数定义
 ************************************************************************************/
__attribute__((section(".app_code")))  int8_t appimg_exec(const char* part, uint32_t offset,void *param)
{

    memset(&s_threadParam,0,sizeof(APP_ImageEntry));

    s_threadParam.entry_addr = (app_entry_func_t)appimg_load(part, offset);
    s_threadParam.arg = param;
    LOG_D("appimg_exec:entry[%x],param[%x]\r\n",s_threadParam.entry_addr,s_threadParam.arg);


    if (s_threadParam.entry_addr) {
        return appimg_run(&s_threadParam);
    }

    return osError;
}

