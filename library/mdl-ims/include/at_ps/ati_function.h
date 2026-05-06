/****************************************************************************************************
 * Copyright (c) 2023, Nanjing Innochip Technology Co.,Ltd.
 * 
 * @file        ati_function.h
 *
 * @brief       This file includes function definitions of AT command
 *
 * @revision
 * Date                   Author            Notes
 * 2023-4-20
*****************************************************************************************************/

#ifndef ATI_FNC_H
#define ATI_FNC_H

#ifndef _OS_LINUX
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#endif
#include <os.h>
#include "ati_atc_api.h"
#include "ati_interface.h"
#include "nvparam_ps.h"
#include "ps_com.h"
#include "ati_common.h"
#include "at_api.h"
#include "sys_func_atcfg.h"

#define AT_SMS_STATE_MAX                                   (uint8_t)8    /*max length of SMS*/

#ifdef TEXT_SMS_SUPPORT
extern const char *AT_SMS_STATE_TYPE[AT_SMS_STATE_MAX];

typedef struct
{
    S_AtiSms_TpAddr         tSca;
    S_AtiSms_TpAddr         tOa;
    uint8_t                 bPid;
    uint8_t                 bDcs;
    uint8_t                 abScts[7];
    uint8_t                 bCharNum;
    char                   *pbTextBuf;
}S_Ati_TextMsgDecodeDeliver;

typedef struct
{
    S_AtiSms_TpAddr         tSca;
    S_AtiSms_TpAddr         tDa;
    uint8_t                 bPid;
    uint8_t                 bDcs;
    uint8_t                 abVp[7];
    uint8_t                 bCharNum;
    char                    *pbTextBuf;
}S_Ati_TextMsgDecodeSubmit;

typedef struct
{
    S_AtiSms_TpAddr         tRa;
    uint8_t                 bMr;
    uint8_t                 abScts[7];
    uint8_t                 abDt[7];
    uint8_t                 bSt;
}S_Ati_TextMsgDecodeStatusReport;

typedef struct
{
    S_AtiSms_TpFo           tFo;
    union{
        S_Ati_TextMsgDecodeDeliver         tDeliver;
        S_Ati_TextMsgDecodeSubmit          tSubmit;
        S_Ati_TextMsgDecodeStatusReport    tStatRpt;
    }Val;
}S_Ati_TextMsgDecode;

#endif


void ati_ResetTimerExpir_cb(void* parameter);
uint8_t ati_PsDecOperNam(uint8_t *dstAbName, uint8_t *oriAbName, uint8_t *bLen, uint8_t bFormat);

void ati_CopsSet(S_Ati_CopsSet_Req *ptCopsSet);

void ati_SysinfoGetEpsSrvStatus(uint8_t* pbSrvStatus);
void ati_SysinfoGetEpsSysmode(uint8_t* pbSysMode);
void ati_SysinfoGetEpsSubmode(uint8_t* pbSubMode, uint8_t* pbSubModeFg);
void ati_SysinfoGetRoamStat(uint8_t* pbRoamStatus);
uint16_t ati_SysinfoGetSimStat(uint8_t* pbCardState);
uint16_t ati_CeerReq(uint8_t bChannelId);
uint8_t ati_UpDateIccid(uint8_t* ptSrcIccid, uint8_t bSrcIccidLen, uint8_t* pDesIccid);
uint8_t ati_CheckPriPdpCont (S_Ati_CidPdpContext *ptPdpCont);
bool_t at_IsLastNvCidPdpCont(uint8_t bCid);
bool_t at_IsLastNvCidPdpCont(uint8_t bCid);
void at_SmDefinePdpContInNv(S_Ati_CidPdpContext *ptPdpCont);
void at_SmUndefineNvPdpCont(uint8_t bCid);

uint16_t ati_SmQueryPdpCont(char* at_rsp);
uint16_t ati_SmSetSecPdpCont(const S_Ati_SmSetParam_Req* ptSmSetReq);

uint8_t ati_DecSmType(char* pPdpType);
void ati_ActCidQueryReq(uint8_t bChannelId,uint8_t bCmdType);

void ati_CgpaddrSetReq(uint8_t bChannelId,uint8_t bCidNum,uint8_t* abCid);

void ati_CgeqosrdpSet(uint8_t bChannelId, uint8_t bCidFg, uint8_t bCid);
void ati_CgcontrdpSet(uint8_t bChannelId, uint8_t bCidFg, uint8_t bCid);
void ati_CgtftrdpSet(uint8_t bChannelId, uint8_t bCidFg, uint8_t bCid);

uint8_t ati_CpsmsSetNV(uint8_t bChannelId, uint8_t bMode, uint8_t bRauTimer, uint8_t bGprsTimer,uint8_t bTauTimer,uint8_t bTauflg,uint8_t bActTimer,uint8_t bActflg);

uint8_t ati_CedrxsSetNV(uint8_t bChannelId, uint8_t bMode, uint8_t bModeFlg,uint8_t bAct,uint8_t bActFlg,uint8_t bEdrxValue,uint8_t bEdrxValueFlg);
uint8_t ati_CgpiaGet(void);

void ati_CmgdSetTest(uint8_t bChannelId,uint8_t bCmdType,uint16_t wIndex,uint8_t bDelflag);
void ati_CmglSet(uint8_t bChannelId,uint8_t bState);
void ati_CpmsSet(S_AtiSms_CpmsReq* ptCpmsReq);
void ati_CmsetQueryCnfContent(char ** ppStrWalk, CommAtCfgCmdParam* pCmsetParamCnf);

#ifdef TEXT_SMS_SUPPORT
uint8_t ati_TextMsgDecode(uint8_t bPduLen, uint8_t *pbPduBuf,uint8_t bIsStatInd,S_Ati_TextMsgDecode *ptText);
#endif
void ati_sendCmgsReq(uint8_t smsType, uint8_t bChannelId, char *arrData, uint16_t arrDataLen);
void ati_sendCmgwReq(uint8_t smsType, uint8_t bChannelId, char *arrData, uint16_t arrDataLen);

void ati_AmtNvReqProc(uint8_t bChannelId, uint8_t cmdType, uint32_t addr, uint32_t len, uint8_t* pData );
void ati_AmtPhyReqProc(uint8_t bChannelId,uint32_t len, uint16_t* pData );

void ati_FtmProc(uint8_t bChannelId, uint32_t flag, uint32_t msgid, uint32_t len, uint8_t* pData );

void EsmApiGetSecPdpCont(S_AtiEsm_SecPdpContext *patSecPdpContext);
bool_t ati_SmGetPdpAddrByCid(uint8_t iCid, S_Ati_SmIpDnsInfo* pAddr);
uint8_t ati_CheckPdpType(uint8_t bPdpType);
extern char *ati_ip6addr_ntoa_r(const S_Ati_ip6_addr *addr, char *buf, int buflen);
extern void esm_ReleasePcoInfo(S_Esm_PcoInfo *pCoInfo);
extern CommAtCfgCmdParam dev_AtCfgGet(char* cmd);
extern CommAtCfgCmdParam dev_AtCfgSet(char* cmd, CommAtCfgCmdParam *cmdParm);

uint8_t ati_GetOperPlmnIdByName(uint8_t* ptAlphName, uint8_t bLen, uint8_t bFormat, S_PS_PlmnId *pPlmnid);
void ati_GetOperPlmnScale(uint8_t* ptAlphName, uint8_t bLen, uint8_t bFormat, uint8_t *pNVStartIdx, uint8_t *pNVEndIdx);
uint8_t ati_GetOperName(uint8_t bFlag,S_PS_PlmnId* pNumName, void* ptAlphName);
void ati_SavePlmnName(S_PS_PlmnId *ptCurPlmnId, char* pSrcStr, uint8_t bIsLongName);
void ati_get_numeric_oper_str(char *pRsp, S_PS_PlmnId *ptPlmn);
void ati_SmSetFilter(const S_Ati_SmSetParam_Req* ptSmSetReq);
void ati_SmQueryFilter(char* at_rsp);
void ati_CgscontrdpSet(uint8_t bChannelId, uint8_t bCidFg, uint8_t bCid);
uint8_t ati_SetTimerLen(uint8_t bTimer, uint32_t dwValue);
uint32_t ati_GetTimerLen(uint8_t bTimer);

#ifdef IMS_SUPPORT
void Ati_ImsCgdcontReq(uint8_t bChannelId, uint8_t *pbMsgBuf);
void Ati_ImsGetPlmnNameReq(uint8_t bChannelId, uint8_t *pbMsgBuf);
void Ati_ImsBarQryReq(uint8_t bChannelId, uint8_t *pbMsgBuf);
#endif
#endif
