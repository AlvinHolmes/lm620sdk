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
#include "cm_modem_info.h"
#include "cm_modem.h"

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
    int32_t ret;
    char buf[CM_TEST_VER_LEN];

    /* 避免睡眠模式导致输出异常 */
    cm_pm_cfg_t pm_cfg;
    pm_cfg.cb_enter = NULL;
    pm_cfg.cb_exit = NULL;
    cm_pm_init(pm_cfg);
    cm_pm_work_lock();

    cm_printf("\r\n======= CM Mobile Test Starts =======\r\n");

    /* ==================== cm_sys.h 测试 ==================== */

    cm_printf("\r\n[TEST] cm_sys_get_cm_ver\r\n");
    memset(buf, 0, CM_TEST_VER_LEN);
    ret = cm_sys_get_cm_ver(NULL, CM_TEST_VER_LEN);
    cm_printf("[ILLEGAL] ver_buff = NULL, ret[%d]\r\n", ret);

    memset(buf, 0, CM_TEST_VER_LEN);
    ret = cm_sys_get_cm_ver(buf, CM_TEST_VER_LEN);
    cm_printf("[LEGAL] SDK VERSION: %s, ret[%d]\r\n", buf, ret);

    /* ==================== cm_sys.h 测试 ==================== */

    cm_printf("\r\n[TEST] cm_sys_get_sn\r\n");
    memset(buf, 0, CM_TEST_VER_LEN);
    ret = cm_sys_get_sn(NULL);
    cm_printf("[ILLEGAL] sn = NULL, ret[%d]\r\n", ret);

    memset(buf, 0, CM_TEST_VER_LEN);
    ret = cm_sys_get_sn(buf);
    cm_printf("[LEGAL] SN: %s, ret[%d]\r\n", buf, ret);

    /* ==================== cm_sys.h 测试 ==================== */

    cm_printf("\r\n[TEST] cm_sys_get_imei\r\n");
    memset(buf, 0, CM_TEST_VER_LEN);
    ret = cm_sys_get_imei(NULL);
    cm_printf("[ILLEGAL] imei = NULL, ret[%d]\r\n", ret);

    memset(buf, 0, CM_TEST_VER_LEN);
    ret = cm_sys_get_imei(buf);
    cm_printf("[LEGAL] IMEI: %s, ret[%d]\r\n", buf, ret);

    /* ==================== cm_sim.h 测试 ==================== */

    cm_printf("\r\n[TEST] cm_sim_get_imsi\r\n");
    memset(buf, 0, CM_TEST_VER_LEN);
    ret = cm_sim_get_imsi(NULL);
    cm_printf("[ILLEGAL] imsi = NULL, ret[%d]\r\n", ret);

    memset(buf, 0, CM_TEST_VER_LEN);
    ret = cm_sim_get_imsi(buf);
    cm_printf("[LEGAL] IMSI: %s, ret[%d]\r\n", buf, ret);

    /* ==================== cm_sim.h 测试 ==================== */

    cm_printf("\r\n[TEST] cm_sim_get_iccid\r\n");
    memset(buf, 0, CM_TEST_VER_LEN);
    ret = cm_sim_get_iccid(NULL);
    cm_printf("[ILLEGAL] iccid = NULL, ret[%d]\r\n", ret);

    memset(buf, 0, CM_TEST_VER_LEN);
    ret = cm_sim_get_iccid(buf);
    cm_printf("[LEGAL] ICCID: %s, ret[%d]\r\n", buf, ret);

    /* ==================== cm_modem_info.h 测试 ==================== */

    cm_printf("\r\n[TEST] cm_modem_info_radio\r\n");
    cm_radio_info_t *radio_info;
    radio_info = (cm_radio_info_t *)malloc(sizeof(cm_radio_info_t));
    memset(radio_info, 0, sizeof(cm_radio_info_t));

    ret = cm_modem_info_radio(NULL);
    cm_printf("[ILLEGAL] radio_info = NULL, ret[%d]\r\n", ret);

    ret = cm_modem_info_radio(radio_info);
    cm_printf("[LEGAL] radio_info: rat=%u, rsrp=%u, rsrq=%u, rssi=%u\r\n",
              radio_info->rat, radio_info->rsrp, radio_info->rsrq, radio_info->rssi);
    cm_printf("[LEGAL] radio_info: rxlev=%u, tx_power=%u, last_cellid=%u\r\n",
              radio_info->rxlev, radio_info->tx_power, radio_info->last_cellid);
    cm_printf("[LEGAL] radio_info: last_ecl=%u, last_snr=%u, last_earfcn=%u\r\n",
              radio_info->last_ecl, radio_info->last_snr, radio_info->last_earfcn);
    cm_printf("[LEGAL] radio_info: last_pci=%u, tx_time=%u, rx_time=%u\r\n",
              radio_info->last_pci, (uint32_t)radio_info->tx_time, (uint32_t)radio_info->rx_time);
    free(radio_info);

    /* 目前只支持查询 state */
    cm_cereg_state_t cereg = {
        .state = 0xff,
    };
    cm_printf("\r\n[TEST] cm_modem_get_cereg_state\r\n");
    ret = cm_modem_get_cereg_state(&cereg);
    cm_printf("[LEGAL] cereg: ret=%d, state=%u\r\n", ret, cereg.state);

    /* ==================== cm_modem_info.h 测试 ==================== */

#define CELL_INFO_MAX_NUM  10

    /* 等待获取小区信息 */
    cm_printf("[INFO] Waiting 20 seconds for cell info...\r\n");
    osDelay(20 * 1000);

    cm_printf("\r\n[TEST] cm_modem_info_cell\r\n");
    cm_cell_info_t *cell_info;
    cell_info = (cm_cell_info_t *)malloc(sizeof(cm_cell_info_t) * CELL_INFO_MAX_NUM);
    memset(cell_info, 0, sizeof(cm_cell_info_t) * CELL_INFO_MAX_NUM);

    ret = cm_modem_info_cell(NULL, CELL_INFO_MAX_NUM);
    cm_printf("[ILLEGAL] cell_info = NULL, ret[%d]\r\n", ret);

    ret = cm_modem_info_cell(cell_info, CELL_INFO_MAX_NUM);
    cm_printf("[LEGAL] get %d cells\r\n", ret);

    for (int i = 0; i < CELL_INFO_MAX_NUM; i++) {
        if (i < ret) {
            cm_printf("[LEGAL] cell_info[%d]: primary_cell=%d\r\n", i, cell_info[i].primary_cell);
            cm_printf("[LEGAL] cell_info[%d]: mcc=%s, mnc=%s\r\n", i, cell_info[i].mcc, cell_info[i].mnc);
            cm_printf("[LEGAL] cell_info[%d]: earfcn=%u, earfcn_offset=%u\r\n",
                      i, cell_info[i].earfcn, cell_info[i].earfcn_offset);
            cm_printf("[LEGAL] cell_info[%d]: tac=%u, pci=%u\r\n", i, cell_info[i].tac, cell_info[i].pci);
            cm_printf("[LEGAL] cell_info[%d]: rsrp=%u, rsrq=%u, rssi=%u, snr=%u\r\n",
                      i, cell_info[i].rsrp, cell_info[i].rsrq, cell_info[i].rssi, cell_info[i].snr);
            cm_printf("[LEGAL] cell_info[%d]: bandwidth=%u, cid=%.8X\r\n",
                      i, cell_info[i].bandwidth, cell_info[i].cid);
        }
    }

    free(cell_info);

#undef CELL_INFO_MAX_NUM

    return 0;
}
