/****************************************************************************************************
 * Copyright (c) 2023, Nanjing Innochip Technology Co.,Ltd.
 * 
 * @file        ati_interface.h
 *
 * @brief       This file includes macro definitions and interfaces structures in ATI
 *
 * @revision
 * Date                   Author            Notes
 * 2023-4-20
*****************************************************************************************************/

#ifndef PS_AP_AT_INTERFACE_H
#define PS_AP_AT_INTERFACE_H

/**************************************************************************
 *                       files included                                    *
 **************************************************************************/
#include "ap_ps_interface.h"
/***************************************************************************
 *                      macro                            *
 ***************************************************************************/

/*========================= macro of EMM ===================================*/
/*sv default*/
#define ATI_IMEISV_RESERVE_VALUE                        (uint8_t)0x99
#define ATI_IMEI_DIGIT_LEN                              (uint8_t)15

/* +CTZR report */
#define ATI_MMINFO_DISABLE                              (uint8_t)0
#define ATI_MMINFO_CTZV                                 (uint8_t)1
#define ATI_MMINFO_CTZE                                 (uint8_t)2
#define ATI_MMINFO_CTZEU                                (uint8_t)3

/* +COPS*/
#define ATI_COPSACT_E_UTRAN                             (uint8_t)7
#define ATI_MAX_NAME_LEN                                (uint8_t)64

#define ATI_LONGNETNAME_MAX_LEN                                  16/*operator long name length */
#define ATI_SHORTNETNAME_MAX_LEN                                 8/*operator short name length*/



/* +CPLS设置参数 */
#define ATI_CPLS_UPLMN                                  (uint8_t)0  /* +cpls UPLMN value set */
#define ATI_CPLS_OPLMN                                  (uint8_t)1  /* +cpls OPLMN value set */
#define ATI_CPLS_HPLMN                                  (uint8_t)2  /* +cpls HPLMN value set */

#define ATI_CFUNFUN_MINIFUN                             (uint8_t)0
#define ATI_CFUNFUN_FULLFUN                             (uint8_t)1
#define ATI_CFUNFUN_DISTRSRF                            (uint8_t)2
#define ATI_CFUNFUN_DISRCVRF                            (uint8_t)3
#define ATI_CFUNFUN_DISBOTHRF                           (uint8_t)4
#define ATI_CFUNFUN_CARDPWROFF                          (uint8_t)5


#define ATI_CFUNRST_NOTRST                              (uint8_t)0
#define ATI_CFUNRST_RST                                 (uint8_t)1

#define ATI_MAX_BORDNAME_LEN                            (uint8_t)20   /*max length of SVN*/

/*========================= macro of ESM ===================================*/

#define ATI_SM_MAX_PDPADDR_MASK_LEN                     (uint8_t)32

/* AT cmd type （T_PS_APAT_SmQueryActCid_Req bCmdType）*/
#define ATI_SM_CMD_CGCMOD                               (uint8_t)0
#define ATI_SM_CMD_CGDSCONT                             (uint8_t)3
#define ATI_SM_CMD_CGCONTRDP                            (uint8_t)4
#define ATI_SM_CMD_CGSCONTRDP                           (uint8_t)5
#define ATI_SM_CMD_CGTFTRDP                             (uint8_t)6
#define ATI_SM_CMD_CGEQOSRDP                            (uint8_t)7

/* PDP context state */
#define ATI_SM_PDP_DEACTIVATE                           (uint8_t)0
#define ATI_SM_PDP_ACTIVATE                             (uint8_t)1

/* existing cid or not in cmd（T_PS_APAT_SmSet_Req bOnlyCidFg） */
#define ATI_SM_CMD_NORMAL                               (uint8_t)0
#define ATI_SM_CMD_ONLY_CID                             (uint8_t)1
#define ATI_SM_CMD_NO_CID                               (uint8_t)2


/* <direction>  */
#define ATI_TFTDIR_PRE_R7_TFT                           (uint8_t)0
#define ATI_TFTDIR_UPLINK                               (uint8_t)1
#define ATI_TFTDIR_DOWNLINK                             (uint8_t)2
#define ATI_TFTDIR_BIRECTIONAL                          (uint8_t)3

#define ATI_IPADDR_COMPRESS_ENABLE                      (uint8_t)1
#define ATI_IPADDR_COMPRESS_DISABLE                     (uint8_t)0

/*=================================================================================
                                       SMS 部分
=================================================================================*/
/* SMS mode */
#define ATI_CMGF_PDU                                    (uint8_t)0    /* PDU mode */
#define ATI_CMGF_TEXT                                   (uint8_t)1    /* TEXT mode */

#define ATI_MAX_SMSTPDU_LEN                             (uint8_t)164
#define ATI_MAX_SMSTEXT_LEN                             (uint8_t)140
#define ATI_MAX_SMSTEXTCHAR_LEN                         (uint8_t)160

#define ATI_SMSADDRESS_MAXLEN                           (uint8_t)10  /* SMS address length */
#define ATI_SCAADDRESS_MAXLEN                           (uint8_t)11  /* SCA address length */

#define ATI_ADDRTYPE_NORM                               (uint8_t)0
#define ATI_ADDRTYPE_SCA                                (uint8_t)1

#define ATI_SMS_TPVP_RELATIVE_LEN                       (uint8_t)1
#define ATI_SMS_TPVP_ABSOLUTEANDENHANCE_LEN             (uint8_t)7

/**************inputSmsType***************/
#define ATI_CMGS_PDU                                    (uint8_t)0
#define ATI_CMGS_TEXT                                   (uint8_t)1
#define ATI_CMGW_PDU                                    (uint8_t)2
#define ATI_CMGW_TEXT                                   (uint8_t)3
#define ATI_NO_SMS_INPUT                                (uint8_t)255

/**************CSCS format***************/
#define ATI_CSCS_GSM                                    (uint8_t)0 /* 7-bit char（00-7F），ASCII char for output   */
#define ATI_CSCS_HEX                                    (uint8_t)1 /* 8-bit char（00-FF），HEX    number for output  */
#define ATI_CSCS_IRA                                    (uint8_t)2 /*International Reference Alphabet */
#define ATI_CSCS_UCS2                                   (uint8_t)3 /* 16-bitUNICODE（0000-FFFF），HEX    number for output */

/* parameter <mode> of +CNMI Command.*/
#define ATI_CNMI_MODE_BUFFER_ALL                        (uint8_t)0    /* buffer all URC */
#define ATI_CNMI_MODE_DISCARD_ONLINE                    (uint8_t)1    /* discard URC */
#define ATI_CNMI_MODE_BUFFER_ONLINE                     (uint8_t)2    /* buffer URC */
#define ATI_CNMI_MODE_FORWARD_ALL                       (uint8_t)3    /* forword URC */

/* parameter <bm> of +CNMI Command.*/
#define ATI_CNMIBM_NOROUTE                              (uint8_t)0
#define ATI_CNMIBM_STOREIND                             (uint8_t)1
#define ATI_CNMIBM_DIRECTROUTE                          (uint8_t)2
#define ATI_CNMIBM_CLASS3ROUTE                          (uint8_t)3

/**************CNMI BFR*************/
#define ATI_CNMI_BFR_FLUSH_ALL                          (uint8_t)0    /* buffered URC is forword */
#define ATI_CNMI_BFR_CLEAN_ALL                          (uint8_t)1    /* clear all URC */
#define ATI_CNMI_BFR_INVALID                            (uint8_t)255

/* parameter <mem> of +CPMS Command.*/
#define ATI_CPMSMEM_SR                                  (uint8_t)0    /* "SR" for status report storage.*/
#define ATI_CPMSMEM_USIM                                (uint8_t)1    /* "SM" for (U)SIM message storage.*/
#define ATI_CPMSMEM_ERR                                 (uint8_t)0xff

/* parameter <mode> of +CMGF Command.*/
#define ATI_CMGFMODE_PDU                                (uint8_t)0
#define ATI_CMGFMODE_TEXT                               (uint8_t)1

/* parameter <show> of +CSDH Command.*/
#define ATI_CSDH_NOSHOW                                 (uint8_t)0
#define ATI_CSDH_SHOW                                   (uint8_t)1

/* TpVpFormat of TpFo */
#define ATI_TPFO_VPFORMAT_NONE                          (uint8_t)0
#define ATI_TPFO_VPFORMAT_ENCH                          (uint8_t)1
#define ATI_TPFO_VPFORMAT_RELA                          (uint8_t)2
#define ATI_TPFO_VPFORMAT_ABSL                          (uint8_t)3

/* default type of address */
#define ATI_TOA_PLUS                                    (uint8_t)145
#define ATI_TOA_NOPLUS                                  (uint8_t)129

/*=================================================================================
                                       UICC
 =================================================================================*/
#define ATI_MAX_PIN_LEN                                 (uint8_t)8
#define ATI_MAX_STR_LEN                                 (uint8_t)64  /*APN auth max length*/

#define ATI_APDU_REQ_MAXLEN                             (uint16_t)261
#define ATI_APDU_CNF_MAXLEN                             (uint16_t)258

/**************+CLCK/+CPWD 参数fac***************/
#define ATI_SS_FACILITY_TYPE_MAX_NUM 22 /* SS used network facility type number */

/*=================================================================================
                                       other
 =================================================================================*/
/**************CEER format***************/
#define ATI_CEER_TEXT                                            0
#define ATI_CEER_NUM                                             1

/**************CEER module*************/
#define ATI_CEER_CC                                              0
#define ATI_CEER_SM                                              1
#define ATI_CEER_ESM                                             2
#define ATI_CEER_EMM                                             3
#define ATI_CEER_MM                                              4

/*===================================PSM, edrx=============================*/
//psm
#define ATI_PSM_MODE_DISABLE                                 (uint8_t)0
#define ATI_PSM_MODE_ENABLE                                  (uint8_t)1
#define ATI_PSM_MODE_RESET                                   (uint8_t)2
#define ATI_PSM_MODE_FORCE_PSM                               (uint8_t)3

//edrx
#define ATI_EDRX_MODE_0                                      (uint8_t)0
#define ATI_EDRX_MODE_1                                      (uint8_t)1
#define ATI_EDRX_MODE_2                                      (uint8_t)2
#define ATI_EDRX_MODE_3                                      (uint8_t)3

#define ATI_EDRX_ACT_WB                                      (uint8_t)4

//Cereg <SubAct>
#define TDD_SUBACT               (uint8_t)0
#define FDD_SUBACT               (uint8_t)1

/************************************************************************************
 *                      interface structure                              *
 ************************************************************************************/
/*=================================================================================
                        common part
 =================================================================================*/

/*=================================================================================
                                   MM
 =================================================================================*/


/****************************************************************************
  Primitive:APAT_CopsSetReq_Ev(ATI->APAT),
  Function:auto / manual net searching, denial regist,
  SET+COPS param: format,manual to auto if fail
  AT CMD:+COPS=[<mode>[,<format>,<oper>[,<AcT>]]]
 ***************************************************************************/
typedef struct {
    uint8_t                                         bChannelId;
    uint8_t                                         bMode;     /*
                                                            * AT+COPS mode
                                                            * COPS_MODE_AUTO (Default value，ATI fill)   auto net searching
                                                            * COPS_MODE_MANU                  manual net searching
                                                            * COPS_MODE_DEREG                 denial regist
                                                            * COPS_MODE_SETONLY               only SET<format>
                                                            * COPS_MODE_NANUAUTO              manual net searching，to auto if fail
                                                            */
    uint8_t                                         bFormat;   /* operator name format (Default value:0，ATI fill)
                                                               value in  APAT_OPERFORMAT_LONGALPHA etc.*/
    uint8_t                                         bActFg;
    uint8_t                                         bAcT;

    uint8_t                                         bOperFg;  /*
                                                             * APAT_INVALID
                                                             * APAT_VALID
                                                             */
    S_Ati_NetName                               tOper;     /* operator name，value in S_Ati_NetName */
}S_Ati_CopsSet_Req;


/****************************************************************************
  Primitive:APAT_CpolQueryCnf_Ev(APAT->ATI)
  Function:return current UPLMN list
  AT CMD:+CPOL read command  Result code
 ***************************************************************************/

typedef struct {
    uint8_t                                         bNumber;                           /* Selector plmn number*/
    S_Usim_Plmn_Act                                 atPlmnSelector[PLMN_LIST_MAX_NUM]; /* PlmnSelector */
}S_Ati_CpolQuery;

/*=================================================================================
                                   SM
 =================================================================================*/

/*dedicated PDP context type*/
typedef struct
{
    uint8_t                                        bCid;               /*  SET command must be */
    uint8_t                                        abPadding[2];

    uint8_t                                        bPrimaryCidFg;
    uint8_t                                        bPrimaryCid;        /* primary PDP contextcid */
    uint8_t                                        bDComp;             /* same as S_Ati_CidPdpContext */
    uint8_t                                        bHComp;
    uint8_t                                        bImcnSignInd;
}S_Ati_CidSecPdpContext;

/****************************************************************************
  Primitive:APAT_SmSetParamReq_Ev(ATI->APAT)
  Function: SET command，SET related param to NV
       APAT_CommonCnf_Ev return setting if OK or not
  AT CMD:+CGDCONT=、+CGDSCONT=、+CGTFT=、S0=
 ****************************************************************************/
typedef struct
{
    uint8_t                                         bSrcIndex;
    uint8_t                                         bCidFg;        /*
                                                                     * ATI_SM_CMD_ONLY_CID
                                                                     * ATI_SM_CMD_NORMAL
                                                                     * ATI_SM_CMD_NO_CID
                                                                     */
    union
    {
        S_Ati_CidPdpContext              tPdpContext;       /* PDP context，bSetType=APAT_SM_OPTYPE_CGDCONT existing */
        S_Ati_CidSecPdpContext           tSecPdpContext;    /* second PDP context，bSetType=APAT_SM_OPTYPE_CGDSCONT existing */
        S_AtiEsm_SetCidFilter             tFilter;           /* filter bSetType=APAT_SM_OPTYPE_CGTFT existing */
    }tVal;
}S_Ati_SmSetParam_Req;

#endif
