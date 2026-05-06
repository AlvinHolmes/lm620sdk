#ifndef __RES_TREE_CORE_H__
#define __RES_TREE_CORE_H__

#include <stdint.h>
#include <stdbool.h>

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define RES_TREE_NAME_LEN           15

#define RES_TREE_PRINT_RAW          osPrintf
#define RES_TREE_DEBUG(fmt, ...)    osPrintf("\033[" "30m" fmt "\033[0m" "\r\n", ##__VA_ARGS__)
#define RES_TREE_INFO(fmt, ...)     osPrintf("\033[" "33m" fmt "\033[0m" "\r\n", ##__VA_ARGS__)
#define RES_TREE_ERROR(fmt, ...)    osPrintf("\033[" "31m" fmt "\033[0m" "\r\n", ##__VA_ARGS__)

typedef uint8_t RES_TREE_TYPE;
typedef uint8_t RES_TREE_REF_TYPE;

#define RES_TREE_TYPE_NORMAL    (1)
#define RES_TREE_TYPE_NULL      (2)
#define RES_TREE_TYPE_SINGLE    (3)
#define RES_TREE_TYPE_MULTI     (4)
#define RES_TREE_TYPE_LEAF      (5)

// 叶子池，尽量小
typedef struct ResTreeLeaf{
    uint8_t refs;
    void (*func)(bool enable);
#if RES_TREE_NAME_LEN
    char    name[RES_TREE_NAME_LEN];
#endif
}ResTreeLeaf_t;

typedef struct {
    RES_TREE_TYPE       type;
#if RES_TREE_NAME_LEN
    char                name[RES_TREE_NAME_LEN];
#endif
}ResTree_BodyNull;

typedef struct {
    RES_TREE_TYPE       type;
    RES_TREE_REF_TYPE   refs;
#if RES_TREE_NAME_LEN
    char                name[RES_TREE_NAME_LEN];
#endif
}ResTree_BodyNormal;

typedef struct {
    RES_TREE_TYPE       type;
    RES_TREE_REF_TYPE   refs;
    void (*func)(bool enable);
#if RES_TREE_NAME_LEN
    char                name[RES_TREE_NAME_LEN];
#endif
}ResTree_BodySingle;

typedef struct {
    RES_TREE_TYPE       type;
    ResTreeLeaf_t       *leafs;
    uint8_t             depth;
#if RES_TREE_NAME_LEN
    char                name[RES_TREE_NAME_LEN];
#endif
}ResTree_BodyLeaf;

// 资源节点
typedef struct ResTree {
    struct ResTree      *parent;    // 父节点
    struct ResTree      *child;     // 子节点
    void                *body;
#if RES_TREE_NAME_LEN
    char                name[RES_TREE_NAME_LEN];
#endif
}ResTree_t;

#define RES_TREE_NODE(tree)         ((tree)->node)
#define RES_TREE_BODY_TYPE(tree)    (*(RES_TREE_TYPE *)((tree)->body))
#define RES_TREE_BODY(tree)         ((tree)->body)

// void ResTree_Register(ResTree_t *tree);
void ResTree_Acquire(ResTree_t *tree);
void ResTree_Release(ResTree_t *tree);

void ResTree_LeafAcquire(ResTree_t *tree, ResTreeLeaf_t *leaf);
void ResTree_LeafRelease(ResTree_t *tree, ResTreeLeaf_t *leaf);



#endif