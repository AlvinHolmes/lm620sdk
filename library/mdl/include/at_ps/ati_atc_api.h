#ifndef ATI_ATC_API_H
#define ATI_ATC_API_H
#include "nvm.h"

#ifndef _OS_WIN
	#include "at_api.h"
	// #include "at_channel.h"
    #define at_ctrl_set_smsmode(channelId, SMSReceiveCB)     AT_ChannelSetSMSMode(channelId, (channel_cb)SMSReceiveCB)
    #define at_ctrl_send_rsp_ok     AT_SendResponseOK
    #define at_ctrl_send_rsp_cmeerr AT_SendResponseCMEError
    #define at_ctrl_send_rsp_cmserr AT_SendResponseCMSError
    #define at_ctrl_send_urc(p_urc, urc_len)  AT_SendUnsolicited(0, p_urc, urc_len)
    #define at_ctrl_send_rsp         AT_SendResponse
#else
    #include "psi_api.h"
    #define at_ctrl_set_smsmode(channelId, SMSReceiveCB)
    int8_t at_ctrl_send_rsp_ok(uint8_t channelId);
    int8_t at_ctrl_send_rsp_cmeerr(uint8_t channelId, uint16_t errCode);
    int8_t at_ctrl_send_rsp_cmserr(uint8_t channelId, uint16_t errCode);
    int8_t at_ctrl_send_urc(char *urc, uint16_t urcLen);
    int8_t at_ctrl_send_rsp(uint8_t channelId, char *rsp, uint16_t rspLen);
#endif

void ati_init(void);
void at_common_ok(uint8_t id);
void at_cmltelc_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cmltelc_get(uint8_t id);
void at_cscs_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cscs_get(uint8_t id);
void at_cscs_test(uint8_t id);
void at_cgmi_exec(uint8_t id);
void at_sysinfo_exec(uint8_t id);
#if 0
void at_cmgmr_exec(uint8_t id);
#endif
void at_f_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_f_exec(uint8_t id);

void at_v_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);

void at_s0_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);

void at_s3_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_s3_get(uint8_t id);
void at_s4_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_s4_get(uint8_t id);
void at_s5_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_s5_get(uint8_t id);
void at_boardnum_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_boardnum_get(uint8_t id);
void at_amtnv_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_amtphy_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_amtphy_get(uint8_t id);
void at_ftm_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_prodtest_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_imei_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_imei_get(uint8_t id);
void at_cmset_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cpls_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cpls_get(uint8_t id);
void at_cpls_test(uint8_t id);
void at_cpol_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cpol_get(uint8_t id);
void at_cpol_test(uint8_t id);

void at_cops_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cops_get(uint8_t id);
void at_cops_test(uint8_t id);

void at_cmlteamtband_get(uint8_t id);
void at_cmcalibtime_get(uint8_t id);

void at_cgatt_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cgatt_get(uint8_t id);
void at_cgatt_test(uint8_t id);

void at_cgsn_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cgsn_exec(uint8_t id);
void at_cgsn_test(uint8_t id);
void at_cimi_exec(uint8_t id);
void at_cmfplmnset_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cmfplmnset_get(uint8_t id);
void at_cmfplmnset_test(uint8_t id);
void at_ctzr_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_ctzr_get(uint8_t id);
void at_ctzr_test(uint8_t id);
void at_ceer_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_ceer_get(uint8_t id);
void at_ceer_test(uint8_t id);
void at_ceer_exec(uint8_t id);
void at_cpin_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cpin_get(uint8_t id);
void at_cmurdy_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cmurdy_get(uint8_t id);
void at_cmurdy_test(uint8_t id);
void at_cmuslot_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cmuslot_get(uint8_t id);
void at_cmuslot_test(uint8_t id);
void at_iccid_exec(uint8_t id);
void at_csim_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_crsm_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_ccho_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cchc_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cgla_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cmrap_get(uint8_t id);
void at_cmrap_test(uint8_t id);
void at_ceid_exec(uint8_t id);
void at_cgdcont_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cgdcont_get(uint8_t id);
void at_cgdcont_test(uint8_t id);

void at_cgdscont_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cgdscont_get(uint8_t id);
void at_cgdscont_test(uint8_t id);
void at_cgtft_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cgtft_get(uint8_t id);
void at_cgtft_test(uint8_t id);
void at_cgact_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cgact_get(uint8_t id);
void at_cgact_test(uint8_t id);

void at_cgcmod_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cgcmod_test(uint8_t id);
void at_cgauth_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cgauth_get(uint8_t id);
void at_cgauth_test(uint8_t id);
void at_clck_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_clck_test(uint8_t id);
void at_cpwd_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cpwd_test(uint8_t id);
void at_cemode_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cemode_get(uint8_t id);
void at_cemode_test(uint8_t id);
void at_cereg_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cereg_get(uint8_t id);
void at_cereg_test(uint8_t id);
void at_cgreg_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cgreg_get(uint8_t id);
void at_cgreg_test(uint8_t id);


void at_cgeqos_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cgeqos_get(uint8_t id);
void at_cgeqos_test(uint8_t id);

void at_cgeqosrdp_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cgeqosrdp_test(uint8_t id);
void at_cgeqosrdp_exec(uint8_t id);
void at_cgcontrdp_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cgcontrdp_test(uint8_t id);
void at_cgcontrdp_exec(uint8_t id);
void at_cgscontrdp_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cgscontrdp_test(uint8_t id);
void at_cgscontrdp_exec(uint8_t id);
void at_cgtftrdp_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cgtftrdp_test(uint8_t id);
void at_cgtftrdp_exec(uint8_t id);
void at_cmlteband_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cmlteband_get(uint8_t id);
void at_cmmaxtxpower_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cmmaxtxpower_get(uint8_t id);

void at_cpsms_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cpsms_get(uint8_t id);
void at_cpsms_test(uint8_t id);
void at_cedrxs_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cedrxs_get(uint8_t id);
void at_cedrxs_test(uint8_t id);
void at_cedrxrdp_exec(uint8_t id);
void at_cgpiaf_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cgpiaf_get(uint8_t id);
void at_cgpiaf_test(uint8_t id);
void at_cmncell_exec(uint8_t id);
void at_cesq_exec(uint8_t id);
void at_cesq_test(uint8_t id);
void at_cmsinr_exec(uint8_t id);
void at_csq_exec(uint8_t id);
void at_csq_test(uint8_t id);


void at_cfun_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cfun_get(uint8_t id);
void at_cfun_test(uint8_t id);
void at_cgpaddr_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cgpaddr_test(uint8_t id);
void at_cgpaddr_exec(uint8_t id);

void at_cmgd_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cmgd_test(uint8_t id);
void at_cmmena_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cmgl_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cmgl_test(uint8_t id);
void at_cmgl_exec(uint8_t id);
void at_cmgr_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cmgw_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cmgw_exec(uint8_t id);
void at_cmss_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cnmi_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cnmi_get(uint8_t id);
void at_cnmi_test(uint8_t id);
void at_cpms_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cpms_get(uint8_t id);
void at_cpms_test(uint8_t id);
void at_csca_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_csca_get(uint8_t id);
void at_cmgs_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);


#ifdef TEXT_SMS_SUPPORT
void at_cmgf_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cmgf_get(uint8_t id);
void at_cmgf_test(uint8_t id);
void at_csdh_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_csdh_get(uint8_t id);
void at_csdh_test(uint8_t id);
void at_csmp_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_csmp_get(uint8_t id);


#endif



void at_cusatt_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cusate_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);

void at_cscon_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cscon_get(uint8_t id);
void at_cscon_test(uint8_t id);
#if defined IMS_SUPPORT || defined USE_TOP_PPP
void at_d_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_h_exec(uint8_t id);
#endif
#ifdef IMS_SUPPORT
void at_cmimsbar_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_a_exec(uint8_t id);
void at_chup_exec(uint8_t id);
void at_cvmod_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cvmod_get(uint8_t id);
void at_cvmod_test(uint8_t id);
void at_vts_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_vts_test(uint8_t id);
void at_imscwa_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
#if 0
void at_ccwa_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_ccwa_get(uint8_t id);
void at_ccwa_test(uint8_t id);
#endif
void at_imsusd_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_imsusd_get(uint8_t id);
void at_imsusd_test(uint8_t id);
void at_clcc_exec(uint8_t id);
void at_chld_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_chld_test(uint8_t id);
void at_imsplus_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_imscfc_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_imslck_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_imsd_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_imsref_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_imsipv_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_imsamrw_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_imsamrw_get(uint8_t id);
void at_imslip_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_imslip_get(uint8_t id);
void at_imslip_test(uint8_t id);
void at_imslir_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_imslir_get(uint8_t id);
void at_imslir_test(uint8_t id);
void at_imsolp_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_imsolp_get(uint8_t id);
void at_imsolp_test(uint8_t id);
void at_imsolr_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_imsolr_exec(uint8_t id);
void at_imsapn_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_imsapn_get(uint8_t id);
void at_imsconfuri_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_imsconfuri_get(uint8_t id);
void at_imstest_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cmsmsoin_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cmsmsoin_get(uint8_t id);
void at_imscdu_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_imsstate_get(uint8_t id);

#endif
void at_cnum_exec(uint8_t id);

void at_cmuinit_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_cmuinit_test(uint8_t id);

  void at_cmslot_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
  void at_cmslot_read(uint8_t id);
  void at_cmslot_test(uint8_t id);

  void at_cmepcg_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);

  void at_iqrel_exec(uint8_t id);

  void at_qgdcnt_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
  void at_qgdcnt_read(uint8_t id);
  void at_qgdcnt_test(uint8_t id);

  void at_qaugdcnt_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
  void at_qaugdcnt_read(uint8_t id);
  void at_qaugdcnt_test(uint8_t id);

  void at_itimerlen_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
  void at_itimerlen_test(uint8_t id);

#ifdef _OS_WIN
  int32_t ati_Send(T_AT_CMD *pDataPtr);
  int32_t ati_SendSMS(T_AT_CMD *pDataPtr);
#endif
int8_t ati_send2SmsReq(uint8_t bChannelId, char *arrData, uint16_t arrDataLen);
uint8_t ati_AtCmdAbortReq(uint8_t channelId, const char *name, uint8_t cmd_type);

#ifdef USE_TOP_WIFISCAN
void at_wifiscan_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_wifiscan_exec(uint8_t id);
void at_wifiscan_test(uint8_t id);
void at_wifiscanap_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_wifiscanstop_exec(uint8_t id);
#endif


void at_qcsq_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_qcsq_get(uint8_t id);
void at_qcsq_exec(uint8_t id);
void at_qcsq_test(uint8_t id);  

#ifdef AT_CMDSET_Q
void at_gsn_set(uint8_t id, uint8_t *para_p, uint16_t lenOfPara);
void at_gsn_exec(uint8_t id);
void at_gsn_test(uint8_t id);
#endif

#endif
