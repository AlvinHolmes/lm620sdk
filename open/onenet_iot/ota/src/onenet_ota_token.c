/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file
 *
 * @brief:OneNET OTA token
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-08-28     ict team          创建
 ************************************************************************************
 */
#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include "os.h"
#include "mbedtls/base64.h"
#include "mbedtls/md.h"
#include "mbedtls/md_internal.h"
#include "onenet_ota_pub.h"
#include "onenet_ota_api.h"
#include "onenet_ota_token.h"

/************************************************************************************
 *                                 函数声明
 ************************************************************************************/
typedef enum
{
    OTA_TOKEN_METHOD_TYPE_MD5 = 0,
    OTA_METHOD_TOKEN_TYPE_SHA1,
    OTA_METHOD_TOKEN_TYPE_SHA256,
    OTA_METHOD_TOKEN_TYPE_MORE,
} OTA_TokenMethodType; /* 计算 Token 采用的方法 */


/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define OTA_TOKEN_ON
 #ifdef OTA_TOKEN_ON
#define OTA_TOKEN_DBG_PRINTF                        OTA_PUB_PRINT_DEBUG
#else
#define OTA_TOKEN_DBG_PRINTF(...)
#endif
#define OTA_TOKEN_ERR_PRINTF                        OTA_PUB_PRINT_ERROR

#define OTA_TOKEN_MAX_PRINT_ONCE                    (126)
#define OTA_TOKEN_USERID_LEN_MAX                    (16)
#define OTA_TOKEN_PROJECTID_LEN_MAX                 (16)
#define OTA_TOKEN_PRODUCTS_LEN_MAX                  (32)
#define OTA_TOKEN_DEVICE_NAME_LEN_MAX               (64)
#define OTA_TOKEN_MD5_HMAC_SIGN_LEN_MAX             (16)
#define OTA_TOKEN_SHA1_HMAC_SIGN_LEN_MAX            (20)
#define OTA_TOKEN_SHA256_HMAC_SIGN_LEN_MAX          (32)
#define OTA_TOKEN_HMAC_SIGN_LEN_MAX                 (32)
#define OTA_TOKEN_FINAL_SIGN_LEN_MAX                (128)
#define OTA_TOKEN_BASE64DECODE_LEN_MAX              (255)

#define OTA_TOKEN_RES_USERID                        "userid"
#define OTA_TOKEN_RES_PROJECTID                     "projectid"
#define OTA_TOKEN_RES_PRODUCTS                      "products"
#define OTA_TOKEN_RES_DEVICES                       "devices"
#define OTA_TOKEN_METHOD_STRING_MD5                 "md5"
#define OTA_TOKEN_METHOD_STRING_SHA1                "sha1"
#define OTA_TOKEN_METHOD_STRING_SHA256              "sha256"
#define OTA_TOKEN_VERISON                           "2018-10-31"



/************************************************************************************
 *                                 类型定义
 ************************************************************************************/


/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/


/************************************************************************************
 *                                 内部函数定义
 ************************************************************************************/
static void OTA_TokenBufPrint(char *buf, const char *func)
{
    int i = 0;
    char tmpBuf[OTA_TOKEN_MAX_PRINT_ONCE + 1] = {0};

    if(NULL == buf)
    {
        OTA_TOKEN_ERR_PRINTF("%s, buf is null\r\n", __FUNCTION__);
        return;
    }

    if(strlen(buf) > ONENET_OTA_TOKEN_AUTH_LEN_MAX)
    {
        OTA_TOKEN_ERR_PRINTF("%s, buf overflow\r\n", __FUNCTION__);
        return ;
    }

    i = strlen(buf) / (OTA_TOKEN_MAX_PRINT_ONCE + 1);
    OTA_TOKEN_DBG_PRINTF("%s, bufLen[%d]: \r\n", func, strlen(buf));
    for(int j = 0; j <= i; j++)
    {
        memcpy(tmpBuf, buf + j * OTA_TOKEN_MAX_PRINT_ONCE, OTA_TOKEN_MAX_PRINT_ONCE);
        tmpBuf[OTA_TOKEN_MAX_PRINT_ONCE] = '\0';
        OTA_TOKEN_DBG_PRINTF("%s", tmpBuf);
        memset(tmpBuf, 0, strlen(tmpBuf));
    }
    OTA_TOKEN_DBG_PRINTF("\r\n\r\n");

    return;
}

/* token 中 key=value 的形式的 value 部分的特殊符号需要经过 URL 编码 */
static int32_t OTA_TokenUrlEncode(char *valueBuf, uint16_t valueBufSize)
{
    uint16_t i = 0;
    uint16_t count = 0;
    uint16_t valueLen = 0;
    char *tmpBuf = NULL;

    if((NULL == valueBuf) || (valueBufSize == 0))
    {
        OTA_TOKEN_ERR_PRINTF("%s, valueBuf[%p], len[%d] err\r\n", __FUNCTION__, valueBuf, valueBufSize);
        return osError;
    }

    valueLen = strlen(valueBuf);
    if (NULL == tmpBuf)
    {
        tmpBuf = osMalloc(valueLen + 1); /* +1 for store '\0' */
        if (NULL == tmpBuf)
        {
            OTA_TOKEN_ERR_PRINTF("%s, memory malloc fail\r\n", __FUNCTION__);
            return osErrorTimeout;
        }
    }
    memset(tmpBuf, 0, valueLen + 1);

    //OTA_TOKEN_DBG_PRINTF("%s, valueBuf[%s], valueBufSize[%d], valueLen[%u]\r\n", __FUNCTION__, valueBuf, valueBufSize, valueLen);
    for (i = 0; i < valueLen; i++)
    {
        if((valueBuf[i] == '+') || (valueBuf[i] == ' ') || (valueBuf[i] == '/') || (valueBuf[i] == '?') ||
            (valueBuf[i] == '%') || (valueBuf[i] == '#') || (valueBuf[i] == '&') || (valueBuf[i] == '='))
        {
            count++;
        }
        tmpBuf[i] = valueBuf[i];
    }

    if (count * 2 + valueLen > valueBufSize)
    {
        OTA_TOKEN_ERR_PRINTF("%s, valueBufSize[%d] less than valueLen[%d]", __FUNCTION__, valueBufSize, valueLen);
        return osErrorResource;
    }

    for(i = 0; i < valueLen; i++)
    {
        switch(tmpBuf[i])
        {
            case '+':
                memcpy(valueBuf, "%2B", 3);
                valueBuf += 3;
                break;
            case ' ':
                memcpy(valueBuf, "%20", 3);
                valueBuf += 3;
                break;
            case '/':
                memcpy(valueBuf, "%2F", 3);
                valueBuf += 3;
                break;
            case '?':
                memcpy(valueBuf, "%3F", 3);
                valueBuf += 3;
                break;
            case '%':
                memcpy(valueBuf, "%25", 3);
                valueBuf += 3;
                break;
            case '#':
                memcpy(valueBuf, "%23", 3);
                valueBuf += 3;
                break;
            case '&':
                memcpy(valueBuf, "%26", 3);
                valueBuf += 3;
                break;
            case '=':
                memcpy(valueBuf, "%3D", 3);
                valueBuf += 3;
                break;
            default:
                *valueBuf = tmpBuf[i];
                valueBuf += 1;
                break;
        }
    }

    if (tmpBuf != NULL)
    {
        osFree(tmpBuf);
        tmpBuf = NULL;
    }

    return osOK;
}


/************************************************************************************
 *                                 外部函数定义
 ************************************************************************************/
/**
 ***********************************************************************************************
 * @brief           1. Token 的签名生成算法为:
                    sign = base64(hmac_<method>(base64decode(accessKey), utf-8(StringForSignature)))
 *                  2. StringForSignature组成:
 *                  StringForSignature = et + '\n' + method + '\n' + res+ '\n' + version
 *
 * @param[in]       res            格式为: userid/xxx 或 projectid/xxx 或 products/xxx/devices/xxx 或 products/xxx
 * @param[in]       et             UNIX时间戳, 从1970年1月1日开始所经过的秒数. 转换成北京时间必须比当前
 *                                 系统时间更大. 1836152085代表北京时间2028-03-09 02:14:45
 * @param[in]       accessKey      对应的秘钥(用户秘钥或产品秘钥或设备秘钥). 秘钥是唯一的
 * @param[in]       method         签名方法. 有 md5, sha1, sha256
 *
 * @param[out]      authorization  生成的安全秘钥鉴权
 *
 * @return          osOK: Token 计算生成成功. 其它: Token 计算生成失败
 ***********************************************************************************************
*/
int32_t OTA_TokenGenerate(const char *res, uint32_t et, const char *accessKey, uint8_t method, char *authorization)
{
    int8_t ret = osOK;
    uint8_t methodSignLen = 0;
    uint16_t base64DecodeLen = OTA_TOKEN_BASE64DECODE_LEN_MAX;
    uint8_t hmacSignLen = OTA_TOKEN_HMAC_SIGN_LEN_MAX;
    uint8_t finalSignLen = OTA_TOKEN_FINAL_SIGN_LEN_MAX;
    uint16_t stringForSignLen= 0;
    char *base64Decode = NULL;
    char *hmacSignBuf = NULL;
    char *finalSignBuf = NULL;
    char *stringForSign = NULL;
    char *sigMethodStr = NULL;
    mbedtls_md_type_t mdType = MBEDTLS_MD_MD5;
    char *tmpToken = NULL;
    char *token = authorization;
    uint16_t resValueLen = 0;

    if((NULL == authorization) || (NULL == res) || (NULL == accessKey))
    {
        OTA_TOKEN_ERR_PRINTF("%s, param error\r\n", __FUNCTION__);
        return osError;
    }

    OTA_TOKEN_DBG_PRINTF("res[%s], et[%lu], accessKey[%s], method[%u]\r\n", res, et, accessKey, method);

    /* 1. version */
    osSprintf(token, "version=%s", OTA_TOKEN_VERISON);

    /* 2. parse res */
    if(!strncmp(res, OTA_TOKEN_RES_USERID, strlen(OTA_TOKEN_RES_USERID))) /* userid */
    {
        resValueLen = strlen(OTA_TOKEN_RES_USERID"/") + OTA_TOKEN_USERID_LEN_MAX * 2; /* 考虑到 res 的值有可能每个字符都需要进行编码, 因此长度乘以 2 */
    }
    else if(!strncmp(res, OTA_TOKEN_RES_PROJECTID, strlen(OTA_TOKEN_RES_PROJECTID))) /* projectid */
    {
        resValueLen = strlen(OTA_TOKEN_RES_PROJECTID"/") + OTA_TOKEN_PROJECTID_LEN_MAX * 2;
    }
    else if(!strncmp(res, OTA_TOKEN_RES_PRODUCTS, strlen(OTA_TOKEN_RES_PRODUCTS))) /* products */
    {
        char *devNamePtr = NULL;
        uint8_t productIdLen = strlen(OTA_TOKEN_RES_PRODUCTS"/");
        uint8_t devNameLen = strlen(OTA_TOKEN_RES_DEVICES"/");
        devNamePtr = (char *) strstr(res, "/"OTA_TOKEN_RES_DEVICES"/");
        if(NULL == devNamePtr)
        {
            resValueLen = productIdLen + OTA_TOKEN_PRODUCTS_LEN_MAX * 2;
        }
        else
        {
            resValueLen = productIdLen + devNameLen + (OTA_TOKEN_PRODUCTS_LEN_MAX + OTA_TOKEN_DEVICE_NAME_LEN_MAX) * 2;
        }
    }
    else
    {
        OTA_TOKEN_ERR_PRINTF("%s, res[%s] error\r\n", __FUNCTION__, res);
        return osErrorISR;
    }
    char *resValue = token + strlen(token);
    osSprintf(token + strlen(token), "&res=%s", res);
    resValue = resValue + strlen("&res=");
    OTA_TokenUrlEncode(resValue, resValueLen);

    /* 3. et */
    osSprintf(token + strlen(token), "&et=%lu", et);

    /* sign gen step1: base64decode(accessKey) */
    size_t olen = 0;
    if (NULL == base64Decode)
    {
        base64Decode = osMalloc(base64DecodeLen + 1); /* +1 for store '\0' */
        if (NULL == base64Decode)
        {
            OTA_TOKEN_ERR_PRINTF("%s, base64Decode malloc fail\r\n", __FUNCTION__);
            return osErrorResourceFull;
        }
    }
    memset(base64Decode, 0, base64DecodeLen + 1);
    ret = mbedtls_base64_decode((unsigned char *)base64Decode, base64DecodeLen, &olen, (const unsigned char *)accessKey, strlen(accessKey));
    //OTA_TokenBufPrint(base64Decode, "base64Decode");

    /* sign gen step2: StringForSignature */
    if(OTA_TOKEN_METHOD_TYPE_MD5 == method)
    {
         sigMethodStr = OTA_TOKEN_METHOD_STRING_MD5;
         methodSignLen = OTA_TOKEN_MD5_HMAC_SIGN_LEN_MAX;
         mdType = MBEDTLS_MD_MD5;
    }
    else if(OTA_METHOD_TOKEN_TYPE_SHA1 == method)
    {
        sigMethodStr = OTA_TOKEN_METHOD_STRING_SHA1;
        methodSignLen = OTA_TOKEN_SHA1_HMAC_SIGN_LEN_MAX;
        mdType = MBEDTLS_MD_SHA1;
    }
    else if(OTA_METHOD_TOKEN_TYPE_SHA256 == method)
    {
        sigMethodStr = OTA_TOKEN_METHOD_STRING_SHA256;
        methodSignLen = OTA_TOKEN_SHA256_HMAC_SIGN_LEN_MAX;
        mdType = MBEDTLS_MD_SHA256;
    }

    /* stringForSign */
    stringForSignLen =strlen(sigMethodStr) + strlen(res) + strlen(OTA_TOKEN_VERISON) + strlen("\n") *4 + 9 + 1; /* et 类型为 uint32_t, 最大值所占的字符大小为 9 */
    if (NULL == stringForSign)
    {
        stringForSign = osMalloc(stringForSignLen + 1); /* +1 for store '\0' */
        if (NULL == stringForSign)
        {
            OTA_TOKEN_ERR_PRINTF("%s, stringForSign malloc fail\r\n", __FUNCTION__);
            OTA_PubBufFree(base64Decode);
            base64Decode = NULL;
            return osErrorBusy;
        }
    }
    memset(stringForSign, 0, stringForSignLen + 1);
    osSprintf(stringForSign, "%lu\n%s\n%s\n%s", et, sigMethodStr, res, OTA_TOKEN_VERISON);

    /* 4. method */
    osSprintf(token + strlen(token), "&method=%s", sigMethodStr);

    /* sign gen step3: hmac_<method> */
    mbedtls_md_context_t hmacCtx;
    mbedtls_md_init(&hmacCtx);
    const mbedtls_md_info_t* mdInfo = mbedtls_md_info_from_type(mdType);
    if (mdInfo == NULL)
    {
        OTA_TOKEN_DBG_PRINTF("%s, md info is null\r\n", __FUNCTION__);
        OTA_PubBufFree(base64Decode);
        OTA_PubBufFree(stringForSign);
        base64Decode = NULL;
        stringForSign = NULL;
        return osErrorNoSys;
    }

    ret = mbedtls_md_setup(&hmacCtx, mdInfo, 1);
    if (ret != 0)
    {
        OTA_TOKEN_DBG_PRINTF("%s, md setup fail\r\n", __FUNCTION__);
        OTA_PubBufFree(base64Decode);
        OTA_PubBufFree(stringForSign);
        base64Decode = NULL;
        stringForSign = NULL;
        return osErrorIO;
    }

    ret = mbedtls_md_hmac_starts(&hmacCtx, (const unsigned char *)base64Decode, strlen(base64Decode));
    if (ret != 0)
    {
        OTA_TOKEN_DBG_PRINTF("%s, md start fail\r\n", __FUNCTION__);
        OTA_PubBufFree(base64Decode);
        OTA_PubBufFree(stringForSign);
        base64Decode = NULL;
        stringForSign = NULL;
        return osErrorIntr;
    }

    ret = mbedtls_md_hmac_update(&hmacCtx, (const unsigned char *)stringForSign, strlen(stringForSign));
    if (ret != 0)
    {
        OTA_TOKEN_DBG_PRINTF("%s, md update fail\r\n", __FUNCTION__);
        OTA_PubBufFree(base64Decode);
        OTA_PubBufFree(stringForSign);
        base64Decode = NULL;
        stringForSign = NULL;
        return osError;
    }

    OTA_PubBufFree(base64Decode);
    OTA_PubBufFree(stringForSign);
    base64Decode = NULL;
    stringForSign = NULL;

    if (NULL == hmacSignBuf)
    {
        hmacSignBuf = osMalloc(hmacSignLen + 1); /* +1 for store '\0' */
        if (NULL == hmacSignBuf)
        {
            OTA_TOKEN_ERR_PRINTF("%s, hmacSignBuf malloc fail\r\n", __FUNCTION__);
            return osErrorTimeout;
        }
    }
    memset(hmacSignBuf, 0, hmacSignLen + 1);

    ret= mbedtls_md_hmac_finish(&hmacCtx, (unsigned char *)hmacSignBuf);

    /* sign gen step4: base64encode */
    olen = 0;
    if (NULL == finalSignBuf)
    {
        finalSignBuf = osMalloc(finalSignLen + 1); /* +1 for store '\0' */
        if (NULL == finalSignBuf)
        {
            OTA_TOKEN_ERR_PRINTF("%s, finalSignBuf malloc fail\r\n", __FUNCTION__);
            OTA_PubBufFree(hmacSignBuf);
            hmacSignBuf = NULL;
            return osErrorResource;
        }
    }
    memset(finalSignBuf, 0, finalSignLen + 1);
    ret = mbedtls_base64_encode((unsigned char *)finalSignBuf, finalSignLen, &olen, (const unsigned char *)hmacSignBuf, methodSignLen);
    OTA_PubBufFree(hmacSignBuf);
    hmacSignBuf = NULL;

    mbedtls_md_free(&hmacCtx);

    //OTA_TokenBufPrint(token, "tokenNoSign");
    tmpToken = token + strlen(token);
    if (strlen(finalSignBuf) >= (ONENET_OTA_TOKEN_AUTH_LEN_MAX - strlen(token)))
    {
        OTA_TOKEN_DBG_PRINTF("%s, finalSignBuf out of range[%d]\r\n", __FUNCTION__, strlen(finalSignBuf));
    }

    /* 5. generate the final sign */
    snprintf(tmpToken, ONENET_OTA_TOKEN_AUTH_LEN_MAX - strlen(token) - 1, "&sign=%s", finalSignBuf);
    //OTA_TokenBufPrint(finalSignBuf, "finalSignBuf");
    OTA_TokenUrlEncode(tmpToken + strlen("&sign="), ONENET_OTA_TOKEN_AUTH_LEN_MAX - strlen(token) - 1);
    OTA_PubBufFree(finalSignBuf);
    finalSignBuf = NULL;
    OTA_TokenBufPrint(token, "token");

    return osOK;
}

#ifdef __cplusplus
}
#endif

