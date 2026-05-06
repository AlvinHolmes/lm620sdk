/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        drv_regulator.h
 *
 * @brief       实现电源配置接口
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-05-11     ICT Team          创建
 ************************************************************************************
 */

#ifndef __DRV_REGULATOR__
#define __DRV_REGULATOR__
/************************************************************************************
 *                                 头文件定义
 ************************************************************************************/
#include "drv_common.h"
#include <psm_common.h>

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/
#define SBY_LSP_LDO_BASE_ADDR            BASE_SBY_LSP

#define AON_SYS_CTL_BASE_ADDR            BASE_AON_SYS_CTRL

#define PMU_CTRL_AON_BASE_ADDR           BASE_PMU_AON_CTRL
#define LPPMU_PMU_LDO_SEL                (PMU_CTRL_AON_BASE_ADDR + 0x100)

#define DCDC2_VO_CH_OFF         0
#define DCDC2_VO_CH_WIDTH       4

#define LDO1_2_ECO_OFF          0
#define LDO1_2_DISCHARGE_OFF    1
#define LDO1_2_TRIM_OFF         4

#define LDO3__8_ECO_MODE_OFF    0
#define LDO3__8_EN_OFF          1
#define LDO3__8_ECO_OFF         3
#define LDO3__8_DISCHARGE_OFF   4
#define LDO3__8_TRIM_OFF        6
#define OPM_LPBGR_TRIM_OFF      0
#define LPPMU_TRIM_WID          4

#define DCDC1_TRIM_OFF          0
#define DCDC1_TRIM_WID          8
#define DCDC2_TRIM_OFF          0
#define DCDC2_TRIM_WID          10

#define DCDC2_VO_CH_BASEVOL     1400
#define DCDC2_VO_CH_STEPVOL     (-25)
#define DCDC2_BASEVOL           517
#define DCDC2_STEPVOL           10

#define LDO1_4_6_BASEVOL        700
#define LDO1_4_6_STEPVOL        25
#define LDO2_3_BASEVOL          1480
#define LDO2_3_STEPVOL          40
#define LDO5_BASEVOL            2100
#define LDO5_STEPVOL            50
#define LDO7_8_BASEVOL          1550
#define LDO7_8_STEPVOL          50
#define OPM_BASEVOL             2290
#define OPM_STEPVOL             45
#define LPBGR_BASEVOL           520
#define LPBGR_STEPVOL           10

#define LPBGR_TRIMVOL           600
#define OPM_TRIMVOL             2650
#define DCDC2_VO_CH_TRIMVOL     1200
#define DCDC2_TRIMVOL           600
#define LDO1_TRIMVOL            800
#define LDO2_TRIMVOL            1800
#define LDO3_TRIMVOL            1800
#define LDO4_TRIMVOL            800
#define LDO5_TRIMVOL            2500
#define LDO6_TRIMVOL            800
#define LDO7_TRIMVOL            3000
#define LDO8_TRIMVOL            3000

#define LDO_INV_VOL             0

#define PSM_LDO6_VOL_ADDR                       (PSM_IDLE_LDO_ADDR_BASE)
#define PSM_LDO6_VOL                            (*(volatile uint32_t *)PSM_LDO6_VOL_ADDR)
#define PSM_DCDC2_VOL_ADDR                      (PSM_IDLE_LDO_ADDR_BASE+0x4)
#define PSM_DCDC2_VOL                           (*(volatile uint32_t *)PSM_DCDC2_VOL_ADDR)
#define PSM_LDO5_VOL_ADDR                       (PSM_IDLE_LDO_ADDR_BASE+0x8)
#define PSM_LDO5_VOL                            (*(volatile uint32_t *)PSM_LDO5_VOL_ADDR)
#define PSM_LDO3_VOL_ADDR                       (PSM_IDLE_LDO_ADDR_BASE+0xC)
#define PSM_LDO3_VOL                            (*(volatile uint32_t *)PSM_LDO3_VOL_ADDR)
#define PSM_LDO2_VOL_ADDR                       (PSM_IDLE_LDO_ADDR_BASE+0x10)
#define PSM_LDO2_VOL                            (*(volatile uint32_t *)PSM_LDO2_VOL_ADDR)

typedef enum
{
    LDO_LPBGR,
    LDO_DCDC2_VO,
    LDO_DCDC2_TRIM,
    LDO_OPM,
    LDO_LDO1,
    LDO_LDO2,
    LDO_LDO3,
    LDO_LDO4,
    LDO_LDO5,
    LDO_LDO6,
    LDO_LDO7_INVLID,
    LDO_LDO8,
    LDO_MAX,
}LDO_Id;

/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
typedef enum
{
    /*** Bit definition for [regSwCtrlSel] register(0x00000070) ****/
    LDO_SW_LPPMU_LDO2_SEL               = (0x00700001),  /*!< */
    LDO_SW_LPPMU_LDO4_SEL               = (0x00700201),  /*!< */
    LDO_SW_LPPMU_LDO5_SEL               = (0x00700301),  /*!< */
    LDO_SW_DCDC1_SEL                    = (0x00700701),  /*!< */

    LDO_SW_LPPMU_LDO3_SEL               = (0x01001501),  /*!< */
    LDO_SW_LPPMU_LDO6_SEL               = (0x01001601),  /*!< */
    LDO_SW_LPPMU_LDO7_SEL               = (0x01001701),  /*!< */
    LDO_SW_LPPMU_LDO8_SEL               = (0x01001801),  /*!< */

    /*** Bit definition for [lppmuOscLdo1Config] register(0x00000224) ****/
    LDO_DIG_LPPMU_LDO1_DISCHARGE        = (0x02240101),  /*!<ldo1 discharge mode: 1: enable; 0 disable */
    LDO_DIG_LPPMU_LDO1_TRIM             = (0x02240404),  /*!<ldo1 voltage output trim;step:25mV;0000:0.7V;0001:0.725V;0010:0.75V;0011:0.775V;0100:0.8V;0101:0.825V;0110:0.85V;0111:0.875V;
    1000:0.9V;1001:0.925V;1010:0.95V;1011:0.975V;1100:1V   ;1101:1.025V;1110:1.05V;1111:1.075V */

    /*** Bit definition for [lppmuOscOpmConfig] register(0x00000244) ****/
    LDO_DIG_LPPMU_OPM_TRIM              = (0x02440004),  /*!<opm voltage output trim;trimstep:45mV;0000:2.29V;0001:2.335V;0010:2.38V;0011:2.425V;0100:2.47V;0101:2.515V;0110:2.56V;0111:2.605V;
    1000:2.65V;1001:2.695V;1010:2.74V;1011:2.785V;1100:2.83V;1101:2.875V;1110:2.92V;1111:2.965V */


    /*** Bit definition for [lppmuOscLpbgrConfig] register(0x00000248) ****/
    LDO_DIG_LPPMU_LPBGR_TRIM            = (0x02480004),  /*!<bandgap voltage output trim;trimstep:10mV;0000:0.52V;0001:0.53V;0010:0.54V;0011:0.55V;0100:0.56V;0101:0.57V;0110:0.58V;0111:0.59V;
    1000:0.6V;1001:0.61V;1010:0.62V;1011:0.63V;1100:0.64V;1101:0.65V;1110:0.66V;1111:0.67V */


    /*** Bit definition for [dcdc2Trim] register(0x00000250) ****/
    LDO_DCDC2_VO_CH                     = (0x02500004),  /*!<dcdc2 output trim, default:1.2V */
    LDO_DCDC2_TRIM_BG                   = (0x02500404),  /*!<dcdc2 bandgap voltage trim */
    LDO_DCDC2_TRIM_BG_IZTC              = (0x02500804),  /*!<dcdc2 bandgap ztc current trim */
    LDO_DCDC2_TRIM_HVLDOVDDA            = (0x02500c04),  /*!<dcdc2 HV LDO output votage trim */
    LDO_DCDC2_TRIM_OSC                  = (0x02501402),  /*!<dcdc2 osc frequency trim */
    LDO_DCDC2_OSC_1OR1P5M_SEL           = (0x02501601),  /*!<dcdc2 switch frequency, 0:1.5MHz;1:1MHz */
    LDO_DCDC2_TRIM_SWITCHCURRENT        = (0x02501703),  /*!<dcdc2 pwm/pfm mode switchcurrent  trim: 000:iload4.73mA hysteresiswindow100mV ,001:iload5.05mA hysteresiswindow130mV, 010:iload5.3mA hysteresiswindow160mV, 011:iload5.53mA hysteresiswindow190mV，100:iload5.71mA hysteresiswindow220mV,101:iload5.88mA hysteresiswindow250mV, 110:iload6.02mA hysteresiswindow280mV, 111:iload6.11mA hysteresiswindow300mV */
    LDO_DCDC2_TRIM_HYSTERESISWINDOW     = (0x02501a02),  /*!<none */
    LDO_DCDC2_SPAREBITS                 = (0x02501c04),  /*!<[31:30] dcdc2 spare bits;[29:28] dcdc2 soft start time：00:460us 01:380us 10:270us 11:180us */


    /*** Bit definition for [lppmuLdo78Trim] register(0x00000254) ****/
    LDO_DIG_LPPMU_LDO7_TRIM             = (0x02540004),  /*!<ldo7 voltage output trim;trimstep:50mV;0000:1.55V;0001:1.6V;0010:1.65V;0011:1.7V;0100:1.75V;0101:1.8V;0110:1.85V;0111:1.9V;
    1000:1.95V;1001:2.85V;1010:2.9V;1011:2.95V;1100:3V     ;1101:3.05V;1110:3.1V;1111:3.15V */
    LDO_DIG_LPPMU_LDO8_TRIM             = (0x02540605),  /*!<ldo8 voltage output trim;trimstep:50mV;00000:1.55V;00001:1.6V;00010:1.65V;00011:1.7V;00100:1.75V;00101:1.8V;00110:1.85V;00111:1.9V;01000:1.95V;01001:2V  ;01010:2.05V;01011:2.10V;
    01100:2.15V;01101:2.20V;01110:2.25V;01111:2.30V;10000:2.35V;10001:2.40V;10010:2.50V;10011:2.55V;10100:2.60V;10101:2.65V;10110:2.70V;10111:2.75V;11000:2.80V;11001:2.85V;11010:2.9V;11011:2.95V;11100:3V;11101:3.05V;11110:3.1V;11111:3.15V */

    LDO_BASE_ADDR_SPLIT_FLAG            = (0x03000000),

    /*** Bit definition for [dcdcEnable] register(0x00000310) ****/
    LDO_DCDC2_EN                        = (0x03100001),  /*!<dcdc enable, active high */
    LDO_DCDC2_SLEEP_EN                  = (0x03100101),  /*!<dcdc sleep enable,active high */
    LDO_DCDC2_SLEEP_MODE                = (0x03100201),  /*!<dcdc sleep mode select,active high, 1:���ֵ�ѹ�����͸��������� 0:�ر���� */
    LDO_DCDC2_DISCHARGE_EN              = (0x03100301),  /*!<dcdc discharge enable,active high, 1:discharge enable */
    LDO_DCDC2_INTEST                    = (0x03100401),  /*!<dcdc test mode enable,active high, 1:test mode enable */
    LDO_DCDC2_MODE                      = (0x03100501),  /*!<dcdc mode select, 1:PWM/PFM�Զ��л���0:ǿ��PWM */

    LDO_DCDC2_TRIM_OCP                  = (0x03141003),  /*!<dcdc over current protect trim */

    /*** Bit definition for [dcdcTest] register(0x00000318) ****/
    LDO_DCDC_TESTE                      = (0x03180003),  /*!<dcdc test select */
    LDO_DCDC_SPAREBITS                  = (0x03180410),  /*!<spare bit *

    /*** Bit definition for [lppmuOscLdo2Config] register(0x00000328) ****/
    LDO_DIG_LPPMU_LDO2_ECO_MODE         = (0x03280001),  /*!<ldo2 eco_mode: 1: keep output ; 0 : close ldo */
    LDO_DIG_LPPMU_LDO2_EN               = (0x03280101),  /*!<ldo2 enable : 1: enable; 0 disable */
    LDO_DIG_LPPMU_LDO2_ECO              = (0x03280301),  /*!<ldo2 eco mode: 1: enable; 0 disable */
    LDO_DIG_LPPMU_LDO2_DISCHARGE        = (0x03280401),  /*!<ldo2 discharge mode: 1: enable; 0 disable */
    LDO_DIG_LPPMU_LDO2_TRIM             = (0x03280604),  /*!<ldo2 voltage output trim;step:40mV;0000:1.48V;0001:1.52V;0010:1.56V;0011:1.6V;0100:1.64V;0101:1.68V;0110:1.72V;
    0111:1.76V;1000:1.8V;1001:1.84V;1010:1.88V;1011:1.92V;1100:1.96V;1101:2V;1110:2.04V;1111:2.08V */


    /*** Bit definition for [lppmuOscLdo3Config] register(0x0000032c) ****/
    LDO_DIG_LPPMU_LDO3_ECO_MODE         = (0x032c0001),  /*!<ldo3 eco_mode: 1: keep output ; 0 : close ldo */
    LDO_DIG_LPPMU_LDO3_EN               = (0x032c0101),  /*!<ldo3 enable : 1: enable; 0 disable */
    LDO_DIG_LPPMU_LDO3_ECO              = (0x032c0301),  /*!<ldo3 eco mode: 1: enable; 0 disable */
    LDO_DIG_LPPMU_LDO3_DISCHARGE        = (0x032c0401),  /*!<ldo3 discharge mode: 1: enable; 0 disable */
    LDO_DIG_LPPMU_LDO3_TRIM             = (0x032c0604),  /*!<ldo3 voltage output trim;step:40mV;0000:1.48V;0001:1.52V;0010:1.56V;0011:1.6V;0100:1.64V;0101:1.68V;0110:1.72V;
    0111:1.76V;1000:1.8V;1001:1.84V;1010:1.88V;1011:1.92V;1100:1.96V;1101:2V;1110:2.04V;1111:2.08V */


    /*** Bit definition for [lppmuOscLdo4Config] register(0x00000330) ****/
    LDO_DIG_LPPMU_LDO4_ECO_MODE         = (0x03300001),  /*!<ldo4 eco_mode: 1: keep output ; 0 : close ldo */
    LDO_DIG_LPPMU_LDO4_EN               = (0x03300101),  /*!<ldo4 enable : 1: enable; 0 disable */
    LDO_DIG_LPPMU_LDO4_ECO              = (0x03300301),  /*!<ldo4 eco mode: 1: enable; 0 disable */
    LDO_DIG_LPPMU_LDO4_DISCHARGE        = (0x03300401),  /*!<ldo4 discharge mode: 1: enable; 0 disable */
    LDO_DIG_LPPMU_LDO4_TRIM             = (0x03300604),  /*!<ldo4 voltage output trim;step:25mV;0000:0.7V;0001:0.725V;0010:0.75V;0011:0.775V;0100:0.8V;0101:0.825V;0110:0.85V;0111:0.875V;1000:0.9V;1001:0.925V;1010:0.95V;1011:0.975V;1100:1V;1101:1.025V;1110:1.05V;1111:1.075V */


    /*** Bit definition for [lppmuOscLdo5Config] register(0x00000334) ****/
    LDO_DIG_LPPMU_LDO5_ECO_MODE         = (0x03340001),  /*!<ldo5 eco_mode: 1: keep output ; 0 : close ldo */
    LDO_DIG_LPPMU_LDO5_EN               = (0x03340101),  /*!<ldo5 enable : 1: enable; 0 disable */
    LDO_DIG_LPPMU_LDO5_ECO              = (0x03340301),  /*!<ldo5 eco mode: 1: enable; 0 disable */
    LDO_DIG_LPPMU_LDO5_DISCHARGE        = (0x03340401),  /*!<ldo5 discharge mode: 1: enable; 0 disable */
    LDO_DIG_LPPMU_LDO5_TRIM             = (0x03340604),  /*!<ldo5 voltage output trim;step:50mV;0000:2.1V;0001:2.15V;0010:2.2V;0011:2.25V;0100:2.3V;0101:2.35V;0110:2.4V;0111:2.45V;1000:2.5V;1001:2.55V;1010:2.6V;1011:2.65V;1100:2.7V;1101:2.75V;1110:2.8V;1111:2.85V */


    /*** Bit definition for [lppmuOscLdo6Config] register(0x00000338) ****/
    LDO_DIG_LPPMU_LDO6_ECO_MODE         = (0x03380001),  /*!<ldo6 eco_mode: 1: keep output ; 0 : close ldo */
    LDO_DIG_LPPMU_LDO6_EN               = (0x03380101),  /*!<ldo6 enable : 1: enable; 0 disable */
    LDO_DIG_LPPMU_LDO6_ECO              = (0x03380301),  /*!<ldo6 eco mode: 1: enable; 0 disable */
    LDO_DIG_LPPMU_LDO6_DISCHARGE        = (0x03380401),  /*!<ldo6 discharge mode: 1: enable; 0 disable */
    LDO_DIG_LPPMU_LDO6_TRIM             = (0x03380604),  /*!<ldo6 voltage output trim;step:25mV;0000:0.7V;0001:0.725V;0010:0.75V;0011:0.775V;0100:0.8V;0101:0.825V;0110:0.85V;0111:0.875V;1000:0.9V;1001:0.925V;1010:0.95V;1011:0.975V;1100:1V;1101:1.025V;1110:1.05V;1111:1.075V */


    /*** Bit definition for [lppmuOscLdo7Config] register(0x0000033c) ****/
    LDO_DIG_LPPMU_LDO7_ECO_MODE         = (0x033c0001),  /*!<ldo7 eco_mode: 1: keep output ; 0 : close ldo */
    LDO_DIG_LPPMU_LDO7_EN               = (0x033c0101),  /*!<ldo7 enable : 1: enable; 0 disable */
    LDO_DIG_LPPMU_LDO7_ECO              = (0x033c0301),  /*!<ldo7 eco mode: 1: enable; 0 disable */
    LDO_DIG_LPPMU_LDO7_DISCHARGE        = (0x033c0401),  /*!<ldo7 discharge mode: 1: enable; 0 disable */


    /*** Bit definition for [lppmuOscLdo8Config] register(0x00000340) ****/
    LDO_DIG_LPPMU_LDO8_ECO_MODE         = (0x03400001),  /*!<ldo8 eco_mode: 1: keep output ; 0 : close ldo */
    LDO_DIG_LPPMU_LDO8_EN               = (0x03400101),  /*!<ldo8 enable : 1: enable; 0 disable */
    LDO_DIG_LPPMU_LDO8_ECO              = (0x03400301),  /*!<ldo8 eco mode: 1: enable; 0 disable */
    LDO_DIG_LPPMU_LDO8_DISCHARGE        = (0x03400401),  /*!<ldo8 discharge mode: 1: enable; 0 disable */


    /*** Bit definition for [lppmuOscTestConfig] register(0x0000034c) ****/
    LDO_DIG_LPPMU_TEST_EN               = (0x034c0001),  /*!<test enable : 1: enable; 0 disable */
    LDO_DIG_LPPMU_TEST_SEL              = (0x034c0202),  /*!<test analog sign sel;00:bandgap vref0p6;01:opm v26_on;10:bandgap ib_125na;11:TBD */


    /*** Bit definition for [lppmuOscOk] register(0x00000350) ****/
    LDO_LDO1_OK                         = (0x03500001),  /*!< */
    LDO_LDO2_OK                         = (0x03500101),  /*!< */
    LDO_LDO3_OK                         = (0x03500201),  /*!< */
    LDO_LDO4_OK                         = (0x03500301),  /*!< */
    LDO_LDO5_OK                         = (0x03500401),  /*!< */
    LDO_LDO6_OK                         = (0x03500501),  /*!< */
    LDO_LDO7_OK                         = (0x03500601),  /*!< */
    LDO_LDO8_OK                         = (0x03500701),  /*!< */
    LDO_LDO7_BYPASS_OK_LV               = (0x03500801),  /*!< */
    LDO_LDO8_BYPASS_OK_LV               = (0x03500901),  /*!< */

} LDO_RegGenLdo;

typedef struct {
    LDO_RegGenLdo reg_addr;
    int16_t reg_basevol;
    int16_t trim_vol;
    int16_t board_basevol;
    int16_t step_vol;
}LDO_BaseInfo;

/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
/**
 ************************************************************************************
 * @brief           获取LDO配置: eco/eco mode/discharge
 *
 * @param[in]       regBit      地址偏移/寄存器BIT偏移/位宽
 *
 * @return          uint32_t    功能配置值
 * @retval
 ************************************************************************************
*/
uint32_t LDO_GetReg(LDO_RegGenLdo regBit);

#if defined (OS_USING_PM)
/**
 ************************************************************************************
 * @brief           Idle 休眠降低ldo电压
 * @param[in]       void
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void Ldo_IdleSuspend(void);

/**
 ************************************************************************************
 * @brief           Idle 唤醒恢复ldo电压
 * @param[in]       void
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void Ldo_IdleResume(void);
#endif

#ifdef _CPU_AP
/**
 ************************************************************************************
 * @brief           配置LDO: eco/eco mode/discharge
 *
 * @param[in]       regBit      地址偏移/寄存器BIT偏移/位宽
 * @param[in]       bitVal      配置值
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void LDO_SetReg(LDO_RegGenLdo regBit, uint32_t bitVal);

/**
 ************************************************************************************
 * @brief           初始化ldo trim信息
 *
 * @param[in]       id      ldo id
 * @param[in]       bitVal  trim值
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void LDO_BaseInfoInit(LDO_Id id, int16_t bitVal);

/**
 ************************************************************************************
 * @brief           配置LDO电压，ldo7电压不连续，不能用该接口
 *
 * @param[in]       id          id
 * @param[in]       volVal      电压值（mv）
 *
 * @return
 * @retval          DRV_OK              配置成功
 *                  DRV_ERR_PARAMETER   参数异常
 ************************************************************************************
*/
uint32_t LDO_SetVol(LDO_Id id, int16_t volVal);
#endif
#endif
