#include "res_tree_core.h"
#include <stddef.h>
#include <os.h>
#include <drv_common.h>

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef void (*res_tree_func_t)(bool);

typedef enum {
    RES_TREE_INC_MODE = 0,
    RES_TREE_DEC_MODE
}RES_TREE_INC_DEC;

/************************************************************************************
 *                                 全局变量
 ************************************************************************************/

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/

// ref加1，返回是否是从0到1
static bool ResTree_AcquireInc(void *body, RES_TREE_TYPE type, RES_TREE_INC_DEC mode)
{
    bool flag = false;
    res_tree_func_t func = NULL;

    unsigned long level = osInterruptDisable();

    switch (type)
    {
        case RES_TREE_TYPE_NORMAL:
            ResTree_BodyNormal *bodyNormal = (ResTree_BodyNormal *)body;
            
            if(mode == RES_TREE_INC_MODE) {
                if(bodyNormal->refs == 0) {
                    flag = true;
                }
                bodyNormal->refs++;
            #if RES_TREE_NAME_LEN
                RES_TREE_DEBUG("[%-10s] increase refs : %d", bodyNormal->name, bodyNormal->refs);
            #else
                RES_TREE_DEBUG("increase refs : %d", bodyNormal->refs);
            #endif
            }
            else {
                bodyNormal->refs--;
                if(bodyNormal->refs == 0) {
                    flag = true;
                }
            #if RES_TREE_NAME_LEN
                RES_TREE_DEBUG("[%-10s] decrease refs : %d", bodyNormal->name, bodyNormal->refs);
            #else
                RES_TREE_DEBUG("decrease refs : %d", bodyNormal->refs);
            #endif
            }
            break;

        case RES_TREE_TYPE_NULL:
            flag = true;
            break;

        case RES_TREE_TYPE_SINGLE:
            ResTree_BodySingle *bodySingle = (ResTree_BodySingle *)body;
            if(mode == RES_TREE_INC_MODE) {
                if(bodySingle->refs == 0) {
                    flag = true;
                    func = bodySingle->func;
                }
                bodySingle->refs++;
            #if RES_TREE_NAME_LEN
                RES_TREE_DEBUG("[%-10s] increase refs : %d", bodySingle->name, bodySingle->refs);
            #else
                RES_TREE_DEBUG("increase refs : %d", bodySingle->refs);
            #endif
            }
            else {
                bodySingle->refs--;
                if(bodySingle->refs == 0) {
                    flag = true;
                    func = bodySingle->func;
                }
            #if RES_TREE_NAME_LEN
                RES_TREE_DEBUG("[%-10s] decrease refs : %d", bodySingle->name, bodySingle->refs);
            #else
                RES_TREE_DEBUG("decrease refs : %d", bodySingle->refs);
            #endif
            }
            break;

        case RES_TREE_TYPE_MULTI:
            
            break;
        
        default:
            break;
    }

    osInterruptEnable(level);

    if(flag && func) {
        if(mode == RES_TREE_INC_MODE) {
            func(true);
        #if RES_TREE_NAME_LEN
            RES_TREE_DEBUG("[%-10s] enable");
        #else
            RES_TREE_DEBUG("enable");
        #endif
        }
        else {
            func(false);
        #if RES_TREE_NAME_LEN
            RES_TREE_DEBUG("[%-10s] disable");
        #else
            RES_TREE_DEBUG("disable");
        #endif
        }
    }

    return flag;
}

void ResTree_Acquire(ResTree_t *tree)
{
    ResTree_t *pos = tree;
    RES_TREE_TYPE type;

    OS_ASSERT(RES_TREE_BODY(pos));

    do {
        type = RES_TREE_BODY_TYPE(pos);
        if(ResTree_AcquireInc(RES_TREE_BODY(pos), type, RES_TREE_INC_MODE) == false) {
            break;
        }
        if(pos->parent == NULL) {
            break;
        }
        pos = pos->parent;
    } while (1);
}

void ResTree_Release(ResTree_t *tree)
{
    ResTree_t *pos = tree;
    RES_TREE_TYPE type;

    OS_ASSERT(RES_TREE_BODY(pos));

    do {
        type = RES_TREE_BODY_TYPE(pos);
        if(ResTree_AcquireInc(RES_TREE_BODY(pos), type, RES_TREE_DEC_MODE) == false) {
            break;
        }
        if(pos->parent == NULL) {
            break;
        }
        pos = pos->parent;
    } while (1);
}

void ResTree_LeafAcquire(ResTree_t *tree, ResTreeLeaf_t *leaf)
{
    bool flag = false;
    unsigned long level = osInterruptDisable();

    if(leaf->refs == 0) {
        flag = true;
    }

    leaf->refs++;

    osInterruptEnable(level);

    #if RES_TREE_NAME_LEN
        RES_TREE_DEBUG("[%-10s] increase refs : %d", leaf->name, leaf->refs);
    #else
        RES_TREE_DEBUG("increase refs : %d", leaf->refs);
    #endif

    if(flag && leaf->func) {
    #if RES_TREE_NAME_LEN
        RES_TREE_DEBUG("[%-10s] enable", leaf->name);
    #else
        RES_TREE_DEBUG("enable");
    #endif
        leaf->func(true);
    }

    if(flag) {
        ResTree_Acquire(tree);
    }
}

void ResTree_LeafRelease(ResTree_t *tree, ResTreeLeaf_t *leaf)
{
    OS_ASSERT(leaf->refs);

    bool flag = false;
    unsigned long level = osInterruptDisable();

    leaf->refs--;
    if(leaf->refs == 0) {
        flag = true;
    }

    osInterruptEnable(level);

    #if RES_TREE_NAME_LEN
        RES_TREE_DEBUG("[%-10s] decrease refs : %d", leaf->name, leaf->refs);
    #else
        RES_TREE_DEBUG("decrease refs : %d", leaf->refs);
    #endif

    if(flag && leaf->func) {
    #if RES_TREE_NAME_LEN
        RES_TREE_DEBUG("[%-10s] disable", leaf->name);
    #else
        RES_TREE_DEBUG("disable");
    #endif
        leaf->func(false);
    }

    if(flag) {
        ResTree_Release(tree);
    }
}