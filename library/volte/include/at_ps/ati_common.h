/****************************************************************************************************
 * Copyright (c) 2023, Nanjing Innochip Technology Co.,Ltd.
 * 
 * @file        ati_common.h
 *
 * @brief       Headfile of Ati Common function and struct
 *
 * @revision
 * Date                   Author            Notes
 * 2023-4-20
*****************************************************************************************************/

#ifndef ATI_COMMON_H
#define ATI_COMMON_H
#include <os.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "at_api.h"
#include "ati_interface.h"
#include "ap_ps_interface.h"
#include "usim_data.h"
#include "at_parser.h"

/**************************************************************************
 *                        macro definitions                              *
 **************************************************************************/
#define  AT_UPCASE( c ) ( ((c) >= 'a' && (c) <= 'z') ? ((c) - 0x20) : (c) )
#define  AT_BCD_TO_DEMICAL(a)  (((a) >> 4) + ((a) & 0x0F) * 10)

#define AT_DEC_STR_MAX_LEN_16B                             10
#define AT_HEX_STR_MAX_LEN_16B                             8

#define AT_DECIMAL_CARRY                                   10
#define AT_HEX_CARRY                                       16

#define AP_DEC_NUMSTR_BCD_MAX_LEN                          160

/* Used in at_Str2Value() */
#define AT_IS_DECIMAL_PARAM                                TRUE
#define AT_IS_HEX_PARAM                                    FALSE

#define AT_PARSER_FORMAT_BUFLEN                            100

/**************************************************************************
 *                        ATI internal data structure                               *
 **************************************************************************/
typedef struct {
    /*needed*/
    uint8_t                          bCplsList   : 2;
    uint8_t                          bCpolFormat : 2;
    uint8_t                          bCurCscsTyp : 2;   /* Used to keep the setting of +CSCS, <chset> */
    uint8_t                          bCmgfMode   : 1;
    uint8_t                          bCsdhShow   : 1;
    uint8_t                          bMsgStatVal;
    uint8_t                          bCscon;
    uint8_t                          bCurAtChId;
    uint8_t                          inputSmsType;
    uint8_t                          bTpduLen;
    S_AtiSms_TpAddr                  tTpAddr;
    uint8_t                          bImsiExsit;//IMSI是否已保存   . R_INVALID:未保存  ; R_VALID:已保存
    S_IMSI                           tImsi;
}S_Ati_Data_Ex;


#ifdef _USE_PSM

/*PSM ATI store data*/
typedef struct
{
    uint8_t                          bCplsList   : 2;
    uint8_t                          bCpolFormat : 2;
    uint8_t                          bCurCscsTyp : 2;   /* Used to keep the setting of +CSCS, <chset> */
    uint8_t                          bCmgfMode   : 1;      /* 0 pdu mode 1 text mode */
    uint8_t                          bCsdhShow   : 1;
    uint8_t                          bCurAtChId;
    uint8_t                          bCscon;
    uint8_t                          bImsiExsit;
    S_IMSI                           tImsi;
}S_Ati_backupData;
#endif

typedef struct ati_ip6_addr
{
    uint32_t addr[4];
} S_Ati_ip6_addr;

/**************************************************************************
 *                        Global viriables                         *
 **************************************************************************/

extern S_Ati_Data_Ex g_ati_atDataEx;
extern const uint8_t g_Ati_Ascii2Gsm7bitTable[128];

/**************************************************************************
 *                        internal function declare                     *
 **************************************************************************/

/*   FUNCTION-DESCRIPTION
**
** DESCRIPTION:    This procedure apply memory and initialize to 0
**
** PARAMETERS:     size:     sized to apply
**
** RETURN VALUES:  no
*/
void * ati_Malloc(uint32_t size);
/*   FUNCTION-DESCRIPTION
**
** DESCRIPTION:    This procedure free memory and initialize to 0
**
** PARAMETERS:     ptr:   point to memory to apply
**
** RETURN VALUES:  no
*/
void ati_Free(void** ptr);

/*   FUNCTION-DESCRIPTION
**
** DESCRIPTION:    This procedure checks TPADDRESS
**
** PARAMETERS:     bLen:   length
**
** RETURN VALUES:  no
*/
void ati_TpaddrToSet(uint8_t* pSetAddr,S_AtiSms_TpAddr* pTpaddr,uint8_t bAddrType);
void ati_SetToTpaddr(S_AtiSms_TpAddr* pTpaddr,uint8_t* pSetAddr,uint8_t bAddrType);
uint16_t ati_EncTpAddrVal(uint8_t bCommaFg,char * pStrTarget, uint8_t bTpType, uint8_t bAddrType, S_AtiSms_TpAddr *ptTpAddr);
uint8_t ati_CharToHex(uint8_t hex);

uint16_t ati_EncTpAddr (uint8_t bCommaFg, char * pStrTarget, uint8_t bTpType, S_AtiSms_TpAddr *ptTpAddr);
uint16_t ati_EncScAddr (uint8_t bCommaFg, char * pStrTarget, uint8_t bTpType, S_AtiSms_TpAddr *ptTpAddr);
void ati_DecSetTpFo(uint8_t bFoTmp, S_AtiSms_TpFo   *ptTpFo );
bool_t ati_EncDate2StrByVpFormat( char * pStrTarget, S_AtiSms_TpVp tpValidPeriod, uint8_t iSmsVpFormat);

uint8_t ati_MsgForward(uint16_t wMsgId, uint8_t* pMsg, uint16_t wMsgLen, uint8_t bDestThreadIdx);
uint8_t ati_MsgSend(uint8_t bChannelId,uint16_t wMsgId, uint8_t* pMsg, uint16_t wMsgLen, uint8_t bDestThreadIdx);

/**************************************************************************
* FUNCTION-DESCRIPTION： at_DecNumValid
** DESCRIPTION： This procedure judges valid of number and output the nmuber
** PARAMETERS：  pStrNum:     store source ACSII string address
**              iStrNumLen:  length of ACSII string
**              pValidArr:   output BCD code address
** RETURN VALUES：length of output，-1: source char error
**/
bool_t ati_DecNumValid(uint8_t *pValidArr, char *pStrNum, uint16_t *iStrNumLen);

/**************************************************************************
* FUNCTION-DESCRIPTION： ati_DecideNumTypeValid
** DESCRIPTION： This procedure parses memory type according to SMS string
** PARAMETERS：iType:   input type of number
** RETURN VALUES： TRUE/FALSE
*/
bool_t ati_DecideNumTypeValid(uint8_t iType);

/**************************************************************************
* FUNCTION-DESCRIPTION： at_DecNumStr2BcdByCscsType
** DESCRIPTION： This procedure changes number string to target BCD array(semi-BYTE)
**               according to CSCS
** PARAMETERS：  pNumArr:          target BCD address
**              iMaxNumLen:       max length of target array, semi-BYTE number
**              pNumStr:          source number string pointer
**              iStrLen:          source number string length
**              cscsType:         current CSCS type
*            (OUT):无
** RETURN VALUES： number of semi-BYTE，-1:source error，0:input param error
** NOTE:   current support GSM,HEX,UCS2
*/
int8_t ati_DecNumStr2BcdByCscsType(uint8_t *pNumArr, uint32_t iMaxNumLen,
                                      char *pNumStr, uint16_t iStrLen, uint8_t cscsType);


/* FUNCTION-DESCRIPTION： ati_DecSmsTextStr
** DESCRIPTION： This procedure parses SMS with Text format
** PARAMETERS：pStrContent:     PDU to be parsed
**             iContentLen:    length of left PDU to be parsed
**             bPduLen:        pdu length
**             pbPduBuf:       text buffer
** RETURN VALUES： TRUE/FALSE
** NOTE： in PDU mode, +CMGC,+CMGS,+CMGW parse
*/
extern uint16_t ati_DecSmsPduStr(char *pStrContent, uint16_t iContentLen, uint8_t bPduLen, uint8_t *pbPduBuf);

#ifdef TEXT_SMS_SUPPORT

uint8_t at_DecSmsTextStr(uint8_t **ppTgtText, uint8_t *pTextLen, uint8_t *pbCharNum,
                        char * pSrcTextStr, uint16_t iSrcTextLen, uint8_t cscsType);

#endif


void ati_HexToStr(uint16_t usLength, char *pOutData, uint8_t *pInputData);
uint8_t ati_StrToHex(uint8_t *pbDest, char *pbSrc, uint16_t wLen);

/*FUNCTION-DESCRIPTION???? at_Str2Value
** DESCRIPTION???? This procedure changes number string to value of DEC or HEX
** PARAMETERS????  sStr: string to be changed
**              DorH: TRUE????DEC;  FALSE????HEX
**              iValue: output value
** RETURN VALUES???? TRUE/FALSE
*/
uint8_t ati_Str2Value( char *sStr , bool_t DorH, uint32_t * iValue );

/* FUNCTION-DESCRIPTION： ati_BinStr2Byte
** DESCRIPTION： This procedure changes number string to value of BIN
** PARAMETERS：sStr: string to be changed
**             pValue: output value
** RETURN VALUES：  TRUE /FALSE
*/
bool_t ati_BinStr2Byte(char *sStr , uint8_t *pValue);

uint8_t ati_UpDateIccid(uint8_t* ptSrcIccid, uint8_t bSrcIccidLen, uint8_t* pDesIccid);
extern uint8_t at_Is7BitCode(uint8_t bTpDcs);
int16_t ati_GetOperNameData( char * pStrTgt, uint8_t * pDataSrc, uint32_t iSrcByteLen,
                          uint8_t dcs, uint8_t sparenum);

extern uint8_t at_CheakSmsDcs(uint8_t bDcs);
extern uint16_t at_EncTpAddrVal(uint8_t bCommaFg,char * pStrTarget, uint8_t bTpType, uint8_t bAddrType, S_AtiSms_TpAddr *ptTpAddr);
extern uint8_t at_EncSmsTextData( char **ppStrTgt, uint8_t *pDataSrc, uint32_t iSrcByteLen, uint8_t iCharNum,
                          uint8_t dcs, uint8_t tpudHeadInd, uint8_t curCscsType );
extern uint8_t ati_IsPlmnIdValid(S_PS_PlmnId* ptPlmnId);
extern bool_t ati_DecDotSeparateStr(char *pDotStr, void *pDotVal, uint8_t *pSectNum, uint8_t iMaxSectNum, uint32_t iMaxVal);
uint8_t Ati_ParseSmsMemTyp(char * sMem);
void ati_EncDotSeparateStr(char** ppStrWalk, uint8_t bSectNum, uint8_t *pbSectNum);
void ati_EncIp6AddrStr(char** ppStrWalk,S_Ati_ip6_addr *addr, char *buf, int buflen);

at_errno_t ati_parser_cmd_end(const char *param_string, uint32_t size, char *format, uint8_t *pbIsEnd);
at_errno_t ati_parser_opt_is_empty(const char *param_string, uint32_t size, const char *format, uint8_t *pbIsEnd);
at_errno_t ati_parser_opt_param(const char *param_string, uint32_t size,const char *format, uint8_t *pOut, uint8_t *pbOptFg, uint8_t *pbIsEnd);
at_errno_t ati_parser_param_end(const char *param_string, uint32_t size,const char *format, uint8_t *pOut, uint8_t *pbOptFg, uint8_t *pbIsEnd);
at_errno_t ati_parser_mand_param(const char *param_string, uint32_t size,const char *format, uint8_t *pOut, uint8_t *pbIsEnd);
at_errno_t ati_parser_parse_cmd(const char *param_string, uint32_t size, uint8_t *pbIsEnd, const char *format, ...);
uint8_t ati_CheckNoParamAfter(const char *param_string,uint8_t bLastIndex);
at_errno_t ati_parser_opt_cid(const char *param_string, uint32_t size, uint8_t *pbCid, uint8_t *pbCidFg,uint8_t *pbIsEnd);
void ati_parser_create_format(char *pWholeFormat,uint8_t bFormatLen, uint8_t bSkipNum, char *pCurFormat);
int8_t at_ctrl_send_rsp_with_ok(uint8_t channelId, char *rsp, uint16_t rspLen);
int8_t at_ctrl_send_rsp_error(uint8_t channelId);
void ati_EncImeiStr(char** ppStrWalk, uint8_t *pbImei, uint8_t bImeiLen, uint8_t bIsImeiSv);
uint8_t ati_MsgSend_ToPhy(uint8_t bChannelId,uint16_t wMsgId, uint8_t* pMsg, uint16_t wMsgLen);
#ifdef USE_TOP_WIFISCAN
void Ati_WifiScanInd(uint8_t *pbMsgBuf);
#endif
void ati_EncBcd2NumStrByCscsType ( char *pStrTarget, uint8_t *pSource, uint8_t iSrcByteLen, uint8_t curCscsType );
#ifdef IMS_SUPPORT
uint8_t ati_MsgSendToIms(uint8_t bChannelId,uint16_t wMsgId, uint8_t* pMsg, uint16_t wMsgLen);
uint8_t ati_MsgSendToIms2(uint8_t bChannelId,uint16_t wMsgId, uint8_t* pMsg, uint16_t wMsgLen);
uint8_t ati_MsgSendToIms2Ps(uint8_t bChannelId,uint16_t wMsgId, uint8_t* pMsg, uint16_t wMsgLen);
#endif
uint8_t ati_CheckBand(uint8_t bAmtband, uint8_t bUserBand);

#endif
