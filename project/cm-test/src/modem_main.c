/**
 * @file example_main.c
 * @brief Mobile接口测试
 * @date 2025-12-26
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "cm_os.h"
#include "cm_sys.h"
#include "cm_pm.h"
#include "cm_sim.h"
#include "cm_modem.h"
#include "cm_mem.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define CM_TEST_VER_LEN    65
#define CM_TEST_IMEI_LEN   16
#define CM_TEST_SN_LEN     65
#define CM_TEST_IMSI_LEN   16
#define CM_TEST_ICCID_LEN  21

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(void)
{
    /* 避免睡眠模式导致输出异常 */
    cm_pm_cfg_t pm_cfg;
    pm_cfg.cb_enter = NULL;
    pm_cfg.cb_exit = NULL;
    cm_pm_init(pm_cfg);
    cm_pm_work_lock();

    /* 延迟一会让 cp 核初始化完 */
    osDelay(3000);

    int cm_get_pin = cm_modem_get_cpin();
    cm_printf("cm_get_pin:%d\n", cm_get_pin);

    char cgmr[32] = {0};
    cm_modem_get_cgmr(cgmr);
    cm_printf("cgmr:%s\n", cgmr);

    char cgmm[10] = {0};
    cm_modem_get_cgmm(cgmm);
    cm_printf("cgmm:%s\n", cgmm);

    char cgmi[10] = {0};
    cm_modem_get_cgmi(cgmi);
    cm_printf("cgmi:%s\n", cgmi);

    cm_cops_info_t *cops = NULL;
    cops = cm_malloc(sizeof(cm_cops_info_t));
    memset(cops, 0, sizeof(cm_cops_info_t));
    cm_modem_get_cops(cops);
    cm_printf("cops->mode:%d\n", cops->mode);
    if (cops->mode) {
        cm_printf("cops->act:%d cops->format:%d, cops->oper:%s\n", cops->act, cops->format, cops->oper);
    }
    cm_free(cops);

    char rssi[10] = {0};
    char ber[10] = {0};
    cm_modem_get_csq(rssi, ber);
    cm_printf("rssi:%s,ber:%s\n", rssi, ber);

    cm_radio_info_t *radio_info = NULL;
    radio_info = cm_malloc(sizeof(cm_radio_info_t));
    memset(radio_info, 0, sizeof(cm_radio_info_t));
    cm_modem_get_radio_info(radio_info);
    cm_printf("radio_info: rat=%d, rsrp=%d, rsrq=%d, rssi=%d\n",
              radio_info->rat, radio_info->rsrp, radio_info->rsrq, radio_info->rssi);
    cm_printf("radio_info: rxlev=%d, tx_power=%d, tx_time=%u, rx_time=%u\n",
              radio_info->rxlev, radio_info->tx_power,
              (uint32_t)radio_info->tx_time, (uint32_t)radio_info->rx_time);
    cm_printf("radio_info: last_cellid=%u, last_earfcn=%u, last_ecl=%u\n",
              radio_info->last_cellid, radio_info->last_earfcn, radio_info->last_ecl);
    cm_printf("radio_info: last_pci=%u, last_snr=%d\n",
              radio_info->last_pci, radio_info->last_snr);
    cm_free(radio_info);

    cm_cell_info_t cell_info[1];
    cm_modem_get_cell_info(cell_info, 1);
    cm_printf("cell_info[0]: primary_cell=%d, mcc=%s, mnc=%s\n",
              cell_info[0].primary_cell, cell_info[0].mcc, cell_info[0].mnc);
    cm_printf("cell_info[0]: earfcn=%u, earfcn_offset=%u, tac=%u, pci=%u\n",
              cell_info[0].earfcn, cell_info[0].earfcn_offset,
              cell_info[0].tac, cell_info[0].pci);
    cm_printf("cell_info[0]: rsrp=%u, rsrq=%u, rssi=%u, snr=%d\n",
              cell_info[0].rsrp, cell_info[0].rsrq, cell_info[0].rssi, cell_info[0].snr);
    cm_printf("cell_info[0]: bandwidth=%u, cid=%.8X\n",
              cell_info[0].bandwidth, cell_info[0].cid);

    int get_fun = cm_modem_get_cfun();
    cm_printf("get_fun:%d\n", get_fun);

    cm_psm_cfg_t *psm_cfg = NULL;
    psm_cfg = cm_malloc(sizeof(cm_psm_cfg_t));
    memset(psm_cfg, 0, sizeof(cm_psm_cfg_t));
    cm_modem_get_psm_cfg(psm_cfg);
    cm_printf("psm_cfg: mode=%d, periodic_tau=%u, active_time=%u\n",
              psm_cfg->mode, psm_cfg->requested_periodic_tau, psm_cfg->requested_active_time);
    cm_free(psm_cfg);

    cm_cereg_state_t *cereg;
    cereg = cm_malloc(sizeof(cm_cereg_state_t));
    memset(cereg, 0, sizeof(cm_cereg_state_t));
    cm_modem_get_cereg_state(cereg);
    cm_printf("cereg: n=%d, state=%d, lac=%u, ci=%.8X, act=%u\n",
              cereg->n, cereg->state, cereg->lac, cereg->ci, cereg->act);
    cm_printf("cereg: rac=%u, cause_type=%d, reject_cause=%d\n",
              cereg->rac, cereg->cause_type, cereg->reject_cause);
    cm_printf("cereg: periodic_tau=%u, active_time=%u\n",
              cereg->periodic_tau, cereg->active_time);
    cm_free(cereg);

    int get_cscon = cm_modem_get_cscon();
    cm_printf("get_cscon:%d\n", get_cscon);

    int pdp_state = cm_modem_get_pdp_state(1);
    cm_printf("pdp_state:%d\n", pdp_state);

    return 0;
}
