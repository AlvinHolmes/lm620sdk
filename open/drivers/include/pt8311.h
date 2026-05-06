/**
  ******************************************************************************
  * @file    pt8311.h
  * @brief   Header file for pt8311 driver
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _PT8311_H_
#define _PT8311_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "pt8311_user_cfg.h"


/** @addtogroup PT8311_Driver
  * @{
  */

#ifdef PT8311_MODULE_ENABLED

/* Exported Constants --------------------------------------------------------*/
/** @defgroup PT8311_Exported_Constants PT8311 Exported Constants
  * @brief    PT8311 Exported Constants
  * @{
  */

/** @defgroup PT8311_Exported_Constants_Group1 PT8311 Common Constants Definition
  * @brief    PT8311 Common Constants Definition
  * @{
  */

/**
  * @brief PT8311 Driver Version Number V1.1
  */
#define PT8311_DRV_VER_MAIN                 (1)                 /*!< PT8311 Driver Main Version         */
#define PT8311_DRV_VER_SUB                  (1)                 /*!< PT8311 Driver Sub Version          */

/**
  * @brief PT8311 Driver Stage String Definition
  * @note  Value range: "Alpha" "Beta" "RC" "Trial" "Release"
  */
#define PT8311_DRV_STAGE_STR                "Release"


/**
  * @brief PT8311 Chip Numbers Max Definition
  */
#define PT8311_CHIP_NUMS_MAX                (2)

/**
  * @brief PT8311 7bit I2C Address Definition
  */
#define PT8311_I2C_ADDR_0                   (0x44)              /*!< PT8311 7bit I2C Address 0: 0x44    */
#define PT8311_I2C_ADDR_1                   (0x45)              /*!< PT8311 7bit I2C Address 1: 0x45    */

/**
  * @brief PT8311 ADC EQ Band numbers Definition
  */
#define PT8311_ADC_EQ_BAND_NUMS             (1)

/**
  * @brief PT8311 DAC EQ Band numbers Definition
  */
#define PT8311_DAC_EQ_BAND_NUMS             (3)

/**
  * @brief PT8311 EQ Band Coff numbers in int32_t Definition
  */
#define PT8311_EQ_BAND_COFF_NUMS            (5)

/**
  * @brief PT8311 DRC Coff numbers in int32_t Definition
  */
#define PT8311_DRC_COFF_NUMS                (8)

/**
  * @brief PT8311 DAC Ramp Up Time in ms Unit Definition
  */
#define PT8311_DAC_RAMP_UP_TIME_MS          (100)

/**
  * @brief PT8311 DAC Ramp Down Time in ms Unit Definition
  */
#define PT8311_DAC_RAMP_DOWN_TIME_MS        (650)

/**
  * @brief PT8311 DAC Digital Volume 0dB Register value Definition
  */
#define PT8311_DAC_DIG_VOL_0dB              (0xBF)

/**
  * @}
  */


/** @defgroup PT8311_Exported_Constants_Group2 PT8311 Register Bit Definition
  * @brief    PT8311 Register Bit Definition
  * @{
  */

/* =========================================================  SRST  =========================================================== */
#define SOFT_RESET_Pos                      (0U)                /*!< SOFT_RESET (Bit 0)                                         */
#define SOFT_RESET_Msk                      (0x1U)              /*!< SOFT_RESET (Bitfield-Mask: 0x1)                            */

/* =======================================================  PWR_CTRL1  ======================================================== */
#define POWER_VREF_BYP_Pos                  (4U)                /*!< VREF filter resistor bypass for test (Bit 4)               */
#define POWER_VREF_BYP_Msk                  (0x10U)             /*!< VREF filter resistor bypass for test (Bitfield-Mask: 0x1)  */
#define POWER_ALDO_VCTRL_Pos                (1U)                /*!< ALDO output voltage control (Bit 1)                        */
#define POWER_ALDO_VCTRL_Msk                (0xeU)              /*!< ALDO output voltage control (Bitfield-Mask: 0x7)           */
#define POWER_ALDO_EN_Pos                   (0U)                /*!< ALDO enable (Bit 0)                                        */
#define POWER_ALDO_EN_Msk                   (0x1U)              /*!< ALDO enable (Bitfield-Mask: 0x1)                           */

/* =======================================================  PWR_CTRL2  ======================================================== */
#define POWER_MICBIAS_BYP_Pos               (7U)                /*!< MICBIAS regulator bypass (Bit 7)                           */
#define POWER_MICBIAS_BYP_Msk               (0x80U)             /*!< MICBIAS regulator bypass (Bitfield-Mask: 0x1)              */
#define POWER_MICBIAS_CHOP_FREQ_Pos         (4U)                /*!< MICBIAS chop frequency control@6144kHz (Bit 4)             */
#define POWER_MICBIAS_CHOP_FREQ_Msk         (0x70U)             /*!< MICBIAS chop frequency control@6144kHz (Bitfield-Mask: 0x7)*/
#define POWER_MICBIAS_CHOP_EN_Pos           (3U)                /*!< MICBIAS chop enable (Bit 3)                                */
#define POWER_MICBIAS_CHOP_EN_Msk           (0x8U)              /*!< MICBIAS chop enable (Bitfield-Mask: 0x1)                   */
#define POWER_MICBIAS_VCTRL_Pos             (1U)                /*!< MICBIAS output voltage contro (Bit 1)                      */
#define POWER_MICBIAS_VCTRL_Msk             (0x6U)              /*!< MICBIAS output voltage contro (Bitfield-Mask: 0x3)         */
#define POWER_MICBIAS_EN_Pos                (0U)                /*!< MICBIAS enable (Bit 0)                                     */
#define POWER_MICBIAS_EN_Msk                (0x1U)              /*!< MICBIAS enable (Bitfield-Mask: 0x1)                        */

/* =======================================================  PLL_CTRL  ========================================================= */
#define PLL_LDO_VCTRL_Pos                   (2U)                /*!< PLL LDO output selection (Bit 2)                           */
#define PLL_LDO_VCTRL_Msk                   (0xcU)              /*!< PLL LDO output selection (Bitfield-Mask: 0x3)              */
#define PLL_LDO_EN_Pos                      (1U)                /*!< PLL LDO circuit enable (Bit 1)                             */
#define PLL_LDO_EN_Msk                      (0x2U)              /*!< PLL LDO circuit enable (Bitfield-Mask: 0x1)                */
#define PLL_EN_Pos                          (0U)                /*!< PLL circuit Enable (Bit 0)                                 */
#define PLL_EN_Msk                          (0x1U)              /*!< PLL circuit Enable (Bitfield-Mask: 0x1)                    */

/* =======================================================  PLL_CFG1  ========================================================= */
#define PLL_P_DIV_Pos                       (0U)                /*!< PLL pre divider control (Bit 0)                            */
#define PLL_P_DIV_Msk                       (0x1fU)             /*!< PLL pre divider control (Bitfield-Mask: 0x1f)              */

/* =======================================================  PLL_CFG2  ========================================================= */
#define PLL_N_DIV_Pos                       (0U)                /*!< PLL feedback divider control (Bit 0)                       */
#define PLL_N_DIV_Msk                       (0xffU)             /*!< PLL feedback divider control (Bitfield-Mask: 0xff)         */

/* =======================================================  PLL_CFG3  ========================================================= */
#define PLL_M_DIV_Pos                       (0U)                /*!< PLL post divider control (Bit 0)                           */
#define PLL_M_DIV_Msk                       (0xfU)              /*!< PLL post divider control (Bitfield-Mask: 0xf)              */

/* ======================================================  SYSCLK_CTRL  ======================================================= */
#define PLLCLK_SRC_Pos                      (4U)                /*!< PLL Clock Source Select (Bit 4)                            */
#define PLLCLK_SRC_Msk                      (0x10U)             /*!< PLL Clock Source Select (Bitfield-Mask: 0x1)               */
#define PLLCLK_EN_Pos                       (3U)                /*!< PLLCLK Enable (Bit 3)                                      */
#define PLLCLK_EN_Msk                       (0x8U)              /*!< PLLCLK Enable (Bitfield-Mask: 0x1)                         */
#define SYSCLK_SRC_Pos                      (1U)                /*!< System Clock Source Select (Bit 1)                         */
#define SYSCLK_SRC_Msk                      (0x6U)              /*!< System Clock Source Select (Bitfield-Mask: 0x3)            */
#define SYSCLK_EN_Pos                       (0U)                /*!< SYSCLK Enable (Bit 0)                                      */
#define SYSCLK_EN_Msk                       (0x1U)              /*!< SYSCLK Enable (Bitfield-Mask: 0x1)                         */

/* ======================================================  MODCLK_EN  ========================================================= */
#define DAC_DSP_CLK_EN_Pos                  (7U)                /*!< DAC Biquad/DRC clock Enable (Bit 7)                        */
#define DAC_DSP_CLK_EN_Msk                  (0x80U)             /*!< DAC Biquad/DRC clock Enable (Bitfield-Mask: 0x1)           */
#define DAC_DIGITAL_CLK_EN_Pos              (6U)                /*!< DAC digital clock Enable (Bit 6)                           */
#define DAC_DIGITAL_CLK_EN_Msk              (0x40U)             /*!< DAC digital clock Enable (Bitfield-Mask: 0x1)              */
#define DAC_ANALOG_CLK_EN_Pos               (5U)                /*!< DAC analog clock Enable (Bit 5)                            */
#define DAC_ANALOG_CLK_EN_Msk               (0x20U)             /*!< DAC analog clock Enable (Bitfield-Mask: 0x1)               */
#define DAC_PATH_CLK_EN_Pos                 (4U)                /*!< DAC path clock Enable (Bit 4)                              */
#define DAC_PATH_CLK_EN_Msk                 (0x10U)             /*!< DAC path clock Enable (Bitfield-Mask: 0x1)                 */
#define ADC_DSP_CLK_EN_Pos                  (3U)                /*!< ADC Biquad/DRC clock Enable (Bit 3)                        */
#define ADC_DSP_CLK_EN_Msk                  (0x8U)              /*!< ADC Biquad/DRC clock Enable (Bitfield-Mask: 0x1)           */
#define ADC_DIGITAL_CLK_EN_Pos              (2U)                /*!< ADC digital clock Enable (Bit 2)                           */
#define ADC_DIGITAL_CLK_EN_Msk              (0x4U)              /*!< ADC digital clock Enable (Bitfield-Mask: 0x1)              */
#define ADC_ANALOG_CLK_EN_Pos               (1U)                /*!< ADC analog clock Enable (Bit 1)                            */
#define ADC_ANALOG_CLK_EN_Msk               (0x2U)              /*!< ADC analog clock Enable (Bitfield-Mask: 0x1)               */
#define ADC_PATH_CLK_EN_Pos                 (0U)                /*!< ADC path clock Enable (Bit 0)                              */
#define ADC_PATH_CLK_EN_Msk                 (0x1U)              /*!< ADC path clock Enable (Bitfield-Mask: 0x1)                 */
#define DAC_MODULE_CLK_ALL_EN_Pos           (4U)                /*!< DAC module clock all Enable (Bit 4)                        */
#define DAC_MODULE_CLK_ALL_EN_Msk           (0xf0U)             /*!< DAC module clock all Enable (Bitfield-Mask: 0xf)           */
#define ADC_MODULE_CLK_ALL_EN_Pos           (0U)                /*!< ADC module clock all Enable (Bit 0)                        */
#define ADC_MODULE_CLK_ALL_EN_Msk           (0xfU)              /*!< ADC module clock all Enable (Bitfield-Mask: 0xf)           */

/* =======================================================  ADDA_FS  ========================================================== */
#define ADC_FS_Pos                          (4U)                /*!< The ADC frequency of sample (Bit 4)                        */
#define ADC_FS_Msk                          (0x70U)             /*!< The ADC frequency of sample (Bitfield-Mask: 0x7)           */
#define DAC_FS_Pos                          (0U)                /*!< The DAC frequency of sample (Bit 0)                        */
#define DAC_FS_Msk                          (0x7U)              /*!< The DAC frequency of sample (Bitfield-Mask: 0x7)           */

/* =====================================================  ADC_CLK_CTRL  ======================================================= */
#define ADC_OSR_Pos                         (4U)                /*!< ADC Oversampling Ratio (Bit 4)                             */
#define ADC_OSR_Msk                         (0x30U)             /*!< ADC Oversampling Ratio (Bitfield-Mask: 0x3)                */
#define ADC_DIV_Pos                         (0U)                /*!< ADC_DCLK divide ratio from SYS_CLK (Bit 0)                 */
#define ADC_DIV_Msk                         (0xfU)              /*!< ADC_DCLK divide ratio from SYS_CLK (Bitfield-Mask: 0xf)    */

/* =====================================================  DAC_CLK_CTRL  ======================================================= */
#define DAC_OSR_Pos                         (4U)                /*!< DAC Oversampling Ratio (Bit 4)                             */
#define DAC_OSR_Msk                         (0x30U)             /*!< DAC Oversampling Ratio (Bitfield-Mask: 0x3)                */
#define DAC_DIV_Pos                         (0U)                /*!< DAC_DCLK divide ratio from SYS_CLK (Bit 0)                 */
#define DAC_DIV_Msk                         (0xfU)              /*!< DAC_DCLK divide ratio from SYS_CLK (Bitfield-Mask: 0xf)    */

/* =======================================================  I2S_CTRL  ========================================================= */
#define MCLK_IO_Pos                         (6U)                /*!< MCLK IO config (Bit 6)                                     */
#define MCLK_IO_Msk                         (0xc0U)             /*!< MCLK IO config (Bitfield-Mask: 0x3)                        */
#define I2S_ROLE_Pos                        (3U)                /*!< I2S Role config (Bit 3)                                    */
#define I2S_ROLE_Msk                        (0x8U)              /*!< I2S Role config (Bitfield-Mask: 0x1)                       */
#define I2S_TXEN_Pos                        (2U)                /*!< I2S Transmitter Block Enable (Bit 2)                       */
#define I2S_TXEN_Msk                        (0x4U)              /*!< I2S Transmitter Block Enable (Bitfield-Mask: 0x1)          */
#define I2S_RXEN_Pos                        (1U)                /*!< I2S Receiver Block Enable (Bit 1)                          */
#define I2S_RXEN_Msk                        (0x2U)              /*!< I2S Receiver Block Enable (Bitfield-Mask: 0x1)             */
#define I2S_EN_Pos                          (0U)                /*!< I2S Interface Enable (Bit 0)                               */
#define I2S_EN_Msk                          (0x1U)              /*!< I2S Interface Enable (Bitfield-Mask: 0x1)                  */

/* =======================================================  I2S_FMT1  ========================================================= */
#define BCLK_DIV_Pos                        (0U)                /*!< BCLK divide from SYSCLK (Bit 0)                            */
#define BCLK_DIV_Msk                        (0xfU)              /*!< BCLK divide from SYSCLK (Bitfield-Mask: 0xf)               */

/* =======================================================  I2S_FMT2  ========================================================= */
#define LRCK_DIV_Pos                        (0U)                /*!< LRCK period numbers of BCLK (Bit 0)                        */
#define LRCK_DIV_Msk                        (0xffU)             /*!< LRCK period numbers of BCLK (Bitfield-Mask: 0xff)          */

/* =======================================================  I2S_FMT3  ========================================================= */
#define BCLK_POLARITY_Pos                   (7U)                /*!< BCLK POLARITY (Bit 7)                                      */
#define BCLK_POLARITY_Msk                   (0x80U)             /*!< BCLK POLARITY (Bitfield-Mask: 0x1)                         */
#define LRCK_POLARITY_Pos                   (6U)                /*!< LRCK POLARITY (Bit 6)                                      */
#define LRCK_POLARITY_Msk                   (0x40U)             /*!< LRCK POLARITY (Bitfield-Mask: 0x1)                         */
#define MODE_SEL_Pos                        (4U)                /*!< Mode Selection (Bit 4)                                     */
#define MODE_SEL_Msk                        (0x30U)             /*!< Mode Selection (Bitfield-Mask: 0x3)                        */
#define WL_Pos                              (2U)                /*!< Word Length (Bit 2)                                        */
#define WL_Msk                              (0xcU)              /*!< Word Length (Bitfield-Mask: 0x3)                           */
#define TX_WORD_HIZ_Pos                     (1U)                /*!< TX WORD HIZ (Bit 1)                                        */
#define TX_WORD_HIZ_Msk                     (0x2U)              /*!< TX WORD HIZ (Bitfield-Mask: 0x1)                           */
#define TX_STATE_Pos                        (0U)                /*!< TX STATE (Bit 0)                                           */
#define TX_STATE_Msk                        (0x1U)              /*!< TX STATE (Bitfield-Mask: 0x1)                              */

/* =====================================================  I2S_TX_CTRL  ======================================================== */
#define TX_SLOT_EN_Pos                      (4U)                /*!< TX SLOT Enable (Bit 4)                                     */
#define TX_SLOT_EN_Msk                      (0x30U)             /*!< TX SLOT Enable (Bitfield-Mask: 0x3)                        */
#define TX_SLOT1_EN_Pos                     (5U)                /*!< TX SLOT1 Enable (Bit 5)                                    */
#define TX_SLOT1_EN_Msk                     (0x20U)             /*!< TX SLOT1 Enable (Bitfield-Mask: 0x1)                       */
#define TX_SLOT0_EN_Pos                     (4U)                /*!< TX SLOT0 Enable (Bit 4)                                    */
#define TX_SLOT0_EN_Msk                     (0x10U)             /*!< TX SLOT0 Enable (Bitfield-Mask: 0x1)                       */
#define TX_SLOT1_MAP_Pos                    (2U)                /*!< TX SLOT1 MAP (Bit 2)                                       */
#define TX_SLOT1_MAP_Msk                    (0x4U)              /*!< TX SLOT1 MAP (Bitfield-Mask: 0x1)                          */
#define TX_SLOT0_MAP_Pos                    (1U)                /*!< TX SLOT0 MAP (Bit 1)                                       */
#define TX_SLOT0_MAP_Msk                    (0x2U)              /*!< TX SLOT0 MAP (Bitfield-Mask: 0x1)                          */
#define TX_SLOT_SEL_Pos                     (0U)                /*!< TX data line Slot number Select (Bit 0)                    */
#define TX_SLOT_SEL_Msk                     (0x1U)              /*!< TX data line Slot number Select (Bitfield-Mask: 0x1)       */

/* =====================================================  I2S_RX_CTRL  ======================================================== */
#define RXR_MAP_Pos                         (2U)                /*!< RX right data Mapping source (Bit 2)                       */
#define RXR_MAP_Msk                         (0x4U)              /*!< RX right data Mapping source (Bitfield-Mask: 0x1)          */
#define RXL_MAP_Pos                         (1U)                /*!< RX left data Mapping source (Bit 1)                        */
#define RXL_MAP_Msk                         (0x2U)              /*!< RX left data Mapping source (Bitfield-Mask: 0x1)           */
#define RX_SLOT_SEL_Pos                     (0U)                /*!< RX data line Slot number Select (Bit 0)                    */
#define RX_SLOT_SEL_Msk                     (0x1U)              /*!< RX data line Slot number Select (Bitfield-Mask: 0x1)       */

/* =====================================================  I2S_TXM_CTRL  ======================================================= */
#define TX_MIXR_GAIN_Pos                    (6U)                /*!< I2S TX right mixer gain control (Bit 6)                    */
#define TX_MIXR_GAIN_Msk                    (0xc0U)             /*!< I2S TX right mixer gain control (Bitfield-Mask: 0x3)       */
#define TX_MIXR_SRC_Pos                     (4U)                /*!< I2S TX right mixer source control (Bit 4)                  */
#define TX_MIXR_SRC_Msk                     (0x30U)             /*!< I2S TX right mixer source control (Bitfield-Mask: 0x3)     */
#define TX_MIXL_GAIN_Pos                    (2U)                /*!< I2S TX left mixer gain control (Bit 2)                     */
#define TX_MIXL_GAIN_Msk                    (0xcU)              /*!< I2S TX left mixer gain control (Bitfield-Mask: 0x3)        */
#define TX_MIXL_SRC_Pos                     (0U)                /*!< I2S TX left mixer source control (Bit 0)                   */
#define TX_MIXL_SRC_Msk                     (0x3U)              /*!< I2S TX left mixer source control (Bitfield-Mask: 0x3)      */

#define TX_MIXR_GAIN_DAC_LOOP_Pos           (7U)                /*!< I2S TXR mixer gain DAC_LOOP control (Bit 7)                */
#define TX_MIXR_GAIN_DAC_LOOP_Msk           (0x80U)             /*!< I2S TXR mixer gain DAC_LOOP control (Bitfield-Mask: 0x1)   */
#define TX_MIXR_GAIN_ADC_DO_Pos             (6U)                /*!< I2S TXR mixer gain ADC_DO control (Bit 6)                  */
#define TX_MIXR_GAIN_ADC_DO_Msk             (0x40U)             /*!< I2S TXR mixer gain ADC_DO control (Bitfield-Mask: 0x1)     */
#define TX_MIXR_SRC_DAC_LOOP_Pos            (5U)                /*!< I2S TXR mixer source DAC_LOOP control (Bit 5)              */
#define TX_MIXR_SRC_DAC_LOOP_Msk            (0x20U)             /*!< I2S TXR mixer source DAC_LOOP control (Bitfield-Mask: 0x1) */
#define TX_MIXR_SRC_ADC_DO_Pos              (4U)                /*!< I2S TXR mixer source ADC_DO control (Bit 4)                */
#define TX_MIXR_SRC_ADC_DO_Msk              (0x10U)             /*!< I2S TXR mixer source ADC_DO control (Bitfield-Mask: 0x1)   */
#define TX_MIXL_GAIN_DAC_DI_Pos             (3U)                /*!< I2S TXL mixer gain DAC_DI control (Bit 3)                  */
#define TX_MIXL_GAIN_DAC_DI_Msk             (0x8U)              /*!< I2S TXL mixer gain DAC_DI control (Bitfield-Mask: 0x1)     */
#define TX_MIXL_GAIN_ADC_DO_Pos             (2U)                /*!< I2S TXL mixer gain ADC_DO control (Bit 2)                  */
#define TX_MIXL_GAIN_ADC_DO_Msk             (0x4U)              /*!< I2S TXL mixer gain ADC_DO control (Bitfield-Mask: 0x1)     */
#define TX_MIXL_SRC_DAC_DI_Pos              (1U)                /*!< I2S TXL mixer source DAC_DI control (Bit 1)                */
#define TX_MIXL_SRC_DAC_DI_Msk              (0x2U)              /*!< I2S TXL mixer source DAC_DI control (Bitfield-Mask: 0x1)   */
#define TX_MIXL_SRC_ADC_DO_Pos              (0U)                /*!< I2S TXL mixer source ADC_DO control (Bit 0)                */
#define TX_MIXL_SRC_ADC_DO_Msk              (0x1U)              /*!< I2S TXL mixer source ADC_DO control (Bitfield-Mask: 0x1)   */

/* =====================================================  I2S_RXM_CTRL  ======================================================= */
#define RX_MIX_GAIN_Pos                     (2U)                /*!< I2S RX mixer gain control (Bit 2)                          */
#define RX_MIX_GAIN_Msk                     (0xcU)              /*!< I2S RX mixer gain control (Bitfield-Mask: 0x3)             */
#define RX_MIX_SRC_Pos                      (0U)                /*!< I2S RX mixer source control (Bit 0)                        */
#define RX_MIX_SRC_Msk                      (0x3U)              /*!< I2S RX mixer source control (Bitfield-Mask: 0x3)           */

#define RX_MIX_GAIN_RXR_Pos                 (3U)                /*!< I2S RX mixer gain RXR control (Bit 3)                      */
#define RX_MIX_GAIN_RXR_Msk                 (0x8U)              /*!< I2S RX mixer gain RXR control (Bitfield-Mask: 0x1)         */
#define RX_MIX_GAIN_RXL_Pos                 (2U)                /*!< I2S RX mixer gain RXL control (Bit 2)                      */
#define RX_MIX_GAIN_RXL_Msk                 (0x4U)              /*!< I2S RX mixer gain RXL control (Bitfield-Mask: 0x1)         */
#define RX_MIX_SRC_RXR_Pos                  (1U)                /*!< I2S RX mixer source RXR control (Bit 1)                    */
#define RX_MIX_SRC_RXR_Msk                  (0x2U)              /*!< I2S RX mixer source RXR control (Bitfield-Mask: 0x1)       */
#define RX_MIX_SRC_RXL_Pos                  (0U)                /*!< I2S RX mixer source RXL control (Bit 0)                    */
#define RX_MIX_SRC_RXL_Msk                  (0x1U)              /*!< I2S RX mixer source RXL control (Bitfield-Mask: 0x1)       */

/* =====================================================  I2S_PAD_CTRL  ======================================================= */
#define I2S_PAD_DRV_LEVEL_Pos               (6U)                /*!< I2S PAD driver level (Bit 6)                               */
#define I2S_PAD_DRV_LEVEL_Msk               (0xc0U)             /*!< I2S PAD driver level (Bitfield-Mask: 0x3)                  */
#define DMICDAT_PAD_DIS_Pos                 (5U)                /*!< DMICDAT pad disable (Bit 5)                                */
#define DMICDAT_PAD_DIS_Msk                 (0x20U)             /*!< DMICDAT pad disable (Bitfield-Mask: 0x1)                   */
#define SDOUT_PAD_DIS_Pos                   (4U)                /*!< SDOUT pad disable (Bit 4)                                  */
#define SDOUT_PAD_DIS_Msk                   (0x10U)             /*!< SDOUT pad disable (Bitfield-Mask: 0x1)                     */
#define SDIN_PAD_DIS_Pos                    (3U)                /*!< SDIN pad disable (Bit 3)                                   */
#define SDIN_PAD_DIS_Msk                    (0x8U)              /*!< SDIN pad disable (Bitfield-Mask: 0x1)                      */
#define LRCK_PAD_DIS_Pos                    (2U)                /*!< LRCK pad disable (Bit 2)                                   */
#define LRCK_PAD_DIS_Msk                    (0x4U)              /*!< LRCK pad disable (Bitfield-Mask: 0x1)                      */
#define BCLK_PAD_DIS_Pos                    (1U)                /*!< BCLK pad disable (Bit 1)                                   */
#define BCLK_PAD_DIS_Msk                    (0x2U)              /*!< BCLK pad disable (Bitfield-Mask: 0x1)                      */
#define MCLK_PAD_DIS_Pos                    (0U)                /*!< MCLK pad disable (Bit 0)                                   */
#define MCLK_PAD_DIS_Msk                    (0x1U)              /*!< MCLK pad disable (Bitfield-Mask: 0x1)                      */

#define I2S_ALL_PAD_DIS_Pos                 (0U)                /*!< I2S All pad disable (Bit 0)                                */
#define I2S_ALL_PAD_DIS_Msk                 (0x1fU)             /*!< I2S All pad disable (Bitfield-Mask: 0x1f)                  */

/* =====================================================  ADC_DIG_CTRL  ======================================================= */
#define PDM_IO_Pos                          (7U)                /*!< PDM IO multiplexing function select (Bit 7)                */
#define PDM_IO_Msk                          (0x80U)             /*!< PDM IO multiplexing function select (Bitfield-Mask: 0x1)   */
#define ADOUT_DLY_Pos                       (5U)                /*!< ADC Delay Function (Bit 5)                                 */
#define ADOUT_DLY_Msk                       (0x60U)             /*!< ADC Delay Function (Bitfield-Mask: 0x3)                    */
#define PDM_TIMING_Pos                      (4U)                /*!< PDM timing control (Bit 4)                                 */
#define PDM_TIMING_Msk                      (0x10U)             /*!< PDM timing control (Bitfield-Mask: 0x1)                    */
#define PDM_EN_Pos                          (3U)                /*!< PDM interface Enable (Bit 3)                               */
#define PDM_EN_Msk                          (0x8U)              /*!< PDM interface Enable (Bitfield-Mask: 0x1)                  */
#define ADC_HPF_EN_Pos                      (2U)                /*!< ADC digital HPF enable (Bit 2)                             */
#define ADC_HPF_EN_Msk                      (0x4U)              /*!< ADC digital HPF enable (Bitfield-Mask: 0x1)                */
#define ADC_DITHEN_Pos                      (1U)                /*!< ADC dithering enable (Bit 1)                               */
#define ADC_DITHEN_Msk                      (0x2U)              /*!< ADC dithering enable (Bitfield-Mask: 0x1)                  */
#define ADC_DIG_EN_Pos                      (0U)                /*!< ADC digital part enable (Bit 0)                            */
#define ADC_DIG_EN_Msk                      (0x1U)              /*!< ADC digital part enable (Bitfield-Mask: 0x1)               */

/* =====================================================  ADC_HPF_COEF  ======================================================= */
#define ADC_COEF_Pos                        (0U)                /*!< ADC HPF Coefficient (Bit 0)                                */
#define ADC_COEF_Msk                        (0x7U)              /*!< ADC HPF Coefficient (Bitfield-Mask: 0x7)                   */

/* =====================================================  ADC_DVC_CTRL  ======================================================= */
#define ADC_DIG_VOL_Pos                     (0U)                /*!< ADC Digital Volume Control (Bit 0)                         */
#define ADC_DIG_VOL_Msk                     (0xffU)             /*!< ADC Digital Volume Control (Bitfield-Mask: 0xff)           */

/* ======================================================  ADC_SMUTE  ========================================================= */
#define ADC_ZERO_CROSS_EN_Pos               (4U)                /*!< ADC Zero-cross enable (Bit 4)                              */
#define ADC_ZERO_CROSS_EN_Msk               (0x10U)             /*!< ADC Zero-cross enable (Bitfield-Mask: 0x1)                 */
#define ADC_SOFT_VOL_RATE_Pos               (1U)                /*!< ADC Volume ramp rate (Bit 1)                               */
#define ADC_SOFT_VOL_RATE_Msk               (0xeU)              /*!< ADC Volume ramp rate (Bitfield-Mask: 0x7)                  */
#define ADC_SOFT_VOL_EN_Pos                 (0U)                /*!< ADC Soft volume enable (Bit 0)                             */
#define ADC_SOFT_VOL_EN_Msk                 (0x1U)              /*!< ADC Soft volume enable (Bitfield-Mask: 0x1)                */

/* =====================================================  ADC_EQ_CTRL  ======================================================== */
#define ADC_EQ1_EN_Pos                      (0U)                /*!< ADC band-1 EQ enable (Bit 0)                               */
#define ADC_EQ1_EN_Msk                      (0x1U)              /*!< ADC band-1 EQ enable (Bitfield-Mask: 0x1)                  */

/* ====================================================  ADC_DRC_CTRL  ======================================================== */
#define ADC_DRC_SMTH_MODE_Pos               (2U)                /*!< ADC DRC smooth filter mode (Bit 2)                         */
#define ADC_DRC_SMTH_MODE_Msk               (0x4U)              /*!< ADC DRC smooth filter mode (Bitfield-Mask: 0x1)            */
#define ADC_DRC_SMTH_EN_Pos                 (1U)                /*!< ADC DRC smooth filter enable (Bit 1)                       */
#define ADC_DRC_SMTH_EN_Msk                 (0x2U)              /*!< ADC DRC smooth filter enable (Bitfield-Mask: 0x1)          */
#define ADC_DRC_EN_Pos                      (0U)                /*!< ADC DRC enable (Bit 0)                                     */
#define ADC_DRC_EN_Msk                      (0x1U)              /*!< ADC DRC enable (Bitfield-Mask: 0x1)                        */

/* ===================================================  ADC_PEAK_CTRL  ======================================================== */
#define ADC_DRC_PEAK_AT_Pos                 (4U)                /*!< ADC DRC peak filter attack time (Bit 4)                    */
#define ADC_DRC_PEAK_AT_Msk                 (0xf0U)             /*!< ADC DRC peak filter attack time (Bitfield-Mask: 0xf)       */
#define ADC_DRC_PEAK_RT_Pos                 (0U)                /*!< ADC DRC peak filter release time (Bit 0)                   */
#define ADC_DRC_PEAK_RT_Msk                 (0xfU)              /*!< ADC DRC peak filter release time (Bitfield-Mask: 0xf)      */

/* ====================================================  ADC_ATK_TIME  ======================================================== */
#define ADC_DRC_ATK_TIME_Pos                (4U)                /*!< ADC DRC attack time control (Bit 4)                        */
#define ADC_DRC_ATK_TIME_Msk                (0xf0U)             /*!< ADC DRC attack time control (Bitfield-Mask: 0xf)           */
#define ADC_DRC_DCY_TIME_Pos                (0U)                /*!< ADC DRC decay time control (Bit 0)                         */
#define ADC_DRC_DCY_TIME_Msk                (0xfU)              /*!< ADC DRC decay time control (Bitfield-Mask: 0xf)            */

/* ====================================================  ADC_DRC_THD1  ======================================================== */
#define ADC_DRC_LMT_THD_Pos                 (0U)                /*!< ADC DRC limiter threshold (Bit 0)                          */
#define ADC_DRC_LMT_THD_Msk                 (0x1fU)             /*!< ADC DRC limiter threshold (Bitfield-Mask: 0x1f)            */

/* ====================================================  ADC_DRC_THD2  ======================================================== */
#define ADC_DRC_CMP_THD_Pos                 (0U)                /*!< ADC DRC compressor threshold (Bit 0)                       */
#define ADC_DRC_CMP_THD_Msk                 (0x3fU)             /*!< ADC DRC compressor threshold (Bitfield-Mask: 0x3f)         */

/* ====================================================  ADC_DRC_THD3  ======================================================== */
#define ADC_DRC_EXP_THD_Pos                 (0U)                /*!< ADC DRC expander threshold (Bit 0)                         */
#define ADC_DRC_EXP_THD_Msk                 (0x3fU)             /*!< ADC DRC expander threshold (Bitfield-Mask: 0x3f)           */

/* ====================================================  ADC_DRC_THD4  ======================================================== */
#define ADC_DRC_NG_THD_Pos                  (0U)                /*!< ADC DRC noise gate threshold (Bit 0)                       */
#define ADC_DRC_NG_THD_Msk                  (0x3fU)             /*!< ADC DRC noise gate threshold (Bitfield-Mask: 0x3f)         */

/* ====================================================  ADC_DRC_SLP1  ======================================================== */
#define ADC_DRC_LMT_SLP_Pos                 (4U)                /*!< ADC DRC limiter slope (Bit 4)                              */
#define ADC_DRC_LMT_SLP_Msk                 (0x70U)             /*!< ADC DRC limiter slope (Bitfield-Mask: 0x7)                 */
#define ADC_DRC_CMP_SLP_Pos                 (0U)                /*!< ADC DRC compressor slope (Bit 0)                           */
#define ADC_DRC_CMP_SLP_Msk                 (0x7U)              /*!< ADC DRC compressor slope (Bitfield-Mask: 0x7)              */

/* ====================================================  ADC_DRC_SLP2  ======================================================== */
#define ADC_DRC_EXP_SLP_Pos                 (6U)                /*!< ADC DRC expander slope (Bit 6)                             */
#define ADC_DRC_EXP_SLP_Msk                 (0xc0U)             /*!< ADC DRC expander slope (Bitfield-Mask: 0x3)                */
#define ADC_DRC_NG_SLP_Pos                  (4U)                /*!< ADC DRC noise gate slope (Bit 4)                           */
#define ADC_DRC_NG_SLP_Msk                  (0x30U)             /*!< ADC DRC noise gate slope (Bitfield-Mask: 0x3)              */
#define ADC_DRC_GAIN_Pos                    (0U)                /*!< ADC DRC makeup gain (Bit 0)                                */
#define ADC_DRC_GAIN_Msk                    (0xfU)              /*!< ADC DRC makeup gain (Bitfield-Mask: 0xf)                   */

/* ====================================================  DAC_DIG_CTRL  ======================================================== */
#define DAC_DITHSEL_Pos                     (2U)                /*!< DAC dithering amplitude selection (Bit 2)                  */
#define DAC_DITHSEL_Msk                     (0xcU)              /*!< DAC dithering amplitude selection (Bitfield-Mask: 0x3)     */
#define DAC_DITHEN_Pos                      (1U)                /*!< DAC dithering enable (Bit 1)                               */
#define DAC_DITHEN_Msk                      (0x2U)              /*!< DAC dithering enable (Bitfield-Mask: 0x1)                  */
#define DAC_DIG_EN_Pos                      (0U)                /*!< DAC Digital part enable (Bit 0)                            */
#define DAC_DIG_EN_Msk                      (0x1U)              /*!< DAC Digital part enable (Bitfield-Mask: 0x1)               */

/* ====================================================  DAC_DVC_CTRL  ======================================================== */
#define DAC_DIG_VOL_Pos                     (0U)                /*!< DAC Digital Volume Control (Bit 0)                         */
#define DAC_DIG_VOL_Msk                     (0xffU)             /*!< DAC Digital Volume Control (Bitfield-Mask: 0xff)           */

/* =====================================================  DAC_SMUTE  ========================================================== */
#define DAC_AMUTE_EN_Pos                    (5U)                /*!< DAC Auto Mute Enable Control (Bit 5)                       */
#define DAC_AMUTE_EN_Msk                    (0x20U)             /*!< DAC Auto Mute Enable Control (Bitfield-Mask: 0x1)          */
#define DAC_ZERO_CROSS_EN_Pos               (4U)                /*!< DAC Zero-cross enable (Bit 4)                              */
#define DAC_ZERO_CROSS_EN_Msk               (0x10U)             /*!< DAC Zero-cross enable (Bitfield-Mask: 0x1)                 */
#define DAC_SOFT_VOL_RATE_Pos               (1U)                /*!< DAC volume ramp rate  (Bit 1)                              */
#define DAC_SOFT_VOL_RATE_Msk               (0xeU)              /*!< DAC volume ramp rate  (Bitfield-Mask: 0x7)                 */
#define DAC_SOFT_VOL_EN_Pos                 (0U)                /*!< DAC Soft volume enable (Bit 0)                             */
#define DAC_SOFT_VOL_EN_Msk                 (0x1U)              /*!< DAC Soft volume enable (Bitfield-Mask: 0x1)                */

/* ===================================================  DAC_DMIX_CTRL  ======================================================== */
#define DAC_DMX_GAIN_Pos                    (2U)                /*!< DAC digital mixer gain control (Bit 2)                     */
#define DAC_DMX_GAIN_Msk                    (0xcU)              /*!< DAC digital mixer gain control (Bitfield-Mask: 0x3)        */
#define DAC_DMX_SRC_Pos                     (0U)                /*!< DAC digital mixer source select (Bit 0)                    */
#define DAC_DMX_SRC_Msk                     (0x3U)              /*!< DAC digital mixer source select (Bitfield-Mask: 0x3)       */

#define DAC_DMX_GAIN_ADC_DO_Pos             (3U)                /*!< DAC digital mixer gain ADC_DO control (Bit 3)              */
#define DAC_DMX_GAIN_ADC_DO_Msk             (0x8U)              /*!< DAC digital mixer gain ADC_DO control (Bitfield-Mask: 0x1) */
#define DAC_DMX_GAIN_DAC_DI_Pos             (2U)                /*!< DAC digital mixer gain DAC_DI control (Bit 2)              */
#define DAC_DMX_GAIN_DAC_DI_Msk             (0x4U)              /*!< DAC digital mixer gain DAC_DI control (Bitfield-Mask: 0x1) */
#define DAC_DMX_SRC_ADC_DO_Pos              (1U)                /*!< DAC digital mixer source ADC_DO enable (Bit 1)             */
#define DAC_DMX_SRC_ADC_DO_Msk              (0x2U)              /*!< DAC digital mixer source ADC_DO enable (Bitfield-Mask: 0x1)*/
#define DAC_DMX_SRC_DAC_DI_Pos              (0U)                /*!< DAC digital mixer source DAC_DI enable (Bit 0)             */
#define DAC_DMX_SRC_DAC_DI_Msk              (0x1U)              /*!< DAC digital mixer source DAC_DI enable (Bitfield-Mask: 0x1)*/

/* ====================================================  DAC_EQ_CTRL  ========================================================= */
#define DAC_EQ3_EN_Pos                      (2U)                /*!< DAC band-3 EQ enable (Bit 2)                               */
#define DAC_EQ3_EN_Msk                      (0x4U)              /*!< DAC band-3 EQ enable (Bitfield-Mask: 0x1)                  */
#define DAC_EQ2_EN_Pos                      (1U)                /*!< DAC band-2 EQ enable (Bit 1)                               */
#define DAC_EQ2_EN_Msk                      (0x2U)              /*!< DAC band-2 EQ enable (Bitfield-Mask: 0x1)                  */
#define DAC_EQ1_EN_Pos                      (0U)                /*!< DAC band-1 EQ enable (Bit 0)                               */
#define DAC_EQ1_EN_Msk                      (0x1U)              /*!< DAC band-1 EQ enable (Bitfield-Mask: 0x1)                  */

/* ===================================================  DAC_DRC_CTRL  ========================================================= */
#define DAC_DRC_SMTH_MODE_Pos               (2U)                /*!< DAC DRC smooth filter mode (Bit 2)                         */
#define DAC_DRC_SMTH_MODE_Msk               (0x4U)              /*!< DAC DRC smooth filter mode (Bitfield-Mask: 0x1)            */
#define DAC_DRC_SMTH_EN_Pos                 (1U)                /*!< DAC DRC smooth filter enable (Bit 1)                       */
#define DAC_DRC_SMTH_EN_Msk                 (0x2U)              /*!< DAC DRC smooth filter enable (Bitfield-Mask: 0x1)          */
#define DAC_DRC_EN_Pos                      (0U)                /*!< DAC DRC enable (Bit 0)                                     */
#define DAC_DRC_EN_Msk                      (0x1U)              /*!< DAC DRC enable (Bitfield-Mask: 0x1)                        */

/* ===================================================  DAC_PEAK_CTRL  ======================================================== */
#define DAC_DRC_PEAK_AT_Pos                 (4U)                /*!< DAC DRC peak filter attack time (Bit 4)                    */
#define DAC_DRC_PEAK_AT_Msk                 (0xf0U)             /*!< DAC DRC peak filter attack time (Bitfield-Mask: 0xf)       */
#define DAC_DRC_PEAK_RT_Pos                 (0U)                /*!< DAC DRC peak filter release time (Bit 0)                   */
#define DAC_DRC_PEAK_RT_Msk                 (0xfU)              /*!< DAC DRC peak filter release time (Bitfield-Mask: 0xf)      */

/* ===================================================  DAC_ATK_TIME  ========================================================= */
#define DAC_DRC_ATK_TIME_Pos                (4U)                /*!< DAC DRC attack time control (Bit 4)                        */
#define DAC_DRC_ATK_TIME_Msk                (0xf0U)             /*!< DAC DRC attack time control (Bitfield-Mask: 0xf)           */
#define DAC_DRC_DCY_TIME_Pos                (0U)                /*!< DAC DRC decay time control (Bit 0)                         */
#define DAC_DRC_DCY_TIME_Msk                (0xfU)              /*!< DAC DRC decay time control (Bitfield-Mask: 0xf)            */

/* ===================================================  DAC_DRC_THD1  ========================================================= */
#define DAC_DRC_LMT_THD_Pos                 (0U)                /*!< DAC DRC limiter threshold (Bit 0)                          */
#define DAC_DRC_LMT_THD_Msk                 (0x1fU)             /*!< DAC DRC limiter threshold (Bitfield-Mask: 0x1f)            */

/* ===================================================  DAC_DRC_THD2  ========================================================= */
#define DAC_DRC_CMP_THD_Pos                 (0U)                /*!< DAC DRC compressor threshold (Bit 0)                       */
#define DAC_DRC_CMP_THD_Msk                 (0x3fU)             /*!< DAC DRC compressor threshold (Bitfield-Mask: 0x3f)         */

/* ===================================================  DAC_DRC_THD3  ========================================================= */
#define DAC_DRC_EXP_THD_Pos                 (0U)                /*!< DAC_DRC_EXP_THD (Bit 0)                                    */
#define DAC_DRC_EXP_THD_Msk                 (0x3fU)             /*!< DAC_DRC_EXP_THD (Bitfield-Mask: 0x3f)                      */

/* ===================================================  DAC_DRC_THD4  ========================================================= */
#define DAC_DRC_NG_THD_Pos                  (0U)                /*!< DAC DRC noise gate threshold (Bit 0)                       */
#define DAC_DRC_NG_THD_Msk                  (0x3fU)             /*!< DAC DRC noise gate threshold (Bitfield-Mask: 0x3f)         */

/* ===================================================  DAC_DRC_SLP1  ========================================================= */
#define DAC_DRC_LMT_SLP_Pos                 (4U)                /*!< DAC DRC limiter slope (Bit 4)                              */
#define DAC_DRC_LMT_SLP_Msk                 (0x70U)             /*!< DAC DRC limiter slope (Bitfield-Mask: 0x7)                 */
#define DAC_DRC_CMP_SLP_Pos                 (0U)                /*!< DAC DRC compressor slope (Bit 0)                           */
#define DAC_DRC_CMP_SLP_Msk                 (0x7U)              /*!< DAC DRC compressor slope (Bitfield-Mask: 0x7)              */

/* ===================================================  DAC_DRC_SLP2  ========================================================= */
#define DAC_DRC_EXP_SLP_Pos                 (6U)                /*!< DAC DRC expander slope (Bit 6)                             */
#define DAC_DRC_EXP_SLP_Msk                 (0xc0U)             /*!< DAC DRC expander slope (Bitfield-Mask: 0x3)                */
#define DAC_DRC_NG_SLP_Pos                  (4U)                /*!< DAC DRC noise gate slope (Bit 4)                           */
#define DAC_DRC_NG_SLP_Msk                  (0x30U)             /*!< DAC DRC noise gate slope (Bitfield-Mask: 0x3)              */
#define DAC_DRC_GAIN_Pos                    (0U)                /*!< DAC DRC makeup gain (Bit 0)                                */
#define DAC_DRC_GAIN_Msk                    (0xfU)              /*!< DAC DRC makeup gain (Bitfield-Mask: 0xf)                   */

/* ==================================================  ANA_ADC_CTRL1  ========================================================= */
#define ADC_PGA_GAIN_Pos                    (4U)                /*!< ADC PGA Gain control (Bit 4)                               */
#define ADC_PGA_GAIN_Msk                    (0xf0U)             /*!< ADC PGA Gain control (Bitfield-Mask: 0xf)                  */
#define ADC_CONV_EN_Pos                     (2U)                /*!< ADC SDM conversion start signal enable (Bit 2)             */
#define ADC_CONV_EN_Msk                     (0x4U)              /*!< ADC SDM conversion start signal enable (Bitfield-Mask: 0x1)*/
#define ADC_SDM_EN_Pos                      (1U)                /*!< ADC SDM related circuit enable (Bit 1)                     */
#define ADC_SDM_EN_Msk                      (0x2U)              /*!< ADC SDM related circuit enable (Bitfield-Mask: 0x1)        */
#define ADC_PGA_EN_Pos                      (0U)                /*!< ADC PGA related circuit enable (Bit 0)                     */
#define ADC_PGA_EN_Msk                      (0x1U)              /*!< ADC PGA related circuit enable (Bitfield-Mask: 0x1)        */

/* ==================================================  ANA_ADC_CTRL2  ========================================================= */
#define ADC_PGA_FAST_SET_Pos                (6U)                /*!< ADC PGA high-pass filter fast settling (Bit 6)             */
#define ADC_PGA_FAST_SET_Msk                (0x40U)             /*!< ADC PGA high-pass filter fast settling (Bitfield-Mask: 0x1)*/
#define ADC_PGA_OPTIMIZE_Pos                (0U)                /*!< ADC PGA optimize control (Bit 0)                           */
#define ADC_PGA_OPTIMIZE_Msk                (0x3fU)             /*!< ADC PGA optimize control (Bitfield-Mask: 0x3f)             */

/* ==================================================  ANA_ADC_CTRL3  ========================================================= */
#define ADC_SDM_OPTIMIZE_Pos                (0U)                /*!< ADC SDM chopper control (Bit 0)                            */
#define ADC_SDM_OPTIMIZE_Msk                (0xfU)              /*!< ADC SDM chopper control (Bitfield-Mask: 0xf)               */

/* ==================================================  ANA_DAC_CTRL1  ========================================================= */
#define DAC_OUT_MODE_Pos                    (5U)                /*!< DAC OUTPUT mode control (Bit 5)                            */
#define DAC_OUT_MODE_Msk                    (0x20U)             /*!< DAC OUTPUT mode control (Bitfield-Mask: 0x1)               */
#define DAC_OUT_SEL_DAC_Pos                 (3U)                /*!< DAC OUTPUT source select dac (Bit 3)                       */
#define DAC_OUT_SEL_DAC_Msk                 (0x8U)              /*!< DAC OUTPUT source select dac (Bitfield-Mask: 0x1)          */
#define DAC_OUT_SEL_MIC_Pos                 (2U)                /*!< DAC OUTPUT source select mic inp (Bit 2)                   */
#define DAC_OUT_SEL_MIC_Msk                 (0x4U)              /*!< DAC OUTPUT source select mic inp (Bitfield-Mask: 0x1)      */
#define DAC_OUT_EN_Pos                      (1U)                /*!< DAC OUTPUT STAGE enable (Bit 1)                            */
#define DAC_OUT_EN_Msk                      (0x2U)              /*!< DAC OUTPUT STAGE enable (Bitfield-Mask: 0x1)               */
#define DAC_DCT_EN_Pos                      (0U)                /*!< DAC DCT enable (Bit 0)                                     */
#define DAC_DCT_EN_Msk                      (0x1U)              /*!< DAC DCT enable (Bitfield-Mask: 0x1)                        */

/* ==================================================  ANA_DAC_CTRL2  ========================================================= */
#define DAC_AUTO_RAMP_Pos                   (7U)                /*!< DAC OUTPUT auto ramp up (Bit 7)                            */
#define DAC_AUTO_RAMP_Msk                   (0x80U)             /*!< DAC OUTPUT auto ramp up (Bitfield-Mask: 0x1)               */
#define DAC_CHARGING_TIME_Pos               (4U)                /*!< DAC OUTPUT charging time after ramp up (Bit 4)             */
#define DAC_CHARGING_TIME_Msk               (0x70U)             /*!< DAC OUTPUT charging time after ramp up (Bitfield-Mask: 0x7)*/
#define DAC_RAMP_TIME_Pos                   (1U)                /*!< DAC OUTPUT ramp time (Bit 1)                               */
#define DAC_RAMP_TIME_Msk                   (0xeU)              /*!< DAC OUTPUT ramp time (Bitfield-Mask: 0x7)                  */
#define DAC_RAMP_EN_Pos                     (0U)                /*!< DAC OUTPUT ramp up function enable (Bit 0)                 */
#define DAC_RAMP_EN_Msk                     (0x1U)              /*!< DAC OUTPUT ramp up function enable (Bitfield-Mask: 0x1)    */

/* ==================================================  ANA_DAC_CTRL3  ========================================================= */
#define DAC_OPOUT_OPTIMIZE_Pos              (0U)                /*!< DAC OPOUT optimize control (Bit 0)                         */
#define DAC_OPOUT_OPTIMIZE_Msk              (0xfU)              /*!< DAC OPOUT optimize control (Bitfield-Mask: 0xf)            */

/* ===================================================  ADDA_DEBUG  =========================================================== */
#define I2C_PAD_DRV_LEVEL_Pos               (6U)                /*!< I2C PAD driver level (Bit 6)                               */
#define I2C_PAD_DRV_LEVEL_Msk               (0xc0U)             /*!< I2C PAD driver level (Bitfield-Mask: 0x3)                  */
#define DAAD_LOOP_EN_Pos                    (4U)                /*!< DAC SDM data loop to ADC filter enable (Bit 4)             */
#define DAAD_LOOP_EN_Msk                    (0x10U)             /*!< DAC SDM data loop to ADC filter enable (Bitfield-Mask: 0x1)*/
#define DBUG_EN_Pos                         (3U)                /*!< ADC/DAC debug function enable (Bit 3)                      */
#define DBUG_EN_Msk                         (0x8U)              /*!< ADC/DAC debug function enable (Bitfield-Mask: 0x1)         */
#define DBUG_SEL_Pos                        (2U)                /*!< ADC/DAC debug function Select (Bit 2)                      */
#define DBUG_SEL_Msk                        (0x4U)              /*!< ADC/DAC debug function Select (Bitfield-Mask: 0x1)         */
#define DEBUG_PTN_Pos                       (0U)                /*!< ADC/DAC Debug Pattern Select (Bit 0)                       */
#define DEBUG_PTN_Msk                       (0x3U)              /*!< ADC/DAC Debug Pattern Select (Bitfield-Mask: 0x3)          */

/**
  * @}
  */

/**
  * @}
  */


/* Exported Macros -----------------------------------------------------------*/
/** @defgroup PT8311_Exported_Macros PT8311 Exported Macros
  * @brief    PT8311 Exported Macros
  * @{
  */

/** @defgroup PT8311_Exported_Macros_Group1 PT8311 Debug Macros Definition
  * @brief    PT8311 Debug Macros Definition
  * @{
  */

/**
  * @brief PT8311 debug print tag definition
  */
#define PT8311_DBG_TAG                                  "PT8311"

/**
  * @brief PT8311 debug print macro definition
  */
#define PT8311_DBG_PRINT(lvl, fmt, ...)                 PT8311_DBG_PRINT_USER("[" lvl "@" PT8311_DBG_TAG "] " fmt, ##__VA_ARGS__)

/**
  * @brief PT8311 Debug level print macro definition
  */
#if (PT8311_DBG_LVL >= PT8311_DBG_LOG)
#define PT8311_LOG_D(fmt, ...)                          PT8311_DBG_PRINT("D", fmt, ##__VA_ARGS__)
#else
#define PT8311_LOG_D(fmt, ...)
#endif

/**
  * @brief PT8311 Information level print macro definition
  */
#if (PT8311_DBG_LVL >= PT8311_DBG_INFO)
#define PT8311_LOG_I(fmt, ...)                          PT8311_DBG_PRINT("I", fmt, ##__VA_ARGS__)
#else
#define PT8311_LOG_I(fmt, ...)
#endif

/**
  * @brief PT8311 Warning level print macro definition
  */
#if (PT8311_DBG_LVL >= PT8311_DBG_WARNING)
#define PT8311_LOG_W(fmt, ...)                          PT8311_DBG_PRINT("W", fmt, ##__VA_ARGS__)
#else
#define PT8311_LOG_W(fmt, ...)
#endif

/**
  * @brief PT8311 Error level print macro definition
  */
#if (PT8311_DBG_LVL >= PT8311_DBG_ERROR)
#define PT8311_LOG_E(fmt, ...)                          PT8311_DBG_PRINT("E", fmt, ##__VA_ARGS__)
#else
#define PT8311_LOG_E(fmt, ...)
#endif

/**
  * @}
  */


/** @defgroup PT8311_Exported_Macros_Group2 PT8311 Register Bits Operation Definition
  * @brief    PT8311 Register Bits Operation Definition
  * @{
  */

/**
  * @brief  PT8311 Bit left shift definition
  * @param  pos left shift position
  * @return Bit left shift value
  */
#define PT8311_BIT(pos)                                         (1U << (pos))

/**
  * @brief  PT8311 Set bit definition
  * @param  i2c_addr PT8311 I2C address
  * @param  reg PT8311 register address
  * @param  bit Bit to set
  * @retval 0      Register set bit Success
  * @retval others Register set bit Failed
  */
#define PT8311_SET_BIT(i2c_addr, reg, bit)                      pt8311_update_bits(i2c_addr, reg, bit, bit)

/**
  * @brief  PT8311 Clear bit definition
  * @param  i2c_addr PT8311 I2C address
  * @param  reg PT8311 register address
  * @param  bit Bit to clear
  * @retval 0      Register clear bit Success
  * @retval others Register clear bit Failed
  */
#define PT8311_CLEAR_BIT(i2c_addr, reg, bit)                    pt8311_update_bits(i2c_addr, reg, bit, 0)

/**
  * @brief  PT8311 Read bit definition
  * @param  i2c_addr PT8311 I2C address
  * @param  reg PT8311 register address
  * @param  bit Bit to read
  * @return Read bit value
  */
#define PT8311_READ_BIT(i2c_addr, reg, bit)                     ((PT8311_I2C_READ_BYTE(i2c_addr, reg)) & (bit))

/**
  * @brief  PT8311 Read bit shift definition
  * @param  i2c_addr PT8311 I2C address
  * @param  reg PT8311 register address
  * @param  bit Bit to read
  * @param  shift right shift after read
  * @return Read bit shift value
  */
#define PT8311_READ_BIT_SHIFT(i2c_addr, reg, bit, shift)        (((PT8311_I2C_READ_BYTE(i2c_addr, reg)) & (bit)) >> (shift))

/**
  * @brief  PT8311 Read Register definition
  * @param  i2c_addr PT8311 I2C address
  * @param  reg PT8311 register address
  * @return Read Register value
  */
#define PT8311_READ_REG(i2c_addr, reg)                          PT8311_I2C_READ_BYTE(i2c_addr, reg)

/**
  * @brief  PT8311 Write Register definition
  * @param  i2c_addr PT8311 I2C address
  * @param  reg PT8311 register address
  * @param  val Register value
  * @retval 0      Register write Success
  * @retval others Register write Failed
  */
#define PT8311_WRITE_REG(i2c_addr, reg, val)                    PT8311_I2C_WRITE_BYTE(i2c_addr, reg, val)

/**
  * @brief  PT8311 Modify register definition
  * @param  i2c_addr PT8311 I2C address
  * @param  reg PT8311 register address
  * @param  clr_mask clear mask
  * @param  set_mask set mask
  * @retval 0      Register modify Success
  * @retval others Register modify Failed
  */
#define PT8311_MODIFY_REG(i2c_addr, reg, clr_mask, set_mask)    pt8311_update_bits(i2c_addr, reg, clr_mask, set_mask)


/**
  * @brief  Return the Minimum Value between Two Input Params
  * @param  a One Input Param
  * @param  b The other Input Param
  * @return the Minimum Value between Two Input Params
  */
#define PT8311_MIN(a, b)                                        (((a) < (b)) ? (a) : (b))

/**
  * @brief  Return the Maximum Value between Two Input Params
  * @param  a One Input Params
  * @param  b The other Input Params
  * @return the Maximum Value between Two Input Params
  */
#define PT8311_MAX(a, b)                                        (((a) > (b)) ? (a) : (b))

/**
  * @brief  To avoid gcc/g++ warnings
  * @param  X avoid warning param
  * @return None
  */
#define PT8311_UNUSED(X)                                        (void)(X)

/**
  * @brief  Macro for counting the element number of an array
  * @param  a Array to be Counted
  * @return size of Array
  */
#define PT8311_ARRAY_SIZE(a)                                    (sizeof((a)) / sizeof((a)[0]))

/**
  * @brief  PT8311 Function Alternative
  * @param  flag Condition Flag
  * @param  func_t True Funciton
  * @param  func_f False Function
  * @return None
  */
#define PT8311_FUNC_ALTER(flag, func_t, func_f) \
        do{                                     \
            if((flag)) {                        \
                func_t;                         \
            } else {                            \
                func_f;                         \
            }                                   \
        } while(0)

/**
  * @}
  */


/** @defgroup PT8311_Exported_Macros_Group3 PT8311 Register Read/Write Macros Definition
  * @brief    PT8311 Register Read/Write Macros Definition
  * @{
  */

/**
  * @brief  Chip Soft Reset
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_Chip_SoftReset(i2c_addr)               PT8311_SET_BIT(i2c_addr, SRST, SOFT_RESET_Msk)


/**
  * @brief  ALDO VREF filter resistor bypass Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ALDO_VrefFilResBypass_En(i2c_addr)     PT8311_SET_BIT(i2c_addr, PWR_CTRL1, POWER_VREF_BYP_Msk)

/**
  * @brief  ALDO VREF filter resistor bypass Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ALDO_VrefFilResBypass_Dis(i2c_addr)    PT8311_CLEAR_BIT(i2c_addr, PWR_CTRL1, POWER_VREF_BYP_Msk)

/**
  * @brief  ALDO Output Voltage set
  * @param  i2c_addr PT8311 I2C address
  * @param  vol ALDO output voltage, see @ref PT8311_ALDOVolETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ALDO_OutputVol_Set(i2c_addr, vol)      \
        PT8311_MODIFY_REG(i2c_addr, PWR_CTRL1, POWER_ALDO_VCTRL_Msk, (vol) << POWER_ALDO_VCTRL_Pos)

/**
  * @brief  ALDO Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ALDO_En(i2c_addr)                      PT8311_SET_BIT(i2c_addr, PWR_CTRL1, POWER_ALDO_EN_Msk)

/**
  * @brief  ALDO Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ALDO_Dis(i2c_addr)                     PT8311_CLEAR_BIT(i2c_addr, PWR_CTRL1, POWER_ALDO_EN_Msk)


/**
  * @brief  Micbias regulator bypass Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_MicbiasRegBypass_En(i2c_addr)          PT8311_SET_BIT(i2c_addr, PWR_CTRL2, POWER_MICBIAS_BYP_Msk)

/**
  * @brief  Micbias regulator bypass Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_MicbiasRegBypass_Dis(i2c_addr)         PT8311_CLEAR_BIT(i2c_addr, PWR_CTRL2, POWER_MICBIAS_BYP_Msk)

/**
  * @brief  Micbias chop frequency control @6144kHz
  * @param  i2c_addr PT8311 I2C address
  * @param  freq Micbias chop frequency, see @ref PT8311_MicbiasChopFreqETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_MicbiasChopFreq_Set(i2c_addr, freq)    \
        PT8311_MODIFY_REG(i2c_addr, PWR_CTRL2, POWER_MICBIAS_CHOP_FREQ_Msk, (freq) << POWER_MICBIAS_CHOP_FREQ_Pos)

/**
  * @brief  Micbias chop Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_MicbiasChop_En(i2c_addr)               PT8311_SET_BIT(i2c_addr, PWR_CTRL2, POWER_MICBIAS_CHOP_EN_Msk)

/**
  * @brief  Micbias chop Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_MicbiasChop_Dis(i2c_addr)              PT8311_CLEAR_BIT(i2c_addr, PWR_CTRL2, POWER_MICBIAS_CHOP_EN_Msk)

/**
  * @brief  Micbias output voltage Set
  * @param  i2c_addr PT8311 I2C address
  * @param  vol Micbias output voltage, see @ref PT8311_MicbiasVolETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_MicbiasOutputVol_Set(i2c_addr, vol)    \
        PT8311_MODIFY_REG(i2c_addr, PWR_CTRL2, POWER_MICBIAS_VCTRL_Msk, (vol) << POWER_MICBIAS_VCTRL_Pos)

/**
  * @brief  Micbias output voltage Set and Enable
  * @param  i2c_addr PT8311 I2C address
  * @param  vol Micbias output voltage, see @ref PT8311_MicbiasVolETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_MicbiasVol_Set_And_En(i2c_addr, vol)   PT8311_MODIFY_REG(i2c_addr, PWR_CTRL2,\
        POWER_MICBIAS_VCTRL_Msk | POWER_MICBIAS_EN_Msk, ((vol) << POWER_MICBIAS_VCTRL_Pos) | POWER_MICBIAS_EN_Msk)

/**
  * @brief  Micbias Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_Micbias_En(i2c_addr)                   PT8311_SET_BIT(i2c_addr, PWR_CTRL2, POWER_MICBIAS_EN_Msk)

/**
  * @brief  Micbias Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_Micbias_Dis(i2c_addr)                  PT8311_CLEAR_BIT(i2c_addr, PWR_CTRL2, POWER_MICBIAS_EN_Msk)


/**
  * @brief  PLL LDO output voltage Set
  * @param  i2c_addr PT8311 I2C address
  * @param  vol PLL LDO output voltage, see @ref PT8311_PLLLDOVolETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_PLLLDO_OutputVol_Set(i2c_addr, vol)    \
        PT8311_MODIFY_REG(i2c_addr, PLL_CTRL, PLL_LDO_VCTRL_Msk, (vol) << PLL_LDO_VCTRL_Pos)

/**
  * @brief  PLL LDO circuit Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_PLLLDO_En(i2c_addr)                    PT8311_SET_BIT(i2c_addr, PLL_CTRL, PLL_LDO_EN_Msk)

/**
  * @brief  PLL LDO circuit Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_PLLLDO_Dis(i2c_addr)                   PT8311_CLEAR_BIT(i2c_addr, PLL_CTRL, PLL_LDO_EN_Msk)

/**
  * @brief  PLL circuit Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_PLL_En(i2c_addr)                       PT8311_SET_BIT(i2c_addr, PLL_CTRL, PLL_EN_Msk)

/**
  * @brief  PLL circuit Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_PLL_Dis(i2c_addr)                      PT8311_CLEAR_BIT(i2c_addr, PLL_CTRL, PLL_EN_Msk)

/**
  * @brief  PLL LDO circuit and PLL circuit Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_PLL_LDOAndCirc_En(i2c_addr)            PT8311_SET_BIT(i2c_addr, PLL_CTRL, PLL_LDO_EN_Msk | PLL_EN_Msk)

/**
  * @brief  PLL LDO circuit and PLL circuit Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_PLL_LDOAndCirc_Dis(i2c_addr)           PT8311_CLEAR_BIT(i2c_addr, PLL_CTRL, PLL_LDO_EN_Msk | PLL_EN_Msk)


/**
  * @brief  PLL pre divider Set
  * @param  i2c_addr PT8311 I2C address
  * @param  pre_div PLL pre divider which range is [0x1, 0x20]
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_PLL_PreDiv_Set(i2c_addr, pre_div)      \
        PT8311_MODIFY_REG(i2c_addr, PLL_CFG1, PLL_P_DIV_Msk, ((pre_div) - 1) << PLL_P_DIV_Pos)


/**
  * @brief  PLL feedback divider Set
  * @param  i2c_addr PT8311 I2C address
  * @param  fb_div PLL feedback divider which range is [0x1, 0x100]
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_PLL_FeedbackDiv_Set(i2c_addr, fb_div)  \
        PT8311_MODIFY_REG(i2c_addr, PLL_CFG2, PLL_N_DIV_Msk, ((fb_div) - 1) << PLL_N_DIV_Pos)


/**
  * @brief  PLL post divider Set
  * @param  i2c_addr PT8311 I2C address
  * @param  post_div PLL post divider which range is [0x2, 0x1e]
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_PLL_PostDiv_Set(i2c_addr, post_div)    \
        PT8311_MODIFY_REG(i2c_addr, PLL_CFG3, PLL_M_DIV_Msk, ((post_div)  / 2) << PLL_M_DIV_Pos)


/**
  * @brief  PLL Clock Source Set
  * @param  i2c_addr PT8311 I2C address
  * @param  clk_src PLL Clock Source, see @ref PT8311_PLLCLKSrcETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_PLLCLK_Src_Set(i2c_addr, clk_src)      \
        PT8311_MODIFY_REG(i2c_addr, SYSCLK_CTRL, PLLCLK_SRC_Msk, (clk_src) << PLLCLK_SRC_Pos)

/**
  * @brief  PLLCLK Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_PLLCLK_En(i2c_addr)                    PT8311_SET_BIT(i2c_addr, SYSCLK_CTRL, PLLCLK_EN_Msk)

/**
  * @brief  PLLCLK Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_PLLCLK_Dis(i2c_addr)                   PT8311_CLEAR_BIT(i2c_addr, SYSCLK_CTRL, PLLCLK_EN_Msk)

/**
  * @brief  System Clock Source Set
  * @param  i2c_addr PT8311 I2C address
  * @param  clk_src System Clock Source, see @ref PT8311_SysclkSrcETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_SYSCLK_Src_Set(i2c_addr, clk_src)      \
        PT8311_MODIFY_REG(i2c_addr, SYSCLK_CTRL, SYSCLK_SRC_Msk, (clk_src) << SYSCLK_SRC_Pos)

/**
  * @brief  SYSCLK Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_SYSCLK_En(i2c_addr)                    PT8311_SET_BIT(i2c_addr, SYSCLK_CTRL, SYSCLK_EN_Msk)

/**
  * @brief  SYSCLK Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_SYSCLK_Dis(i2c_addr)                   PT8311_CLEAR_BIT(i2c_addr, SYSCLK_CTRL, SYSCLK_EN_Msk)


/**
  * @brief  DAC DSP(Biquad/DRC) clock Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DSP_Clk_En(i2c_addr)               PT8311_SET_BIT(i2c_addr, MODCLK_EN, DAC_DSP_CLK_EN_Msk)

/**
  * @brief  DAC DSP(Biquad/DRC) clock Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DSP_Clk_Dis(i2c_addr)              PT8311_CLEAR_BIT(i2c_addr, MODCLK_EN, DAC_DSP_CLK_EN_Msk)

/**
  * @brief  DAC Digital clock Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_Dig_Clk_En(i2c_addr)               PT8311_SET_BIT(i2c_addr, MODCLK_EN, DAC_DIGITAL_CLK_EN_Msk)

/**
  * @brief  DAC Digital clock Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_Dig_Clk_Dis(i2c_addr)              PT8311_CLEAR_BIT(i2c_addr, MODCLK_EN, DAC_DIGITAL_CLK_EN_Msk)

/**
  * @brief  DAC Analog clock Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_Analog_Clk_En(i2c_addr)            PT8311_SET_BIT(i2c_addr, MODCLK_EN, DAC_ANALOG_CLK_EN_Msk)

/**
  * @brief  DAC Analog clock Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_Analog_Clk_Dis(i2c_addr)           PT8311_CLEAR_BIT(i2c_addr, MODCLK_EN, DAC_ANALOG_CLK_EN_Msk)

/**
  * @brief  DAC Path clock Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_Path_Clk_En(i2c_addr)              PT8311_SET_BIT(i2c_addr, MODCLK_EN, DAC_PATH_CLK_EN_Msk)

/**
  * @brief  DAC Path clock Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_Path_Clk_Dis(i2c_addr)             PT8311_CLEAR_BIT(i2c_addr, MODCLK_EN, DAC_PATH_CLK_EN_Msk)

/**
  * @brief  DAC Module clock all Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_ModClkAll_En(i2c_addr)             PT8311_SET_BIT(i2c_addr, MODCLK_EN, DAC_MODULE_CLK_ALL_EN_Msk)

/**
  * @brief  DAC Module clock all Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_ModClkAll_Dis(i2c_addr)            PT8311_CLEAR_BIT(i2c_addr, MODCLK_EN, DAC_MODULE_CLK_ALL_EN_Msk)

/**
  * @brief  ADC DSP(Biquad/DRC) clock Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DSP_Clk_En(i2c_addr)               PT8311_SET_BIT(i2c_addr, MODCLK_EN, ADC_DSP_CLK_EN_Msk)

/**
  * @brief  ADC DSP(Biquad/DRC) clock Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DSP_Clk_Dis(i2c_addr)              PT8311_CLEAR_BIT(i2c_addr, MODCLK_EN, ADC_DSP_CLK_EN_Msk)

/**
  * @brief  ADC Digital clock Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_Dig_Clk_En(i2c_addr)               PT8311_SET_BIT(i2c_addr, MODCLK_EN, ADC_DIGITAL_CLK_EN_Msk)

/**
  * @brief  ADC Digital clock Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_Dig_Clk_Dis(i2c_addr)              PT8311_CLEAR_BIT(i2c_addr, MODCLK_EN, ADC_DIGITAL_CLK_EN_Msk)

/**
  * @brief  ADC Analog clock Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_Analog_Clk_En(i2c_addr)            PT8311_SET_BIT(i2c_addr, MODCLK_EN, ADC_ANALOG_CLK_EN_Msk)

/**
  * @brief  ADC Analog clock Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_Analog_Clk_Dis(i2c_addr)           PT8311_CLEAR_BIT(i2c_addr, MODCLK_EN, ADC_ANALOG_CLK_EN_Msk)

/**
  * @brief  ADC Path clock Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_Path_Clk_En(i2c_addr)              PT8311_SET_BIT(i2c_addr, MODCLK_EN, ADC_PATH_CLK_EN_Msk)

/**
  * @brief  ADC Path clock Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_Path_Clk_Dis(i2c_addr)             PT8311_CLEAR_BIT(i2c_addr, MODCLK_EN, ADC_PATH_CLK_EN_Msk)

/**
  * @brief  ADC Module clock all Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_ModClkAll_En(i2c_addr)             PT8311_SET_BIT(i2c_addr, MODCLK_EN, ADC_MODULE_CLK_ALL_EN_Msk)

/**
  * @brief  ADC Module clock all Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_ModClkAll_Dis(i2c_addr)            PT8311_CLEAR_BIT(i2c_addr, MODCLK_EN, ADC_MODULE_CLK_ALL_EN_Msk)


/**
  * @brief  ADC Sample Rate Set
  * @param  i2c_addr PT8311 I2C address
  * @param  rate ADC Sample Rate, see @ref PT8311_ADDA_SmpRateETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_SampleRate_Set(i2c_addr, rate)     PT8311_MODIFY_REG(i2c_addr, ADDA_FS, ADC_FS_Msk, (rate) << ADC_FS_Pos)

/**
  * @brief  DAC Sample Rate Set
  * @param  i2c_addr PT8311 I2C address
  * @param  rate DAC Sample Rate, see @ref PT8311_ADDA_SmpRateETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_SampleRate_Set(i2c_addr, rate)     PT8311_MODIFY_REG(i2c_addr, ADDA_FS, DAC_FS_Msk, (rate) << DAC_FS_Pos)

/**
  * @brief  ADC and DAC Sample Rate Set
  * @param  i2c_addr PT8311 I2C address
  * @param  adc_rate ADC Sample Rate, see @ref PT8311_ADDA_SmpRateETypeDef
  * @param  dac_rate DAC Sample Rate, see @ref PT8311_ADDA_SmpRateETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADDA_SampleRate_Set(i2c_addr, adc_rate, dac_rate)  \
        PT8311_MODIFY_REG(i2c_addr, ADDA_FS, ADC_FS_Msk | DAC_FS_Msk, ((adc_rate) << ADC_FS_Pos) | ((dac_rate) << DAC_FS_Pos))


/**
  * @brief  ADC Oversampling Ratio Set
  * @param  i2c_addr PT8311 I2C address
  * @param  ratio ADC Oversampling Ratio, see @ref PT8311_ADDA_OSRETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_OverSampleRatio_Set(i2c_addr, ratio)   \
        PT8311_MODIFY_REG(i2c_addr, ADC_CLK_CTRL, ADC_OSR_Msk, (ratio) << ADC_OSR_Pos)

/**
  * @brief  ADC Oversampling CLK (ADC_DCLK) divide ratio Set
  * @param  i2c_addr PT8311 I2C address
  * @param  div ADC_DCLK divide ratio, see @ref PT8311_ADDA_DCLKDivETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DCLK_Div_Set(i2c_addr, div)        PT8311_MODIFY_REG(i2c_addr, ADC_CLK_CTRL, ADC_DIV_Msk, (div) << ADC_DIV_Pos)

/**
  * @brief  ADC Oversampling Ratio and Clock divide Set
  * @param  i2c_addr PT8311 I2C address
  * @param  ratio ADC Oversampling Ratio, see @ref PT8311_ADDA_OSRETypeDef
  * @param  div ADC_DCLK divide ratio, see @ref PT8311_ADDA_DCLKDivETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_OsrClkDiv_Set(i2c_addr, ratio, div)    \
        PT8311_WRITE_REG(i2c_addr, ADC_CLK_CTRL, ((ratio) << ADC_OSR_Pos) | ((div) << ADC_DIV_Pos))


/**
  * @brief  DAC Oversampling Ratio Set
  * @param  i2c_addr PT8311 I2C address
  * @param  ratio DAC Oversampling Ratio, see @ref PT8311_ADDA_OSRETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_OverSampleRatio_Set(i2c_addr, ratio)   \
        PT8311_MODIFY_REG(i2c_addr, DAC_CLK_CTRL, DAC_OSR_Msk, (ratio) << DAC_OSR_Pos)

/**
  * @brief  DAC Oversampling CLK (DAC_DCLK) divide ratio Set
  * @param  i2c_addr PT8311 I2C address
  * @param  div DAC_DCLK divide ratio, see @ref PT8311_ADDA_DCLKDivETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DCLK_Div_Set(i2c_addr, div)        PT8311_MODIFY_REG(i2c_addr, DAC_CLK_CTRL, DAC_DIV_Msk, (div) << DAC_DIV_Pos)

/**
  * @brief  DAC Oversampling Ratio and Clock divide Set
  * @param  i2c_addr PT8311 I2C address
  * @param  ratio DAC Oversampling Ratio, see @ref PT8311_ADDA_OSRETypeDef
  * @param  div DAC_DCLK divide ratio, see @ref PT8311_ADDA_DCLKDivETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_OsrClkDiv_Set(i2c_addr, ratio, div)    \
        PT8311_WRITE_REG(i2c_addr, DAC_CLK_CTRL, ((ratio) << DAC_OSR_Pos) | ((div) << DAC_DIV_Pos))


/**
  * @brief  MCLK IO Pin Function Set
  * @param  i2c_addr PT8311 I2C address
  * @param  func MCLK IO Pin Function, see @ref PT8311_MclkIOPinFuncETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_MCLK_IO_PinFunc_Set(i2c_addr, func)    PT8311_MODIFY_REG(i2c_addr, I2S_CTRL, MCLK_IO_Msk, (func) << MCLK_IO_Pos)

/**
  * @brief  I2S Role Set
  * @param  i2c_addr PT8311 I2C address
  * @param  role I2S Role, see @ref PT8311_I2SRoleETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Role_Set(i2c_addr, role)           PT8311_MODIFY_REG(i2c_addr, I2S_CTRL, I2S_ROLE_Msk, (role) << I2S_ROLE_Pos)

/**
  * @brief  I2S Transmitter Block Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Tx_En(i2c_addr)                    PT8311_SET_BIT(i2c_addr, I2S_CTRL, I2S_TXEN_Msk)

/**
  * @brief  I2S Transmitter Block Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Tx_Dis(i2c_addr)                   PT8311_CLEAR_BIT(i2c_addr, I2S_CTRL, I2S_TXEN_Msk)

/**
  * @brief  I2S Receiver Block Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Rx_En(i2c_addr)                    PT8311_SET_BIT(i2c_addr, I2S_CTRL, I2S_RXEN_Msk)

/**
  * @brief  I2S Receiver Block Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Rx_Dis(i2c_addr)                   PT8311_CLEAR_BIT(i2c_addr, I2S_CTRL, I2S_RXEN_Msk)

/**
  * @brief  I2S Audio Interface Global Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Global_En(i2c_addr)                PT8311_SET_BIT(i2c_addr, I2S_CTRL, I2S_EN_Msk)

/**
  * @brief  I2S Audio Interface Global Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Global_Dis(i2c_addr)               PT8311_CLEAR_BIT(i2c_addr, I2S_CTRL, I2S_EN_Msk)


/**
  * @brief  I2S BCLK Divide from SYSCLK Set
  * @param  i2c_addr PT8311 I2C address
  * @param  div I2S BCLK Divide, see @ref PT8311_I2S_BCLKDivETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_BCLK_Div_Set(i2c_addr, div)        PT8311_WRITE_REG(i2c_addr, I2S_FMT1, div)


/**
  * @brief  I2S LRCK Divide from BCLK Set
  * @param  i2c_addr PT8311 I2C address
  * @param  div I2S LRCK Divide from BCLK
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_LRCK_Div_Set(i2c_addr, div)        PT8311_WRITE_REG(i2c_addr, I2S_FMT2, ((div) - 1))


/**
  * @brief  I2S BCLK Polarity Normal
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_BCLK_Pol_Normal(i2c_addr)          PT8311_CLEAR_BIT(i2c_addr, I2S_FMT3, BCLK_POLARITY_Msk)

/**
  * @brief  I2S BCLK Polarity Invert
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_BCLK_Pol_Invert(i2c_addr)          PT8311_SET_BIT(i2c_addr, I2S_FMT3, BCLK_POLARITY_Msk)

/**
  * @brief  I2S LRCK Polarity Normal
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_LRCK_Pol_Normal(i2c_addr)          PT8311_CLEAR_BIT(i2c_addr, I2S_FMT3, LRCK_POLARITY_Msk)

/**
  * @brief  I2S LRCK Polarity Invert
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_LRCK_Pol_Invert(i2c_addr)          PT8311_SET_BIT(i2c_addr, I2S_FMT3, LRCK_POLARITY_Msk)

/**
  * @brief  I2S Format Set
  * @param  i2c_addr PT8311 I2C address
  * @param  fmt I2S Format, see @ref PT8311_I2S_FmtETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Format_Set(i2c_addr, fmt)          PT8311_MODIFY_REG(i2c_addr, I2S_FMT3, MODE_SEL_Msk, (fmt) << MODE_SEL_Pos)

/**
  * @brief  I2S Word Length Set
  * @param  i2c_addr PT8311 I2C address
  * @param  len I2S Word Length, see @ref PT8311_I2S_WordLenETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_WordLen_Set(i2c_addr, len)         PT8311_MODIFY_REG(i2c_addr, I2S_FMT3, WL_Msk, (len) << WL_Pos)

/**
  * @brief  I2S Tx normal mode for the last half cycle of BCLK in the word
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Tx_LastWordState_Normal(i2c_addr)  PT8311_CLEAR_BIT(i2c_addr, I2S_FMT3, TX_WORD_HIZ_Msk)

/**
  * @brief  I2S Tx turn to hi-z state for the last half cycle of BCLK in the word
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Tx_LastWordState_HiZ(i2c_addr)     PT8311_SET_BIT(i2c_addr, I2S_FMT3, TX_WORD_HIZ_Msk)

/**
  * @brief  I2S Tx transfer level 0 when not transferring word
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Tx_IdleState_Zero(i2c_addr)        PT8311_CLEAR_BIT(i2c_addr, I2S_FMT3, TX_STATE_Msk)

/**
  * @brief  I2S Tx turn to hi-z state when not transferring word
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Tx_IdleState_HiZ(i2c_addr)         PT8311_SET_BIT(i2c_addr, I2S_FMT3, TX_STATE_Msk)

/**
  * @brief  I2S FMT3 Register Config
  * @param  i2c_addr PT8311 I2C address
  * @param  bclk_inv BCLK invert flag
  * @param  lrck_inv LRCK invert flag
  * @param  fmt I2S Format, see @ref PT8311_I2S_FmtETypeDef
  * @param  len I2S Word Length, see @ref PT8311_I2S_WordLenETypeDef
  * @param  tx_word_hz tx word HiZ flag
  * @param  tx_state_hz tx state HiZ flag
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_FMT3_Reg_Cfg(i2c_addr, bclk_inv, lrck_inv, fmt, len, tx_word_hz, tx_state_hz)  \
        PT8311_WRITE_REG(i2c_addr, I2S_FMT3, (!!(bclk_inv) << BCLK_POLARITY_Pos) | (!!(lrck_inv) << LRCK_POLARITY_Pos) |\
        ((fmt) << MODE_SEL_Pos) | ((len) << WL_Pos) | (!!(tx_word_hz) << TX_WORD_HIZ_Pos) | (!!(tx_state_hz) << TX_STATE_Pos))


/**
  * @brief  I2S Tx Slot 1 Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Tx_Slot1_En(i2c_addr)              PT8311_SET_BIT(i2c_addr,  I2S_TX_CTRL, TX_SLOT1_EN_Msk)

/**
  * @brief  I2S Tx Slot 1 Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Tx_Slot1_Dis(i2c_addr)             PT8311_CLEAR_BIT(i2c_addr,  I2S_TX_CTRL, TX_SLOT1_EN_Msk)

/**
  * @brief  I2S Tx Slot 0 Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Tx_Slot0_En(i2c_addr)              PT8311_SET_BIT(i2c_addr,  I2S_TX_CTRL, TX_SLOT0_EN_Msk)

/**
  * @brief  I2S Tx Slot 0 Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Tx_Slot0_Dis(i2c_addr)             PT8311_CLEAR_BIT(i2c_addr,  I2S_TX_CTRL, TX_SLOT0_EN_Msk)

/**
  * @brief  I2S Tx Slot 0&1 Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Tx_Slot0And1_En(i2c_addr)          PT8311_SET_BIT(i2c_addr,  I2S_TX_CTRL, TX_SLOT_EN_Msk)

/**
  * @brief  I2S Tx Slot 0&1 Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Tx_Slot0And1_Dis(i2c_addr)         PT8311_CLEAR_BIT(i2c_addr,  I2S_TX_CTRL, TX_SLOT_EN_Msk)

/**
  * @brief  I2S Tx Slot Enable Set
  * @param  i2c_addr PT8311 I2C address
  * @param  tx_en I2S Tx Slot Enable value
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Tx_SlotEn_Set(i2c_addr, tx_en)     \
        PT8311_MODIFY_REG(i2c_addr,  I2S_TX_CTRL, TX_SLOT_EN_Msk, (tx_en) << TX_SLOT_EN_Pos)

/**
  * @brief  I2S Tx Slot 1 Mapping Set
  * @param  i2c_addr PT8311 I2C address
  * @param  map I2S Tx Slot 1 Mapping, see @ref PT8311_I2S_TxSlotMapETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Tx_Slot1Map_Set(i2c_addr, map)     \
        PT8311_MODIFY_REG(i2c_addr,  I2S_TX_CTRL, TX_SLOT1_MAP_Msk, (map) << TX_SLOT1_MAP_Pos)

/**
  * @brief  I2S Tx Slot 0 Mapping Set
  * @param  i2c_addr PT8311 I2C address
  * @param  map I2S Tx Slot 0 Mapping, see @ref PT8311_I2S_TxSlotMapETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Tx_Slot0Map_Set(i2c_addr, map)     \
        PT8311_MODIFY_REG(i2c_addr,  I2S_TX_CTRL, TX_SLOT0_MAP_Msk, (map) << TX_SLOT0_MAP_Pos)

/**
  * @brief  I2S Tx Slot numbers Set
  * @param  i2c_addr PT8311 I2C address
  * @param  num I2S Tx Slot numbers range in [1, 2]
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Tx_SlotNums_Set(i2c_addr, num)     \
        PT8311_MODIFY_REG(i2c_addr,  I2S_TX_CTRL, TX_SLOT_SEL_Msk, ((num) - 1) << TX_SLOT_SEL_Pos)


/**
  * @brief  I2S Rx Right channel Mapping Set
  * @param  i2c_addr PT8311 I2C address
  * @param  map I2S Rx Right channel Mapping slot, see @ref PT8311_I2S_RxChMapETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Rx_RightMap_Set(i2c_addr, map)     PT8311_MODIFY_REG(i2c_addr, I2S_RX_CTRL, RXR_MAP_Msk, (map) << RXR_MAP_Pos)

/**
  * @brief  I2S Rx Left channel Mapping Set
  * @param  i2c_addr PT8311 I2C address
  * @param  map I2S Rx Left channel Mapping slot, see @ref PT8311_I2S_RxChMapETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Rx_LeftMap_Set(i2c_addr, map)      PT8311_MODIFY_REG(i2c_addr, I2S_RX_CTRL, RXL_MAP_Msk, (map) << RXL_MAP_Pos)

/**
  * @brief  I2S Rx Slot numbers Set
  * @param  i2c_addr PT8311 I2C address
  * @param  num I2S Rx Slot numbers range in [1, 2]
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Rx_SlotNums_Set(i2c_addr, num)     \
        PT8311_MODIFY_REG(i2c_addr,  I2S_RX_CTRL, RX_SLOT_SEL_Msk, ((num) - 1) << RX_SLOT_SEL_Pos)


/**
  * @brief  I2S Tx Right Mixer Gain DAC_LOOP 0dB
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_TxR_MixerGainDACLOOP_0dB(i2c_addr)         PT8311_CLEAR_BIT(i2c_addr, I2S_TXM_CTRL, TX_MIXR_GAIN_DAC_LOOP_Msk)

/**
  * @brief  I2S Tx Right Mixer Gain DAC_LOOP -6dB
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_TxR_MixerGainDACLOOP_Minus6dB(i2c_addr)    PT8311_SET_BIT(i2c_addr, I2S_TXM_CTRL, TX_MIXR_GAIN_DAC_LOOP_Msk)

/**
  * @brief  I2S Tx Right Mixer Gain ADC_DO 0dB
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_TxR_MixerGainADCDO_0dB(i2c_addr)           PT8311_CLEAR_BIT(i2c_addr, I2S_TXM_CTRL, TX_MIXR_GAIN_ADC_DO_Msk)

/**
  * @brief  I2S Tx Right Mixer Gain ADC_DO -6dB
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_TxR_MixerGainADCDO_Minus6dB(i2c_addr)      PT8311_SET_BIT(i2c_addr, I2S_TXM_CTRL, TX_MIXR_GAIN_ADC_DO_Msk)

/**
  * @brief  I2S Tx Right Mixer Source DAC_LOOP Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_TxR_MixerSrcDACLOOP_En(i2c_addr)           PT8311_SET_BIT(i2c_addr, I2S_TXM_CTRL, TX_MIXR_SRC_DAC_LOOP_Msk)

/**
  * @brief  I2S Tx Right Mixer Source DAC_LOOP Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_TxR_MixerSrcDACLOOP_Dis(i2c_addr)          PT8311_CLEAR_BIT(i2c_addr, I2S_TXM_CTRL, TX_MIXR_SRC_DAC_LOOP_Msk)

/**
  * @brief  I2S Tx Right Mixer Source ADC_DO Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_TxR_MixerSrcADCDO_En(i2c_addr)             PT8311_SET_BIT(i2c_addr, I2S_TXM_CTRL, TX_MIXR_SRC_ADC_DO_Msk)

/**
  * @brief  I2S Tx Right Mixer Source ADC_DO Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_TxR_MixerSrcADCDO_Dis(i2c_addr)            PT8311_CLEAR_BIT(i2c_addr, I2S_TXM_CTRL, TX_MIXR_SRC_ADC_DO_Msk)

/**
  * @brief  I2S Tx Left Mixer Gain DAC_DI 0dB
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_TxL_MixerGainDACDI_0dB(i2c_addr)           PT8311_CLEAR_BIT(i2c_addr, I2S_TXM_CTRL, TX_MIXL_GAIN_DAC_DI_Msk)

/**
  * @brief  I2S Tx Left Mixer Gain DAC_DI -6dB
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_TxL_MixerGainDACDI_Minus6dB(i2c_addr)      PT8311_SET_BIT(i2c_addr, I2S_TXM_CTRL, TX_MIXL_GAIN_DAC_DI_Msk)

/**
  * @brief  I2S Tx Left Mixer Gain ADC_DO 0dB
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_TxL_MixerGainADCDO_0dB(i2c_addr)           PT8311_CLEAR_BIT(i2c_addr, I2S_TXM_CTRL, TX_MIXL_GAIN_ADC_DO_Msk)

/**
  * @brief  I2S Tx Left Mixer Gain ADC_DO -6dB
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_TxL_MixerGainADCDO_Minus6dB(i2c_addr)      PT8311_SET_BIT(i2c_addr, I2S_TXM_CTRL, TX_MIXL_GAIN_ADC_DO_Msk)

/**
  * @brief  I2S Tx Left Mixer Source DAC_DI Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_TxL_MixerSrcDACDI_En(i2c_addr)             PT8311_SET_BIT(i2c_addr, I2S_TXM_CTRL, TX_MIXL_SRC_DAC_DI_Msk)

/**
  * @brief  I2S Tx Left Mixer Source DAC_DI Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_TxL_MixerSrcDACDI_Dis(i2c_addr)            PT8311_CLEAR_BIT(i2c_addr, I2S_TXM_CTRL, TX_MIXL_SRC_DAC_DI_Msk)

/**
  * @brief  I2S Tx Left Mixer Source ADC_DO Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_TxL_MixerSrcADCDO_En(i2c_addr)             PT8311_SET_BIT(i2c_addr, I2S_TXM_CTRL, TX_MIXL_SRC_ADC_DO_Msk)

/**
  * @brief  I2S Tx Left Mixer Source ADC_DO Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_TxL_MixerSrcADCDO_Dis(i2c_addr)            PT8311_CLEAR_BIT(i2c_addr, I2S_TXM_CTRL, TX_MIXL_SRC_ADC_DO_Msk)


/**
  * @brief  I2S Rx Mixer Gain RxR 0dB
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Rx_MixerGainRxR_0dB(i2c_addr)      PT8311_CLEAR_BIT(i2c_addr, I2S_RXM_CTRL, RX_MIX_GAIN_RXR_Msk)

/**
  * @brief  I2S Rx Mixer Gain RxR -6dB
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Rx_MixerGainRxR_Minus6dB(i2c_addr) PT8311_SET_BIT(i2c_addr, I2S_RXM_CTRL, RX_MIX_GAIN_RXR_Msk)

/**
  * @brief  I2S Rx Mixer Gain RxL 0dB
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Rx_MixerGainRxL_0dB(i2c_addr)      PT8311_CLEAR_BIT(i2c_addr, I2S_RXM_CTRL, RX_MIX_GAIN_RXL_Msk)

/**
  * @brief  I2S Rx Mixer Gain RxL -6dB
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Rx_MixerGainRxL_Minus6dB(i2c_addr) PT8311_SET_BIT(i2c_addr, I2S_RXM_CTRL, RX_MIX_GAIN_RXL_Msk)

/**
  * @brief  I2S Rx Mixer Source RxR Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Rx_MixerSrcRxR_En(i2c_addr)        PT8311_SET_BIT(i2c_addr, I2S_RXM_CTRL, RX_MIX_SRC_RXR_Msk)

/**
  * @brief  I2S Rx Mixer Source RxR Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Rx_MixerSrcRxR_Dis(i2c_addr)       PT8311_CLEAR_BIT(i2c_addr, I2S_RXM_CTRL, RX_MIX_SRC_RXR_Msk)

/**
  * @brief  I2S Rx Mixer Source RxL Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Rx_MixerSrcRxL_En(i2c_addr)        PT8311_SET_BIT(i2c_addr, I2S_RXM_CTRL, RX_MIX_SRC_RXL_Msk)

/**
  * @brief  I2S Rx Mixer Source RxL Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Rx_MixerSrcRxL_Dis(i2c_addr)       PT8311_CLEAR_BIT(i2c_addr, I2S_RXM_CTRL, RX_MIX_SRC_RXL_Msk)

/**
  * @brief  I2S Rx Mixer Source All Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Rx_MixerSrc_All_En(i2c_addr)       PT8311_SET_BIT(i2c_addr, I2S_RXM_CTRL, RX_MIX_SRC_Msk)

/**
  * @brief  I2S Rx Mixer Source All Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Rx_MixerSrc_All_Dis(i2c_addr)      PT8311_CLEAR_BIT(i2c_addr, I2S_RXM_CTRL, RX_MIX_SRC_Msk)

/**
  * @brief  I2S Rx Mixer Source Enable Set
  * @param  i2c_addr PT8311 I2C address
  * @param  rx_en I2S Rx Mixer Source Enable value
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_Rx_MixerSrcEn_Set(i2c_addr, rx_en) \
        PT8311_MODIFY_REG(i2c_addr, I2S_RXM_CTRL, RX_MIX_SRC_Msk, (rx_en) << RX_MIX_SRC_Pos)


/**
  * @brief  I2S Pad Driver Level Set
  * @param  i2c_addr PT8311 I2C address
  * @param  lvl I2S Pad Driver Level, see @ref PT8311_PadDrvLvlETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_PadDrvLvl_Set(i2c_addr, lvl)       \
        PT8311_MODIFY_REG(i2c_addr, I2S_PAD_CTRL, I2S_PAD_DRV_LEVEL_Msk, (lvl) << I2S_PAD_DRV_LEVEL_Pos)

/**
  * @brief  DMICDAT Pad Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DMICDAT_Pad_En(i2c_addr)               PT8311_CLEAR_BIT(i2c_addr, I2S_PAD_CTRL, DMICDAT_PAD_DIS_Msk)

/**
  * @brief  DMICDAT Pad Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DMICDAT_Pad_Dis(i2c_addr)              PT8311_SET_BIT(i2c_addr, I2S_PAD_CTRL, DMICDAT_PAD_DIS_Msk)

/**
  * @brief  I2S SDOUT Pad Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_SDOUT_Pad_En(i2c_addr)             PT8311_CLEAR_BIT(i2c_addr, I2S_PAD_CTRL, SDOUT_PAD_DIS_Msk)

/**
  * @brief  I2S SDOUT Pad Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_SDOUT_Pad_Dis(i2c_addr)            PT8311_SET_BIT(i2c_addr, I2S_PAD_CTRL, SDOUT_PAD_DIS_Msk)

/**
  * @brief  I2S SDIN Pad Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_SDIN_Pad_En(i2c_addr)              PT8311_CLEAR_BIT(i2c_addr, I2S_PAD_CTRL, SDIN_PAD_DIS_Msk)

/**
  * @brief  I2S SDIN Pad Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_SDIN_Pad_Dis(i2c_addr)             PT8311_SET_BIT(i2c_addr, I2S_PAD_CTRL, SDIN_PAD_DIS_Msk)

/**
  * @brief  I2S LRCK Pad Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_LRCK_Pad_En(i2c_addr)              PT8311_CLEAR_BIT(i2c_addr, I2S_PAD_CTRL, LRCK_PAD_DIS_Msk)

/**
  * @brief  I2S LRCK Pad Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_LRCK_Pad_Dis(i2c_addr)             PT8311_SET_BIT(i2c_addr, I2S_PAD_CTRL, LRCK_PAD_DIS_Msk)

/**
  * @brief  I2S BCLK Pad Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_BCLK_Pad_En(i2c_addr)              PT8311_CLEAR_BIT(i2c_addr, I2S_PAD_CTRL, BCLK_PAD_DIS_Msk)

/**
  * @brief  I2S BCLK Pad Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_BCLK_Pad_Dis(i2c_addr)             PT8311_SET_BIT(i2c_addr, I2S_PAD_CTRL, BCLK_PAD_DIS_Msk)

/**
  * @brief  I2S MCLK Pad Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_MCLK_Pad_En(i2c_addr)              PT8311_CLEAR_BIT(i2c_addr, I2S_PAD_CTRL, MCLK_PAD_DIS_Msk)

/**
  * @brief  I2S MCLK Pad Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_MCLK_Pad_Dis(i2c_addr)             PT8311_SET_BIT(i2c_addr, I2S_PAD_CTRL, MCLK_PAD_DIS_Msk)

/**
  * @brief  I2S All Pad Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_All_Pad_En(i2c_addr)               PT8311_CLEAR_BIT(i2c_addr, I2S_PAD_CTRL, I2S_ALL_PAD_DIS_Msk)

/**
  * @brief  I2S All Pad Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2S_All_Pad_Dis(i2c_addr)              PT8311_SET_BIT(i2c_addr, I2S_PAD_CTRL, I2S_ALL_PAD_DIS_Msk)


/**
  * @brief  PDM IO Pin Function output as PDMCLK
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_PDMIO_PinFunc_OutPDMCLK(i2c_addr)      PT8311_SET_BIT(i2c_addr, ADC_DIG_CTRL, PDM_IO_Msk)

/**
  * @brief  PDM IO Pin Function Input as Device ID
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_PDMIO_PinFunc_InDevID(i2c_addr)        PT8311_CLEAR_BIT(i2c_addr, ADC_DIG_CTRL, PDM_IO_Msk)

/**
  * @brief  ADC Delay Function Set
  * @param  i2c_addr PT8311 I2C address
  * @param  func ADC Delay Function, see @ref PT8311_ADCDelayFuncETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DelayFunc_Set(i2c_addr, func)      \
        PT8311_MODIFY_REG(i2c_addr, ADC_DIG_CTRL, ADOUT_DLY_Msk, (func) << ADOUT_DLY_Pos)

/**
  * @brief  PDM Timing Normal
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_PDM_Timing_Normal(i2c_addr)            PT8311_CLEAR_BIT(i2c_addr, ADC_DIG_CTRL, PDM_TIMING_Msk)

/**
  * @brief  PDM Timing Invert
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_PDM_Timing_Invert(i2c_addr)            PT8311_SET_BIT(i2c_addr, ADC_DIG_CTRL, PDM_TIMING_Msk)

/**
  * @brief  PDM Interface Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_PDM_En(i2c_addr)                       PT8311_SET_BIT(i2c_addr, ADC_DIG_CTRL, PDM_EN_Msk)

/**
  * @brief  PDM Interface Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_PDM_Dis(i2c_addr)                      PT8311_CLEAR_BIT(i2c_addr, ADC_DIG_CTRL, PDM_EN_Msk)

/**
  * @brief  ADC HPF Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_HPF_En(i2c_addr)                   PT8311_SET_BIT(i2c_addr, ADC_DIG_CTRL, ADC_HPF_EN_Msk)

/**
  * @brief  ADC HPF Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_HPF_Dis(i2c_addr)                  PT8311_CLEAR_BIT(i2c_addr, ADC_DIG_CTRL, ADC_HPF_EN_Msk)

/**
  * @brief  ADC Dither Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_Dither_En(i2c_addr)                PT8311_SET_BIT(i2c_addr, ADC_DIG_CTRL, ADC_DITHEN_Msk)

/**
  * @brief  ADC Dither Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_Dither_Dis(i2c_addr)               PT8311_CLEAR_BIT(i2c_addr, ADC_DIG_CTRL, ADC_DITHEN_Msk)

/**
  * @brief  ADC Digital Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_Dig_En(i2c_addr)                   PT8311_SET_BIT(i2c_addr, ADC_DIG_CTRL, ADC_DIG_EN_Msk)

/**
  * @brief  ADC Digital Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_Dig_Dis(i2c_addr)                  PT8311_CLEAR_BIT(i2c_addr, ADC_DIG_CTRL, ADC_DIG_EN_Msk)


/**
  * @brief  ADC HPF Coefficient Set
  * @param  i2c_addr PT8311 I2C address
  * @param  coff ADC HPF Coefficient, see @ref PT8311_ADC_HPFCoffETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_HPF_Coff_Set(i2c_addr, coff)       PT8311_WRITE_REG(i2c_addr, ADC_HPF_COEF, coff)


/**
  * @brief  ADC Digital Volume Set
  * @param  i2c_addr PT8311 I2C address
  * @param  vol ADC Digital Volume, range in [0x0, 0xff], which means [-95dB, 32dB], 0.5dB/Step, while 0 means Mute, 1 means -95dB
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DigVol_Set(i2c_addr, vol)          PT8311_WRITE_REG(i2c_addr, ADC_DVC_CTRL, vol)


/**
  * @brief  ADC Zero Cross Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_ZeroCross_En(i2c_addr)             PT8311_SET_BIT(i2c_addr, ADC_SMUTE, ADC_ZERO_CROSS_EN_Msk)

/**
  * @brief  ADC Zero Cross Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_ZeroCross_Dis(i2c_addr)            PT8311_CLEAR_BIT(i2c_addr, ADC_SMUTE, ADC_ZERO_CROSS_EN_Msk)

/**
  * @brief  ADC Soft Volume Ramp Rate Set
  * @param  i2c_addr PT8311 I2C address
  * @param  rate ADC Soft Volume Ramp Rate, see @ref PT8311_SoftVolRampRateETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_SoftVolRampRate_Set(i2c_addr, rate)    \
        PT8311_MODIFY_REG(i2c_addr, ADC_SMUTE, ADC_SOFT_VOL_RATE_Msk, (rate) << ADC_SOFT_VOL_RATE_Pos)

/**
  * @brief  ADC Soft Volume Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_SoftVol_En(i2c_addr)               PT8311_SET_BIT(i2c_addr, ADC_SMUTE, ADC_SOFT_VOL_EN_Msk)

/**
  * @brief  ADC Soft Volume Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_SoftVol_Dis(i2c_addr)              PT8311_CLEAR_BIT(i2c_addr, ADC_SMUTE, ADC_SOFT_VOL_EN_Msk)


/**
  * @brief  ADC EQ Band1 Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_EQ1_En(i2c_addr)                   PT8311_WRITE_REG(i2c_addr, ADC_EQ_CTRL, 0x1)

/**
  * @brief  ADC EQ Band1 Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_EQ1_Dis(i2c_addr)                  PT8311_WRITE_REG(i2c_addr, ADC_EQ_CTRL, 0x0)


/**
  * @brief  ADC EQ Band1 Coefficient b0 High 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b0h ADC EQ Band1 Coefficient b0 High 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_EQ1_Coff_b0h_Set(i2c_addr, b0h)    PT8311_WRITE_REG(i2c_addr, ADC_EQ1_b0_H, b0h)


/**
  * @brief  ADC EQ Band1 Coefficient b0 Middle 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b0m ADC EQ Band1 Coefficient b0 Middle 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_EQ1_Coff_b0m_Set(i2c_addr, b0m)    PT8311_WRITE_REG(i2c_addr, ADC_EQ1_b0_M, b0m)


/**
  * @brief  ADC EQ Band1 Coefficient b0 Low 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b0l ADC EQ Band1 Coefficient b0 Low 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_EQ1_Coff_b0l_Set(i2c_addr, b0l)    PT8311_WRITE_REG(i2c_addr, ADC_EQ1_b0_L, b0l)


/**
  * @brief  ADC EQ Band1 Coefficient b1 High 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b1h ADC EQ Band1 Coefficient b1 High 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_EQ1_Coff_b1h_Set(i2c_addr, b1h)    PT8311_WRITE_REG(i2c_addr, ADC_EQ1_b1_H, b1h)


/**
  * @brief  ADC EQ Band1 Coefficient b1 Middle 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b1m ADC EQ Band1 Coefficient b1 Middle 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_EQ1_Coff_b1m_Set(i2c_addr, b1m)    PT8311_WRITE_REG(i2c_addr, ADC_EQ1_b1_M, b1m)


/**
  * @brief  ADC EQ Band1 Coefficient b1 Low 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b1l ADC EQ Band1 Coefficient b1 Low 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_EQ1_Coff_b1l_Set(i2c_addr, b1l)    PT8311_WRITE_REG(i2c_addr, ADC_EQ1_b1_L, b1l)


/**
  * @brief  ADC EQ Band1 Coefficient b2 High 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b2h ADC EQ Band1 Coefficient b2 High 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_EQ1_Coff_b2h_Set(i2c_addr, b2h)    PT8311_WRITE_REG(i2c_addr, ADC_EQ1_b2_H, b2h)


/**
  * @brief  ADC EQ Band1 Coefficient b2 Middle 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b2m ADC EQ Band1 Coefficient b2 Middle 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_EQ1_Coff_b2m_Set(i2c_addr, b2m)    PT8311_WRITE_REG(i2c_addr, ADC_EQ1_b2_M, b2m)


/**
  * @brief  ADC EQ Band1 Coefficient b2 Low 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b2l ADC EQ Band1 Coefficient b2 Low 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_EQ1_Coff_b2l_Set(i2c_addr, b2l)    PT8311_WRITE_REG(i2c_addr, ADC_EQ1_b2_L, b2l)


/**
  * @brief  ADC EQ Band1 Coefficient a1 High 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a1h ADC EQ Band1 Coefficient a1 High 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_EQ1_Coff_a1h_Set(i2c_addr, a1h)    PT8311_WRITE_REG(i2c_addr, ADC_EQ1_a1_H, a1h)


/**
  * @brief  ADC EQ Band1 Coefficient a1 Middle 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a1m ADC EQ Band1 Coefficient a1 Middle 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_EQ1_Coff_a1m_Set(i2c_addr, a1m)    PT8311_WRITE_REG(i2c_addr, ADC_EQ1_a1_M, a1m)


/**
  * @brief  ADC EQ Band1 Coefficient a1 Low 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a1l ADC EQ Band1 Coefficient a1 Low 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_EQ1_Coff_a1l_Set(i2c_addr, a1l)    PT8311_WRITE_REG(i2c_addr, ADC_EQ1_a1_L, a1l)


/**
  * @brief  ADC EQ Band1 Coefficient a2 High 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a2h ADC EQ Band1 Coefficient a2 High 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_EQ1_Coff_a2h_Set(i2c_addr, a2h)    PT8311_WRITE_REG(i2c_addr, ADC_EQ1_a2_H, a2h)


/**
  * @brief  ADC EQ Band1 Coefficient a2 Middle 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a2m ADC EQ Band1 Coefficient a2 Middle 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_EQ1_Coff_a2m_Set(i2c_addr, a2m)    PT8311_WRITE_REG(i2c_addr, ADC_EQ1_a2_M, a2m)


/**
  * @brief  ADC EQ Band1 Coefficient a2 Low 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a2l ADC EQ Band1 Coefficient a2 Low 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_EQ1_Coff_a2l_Set(i2c_addr, a2l)    PT8311_WRITE_REG(i2c_addr, ADC_EQ1_a2_L, a2l)


/**
  * @brief  ADC DRC smooth filter mode Set
  * @param  i2c_addr PT8311 I2C address
  * @param  mode ADC DRC smooth filter mode, see @ref PT8311_DRCSmoothModeETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DRCSmoothMode_Set(i2c_addr, mode)  \
        PT8311_MODIFY_REG(i2c_addr, ADC_DRC_CTRL, ADC_DRC_SMTH_MODE_Msk, (mode) << ADC_DRC_SMTH_MODE_Pos)

/**
  * @brief  ADC DRC smooth filter Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DRCSmooth_En(i2c_addr)             PT8311_SET_BIT(i2c_addr, ADC_DRC_CTRL, ADC_DRC_SMTH_EN_Msk)

/**
  * @brief  ADC DRC smooth filter Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DRCSmooth_Dis(i2c_addr)            PT8311_CLEAR_BIT(i2c_addr, ADC_DRC_CTRL, ADC_DRC_SMTH_EN_Msk)

/**
  * @brief  ADC DRC Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DRC_En(i2c_addr)                   PT8311_SET_BIT(i2c_addr, ADC_DRC_CTRL, ADC_DRC_EN_Msk)

/**
  * @brief  ADC DRC Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DRC_Dis(i2c_addr)                  PT8311_CLEAR_BIT(i2c_addr, ADC_DRC_CTRL, ADC_DRC_EN_Msk)


/**
  * @brief  ADC DRC peak filter attack time Set
  * @param  i2c_addr PT8311 I2C address
  * @param  time ADC DRC peak filter attack time, see @ref PT8311_DRCPeakAtkTimeETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DRCPeakAtkTime_Set(i2c_addr, time) \
        PT8311_MODIFY_REG(i2c_addr, ADC_PEAK_CTRL, ADC_DRC_PEAK_AT_Msk, (time) << ADC_DRC_PEAK_AT_Pos)

/**
  * @brief  ADC DRC peak filter release time Set
  * @param  i2c_addr PT8311 I2C address
  * @param  time ADC DRC peak filter release time, see @ref PT8311_DRCPeakRlsTimeETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DRCPeakRlsTime_Set(i2c_addr, time) \
        PT8311_MODIFY_REG(i2c_addr, ADC_PEAK_CTRL, ADC_DRC_PEAK_RT_Msk, (time) << ADC_DRC_PEAK_RT_Pos)


/**
  * @brief  ADC DRC attack time Set
  * @param  i2c_addr PT8311 I2C address
  * @param  time ADC DRC attack time, see @ref PT8311_DRCAtkTimeETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DRCAtkTime_Set(i2c_addr, time)     \
        PT8311_MODIFY_REG(i2c_addr, ADC_ATK_TIME, ADC_DRC_ATK_TIME_Msk, (time) << ADC_DRC_ATK_TIME_Pos)

/**
  * @brief  ADC DRC decay time Set
  * @param  i2c_addr PT8311 I2C address
  * @param  time ADC DRC decay time, see @ref PT8311_DRCDcyTimeETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DRCDcyTime_Set(i2c_addr, time)     \
        PT8311_MODIFY_REG(i2c_addr, ADC_ATK_TIME, ADC_DRC_DCY_TIME_Msk, (time) << ADC_DRC_DCY_TIME_Pos)


/**
  * @brief  ADC DRC limiter threshold Set
  * @param  i2c_addr PT8311 I2C address
  * @param  thres ADC DRC limiter threshold, range in [0, 31], which means [0, -31dB] threshold, with step size -1dB
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DRCLmtThres_Set(i2c_addr, thres)   \
        PT8311_MODIFY_REG(i2c_addr, ADC_DRC_THD1, ADC_DRC_LMT_THD_Msk, (thres) << ADC_DRC_LMT_THD_Pos)


/**
  * @brief  ADC DRC compressor threshold Set
  * @param  i2c_addr PT8311 I2C address
  * @param  thres ADC DRC compressor threshold, range in [0, 63], which means [0, -63dB] threshold, with step size -1dB
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DRCCmpThres_Set(i2c_addr, thres)   \
        PT8311_MODIFY_REG(i2c_addr, ADC_DRC_THD2, ADC_DRC_CMP_THD_Msk, (thres) << ADC_DRC_CMP_THD_Pos)


/**
  * @brief  ADC DRC expander threshold Set
  * @param  i2c_addr PT8311 I2C address
  * @param  thres ADC DRC expander threshold, range in [0, 63], which means [-18dB, -81dB] threshold, with step size -1dB
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DRCExpThres_Set(i2c_addr, thres)   \
        PT8311_MODIFY_REG(i2c_addr, ADC_DRC_THD3, ADC_DRC_EXP_THD_Msk, (thres) << ADC_DRC_EXP_THD_Pos)


/**
  * @brief  ADC DRC noise gate threshold Set
  * @param  i2c_addr PT8311 I2C address
  * @param  thres ADC DRC noise gate threshold, range in [0, 63], which means [-35dB, -98dB] threshold, with step size -1dB
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DRCNgThres_Set(i2c_addr, thres)   \
        PT8311_MODIFY_REG(i2c_addr, ADC_DRC_THD4, ADC_DRC_NG_THD_Msk, (thres) << ADC_DRC_NG_THD_Pos)


/**
  * @brief  ADC DRC limiter slope Set
  * @param  i2c_addr PT8311 I2C address
  * @param  slope ADC DRC limiter slope, see @ref PT8311_DRCLmtSlopeETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DRCLmtSlope_Set(i2c_addr, slope)   \
        PT8311_MODIFY_REG(i2c_addr, ADC_DRC_SLP1, ADC_DRC_LMT_SLP_Msk, (slope) << ADC_DRC_LMT_SLP_Pos)

/**
  * @brief  ADC DRC compressor slope Set
  * @param  i2c_addr PT8311 I2C address
  * @param  slope ADC DRC compressor slope, see @ref PT8311_DRCCmpSlopeETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DRCCmpSlope_Set(i2c_addr, slope)   \
        PT8311_MODIFY_REG(i2c_addr, ADC_DRC_SLP1, ADC_DRC_CMP_SLP_Msk, (slope) << ADC_DRC_CMP_SLP_Pos)


/**
  * @brief  ADC DRC expander slope Set
  * @param  i2c_addr PT8311 I2C address
  * @param  slope ADC DRC expander slope, see @ref PT8311_DRCExpSlopeETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DRCExpSlope_Set(i2c_addr, slope)   \
        PT8311_MODIFY_REG(i2c_addr, ADC_DRC_SLP2, ADC_DRC_EXP_SLP_Msk, (slope) << ADC_DRC_EXP_SLP_Pos)

/**
  * @brief  ADC DRC noise gate slope Set
  * @param  i2c_addr PT8311 I2C address
  * @param  slope ADC DRC noise gate slope, see @ref PT8311_DRCNgSlopeETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DRCNgSlope_Set(i2c_addr, slope)    \
        PT8311_MODIFY_REG(i2c_addr, ADC_DRC_SLP2, ADC_DRC_NG_SLP_Msk, (slope) << ADC_DRC_NG_SLP_Pos)

/**
  * @brief  ADC DRC makeup gain Set
  * @param  i2c_addr PT8311 I2C address
  * @param  gain ADC DRC makeup gain, range in [0, 31], which means [0dB, 31dB], with step size 1dB
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_DRCGain_Set(i2c_addr, gain)        \
        PT8311_MODIFY_REG(i2c_addr, ADC_DRC_SLP2, ADC_DRC_GAIN_Msk, (gain) << ADC_DRC_GAIN_Pos)


/**
  * @brief  DAC Dither amplitude Set
  * @param  i2c_addr PT8311 I2C address
  * @param  amp DAC Dither amplitude, see @ref PT8311_DAC_DitherAmpETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DitherAmp_Set(i2c_addr, amp)       \
        PT8311_MODIFY_REG(i2c_addr, DAC_DIG_CTRL, DAC_DITHSEL_Msk, (amp) << DAC_DITHSEL_Pos)

/**
  * @brief  DAC Dither Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_Dither_En(i2c_addr)                PT8311_SET_BIT(i2c_addr, DAC_DIG_CTRL, DAC_DITHEN_Msk)

/**
  * @brief  DAC Dither Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_Dither_Dis(i2c_addr)               PT8311_CLEAR_BIT(i2c_addr, DAC_DIG_CTRL, DAC_DITHEN_Msk)

/**
  * @brief  DAC Digital part Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_Dig_En(i2c_addr)                   PT8311_SET_BIT(i2c_addr, DAC_DIG_CTRL, DAC_DIG_EN_Msk)

/**
  * @brief  DAC Digital part Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_Dig_Dis(i2c_addr)                  PT8311_CLEAR_BIT(i2c_addr, DAC_DIG_CTRL, DAC_DIG_EN_Msk)

/**
  * @brief  DAC Digital Volume Set
  * @param  i2c_addr PT8311 I2C address
  * @param  vol DAC Digital Volume, range in [0x0, 0xff], which means [-95dB, 32dB], 0.5dB/Step, while 0 means Mute, 1 means -95dB
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DigVol_Set(i2c_addr, vol)          PT8311_WRITE_REG(i2c_addr, DAC_DVC_CTRL, vol)

/**
  * @brief  DAC Digital Volume Get
  * @param  i2c_addr PT8311 I2C address
  * @return DAC Digital Volume, range in [0x0, 0xff], which means [-95dB, 32dB], 0.5dB/Step, while 0 means Mute, 1 means -95dB
  */
#define __PT8311_DAC_DigVol_Get(i2c_addr)               PT8311_READ_REG(i2c_addr, DAC_DVC_CTRL)


/**
  * @brief  DAC Auto Mute Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_AutoMute_En(i2c_addr)              PT8311_SET_BIT(i2c_addr, DAC_SMUTE, DAC_AMUTE_EN_Msk)

/**
  * @brief  DAC Auto Mute Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_AutoMute_Dis(i2c_addr)             PT8311_CLEAR_BIT(i2c_addr, DAC_SMUTE, DAC_AMUTE_EN_Msk)

/**
  * @brief  DAC Zero Cross Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_ZeroCross_En(i2c_addr)             PT8311_SET_BIT(i2c_addr, DAC_SMUTE, DAC_ZERO_CROSS_EN_Msk)

/**
  * @brief  DAC Zero Cross Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_ZeroCross_Dis(i2c_addr)            PT8311_CLEAR_BIT(i2c_addr, DAC_SMUTE, DAC_ZERO_CROSS_EN_Msk)

/**
  * @brief  DAC Soft Volume Ramp Rate Set
  * @param  i2c_addr PT8311 I2C address
  * @param  rate DAC Soft Volume Ramp Rate, see @ref PT8311_SoftVolRampRateETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_SoftVolRampRate_Set(i2c_addr, rate)    \
        PT8311_MODIFY_REG(i2c_addr, DAC_SMUTE, DAC_SOFT_VOL_RATE_Msk, (rate) << DAC_SOFT_VOL_RATE_Pos)

/**
  * @brief  DAC Soft Volume Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_SoftVol_En(i2c_addr)               PT8311_SET_BIT(i2c_addr, DAC_SMUTE, DAC_SOFT_VOL_EN_Msk)

/**
  * @brief  DAC Soft Volume Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_SoftVol_Dis(i2c_addr)              PT8311_CLEAR_BIT(i2c_addr, DAC_SMUTE, DAC_SOFT_VOL_EN_Msk)


/**
  * @brief  DAC Digital Mixer Gain ADC_DO 0dB
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_Dig_MixerGainADCDO_0dB(i2c_addr)       PT8311_CLEAR_BIT(i2c_addr, DAC_DMIX_CTRL, DAC_DMX_GAIN_ADC_DO_Msk)

/**
  * @brief  DAC Digital Mixer Gain ADC_DO -6dB
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_Dig_MixerGainADCDO_Minus6dB(i2c_addr)  PT8311_SET_BIT(i2c_addr, DAC_DMIX_CTRL, DAC_DMX_GAIN_ADC_DO_Msk)

/**
  * @brief  DAC Digital Mixer Gain DAC_DI 0dB
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_Dig_MixerGainDACDI_0dB(i2c_addr)       PT8311_CLEAR_BIT(i2c_addr, DAC_DMIX_CTRL, DAC_DMX_GAIN_DAC_DI_Msk)

/**
  * @brief  DAC Digital Mixer Gain DAC_DI -6dB
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_Dig_MixerGainDACDI_Minus6dB(i2c_addr)  PT8311_SET_BIT(i2c_addr, DAC_DMIX_CTRL, DAC_DMX_GAIN_DAC_DI_Msk)

/**
  * @brief  DAC Digital Mixer Source ADC_DO Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_Dig_MixerSrcADCDO_En(i2c_addr)         PT8311_SET_BIT(i2c_addr, DAC_DMIX_CTRL, DAC_DMX_SRC_ADC_DO_Msk)

/**
  * @brief  DAC Digital Mixer Source ADC_DO Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_Dig_MixerSrcADCDO_Dis(i2c_addr)        PT8311_CLEAR_BIT(i2c_addr, DAC_DMIX_CTRL, DAC_DMX_SRC_ADC_DO_Msk)

/**
  * @brief  DAC Digital Mixer Source DAC_DI Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_Dig_MixerSrcDACDI_En(i2c_addr)         PT8311_SET_BIT(i2c_addr, DAC_DMIX_CTRL, DAC_DMX_SRC_DAC_DI_Msk)

/**
  * @brief  DAC Digital Mixer Source DAC_DI Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_Dig_MixerSrcDACDI_Dis(i2c_addr)        PT8311_CLEAR_BIT(i2c_addr, DAC_DMIX_CTRL, DAC_DMX_SRC_DAC_DI_Msk)


/**
  * @brief  DAC EQ Band3 Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ3_En(i2c_addr)                   PT8311_SET_BIT(i2c_addr, DAC_EQ_CTRL, DAC_EQ3_EN_Msk)

/**
  * @brief  DAC EQ Band3 Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ3_Dis(i2c_addr)                  PT8311_CLEAR_BIT(i2c_addr, DAC_EQ_CTRL, DAC_EQ3_EN_Msk)

/**
  * @brief  DAC EQ Band2 Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ2_En(i2c_addr)                   PT8311_SET_BIT(i2c_addr, DAC_EQ_CTRL, DAC_EQ2_EN_Msk)

/**
  * @brief  DAC EQ Band2 Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ2_Dis(i2c_addr)                  PT8311_CLEAR_BIT(i2c_addr, DAC_EQ_CTRL, DAC_EQ2_EN_Msk)

/**
  * @brief  DAC EQ Band1 Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ1_En(i2c_addr)                   PT8311_SET_BIT(i2c_addr, DAC_EQ_CTRL, DAC_EQ1_EN_Msk)

/**
  * @brief  DAC EQ Band1 Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ1_Dis(i2c_addr)                  PT8311_CLEAR_BIT(i2c_addr, DAC_EQ_CTRL, DAC_EQ1_EN_Msk)

/**
  * @brief  DAC EQ All Band Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ_AllBand_En(i2c_addr)            PT8311_WRITE_REG(i2c_addr, DAC_EQ_CTRL, 0x7)

/**
  * @brief  DAC EQ All Band Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ_AllBand_Dis(i2c_addr)           PT8311_WRITE_REG(i2c_addr, DAC_EQ_CTRL, 0x0)


/**
  * @brief  DAC EQ Band1 Coefficient b0 High 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b0h DAC EQ Band1 Coefficient b0 High 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ1_Coff_b0h_Set(i2c_addr, b0h)    PT8311_WRITE_REG(i2c_addr, DAC_EQ1_b0_H, b0h)


/**
  * @brief  DAC EQ Band1 Coefficient b0 Middle 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b0m DAC EQ Band1 Coefficient b0 Middle 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ1_Coff_b0m_Set(i2c_addr, b0m)    PT8311_WRITE_REG(i2c_addr, DAC_EQ1_b0_M, b0m)


/**
  * @brief  DAC EQ Band1 Coefficient b0 Low 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b0l DAC EQ Band1 Coefficient b0 Low 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ1_Coff_b0l_Set(i2c_addr, b0l)    PT8311_WRITE_REG(i2c_addr, DAC_EQ1_b0_L, b0l)


/**
  * @brief  DAC EQ Band1 Coefficient b1 High 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b1h DAC EQ Band1 Coefficient b1 High 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ1_Coff_b1h_Set(i2c_addr, b1h)    PT8311_WRITE_REG(i2c_addr, DAC_EQ1_b1_H, b1h)


/**
  * @brief  DAC EQ Band1 Coefficient b1 Middle 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b1m DAC EQ Band1 Coefficient b1 Middle 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ1_Coff_b1m_Set(i2c_addr, b1m)    PT8311_WRITE_REG(i2c_addr, DAC_EQ1_b1_M, b1m)


/**
  * @brief  DAC EQ Band1 Coefficient b1 Low 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b1l DAC EQ Band1 Coefficient b1 Low 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ1_Coff_b1l_Set(i2c_addr, b1l)    PT8311_WRITE_REG(i2c_addr, DAC_EQ1_b1_L, b1l)


/**
  * @brief  DAC EQ Band1 Coefficient b2 High 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b2h DAC EQ Band1 Coefficient b2 High 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ1_Coff_b2h_Set(i2c_addr, b2h)    PT8311_WRITE_REG(i2c_addr, DAC_EQ1_b2_H, b2h)


/**
  * @brief  DAC EQ Band1 Coefficient b2 Middle 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b2m DAC EQ Band1 Coefficient b2 Middle 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ1_Coff_b2m_Set(i2c_addr, b2m)    PT8311_WRITE_REG(i2c_addr, DAC_EQ1_b2_M, b2m)


/**
  * @brief  DAC EQ Band1 Coefficient b2 Low 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b2l DAC EQ Band1 Coefficient b2 Low 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ1_Coff_b2l_Set(i2c_addr, b2l)    PT8311_WRITE_REG(i2c_addr, DAC_EQ1_b2_L, b2l)


/**
  * @brief  DAC EQ Band1 Coefficient a1 High 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a1h DAC EQ Band1 Coefficient a1 High 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ1_Coff_a1h_Set(i2c_addr, a1h)    PT8311_WRITE_REG(i2c_addr, DAC_EQ1_a1_H, a1h)


/**
  * @brief  DAC EQ Band1 Coefficient a1 Middle 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a1m DAC EQ Band1 Coefficient a1 Middle 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ1_Coff_a1m_Set(i2c_addr, a1m)    PT8311_WRITE_REG(i2c_addr, DAC_EQ1_a1_M, a1m)


/**
  * @brief  DAC EQ Band1 Coefficient a1 Low 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a1l DAC EQ Band1 Coefficient a1 Low 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ1_Coff_a1l_Set(i2c_addr, a1l)    PT8311_WRITE_REG(i2c_addr, DAC_EQ1_a1_L, a1l)


/**
  * @brief  DAC EQ Band1 Coefficient a2 High 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a2h DAC EQ Band1 Coefficient a2 High 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ1_Coff_a2h_Set(i2c_addr, a2h)    PT8311_WRITE_REG(i2c_addr, DAC_EQ1_a2_H, a2h)


/**
  * @brief  DAC EQ Band1 Coefficient a2 Middle 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a2m DAC EQ Band1 Coefficient a2 Middle 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ1_Coff_a2m_Set(i2c_addr, a2m)    PT8311_WRITE_REG(i2c_addr, DAC_EQ1_a2_M, a2m)


/**
  * @brief  DAC EQ Band1 Coefficient a2 Low 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a2l DAC EQ Band1 Coefficient a2 Low 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ1_Coff_a2l_Set(i2c_addr, a2l)    PT8311_WRITE_REG(i2c_addr, DAC_EQ1_a2_L, a2l)


/**
  * @brief  DAC EQ Band2 Coefficient b0 High 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b0h DAC EQ Band2 Coefficient b0 High 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ2_Coff_b0h_Set(i2c_addr, b0h)    PT8311_WRITE_REG(i2c_addr, DAC_EQ2_b0_H, b0h)


/**
  * @brief  DAC EQ Band2 Coefficient b0 Middle 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b0m DAC EQ Band2 Coefficient b0 Middle 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ2_Coff_b0m_Set(i2c_addr, b0m)    PT8311_WRITE_REG(i2c_addr, DAC_EQ2_b0_M, b0m)


/**
  * @brief  DAC EQ Band2 Coefficient b0 Low 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b0l DAC EQ Band2 Coefficient b0 Low 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ2_Coff_b0l_Set(i2c_addr, b0l)    PT8311_WRITE_REG(i2c_addr, DAC_EQ2_b0_L, b0l)


/**
  * @brief  DAC EQ Band2 Coefficient b1 High 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b1h DAC EQ Band2 Coefficient b1 High 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ2_Coff_b1h_Set(i2c_addr, b1h)    PT8311_WRITE_REG(i2c_addr, DAC_EQ2_b1_H, b1h)


/**
  * @brief  DAC EQ Band2 Coefficient b1 Middle 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b1m DAC EQ Band2 Coefficient b1 Middle 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ2_Coff_b1m_Set(i2c_addr, b1m)    PT8311_WRITE_REG(i2c_addr, DAC_EQ2_b1_M, b1m)


/**
  * @brief  DAC EQ Band2 Coefficient b1 Low 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b1l DAC EQ Band2 Coefficient b1 Low 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ2_Coff_b1l_Set(i2c_addr, b1l)    PT8311_WRITE_REG(i2c_addr, DAC_EQ2_b1_L, b1l)


/**
  * @brief  DAC EQ Band2 Coefficient b2 High 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b2h DAC EQ Band2 Coefficient b2 High 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ2_Coff_b2h_Set(i2c_addr, b2h)    PT8311_WRITE_REG(i2c_addr, DAC_EQ2_b2_H, b2h)


/**
  * @brief  DAC EQ Band2 Coefficient b2 Middle 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b2m DAC EQ Band2 Coefficient b2 Middle 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ2_Coff_b2m_Set(i2c_addr, b2m)    PT8311_WRITE_REG(i2c_addr, DAC_EQ2_b2_M, b2m)


/**
  * @brief  DAC EQ Band2 Coefficient b2 Low 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b2l DAC EQ Band2 Coefficient b2 Low 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ2_Coff_b2l_Set(i2c_addr, b2l)    PT8311_WRITE_REG(i2c_addr, DAC_EQ2_b2_L, b2l)


/**
  * @brief  DAC EQ Band2 Coefficient a1 High 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a1h DAC EQ Band2 Coefficient a1 High 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ2_Coff_a1h_Set(i2c_addr, a1h)    PT8311_WRITE_REG(i2c_addr, DAC_EQ2_a1_H, a1h)


/**
  * @brief  DAC EQ Band2 Coefficient a1 Middle 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a1m DAC EQ Band2 Coefficient a1 Middle 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ2_Coff_a1m_Set(i2c_addr, a1m)    PT8311_WRITE_REG(i2c_addr, DAC_EQ2_a1_M, a1m)


/**
  * @brief  DAC EQ Band2 Coefficient a1 Low 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a1l DAC EQ Band2 Coefficient a1 Low 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ2_Coff_a1l_Set(i2c_addr, a1l)    PT8311_WRITE_REG(i2c_addr, DAC_EQ2_a1_L, a1l)


/**
  * @brief  DAC EQ Band2 Coefficient a2 High 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a2h DAC EQ Band2 Coefficient a2 High 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ2_Coff_a2h_Set(i2c_addr, a2h)    PT8311_WRITE_REG(i2c_addr, DAC_EQ2_a2_H, a2h)


/**
  * @brief  DAC EQ Band2 Coefficient a2 Middle 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a2m DAC EQ Band2 Coefficient a2 Middle 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ2_Coff_a2m_Set(i2c_addr, a2m)    PT8311_WRITE_REG(i2c_addr, DAC_EQ2_a2_M, a2m)


/**
  * @brief  DAC EQ Band2 Coefficient a2 Low 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a2l DAC EQ Band2 Coefficient a2 Low 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ2_Coff_a2l_Set(i2c_addr, a2l)    PT8311_WRITE_REG(i2c_addr, DAC_EQ2_a2_L, a2l)


/**
  * @brief  DAC EQ Band3 Coefficient b0 High 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b0h DAC EQ Band3 Coefficient b0 High 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ3_Coff_b0h_Set(i2c_addr, b0h)    PT8311_WRITE_REG(i2c_addr, DAC_EQ3_b0_H, b0h)


/**
  * @brief  DAC EQ Band3 Coefficient b0 Middle 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b0m DAC EQ Band3 Coefficient b0 Middle 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ3_Coff_b0m_Set(i2c_addr, b0m)    PT8311_WRITE_REG(i2c_addr, DAC_EQ3_b0_M, b0m)


/**
  * @brief  DAC EQ Band3 Coefficient b0 Low 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b0l DAC EQ Band3 Coefficient b0 Low 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ3_Coff_b0l_Set(i2c_addr, b0l)    PT8311_WRITE_REG(i2c_addr, DAC_EQ3_b0_L, b0l)


/**
  * @brief  DAC EQ Band3 Coefficient b1 High 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b1h DAC EQ Band3 Coefficient b1 High 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ3_Coff_b1h_Set(i2c_addr, b1h)    PT8311_WRITE_REG(i2c_addr, DAC_EQ3_b1_H, b1h)


/**
  * @brief  DAC EQ Band3 Coefficient b1 Middle 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b1m DAC EQ Band3 Coefficient b1 Middle 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ3_Coff_b1m_Set(i2c_addr, b1m)    PT8311_WRITE_REG(i2c_addr, DAC_EQ3_b1_M, b1m)


/**
  * @brief  DAC EQ Band3 Coefficient b1 Low 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b1l DAC EQ Band3 Coefficient b1 Low 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ3_Coff_b1l_Set(i2c_addr, b1l)    PT8311_WRITE_REG(i2c_addr, DAC_EQ3_b1_L, b1l)


/**
  * @brief  DAC EQ Band3 Coefficient b2 High 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b2h DAC EQ Band3 Coefficient b2 High 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ3_Coff_b2h_Set(i2c_addr, b2h)    PT8311_WRITE_REG(i2c_addr, DAC_EQ3_b2_H, b2h)


/**
  * @brief  DAC EQ Band3 Coefficient b2 Middle 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b2m DAC EQ Band3 Coefficient b2 Middle 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ3_Coff_b2m_Set(i2c_addr, b2m)    PT8311_WRITE_REG(i2c_addr, DAC_EQ3_b2_M, b2m)


/**
  * @brief  DAC EQ Band3 Coefficient b2 Low 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  b2l DAC EQ Band3 Coefficient b2 Low 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ3_Coff_b2l_Set(i2c_addr, b2l)    PT8311_WRITE_REG(i2c_addr, DAC_EQ3_b2_L, b2l)


/**
  * @brief  DAC EQ Band3 Coefficient a1 High 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a1h DAC EQ Band3 Coefficient a1 High 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ3_Coff_a1h_Set(i2c_addr, a1h)    PT8311_WRITE_REG(i2c_addr, DAC_EQ3_a1_H, a1h)


/**
  * @brief  DAC EQ Band3 Coefficient a1 Middle 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a1m DAC EQ Band3 Coefficient a1 Middle 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ3_Coff_a1m_Set(i2c_addr, a1m)    PT8311_WRITE_REG(i2c_addr, DAC_EQ3_a1_M, a1m)


/**
  * @brief  DAC EQ Band3 Coefficient a1 Low 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a1l DAC EQ Band3 Coefficient a1 Low 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ3_Coff_a1l_Set(i2c_addr, a1l)    PT8311_WRITE_REG(i2c_addr, DAC_EQ3_a1_L, a1l)


/**
  * @brief  DAC EQ Band3 Coefficient a2 High 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a2h DAC EQ Band3 Coefficient a2 High 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ3_Coff_a2h_Set(i2c_addr, a2h)    PT8311_WRITE_REG(i2c_addr, DAC_EQ3_a2_H, a2h)


/**
  * @brief  DAC EQ Band3 Coefficient a2 Middle 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a2m DAC EQ Band3 Coefficient a2 Middle 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ3_Coff_a2m_Set(i2c_addr, a2m)    PT8311_WRITE_REG(i2c_addr, DAC_EQ3_a2_M, a2m)


/**
  * @brief  DAC EQ Band3 Coefficient a2 Low 8bit Set
  * @param  i2c_addr PT8311 I2C address
  * @param  a2l DAC EQ Band3 Coefficient a2 Low 8bit
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_EQ3_Coff_a2l_Set(i2c_addr, a2l)    PT8311_WRITE_REG(i2c_addr, DAC_EQ3_a2_L, a2l)


/**
  * @brief  DAC DRC smooth filter mode Set
  * @param  i2c_addr PT8311 I2C address
  * @param  mode DAC DRC smooth filter mode, see @ref PT8311_DRCSmoothModeETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DRCSmoothMode_Set(i2c_addr, mode)  \
        PT8311_MODIFY_REG(i2c_addr, DAC_DRC_CTRL, DAC_DRC_SMTH_MODE_Msk, (mode) << DAC_DRC_SMTH_MODE_Pos)

/**
  * @brief  DAC DRC smooth filter Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DRCSmooth_En(i2c_addr)             PT8311_SET_BIT(i2c_addr, DAC_DRC_CTRL, DAC_DRC_SMTH_EN_Msk)

/**
  * @brief  DAC DRC smooth filter Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DRCSmooth_Dis(i2c_addr)            PT8311_CLEAR_BIT(i2c_addr, DAC_DRC_CTRL, DAC_DRC_SMTH_EN_Msk)

/**
  * @brief  DAC DRC Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DRC_En(i2c_addr)                   PT8311_SET_BIT(i2c_addr, DAC_DRC_CTRL, DAC_DRC_EN_Msk)

/**
  * @brief  DAC DRC Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DRC_Dis(i2c_addr)                  PT8311_CLEAR_BIT(i2c_addr, DAC_DRC_CTRL, DAC_DRC_EN_Msk)


/**
  * @brief  DAC DRC peak filter attack time Set
  * @param  i2c_addr PT8311 I2C address
  * @param  time DAC DRC peak filter attack time, see @ref PT8311_DRCPeakAtkTimeETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DRCPeakAtkTime_Set(i2c_addr, time) \
        PT8311_MODIFY_REG(i2c_addr, DAC_PEAK_CTRL, DAC_DRC_PEAK_AT_Msk, (time) << DAC_DRC_PEAK_AT_Pos)

/**
  * @brief  DAC DRC peak filter release time Set
  * @param  i2c_addr PT8311 I2C address
  * @param  time DAC DRC peak filter release time, see @ref PT8311_DRCPeakRlsTimeETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DRCPeakRlsTime_Set(i2c_addr, time) \
        PT8311_MODIFY_REG(i2c_addr, DAC_PEAK_CTRL, DAC_DRC_PEAK_RT_Msk, (time) << DAC_DRC_PEAK_RT_Pos)


/**
  * @brief  DAC DRC attack time Set
  * @param  i2c_addr PT8311 I2C address
  * @param  time DAC DRC attack time, see @ref PT8311_DRCAtkTimeETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DRCAtkTime_Set(i2c_addr, time)     \
        PT8311_MODIFY_REG(i2c_addr, DAC_ATK_TIME, DAC_DRC_ATK_TIME_Msk, (time) << DAC_DRC_ATK_TIME_Pos)

/**
  * @brief  DAC DRC decay time Set
  * @param  i2c_addr PT8311 I2C address
  * @param  time DAC DRC decay time, see @ref PT8311_DRCDcyTimeETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DRCDcyTime_Set(i2c_addr, time)     \
        PT8311_MODIFY_REG(i2c_addr, DAC_ATK_TIME, DAC_DRC_DCY_TIME_Msk, (time) << DAC_DRC_DCY_TIME_Pos)


/**
  * @brief  DAC DRC limiter threshold Set
  * @param  i2c_addr PT8311 I2C address
  * @param  thres DAC DRC limiter threshold, range in [0, 31], which means [0, -31dB] threshold, with step size -1dB
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DRCLmtThres_Set(i2c_addr, thres)   \
        PT8311_MODIFY_REG(i2c_addr, DAC_DRC_THD1, DAC_DRC_LMT_THD_Msk, (thres) << DAC_DRC_LMT_THD_Pos)


/**
  * @brief  DAC DRC compressor threshold Set
  * @param  i2c_addr PT8311 I2C address
  * @param  thres DAC DRC compressor threshold, range in [0, 63], which means [0, -63dB] threshold, with step size -1dB
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DRCCmpThres_Set(i2c_addr, thres)   \
        PT8311_MODIFY_REG(i2c_addr, DAC_DRC_THD2, DAC_DRC_CMP_THD_Msk, (thres) << DAC_DRC_CMP_THD_Pos)


/**
  * @brief  DAC DRC expander threshold Set
  * @param  i2c_addr PT8311 I2C address
  * @param  thres DAC DRC expander threshold, range in [0, 63], which means [-18dB, -81dB] threshold, with step size -1dB
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DRCExpThres_Set(i2c_addr, thres)   \
        PT8311_MODIFY_REG(i2c_addr, DAC_DRC_THD3, DAC_DRC_EXP_THD_Msk, (thres) << DAC_DRC_EXP_THD_Pos)


/**
  * @brief  DAC DRC noise gate threshold Set
  * @param  i2c_addr PT8311 I2C address
  * @param  thres DAC DRC noise gate threshold, range in [0, 63], which means [-35dB, -98dB] threshold, with step size -1dB
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DRCNgThres_Set(i2c_addr, thres)   \
        PT8311_MODIFY_REG(i2c_addr, DAC_DRC_THD4, DAC_DRC_NG_THD_Msk, (thres) << DAC_DRC_NG_THD_Pos)


/**
  * @brief  DAC DRC limiter slope Set
  * @param  i2c_addr PT8311 I2C address
  * @param  slope DAC DRC limiter slope, see @ref PT8311_DRCLmtSlopeETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DRCLmtSlope_Set(i2c_addr, slope)   \
        PT8311_MODIFY_REG(i2c_addr, DAC_DRC_SLP1, DAC_DRC_LMT_SLP_Msk, (slope) << DAC_DRC_LMT_SLP_Pos)

/**
  * @brief  DAC DRC compressor slope Set
  * @param  i2c_addr PT8311 I2C address
  * @param  slope DAC DRC compressor slope, see @ref PT8311_DRCCmpSlopeETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DRCCmpSlope_Set(i2c_addr, slope)   \
        PT8311_MODIFY_REG(i2c_addr, DAC_DRC_SLP1, DAC_DRC_CMP_SLP_Msk, (slope) << DAC_DRC_CMP_SLP_Pos)


/**
  * @brief  DAC DRC expander slope Set
  * @param  i2c_addr PT8311 I2C address
  * @param  slope DAC DRC expander slope, see @ref PT8311_DRCExpSlopeETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DRCExpSlope_Set(i2c_addr, slope)   \
        PT8311_MODIFY_REG(i2c_addr, DAC_DRC_SLP2, DAC_DRC_EXP_SLP_Msk, (slope) << DAC_DRC_EXP_SLP_Pos)

/**
  * @brief  DAC DRC noise gate slope Set
  * @param  i2c_addr PT8311 I2C address
  * @param  slope DAC DRC noise gate slope, see @ref PT8311_DRCNgSlopeETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DRCNgSlope_Set(i2c_addr, slope)    \
        PT8311_MODIFY_REG(i2c_addr, DAC_DRC_SLP2, DAC_DRC_NG_SLP_Msk, (slope) << DAC_DRC_NG_SLP_Pos)

/**
  * @brief  DAC DRC makeup gain Set
  * @param  i2c_addr PT8311 I2C address
  * @param  gain DAC DRC makeup gain, range in [0, 31], which means [0dB, 31dB], with step size 1dB
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DRCGain_Set(i2c_addr, gain)        \
        PT8311_MODIFY_REG(i2c_addr, DAC_DRC_SLP2, DAC_DRC_GAIN_Msk, (gain) << DAC_DRC_GAIN_Pos)


/**
  * @brief  ADC PGA Gain Set
  * @param  i2c_addr PT8311 I2C address
  * @param  gain ADC PGA Gain, see @ref PT8311_ADC_PGAGainETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_PGAGain_Set(i2c_addr, gain)        \
        PT8311_MODIFY_REG(i2c_addr, ANA_ADC_CTRL1, ADC_PGA_GAIN_Msk, (gain) << ADC_PGA_GAIN_Pos)

/**
  * @brief  ADC SDM conversion Start
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_SDM_Conv_Start(i2c_addr)           PT8311_SET_BIT(i2c_addr, ANA_ADC_CTRL1, ADC_CONV_EN_Msk)

/**
  * @brief  ADC SDM conversion remain resetting status
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_SDM_Conv_Reset(i2c_addr)           PT8311_CLEAR_BIT(i2c_addr, ANA_ADC_CTRL1, ADC_CONV_EN_Msk)

/**
  * @brief  ADC SDM related circuit Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_SDM_En(i2c_addr)                   PT8311_SET_BIT(i2c_addr, ANA_ADC_CTRL1, ADC_SDM_EN_Msk)

/**
  * @brief  ADC SDM related circuit Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_SDM_Dis(i2c_addr)                  PT8311_CLEAR_BIT(i2c_addr, ANA_ADC_CTRL1, ADC_SDM_EN_Msk)

/**
  * @brief  ADC PGA related circuit Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_PGA_En(i2c_addr)                   PT8311_SET_BIT(i2c_addr, ANA_ADC_CTRL1, ADC_PGA_EN_Msk)

/**
  * @brief  ADC PGA related circuit Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_PGA_Dis(i2c_addr)                  PT8311_CLEAR_BIT(i2c_addr, ANA_ADC_CTRL1, ADC_PGA_EN_Msk)

/**
  * @brief  ADC SDM conversion Start/Enable and PGA Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_SDMConvPGA_En(i2c_addr)            \
        PT8311_SET_BIT(i2c_addr, ANA_ADC_CTRL1, ADC_CONV_EN_Msk | ADC_SDM_EN_Msk | ADC_PGA_EN_Msk)

/**
  * @brief  ADC SDM conversion Start/Enable and PGA Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_SDMConvPGA_Dis(i2c_addr)           \
        PT8311_CLEAR_BIT(i2c_addr, ANA_ADC_CTRL1, ADC_CONV_EN_Msk | ADC_SDM_EN_Msk | ADC_PGA_EN_Msk)


/**
  * @brief  ADC PGA fast Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_PGA_Fast_En(i2c_addr)              PT8311_SET_BIT(i2c_addr, ANA_ADC_CTRL2, ADC_PGA_FAST_SET_Msk)

/**
  * @brief  ADC PGA fast Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_PGA_Fast_Dis(i2c_addr)             PT8311_CLEAR_BIT(i2c_addr, ANA_ADC_CTRL2, ADC_PGA_FAST_SET_Msk)

/**
  * @brief  ADC PGA optimize Set
  * @param  i2c_addr PT8311 I2C address
  * @param  val ADC PGA optimize Set value
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_PGA_Optimize_Set(i2c_addr, val)    \
        PT8311_MODIFY_REG(i2c_addr, ANA_ADC_CTRL2, ADC_PGA_OPTIMIZE_Msk, (val) << ADC_PGA_OPTIMIZE_Pos)


/**
  * @brief  ADC SDM optimize Set
  * @param  i2c_addr PT8311 I2C address
  * @param  val ADC SDM optimize Set value
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_ADC_SDM_Optimize_Set(i2c_addr, val)    \
        PT8311_MODIFY_REG(i2c_addr, ANA_ADC_CTRL3, ADC_SDM_OPTIMIZE_Msk, (val) << ADC_SDM_OPTIMIZE_Pos)


/**
  * @brief  DAC Output Mode Set
  * @param  i2c_addr PT8311 I2C address
  * @param  mode DAC Output Mode, see @ref PT8311_DAC_OutModeETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_OutMode_Set(i2c_addr, mode)        \
        PT8311_MODIFY_REG(i2c_addr, ANA_DAC_CTRL1, DAC_OUT_MODE_Msk, (mode) << DAC_OUT_MODE_Pos)

/**
  * @brief  DAC Output Source DAC Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_OutSrc_DAC_En(i2c_addr)            PT8311_SET_BIT(i2c_addr, ANA_DAC_CTRL1, DAC_OUT_SEL_DAC_Msk)

/**
  * @brief  DAC Output Source DAC Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_OutSrc_DAC_Dis(i2c_addr)           PT8311_CLEAR_BIT(i2c_addr, ANA_DAC_CTRL1, DAC_OUT_SEL_DAC_Msk)

/**
  * @brief  DAC Output Source MIC Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_OutSrc_MIC_En(i2c_addr)            PT8311_SET_BIT(i2c_addr, ANA_DAC_CTRL1, DAC_OUT_SEL_MIC_Msk)

/**
  * @brief  DAC Output Source MIC Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_OutSrc_MIC_Dis(i2c_addr)           PT8311_CLEAR_BIT(i2c_addr, ANA_DAC_CTRL1, DAC_OUT_SEL_MIC_Msk)

/**
  * @brief  DAC Output Stage Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_Out_En(i2c_addr)                   PT8311_SET_BIT(i2c_addr, ANA_DAC_CTRL1, DAC_OUT_EN_Msk)

/**
  * @brief  DAC Output Stage Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_Out_Dis(i2c_addr)                  PT8311_CLEAR_BIT(i2c_addr, ANA_DAC_CTRL1, DAC_OUT_EN_Msk)

/**
  * @brief  DAC DCT Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DCT_En(i2c_addr)                   PT8311_SET_BIT(i2c_addr, ANA_DAC_CTRL1, DAC_DCT_EN_Msk)

/**
  * @brief  DAC DCT Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_DCT_Dis(i2c_addr)                  PT8311_CLEAR_BIT(i2c_addr, ANA_DAC_CTRL1, DAC_DCT_EN_Msk)

/**
  * @brief  DAC Output Stage And DCT Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_OutDCT_En(i2c_addr)                PT8311_SET_BIT(i2c_addr, ANA_DAC_CTRL1, DAC_OUT_EN_Msk | DAC_DCT_EN_Msk)

/**
  * @brief  DAC Output Stage And DCT Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_OutDCT_Dis(i2c_addr)               PT8311_CLEAR_BIT(i2c_addr, ANA_DAC_CTRL1, DAC_OUT_EN_Msk | DAC_DCT_EN_Msk)


/**
  * @brief  DAC Output Auto Ramp Up Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_OutAutoRampUp_En(i2c_addr)         PT8311_SET_BIT(i2c_addr, ANA_DAC_CTRL2, DAC_AUTO_RAMP_Msk)

/**
  * @brief  DAC Output Auto Ramp Up Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_OutAutoRampUp_Dis(i2c_addr)        PT8311_CLEAR_BIT(i2c_addr, ANA_DAC_CTRL2, DAC_AUTO_RAMP_Msk)

/**
  * @brief  DAC Output charging time after ramp up Set
  * @param  i2c_addr PT8311 I2C address
  * @param  time DAC Output charging time, see @ref PT8311_DAC_OutChargeTimeETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_OutChargeTime_Set(i2c_addr, time)  \
        PT8311_MODIFY_REG(i2c_addr, ANA_DAC_CTRL2, DAC_CHARGING_TIME_Msk, (time) << DAC_CHARGING_TIME_Pos)

/**
  * @brief  DAC Output Ramp Time Set
  * @param  i2c_addr PT8311 I2C address
  * @param  time DAC Output Ramp Time, see @ref PT8311_DAC_OutRampTimeETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_OutRampTime_Set(i2c_addr, time)    \
        PT8311_MODIFY_REG(i2c_addr, ANA_DAC_CTRL2, DAC_RAMP_TIME_Msk, (time) << DAC_RAMP_TIME_Pos)

/**
  * @brief  DAC Output ramp up function Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_OutRampUp_En(i2c_addr)             PT8311_SET_BIT(i2c_addr, ANA_DAC_CTRL2, DAC_RAMP_EN_Msk)

/**
  * @brief  DAC Output ramp up function Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_OutRampUp_Dis(i2c_addr)            PT8311_CLEAR_BIT(i2c_addr, ANA_DAC_CTRL2, DAC_RAMP_EN_Msk)

/**
  * @brief  DAC Output ramp up function Enable State Get
  * @param  i2c_addr PT8311 I2C address
  * @return DAC Output ramp up function Enable State
  */
#define __PT8311_DAC_OutRampUpEn_Get(i2c_addr)          \
        PT8311_READ_BIT_SHIFT(i2c_addr, ANA_DAC_CTRL2, DAC_RAMP_EN_Msk, DAC_RAMP_EN_Pos)


/**
  * @brief  DAC OPOUT optimize Set
  * @param  i2c_addr PT8311 I2C address
  * @param  val DAC OPOUT optimize value
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAC_OPOUT_Optimize_Set(i2c_addr, val)  \
        PT8311_MODIFY_REG(i2c_addr, ANA_DAC_CTRL3, DAC_OPOUT_OPTIMIZE_Msk, (val) << DAC_OPOUT_OPTIMIZE_Pos)


/**
  * @brief  I2C Pad Driver Level Set
  * @param  i2c_addr PT8311 I2C address
  * @param  lvl I2C Pad Driver Level, see @ref PT8311_PadDrvLvlETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_I2C_PadDrvLvl_Set(i2c_addr, lvl)       \
        PT8311_MODIFY_REG(i2c_addr, ADDA_DEBUG, I2C_PAD_DRV_LEVEL_Msk, (lvl) << I2C_PAD_DRV_LEVEL_Pos)

/**
  * @brief  DAAD Loop Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAAD_Loop_En(i2c_addr)                 PT8311_SET_BIT(i2c_addr, ADDA_DEBUG, DAAD_LOOP_EN_Msk)

/**
  * @brief  DAAD Loop Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DAAD_Loop_Dis(i2c_addr)                PT8311_CLEAR_BIT(i2c_addr, ADDA_DEBUG, DAAD_LOOP_EN_Msk)

/**
  * @brief  ADC/DAC Debug Function Enable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_Debug_En(i2c_addr)                     PT8311_SET_BIT(i2c_addr, ADDA_DEBUG, DBUG_EN_Msk)

/**
  * @brief  ADC/DAC Debug Function Disable
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_Debug_Dis(i2c_addr)                    PT8311_CLEAR_BIT(i2c_addr, ADDA_DEBUG, DBUG_EN_Msk)

/**
  * @brief  ADC/DAC Debug Function Enable Get
  * @param  i2c_addr PT8311 I2C address
  * @return ADC/DAC Debug Function Enable Status
  */
#define __PT8311_DebugEn_Get(i2c_addr)                  PT8311_READ_BIT_SHIFT(i2c_addr, ADDA_DEBUG, DBUG_EN_Msk, DBUG_EN_Pos)

/**
  * @brief  Debug Function Select DAC
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DebugSel_DAC(i2c_addr)                 PT8311_SET_BIT(i2c_addr, ADDA_DEBUG, DBUG_SEL_Msk)

/**
  * @brief  Debug Function Select ADC
  * @param  i2c_addr PT8311 I2C address
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DebugSel_ADC(i2c_addr)                 PT8311_CLEAR_BIT(i2c_addr, ADDA_DEBUG, DBUG_SEL_Msk)

/**
  * @brief  Debug Function Get
  * @param  i2c_addr PT8311 I2C address
  * @return Debug Function: 0->ADC; 1->DAC
  */
#define __PT8311_DebugFunc_Get(i2c_addr)                PT8311_READ_BIT_SHIFT(i2c_addr, ADDA_DEBUG, DBUG_SEL_Msk, DBUG_SEL_Pos)

/**
  * @brief  Debug Pattern Set
  * @param  i2c_addr PT8311 I2C address
  * @param  pat Debug Pattern, see @ref PT8311_DbgPatETypeDef
  * @retval 0      Success
  * @retval others Failed
  */
#define __PT8311_DebugPat_Set(i2c_addr, pat)            \
        PT8311_MODIFY_REG(i2c_addr, ADDA_DEBUG, DEBUG_PTN_Msk, (pat) << DEBUG_PTN_Pos)

/**
  * @brief  Debug Pattern Get
  * @param  i2c_addr PT8311 I2C address
  * @return Debug Pattern
  */
#define __PT8311_DebugPat_Get(i2c_addr)                 PT8311_READ_BIT_SHIFT(i2c_addr, ADDA_DEBUG, DEBUG_PTN_Msk, DEBUG_PTN_Pos)

/**
  * @}
  */

/**
  * @}
  */


/* Exported Types ------------------------------------------------------------*/
/** @defgroup PT8311_Exported_Types PT8311 Exported Types
  * @brief    PT8311 Exported Types
  * @{
  */

/** @defgroup PT8311_Exported_Types_Group1 PT8311 Register Address Definition
  * @brief    PT8311 Register Address Definition
  * @{
  */

/**
  * @brief PT8311 Register Definition
  */
typedef enum {
    // Power and PLL control register
    SRST = 0x00,            /*!< Chip Soft Reset Register                       */
    PWR_CTRL1,              /*!< Power Control Register 1                       */
    PWR_CTRL2,              /*!< Power Control Register 2                       */
    PLL_CTRL,               /*!< PLL Control Register                           */
    PLL_CFG1,               /*!< PLL Configure Register 1                       */
    PLL_CFG2,               /*!< PLL Configure Register 2                       */
    PLL_CFG3,               /*!< PLL Configure Register 3                       */

    // Clock Control Register
    SYSCLK_CTRL = 0x0A,     /*!< System Clock Control Register                  */
    MODCLK_EN,              /*!< Module Clock Control Register                  */
    ADDA_FS,                /*!< ADDA Sampling Frequency Control Register       */
    ADC_CLK_CTRL,           /*!< ADC Clock Control Register                     */
    DAC_CLK_CTRL,           /*!< DAC Clock Control Register                     */

    // I2S Control Register
    I2S_CTRL = 0x10,        /*!< I2S Control Register                           */
    I2S_FMT1,               /*!< I2S Format Configure Register 1                */
    I2S_FMT2,               /*!< I2S Format Configure Register 2                */
    I2S_FMT3,               /*!< I2S Format Configure Register 3                */
    I2S_TX_CTRL,            /*!< I2S TX Control Register                        */
    I2S_RX_CTRL,            /*!< I2S RX Control Register                        */
    I2S_TXM_CTRL,           /*!< I2S TX Mixer Control Register                  */
    I2S_RXM_CTRL,           /*!< I2S RX Mixer Control Register                  */
    I2S_PAD_CTRL,           /*!< I2S PAD Control Register                       */

    // ADC Digital control Register
    ADC_DIG_CTRL = 0x1A,    /*!< ADC Digital Control Register                   */
    ADC_HPF_COEF,           /*!< ADC HPF Coefficient Register                   */
    ADC_DVC_CTRL,           /*!< ADC Digital Volume Control Register            */
    ADC_SMUTE,              /*!< ADC Digital Soft-Mute Control Register         */

    // ADC EQ Control Register
    ADC_EQ_CTRL = 0x20,     /*!< ADC EQ Control Register                        */
    ADC_EQ1_b0_H,           /*!< ADC EQ Band1 Coefficient b0 Register 1         */
    ADC_EQ1_b0_M,           /*!< ADC EQ Band1 Coefficient b0 Register 2         */
    ADC_EQ1_b0_L,           /*!< ADC EQ Band1 Coefficient b0 Register 3         */
    ADC_EQ1_b1_H,           /*!< ADC EQ Band1 Coefficient b1 Register 1         */
    ADC_EQ1_b1_M,           /*!< ADC EQ Band1 Coefficient b1 Register 2         */
    ADC_EQ1_b1_L,           /*!< ADC EQ Band1 Coefficient b1 Register 3         */
    ADC_EQ1_b2_H,           /*!< ADC EQ Band1 Coefficient b2 Register 1         */
    ADC_EQ1_b2_M,           /*!< ADC EQ Band1 Coefficient b2 Register 2         */
    ADC_EQ1_b2_L,           /*!< ADC EQ Band1 Coefficient b2 Register 3         */
    ADC_EQ1_a1_H,           /*!< ADC EQ Band1 Coefficient a1 Register 1         */
    ADC_EQ1_a1_M,           /*!< ADC EQ Band1 Coefficient a1 Register 2         */
    ADC_EQ1_a1_L,           /*!< ADC EQ Band1 Coefficient a1 Register 3         */
    ADC_EQ1_a2_H,           /*!< ADC EQ Band1 Coefficient a2 Register 1         */
    ADC_EQ1_a2_M,           /*!< ADC EQ Band1 Coefficient a2 Register 2         */
    ADC_EQ1_a2_L,           /*!< ADC EQ Band1 Coefficient a2 Register 3         */

    // ADC DRC Control Register
    ADC_DRC_CTRL = 0x30,    /*!< ADC DRC Control Register                       */
    ADC_PEAK_CTRL,          /*!< ADC DRC Peak Filter Control Register           */
    ADC_ATK_TIME,           /*!< ADC DRC ATK/DCY Time Register                  */
    ADC_DRC_THD1,           /*!< ADC DRC Threshold Register 1                   */
    ADC_DRC_THD2,           /*!< ADC DRC Threshold Register 2                   */
    ADC_DRC_THD3,           /*!< ADC DRC Threshold Register 3                   */
    ADC_DRC_THD4,           /*!< ADC DRC Threshold Register 4                   */
    ADC_DRC_SLP1,           /*!< ADC DRC Slope Register 1                       */
    ADC_DRC_SLP2,           /*!< ADC DRC Slope Register 2                       */

    // DAC Digital control Register
    DAC_DIG_CTRL = 0x3A,    /*!< DAC Digital Control Register                   */
    DAC_DVC_CTRL,           /*!< DAC Digital Volume Control Register            */
    DAC_SMUTE,              /*!< DAC Digital Soft-Mute Control Register         */
    DAC_DMIX_CTRL,          /*!< DAC Digital Mixer Control Register             */

    // DAC EQ Control Register
    DAC_EQ_CTRL = 0x40,     /*!< DAC EQ Control Register                        */
    DAC_EQ1_b0_H,           /*!< DAC EQ Band1 Coefficient b0 Register 1         */
    DAC_EQ1_b0_M,           /*!< DAC EQ Band1 Coefficient b0 Register 2         */
    DAC_EQ1_b0_L,           /*!< DAC EQ Band1 Coefficient b0 Register 3         */
    DAC_EQ1_b1_H,           /*!< DAC EQ Band1 Coefficient b1 Register 1         */
    DAC_EQ1_b1_M,           /*!< DAC EQ Band1 Coefficient b1 Register 2         */
    DAC_EQ1_b1_L,           /*!< DAC EQ Band1 Coefficient b1 Register 3         */
    DAC_EQ1_b2_H,           /*!< DAC EQ Band1 Coefficient b2 Register 1         */
    DAC_EQ1_b2_M,           /*!< DAC EQ Band1 Coefficient b2 Register 2         */
    DAC_EQ1_b2_L,           /*!< DAC EQ Band1 Coefficient b2 Register 3         */
    DAC_EQ1_a1_H,           /*!< DAC EQ Band1 Coefficient a1 Register 1         */
    DAC_EQ1_a1_M,           /*!< DAC EQ Band1 Coefficient a1 Register 2         */
    DAC_EQ1_a1_L,           /*!< DAC EQ Band1 Coefficient a1 Register 3         */
    DAC_EQ1_a2_H,           /*!< DAC EQ Band1 Coefficient a2 Register 1         */
    DAC_EQ1_a2_M,           /*!< DAC EQ Band1 Coefficient a2 Register 2         */
    DAC_EQ1_a2_L,           /*!< DAC EQ Band1 Coefficient a2 Register 3         */
    DAC_EQ2_b0_H = 0x51,    /*!< DAC EQ Band2 Coefficient b0 Register 1         */
    DAC_EQ2_b0_M,           /*!< DAC EQ Band2 Coefficient b0 Register 2         */
    DAC_EQ2_b0_L,           /*!< DAC EQ Band2 Coefficient b0 Register 3         */
    DAC_EQ2_b1_H,           /*!< DAC EQ Band2 Coefficient b1 Register 1         */
    DAC_EQ2_b1_M,           /*!< DAC EQ Band2 Coefficient b1 Register 2         */
    DAC_EQ2_b1_L,           /*!< DAC EQ Band2 Coefficient b1 Register 3         */
    DAC_EQ2_b2_H,           /*!< DAC EQ Band2 Coefficient b2 Register 1         */
    DAC_EQ2_b2_M,           /*!< DAC EQ Band2 Coefficient b2 Register 2         */
    DAC_EQ2_b2_L,           /*!< DAC EQ Band2 Coefficient b2 Register 3         */
    DAC_EQ2_a1_H,           /*!< DAC EQ Band2 Coefficient a1 Register 1         */
    DAC_EQ2_a1_M,           /*!< DAC EQ Band2 Coefficient a1 Register 2         */
    DAC_EQ2_a1_L,           /*!< DAC EQ Band2 Coefficient a1 Register 3         */
    DAC_EQ2_a2_H,           /*!< DAC EQ Band2 Coefficient a2 Register 1         */
    DAC_EQ2_a2_M,           /*!< DAC EQ Band2 Coefficient a2 Register 2         */
    DAC_EQ2_a2_L,           /*!< DAC EQ Band2 Coefficient a2 Register 3         */
    DAC_EQ3_b0_H = 0x61,    /*!< DAC EQ Band3 Coefficient b0 Register 1         */
    DAC_EQ3_b0_M,           /*!< DAC EQ Band3 Coefficient b0 Register 2         */
    DAC_EQ3_b0_L,           /*!< DAC EQ Band3 Coefficient b0 Register 3         */
    DAC_EQ3_b1_H,           /*!< DAC EQ Band3 Coefficient b1 Register 1         */
    DAC_EQ3_b1_M,           /*!< DAC EQ Band3 Coefficient b1 Register 2         */
    DAC_EQ3_b1_L,           /*!< DAC EQ Band3 Coefficient b1 Register 3         */
    DAC_EQ3_b2_H,           /*!< DAC EQ Band3 Coefficient b2 Register 1         */
    DAC_EQ3_b2_M,           /*!< DAC EQ Band3 Coefficient b2 Register 2         */
    DAC_EQ3_b2_L,           /*!< DAC EQ Band3 Coefficient b2 Register 3         */
    DAC_EQ3_a1_H,           /*!< DAC EQ Band3 Coefficient a1 Register 1         */
    DAC_EQ3_a1_M,           /*!< DAC EQ Band3 Coefficient a1 Register 2         */
    DAC_EQ3_a1_L,           /*!< DAC EQ Band3 Coefficient a1 Register 3         */
    DAC_EQ3_a2_H,           /*!< DAC EQ Band3 Coefficient a2 Register 1         */
    DAC_EQ3_a2_M,           /*!< DAC EQ Band3 Coefficient a2 Register 2         */
    DAC_EQ3_a2_L,           /*!< DAC EQ Band3 Coefficient a2 Register 3         */

    // DAC DRC Control Register
    DAC_DRC_CTRL = 0x70,    /*!< DAC DRC Control Register                       */
    DAC_PEAK_CTRL,          /*!< DAC DRC Peak Filter Control Register           */
    DAC_ATK_TIME,           /*!< DAC DRC ATK/DCY Time Register                  */
    DAC_DRC_THD1,           /*!< DAC DRC Threshold Register 1                   */
    DAC_DRC_THD2,           /*!< DAC DRC Threshold Register 2                   */
    DAC_DRC_THD3,           /*!< DAC DRC Threshold Register 3                   */
    DAC_DRC_THD4,           /*!< DAC DRC Threshold Register 4                   */
    DAC_DRC_SLP1,           /*!< DAC DRC Slope Register 1                       */
    DAC_DRC_SLP2,           /*!< DAC DRC Slope Register 2                       */

    // ADC Analog Control Register
    ANA_ADC_CTRL1 = 0x7A,   /*!< ADC Analog Control Register 1                  */
    ANA_ADC_CTRL2,          /*!< ADC Analog Control Register 2                  */
    ANA_ADC_CTRL3,          /*!< ADC Analog Control Register 3                  */
    ANA_ADC_TUNE1 = 0x7E,   /*!< ADC Analog Tuning Register 1                   */
    ANA_ADC_TUNE2,          /*!< ADC Analog Tuning Register 2                   */

    // DAC Analog Control Register
    ANA_DAC_CTRL1 = 0x80,   /*!< DAC Analog Control Register 1                  */
    ANA_DAC_CTRL2,          /*!< DAC Analog Control Register 2                  */
    ANA_DAC_CTRL3 = 0x83,   /*!< DAC Analog Control Register 3                  */
    ANA_DAC_TUNE1,          /*!< DAC Analog Tuning Register 1                   */
    ANA_DAC_TUNE2,          /*!< DAC Analog Tuning Register 2                   */

    // Debug Register
    ADDA_DEBUG = 0xFA,      /*!< ADDA Digital Debug Register                    */

    // PT8311 Max Register
    PT8311_MAX_REG = ADDA_DEBUG,    /*!< PT8311 Max Register                    */
} PT8311_RegETypeDef;

/**
  * @}
  */


/** @defgroup PT8311_Exported_Types_Group2 PT8311 Register Config Enum Definition
  * @brief    PT8311 Register Config Enum Definition
  * @{
  */

/**
  * @brief PT8311 ALDO Output Voltage Definition
  */
typedef enum {
    PT8311_ALDO_VOL_1V6 = 0,                /*!< ALDO Output Voltage 1.6V       */
    PT8311_ALDO_VOL_1V65,                   /*!< ALDO Output Voltage 1.65V      */
    PT8311_ALDO_VOL_1V7,                    /*!< ALDO Output Voltage 1.7V       */
    PT8311_ALDO_VOL_1V75,                   /*!< ALDO Output Voltage 1.75V      */
    PT8311_ALDO_VOL_1V8,                    /*!< ALDO Output Voltage 1.8V       */
    PT8311_ALDO_VOL_1V85,                   /*!< ALDO Output Voltage 1.85V      */
    PT8311_ALDO_VOL_1V9,                    /*!< ALDO Output Voltage 1.9V       */
    PT8311_ALDO_VOL_1V95,                   /*!< ALDO Output Voltage 1.95V      */
} PT8311_ALDOVolETypeDef;

/**
  * @brief PT8311 Micbias chop frequency @6144kHz Definition
  */
typedef enum {
    PT8311_MICBIAS_CHOP_FREQ_24KHz = 0,     /*!< Micbias chop frequency 24KHz   */
    PT8311_MICBIAS_CHOP_FREQ_48KHz,         /*!< Micbias chop frequency 48KHz   */
    PT8311_MICBIAS_CHOP_FREQ_96KHz,         /*!< Micbias chop frequency 96KHz   */
    PT8311_MICBIAS_CHOP_FREQ_192KHz,        /*!< Micbias chop frequency 192KHz  */
    PT8311_MICBIAS_CHOP_FREQ_384KHz,        /*!< Micbias chop frequency 384KHz  */
    PT8311_MICBIAS_CHOP_FREQ_768KHz,        /*!< Micbias chop frequency 768KHz  */
    PT8311_MICBIAS_CHOP_FREQ_1536KHz,       /*!< Micbias chop frequency 1536KHz */
    PT8311_MICBIAS_CHOP_FREQ_3072KHz,       /*!< Micbias chop frequency 3072KHz */
} PT8311_MicbiasChopFreqETypeDef;

/**
  * @brief PT8311 Micbias output voltage Definition
  */
typedef enum {
    PT8311_MICBIAS_VOL_1V8 = 0,             /*!< Micbias output voltage 1.8V    */
    PT8311_MICBIAS_VOL_2V1,                 /*!< Micbias output voltage 2.1V    */
    PT8311_MICBIAS_VOL_2V3,                 /*!< Micbias output voltage 2.3V    */
    PT8311_MICBIAS_VOL_2V5,                 /*!< Micbias output voltage 2.5V    */
} PT8311_MicbiasVolETypeDef;

/**
  * @brief PT8311 PLL LDO Output Voltage Definition
  */
typedef enum {
    PT8311_PLL_LDO_VOL_1V4 = 0,             /*!< PLL LDO Output Voltage 1.4V    */
    PT8311_PLL_LDO_VOL_1V5,                 /*!< PLL LDO Output Voltage 1.5V    */
    PT8311_PLL_LDO_VOL_1V6,                 /*!< PLL LDO Output Voltage 1.6V    */
    PT8311_PLL_LDO_VOL_1V7,                 /*!< PLL LDO Output Voltage 1.7V    */
} PT8311_PLLLDOVolETypeDef;

/**
  * @brief PT8311 PLL Clock Source Definition
  */
typedef enum {
    PT8311_PLLCLK_SRC_MCLK = 0,             /*!< PLL Clock Source MCLK          */
    PT8311_PLLCLK_SRC_BCLK,                 /*!< PLL Clock Source BCLK          */
} PT8311_PLLCLKSrcETypeDef;

/**
  * @brief PT8311 System Clock Source Definition
  */
typedef enum {
    PT8311_SYSCLK_SRC_MCLK = 0,             /*!< System Clock Source MCLK       */
    PT8311_SYSCLK_SRC_BCLK,                 /*!< System Clock Source BCLK       */
    PT8311_SYSCLK_SRC_PLL,                  /*!< System Clock Source PLL        */
} PT8311_SysclkSrcETypeDef;

/**
  * @brief PT8311 ADC/DAC Sample Rate Definition
  */
typedef enum {
    PT8311_ADDA_SR_8K = 0,                  /*!< ADC/DAC Sample Rate 8K         */
    PT8311_ADDA_SR_12K,                     /*!< ADC/DAC Sample Rate 12K        */
    PT8311_ADDA_SR_16K,                     /*!< ADC/DAC Sample Rate 16K        */
    PT8311_ADDA_SR_24K,                     /*!< ADC/DAC Sample Rate 24K        */
    PT8311_ADDA_SR_32K,                     /*!< ADC/DAC Sample Rate 32K        */
    PT8311_ADDA_SR_48K,                     /*!< ADC/DAC Sample Rate 48K        */
    PT8311_ADDA_SR_96K,                     /*!< ADC/DAC Sample Rate 96K        */
    PT8311_ADDA_SR_192K,                    /*!< ADC/DAC Sample Rate 192K       */

    PT8311_ADDA_SR_11025 = 1,               /*!< ADC/DAC Sample Rate 11.025K    */
    PT8311_ADDA_SR_22050 = 3,               /*!< ADC/DAC Sample Rate 22.05K     */
    PT8311_ADDA_SR_44100 = 5,               /*!< ADC/DAC Sample Rate 44.1K      */
    PT8311_ADDA_SR_88200,                   /*!< ADC/DAC Sample Rate 88.2K      */
    PT8311_ADDA_SR_176400,                  /*!< ADC/DAC Sample Rate 176.4K     */
} PT8311_ADDA_SmpRateETypeDef;

/**
  * @brief PT8311 ADC/DAC Oversampling Ratio Definition
  */
typedef enum {
    PT8311_ADDA_OSR_32 = 0,                 /*!< ADC/DAC OSR 32                 */
    PT8311_ADDA_OSR_64,                     /*!< ADC/DAC OSR 64                 */
    PT8311_ADDA_OSR_128,                    /*!< ADC/DAC OSR 128                */
    PT8311_ADDA_OSR_256,                    /*!< ADC/DAC OSR 256                */
} PT8311_ADDA_OSRETypeDef;

/**
  * @brief PT8311 ADC/DAC DCLK(Oversampling Clock) Divide Definition
  */
typedef enum {
    PT8311_ADDA_DCLK_DIV_1 = 0,             /*!< ADC/DAC DCLK divide 1          */
    PT8311_ADDA_DCLK_DIV_2,                 /*!< ADC/DAC DCLK divide 2          */
    PT8311_ADDA_DCLK_DIV_3,                 /*!< ADC/DAC DCLK divide 3          */
    PT8311_ADDA_DCLK_DIV_4,                 /*!< ADC/DAC DCLK divide 4          */
    PT8311_ADDA_DCLK_DIV_6,                 /*!< ADC/DAC DCLK divide 6          */
    PT8311_ADDA_DCLK_DIV_8,                 /*!< ADC/DAC DCLK divide 8          */
    PT8311_ADDA_DCLK_DIV_12,                /*!< ADC/DAC DCLK divide 12         */
    PT8311_ADDA_DCLK_DIV_16,                /*!< ADC/DAC DCLK divide 16         */
    PT8311_ADDA_DCLK_DIV_24,                /*!< ADC/DAC DCLK divide 24         */
} PT8311_ADDA_DCLKDivETypeDef;

/**
  * @brief PT8311 MCLK IO Pin Function Definition
  */
typedef enum {
    PT8311_MCLK_IO_PIN_FUNC_INPUT = 0,      /*!< Pin Func Input                 */
    PT8311_MCLK_IO_PIN_FUNC_OUTPUT_PDMCLK,  /*!< Pin Func OUTPUT_PDMCLK         */
    PT8311_MCLK_IO_PIN_FUNC_OUTPUT_SYSCLK,  /*!< Pin Func OUTPUT_SYSCLK         */
} PT8311_MclkIOPinFuncETypeDef;

/**
  * @brief PT8311 I2S Role Definition
  */
typedef enum {
    PT8311_I2S_ROLE_SLAVE = 0,              /*!< I2S Role Slave                 */
    PT8311_I2S_ROLE_MASTER,                 /*!< I2S Role Master                */
} PT8311_I2SRoleETypeDef;

/**
  * @brief PT8311 I2S BCLK Divide from SYSCLK Definition
  */
typedef enum {
    PT8311_I2S_BCLK_DIV_1 = 0,              /*!< I2S BCLK divide 1              */
    PT8311_I2S_BCLK_DIV_2,                  /*!< I2S BCLK divide 2              */
    PT8311_I2S_BCLK_DIV_3,                  /*!< I2S BCLK divide 3              */
    PT8311_I2S_BCLK_DIV_4,                  /*!< I2S BCLK divide 4              */
    PT8311_I2S_BCLK_DIV_6,                  /*!< I2S BCLK divide 6              */
    PT8311_I2S_BCLK_DIV_8,                  /*!< I2S BCLK divide 8              */
    PT8311_I2S_BCLK_DIV_12,                 /*!< I2S BCLK divide 12             */
    PT8311_I2S_BCLK_DIV_16,                 /*!< I2S BCLK divide 16             */
    PT8311_I2S_BCLK_DIV_24,                 /*!< I2S BCLK divide 24             */
} PT8311_I2S_BCLKDivETypeDef;

/**
  * @brief PT8311 I2S Format Definition
  */
typedef enum {
    PT8311_I2S_FMT_I2S = 0,                 /*!< I2S Format I2S_Standard        */
    PT8311_I2S_FMT_LEFT_J,                  /*!< I2S Format Left-Justified      */
    PT8311_I2S_FMT_PCM_A,                   /*!< I2S Format PCM_A               */
    PT8311_I2S_FMT_PCM_B,                   /*!< I2S Format PCM_B               */
} PT8311_I2S_FmtETypeDef;

/**
  * @brief PT8311 I2S Word Length Definition
  */
typedef enum {
    PT8311_I2S_WORD_LEN_16bit = 0,          /*!< I2S Word Length 16bit          */
    PT8311_I2S_WORD_LEN_20bit,              /*!< I2S Word Length 20bit          */
    PT8311_I2S_WORD_LEN_24bit,              /*!< I2S Word Length 24bit          */
    PT8311_I2S_WORD_LEN_32bit,              /*!< I2S Word Length 32bit          */
} PT8311_I2S_WordLenETypeDef;

/**
  * @brief PT8311 I2S Tx Slot Mapping Definition
  */
typedef enum {
    PT8311_TX_SLOT_MAP_TXL = 0,             /*!< I2S Tx Slot Map TXL            */
    PT8311_TX_SLOT_MAP_TXR,                 /*!< I2S Tx Slot Map TXR            */
} PT8311_I2S_TxSlotMapETypeDef;

/**
  * @brief PT8311 I2S Rx Channel Mapping Definition
  */
typedef enum {
    PT8311_I2S_RX_CH_MAP_SLOT0 = 0,         /*!< I2S Rx Channel Map Slot0       */
    PT8311_I2S_RX_CH_MAP_SLOT1,             /*!< I2S Rx Channel Map Slot1       */
} PT8311_I2S_RxChMapETypeDef;

/**
  * @brief PT8311 I2S/I2C Pad Driver Level Definition
  */
typedef enum {
    PT8311_PAD_DRV_LVL_0 = 0,               /*!< Pad Driver Level 0             */
    PT8311_PAD_DRV_LVL_1,                   /*!< Pad Driver Level 1             */
    PT8311_PAD_DRV_LVL_2,                   /*!< Pad Driver Level 2             */
    PT8311_PAD_DRV_LVL_3,                   /*!< Pad Driver Level 3             */
} PT8311_PadDrvLvlETypeDef;

/**
  * @brief PT8311 ADC Delay Function Definition
  */
typedef enum {
    PT8311_ADC_DELAY_FUNC_0ms = 0,          /*!< ADC Delay Function 0ms         */
    PT8311_ADC_DELAY_FUNC_4ms,              /*!< ADC Delay Function 4ms         */
    PT8311_ADC_DELAY_FUNC_8ms,              /*!< ADC Delay Function 8ms         */
    PT8311_ADC_DELAY_FUNC_16ms,             /*!< ADC Delay Function 16ms        */
} PT8311_ADCDelayFuncETypeDef;

/**
  * @brief PT8311 ADC HPF Coefficient Definition
  */
typedef enum {
    PT8311_ADC_HPF_COFF_1Hz = 0,            /*!< ADC HPF Coefficient 1Hz        */
    PT8311_ADC_HPF_COFF_10Hz,               /*!< ADC HPF Coefficient 10Hz       */
    PT8311_ADC_HPF_COFF_20Hz,               /*!< ADC HPF Coefficient 20Hz       */
    PT8311_ADC_HPF_COFF_70Hz,               /*!< ADC HPF Coefficient 70Hz       */
    PT8311_ADC_HPF_COFF_200Hz,              /*!< ADC HPF Coefficient 200Hz      */
} PT8311_ADC_HPFCoffETypeDef;

/**
  * @brief PT8311 ADC/DAC Soft Volume Ramp Rate Definition
  */
typedef enum {
    PT8311_SOFT_VOL_RAMP_RATE_4RAMPCLK = 0, /*!< Volume Ramp Rate 4RAMPCLK      */
    PT8311_SOFT_VOL_RAMP_RATE_8RAMPCLK,     /*!< Volume Ramp Rate 8RAMPCLK      */
    PT8311_SOFT_VOL_RAMP_RATE_16RAMPCLK,    /*!< Volume Ramp Rate 16RAMPCLK     */
    PT8311_SOFT_VOL_RAMP_RATE_32RAMPCLK,    /*!< Volume Ramp Rate 32RAMPCLK     */
    PT8311_SOFT_VOL_RAMP_RATE_64RAMPCLK,    /*!< Volume Ramp Rate 64RAMPCLK     */
    PT8311_SOFT_VOL_RAMP_RATE_128RAMPCLK,   /*!< Volume Ramp Rate 128RAMPCLK    */
    PT8311_SOFT_VOL_RAMP_RATE_256RAMPCLK,   /*!< Volume Ramp Rate 256RAMPCLK    */
    PT8311_SOFT_VOL_RAMP_RATE_512RAMPCLK,   /*!< Volume Ramp Rate 512RAMPCLK    */
} PT8311_SoftVolRampRateETypeDef;

/**
  * @brief PT8311 ADC/DAC DRC Smooth Filter Mode Definition
  */
typedef enum {
    PT8311_DRC_SMOOTH_MODE_IMPROVED = 0,    /*!< DRC Smooth mode Improved       */
    PT8311_DRC_SMOOTH_MODE_TRADITIONAL,     /*!< DRC Smooth mode Traditional    */
} PT8311_DRCSmoothModeETypeDef;

/**
  * @brief PT8311 ADC/DAC DRC peak filter attack time Definition
  */
typedef enum {
    PT8311_DRC_PEAK_ATK_TIME_1Ts = 0,       /*!< peak attack time 1Ts           */
    PT8311_DRC_PEAK_ATK_TIME_2Ts,           /*!< peak attack time 2Ts           */
    PT8311_DRC_PEAK_ATK_TIME_4Ts,           /*!< peak attack time 4Ts           */
    PT8311_DRC_PEAK_ATK_TIME_8Ts,           /*!< peak attack time 8Ts           */
    PT8311_DRC_PEAK_ATK_TIME_16Ts,          /*!< peak attack time 16Ts          */
    PT8311_DRC_PEAK_ATK_TIME_32Ts,          /*!< peak attack time 32Ts          */
    PT8311_DRC_PEAK_ATK_TIME_64Ts,          /*!< peak attack time 64Ts          */
    PT8311_DRC_PEAK_ATK_TIME_128Ts,         /*!< peak attack time 128Ts         */
    PT8311_DRC_PEAK_ATK_TIME_256Ts,         /*!< peak attack time 256Ts         */
    PT8311_DRC_PEAK_ATK_TIME_1024Ts,        /*!< peak attack time 1024Ts        */
} PT8311_DRCPeakAtkTimeETypeDef;

/**
  * @brief PT8311 ADC/DAC DRC peak filter release time Definition
  */
typedef enum {
    PT8311_DRC_PEAK_RLS_TIME_64Ts = 0,      /*!< peak release time 64Ts         */
    PT8311_DRC_PEAK_RLS_TIME_128Ts,         /*!< peak release time 128Ts        */
    PT8311_DRC_PEAK_RLS_TIME_256Ts,         /*!< peak release time 256Ts        */
    PT8311_DRC_PEAK_RLS_TIME_512Ts,         /*!< peak release time 512Ts        */
    PT8311_DRC_PEAK_RLS_TIME_1024Ts,        /*!< peak release time 1024Ts       */
    PT8311_DRC_PEAK_RLS_TIME_2048Ts,        /*!< peak release time 2048Ts       */
    PT8311_DRC_PEAK_RLS_TIME_4096Ts,        /*!< peak release time 4096Ts       */
    PT8311_DRC_PEAK_RLS_TIME_8192Ts,        /*!< peak release time 8192Ts       */
    PT8311_DRC_PEAK_RLS_TIME_16384Ts,       /*!< peak release time 16384Ts      */
    PT8311_DRC_PEAK_RLS_TIME_32768Ts,       /*!< peak release time 32768Ts      */
} PT8311_DRCPeakRlsTimeETypeDef;

/**
  * @brief PT8311 ADC/DAC DRC attack time Definition
  */
typedef enum {
    PT8311_DRC_ATK_TIME_2Ts = 0,            /*!< attack time 2Ts                */
    PT8311_DRC_ATK_TIME_4Ts,                /*!< attack time 4Ts                */
    PT8311_DRC_ATK_TIME_8Ts,                /*!< attack time 8Ts                */
    PT8311_DRC_ATK_TIME_16Ts,               /*!< attack time 16Ts               */
    PT8311_DRC_ATK_TIME_32Ts,               /*!< attack time 32Ts               */
    PT8311_DRC_ATK_TIME_64Ts,               /*!< attack time 64Ts               */
    PT8311_DRC_ATK_TIME_128Ts,              /*!< attack time 128Ts              */
    PT8311_DRC_ATK_TIME_256Ts,              /*!< attack time 256Ts              */
    PT8311_DRC_ATK_TIME_512Ts,              /*!< attack time 512Ts              */
    PT8311_DRC_ATK_TIME_1024Ts,             /*!< attack time 1024Ts             */
    PT8311_DRC_ATK_TIME_2048Ts,             /*!< attack time 2048Ts             */
    PT8311_DRC_ATK_TIME_4096Ts,             /*!< attack time 4096Ts             */
} PT8311_DRCAtkTimeETypeDef;

/**
  * @brief PT8311 ADC/DAC DRC decay time Definition
  */
typedef enum {
    PT8311_DRC_DCY_TIME_64Ts = 0,           /*!< decay time 64Ts                */
    PT8311_DRC_DCY_TIME_128Ts,              /*!< decay time 128Ts               */
    PT8311_DRC_DCY_TIME_256Ts,              /*!< decay time 256Ts               */
    PT8311_DRC_DCY_TIME_512Ts,              /*!< decay time 512Ts               */
    PT8311_DRC_DCY_TIME_1024Ts,             /*!< decay time 1024Ts              */
    PT8311_DRC_DCY_TIME_2048Ts,             /*!< decay time 2048Ts              */
    PT8311_DRC_DCY_TIME_4096Ts,             /*!< decay time 4096Ts              */
    PT8311_DRC_DCY_TIME_8192Ts,             /*!< decay time 8192Ts              */
    PT8311_DRC_DCY_TIME_16384Ts,            /*!< decay time 16384Ts             */
    PT8311_DRC_DCY_TIME_32768Ts,            /*!< decay time 32768Ts             */
    PT8311_DRC_DCY_TIME_65536Ts,            /*!< decay time 65536Ts             */
    PT8311_DRC_DCY_TIME_131072Ts,           /*!< decay time 131072Ts            */
} PT8311_DRCDcyTimeETypeDef;

/**
  * @brief PT8311 ADC/DAC DRC limiter slope Definition
  */
typedef enum {
    PT8311_DRC_LMT_SLOPE_1_1 = 0,           /*!< DRC limiter slope 1/1          */
    PT8311_DRC_LMT_SLOPE_1_2,               /*!< DRC limiter slope 1/2          */
    PT8311_DRC_LMT_SLOPE_1_4,               /*!< DRC limiter slope 1/4          */
    PT8311_DRC_LMT_SLOPE_1_8,               /*!< DRC limiter slope 1/8          */
    PT8311_DRC_LMT_SLOPE_1_16,              /*!< DRC limiter slope 1/16         */
    PT8311_DRC_LMT_SLOPE_1_32,              /*!< DRC limiter slope 1/32         */
    PT8311_DRC_LMT_SLOPE_1_64,              /*!< DRC limiter slope 1/64         */
    PT8311_DRC_LMT_SLOPE_0_1,               /*!< DRC limiter slope 0/1          */
} PT8311_DRCLmtSlopeETypeDef;

/**
  * @brief PT8311 ADC/DAC DRC compressor slope Definition
  */
typedef enum {
    PT8311_DRC_CMP_SLOPE_1_1 = 0,           /*!< DRC compressor slope 1/1       */
    PT8311_DRC_CMP_SLOPE_3_4,               /*!< DRC compressor slope 3/4       */
    PT8311_DRC_CMP_SLOPE_1_2,               /*!< DRC compressor slope 1/2       */
    PT8311_DRC_CMP_SLOPE_1_4,               /*!< DRC compressor slope 1/4       */
    PT8311_DRC_CMP_SLOPE_1_8,               /*!< DRC compressor slope 1/8       */
    PT8311_DRC_CMP_SLOPE_1_16,              /*!< DRC compressor slope 1/16      */
    PT8311_DRC_CMP_SLOPE_1_32,              /*!< DRC compressor slope 1/32      */
    PT8311_DRC_CMP_SLOPE_0_1,               /*!< DRC compressor slope 0/1       */
} PT8311_DRCCmpSlopeETypeDef;

/**
  * @brief PT8311 ADC/DAC DRC expander slope Definition
  */
typedef enum {
    PT8311_DRC_EXP_SLOPE_1_1 = 0,           /*!< DRC expander slope 1/1         */
    PT8311_DRC_EXP_SLOPE_2_1,               /*!< DRC expander slope 2/1         */
    PT8311_DRC_EXP_SLOPE_4_1,               /*!< DRC expander slope 4/1         */
    PT8311_DRC_EXP_SLOPE_8_1,               /*!< DRC expander slope 8/1         */
} PT8311_DRCExpSlopeETypeDef;

/**
  * @brief PT8311 ADC/DAC DRC noise gate slope Definition
  */
typedef enum {
    PT8311_DRC_NG_SLOPE_1_1 = 0,            /*!< DRC noise gate slope 1/1       */
    PT8311_DRC_NG_SLOPE_2_1,                /*!< DRC noise gate slope 2/1       */
    PT8311_DRC_NG_SLOPE_4_1,                /*!< DRC noise gate slope 4/1       */
    PT8311_DRC_NG_SLOPE_8_1,                /*!< DRC noise gate slope 8/1       */
} PT8311_DRCNgSlopeETypeDef;

/**
  * @brief PT8311 DAC Dither Amplitude Definition
  */
typedef enum {
    PT8311_DAC_DITHER_AMP_1_8LSB = 0,       /*!< DAC Dither Amplitude 1/8LSB    */
    PT8311_DAC_DITHER_AMP_2_8LSB,           /*!< DAC Dither Amplitude 2/8LSB    */
    PT8311_DAC_DITHER_AMP_3_8LSB,           /*!< DAC Dither Amplitude 3/8LSB    */
    PT8311_DAC_DITHER_AMP_4_8LSB,           /*!< DAC Dither Amplitude 4/8LSB    */
} PT8311_DAC_DitherAmpETypeDef;

/**
  * @brief PT8311 ADC PGA Gain Definition
  */
typedef enum {
    PT8311_ADC_PGA_GAIN_0dB = 0,            /*!< ADC PGA Gain 0dB               */
    PT8311_ADC_PGA_GAIN_6dB = 2,            /*!< ADC PGA Gain 6dB               */
    PT8311_ADC_PGA_GAIN_9dB,                /*!< ADC PGA Gain 9dB               */
    PT8311_ADC_PGA_GAIN_12dB,               /*!< ADC PGA Gain 12dB              */
    PT8311_ADC_PGA_GAIN_15dB,               /*!< ADC PGA Gain 15dB              */
    PT8311_ADC_PGA_GAIN_18dB,               /*!< ADC PGA Gain 18dB              */
    PT8311_ADC_PGA_GAIN_21dB,               /*!< ADC PGA Gain 21dB              */
    PT8311_ADC_PGA_GAIN_24dB,               /*!< ADC PGA Gain 24dB              */
    PT8311_ADC_PGA_GAIN_27dB,               /*!< ADC PGA Gain 27dB              */
    PT8311_ADC_PGA_GAIN_30dB,               /*!< ADC PGA Gain 30dB              */
    PT8311_ADC_PGA_GAIN_33dB,               /*!< ADC PGA Gain 33dB              */
    PT8311_ADC_PGA_GAIN_36dB,               /*!< ADC PGA Gain 36dB              */
    PT8311_ADC_PGA_GAIN_39dB,               /*!< ADC PGA Gain 39dB              */
    PT8311_ADC_PGA_GAIN_42dB,               /*!< ADC PGA Gain 42dB              */
} PT8311_ADC_PGAGainETypeDef;

/**
  * @brief PT8311 DAC Output Mode Definition
  */
typedef enum {
    PT8311_DAC_OUT_MODE_SINGLE = 0,         /*!< DAC Out Mode Single            */
    PT8311_DAC_OUT_MODE_DIFFERENTIAL,       /*!< DAC Out Mode Differential      */
} PT8311_DAC_OutModeETypeDef;

/**
  * @brief PT8311 DAC Output charge time after ramp up Definition
  */
typedef enum {
    PT8311_DAC_OUT_CHARGE_TIME_128ms = 0,   /*!< DAC Out charge time 128ms      */
    PT8311_DAC_OUT_CHARGE_TIME_256ms,       /*!< DAC Out charge time 256ms      */
    PT8311_DAC_OUT_CHARGE_TIME_384ms,       /*!< DAC Out charge time 384ms      */
    PT8311_DAC_OUT_CHARGE_TIME_512ms,       /*!< DAC Out charge time 512ms      */
    PT8311_DAC_OUT_CHARGE_TIME_640ms,       /*!< DAC Out charge time 640ms      */
    PT8311_DAC_OUT_CHARGE_TIME_768ms,       /*!< DAC Out charge time 768ms      */
    PT8311_DAC_OUT_CHARGE_TIME_896ms,       /*!< DAC Out charge time 896ms      */
    PT8311_DAC_OUT_CHARGE_TIME_1024ms,      /*!< DAC Out charge time 1024ms     */
} PT8311_DAC_OutChargeTimeETypeDef;

/**
  * @brief PT8311 DAC Output ramp time Definition
  */
typedef enum {
    PT8311_DAC_OUT_RAMP_TIME_256ms = 0,     /*!< DAC Out Ramp time 256ms        */
    PT8311_DAC_OUT_RAMP_TIME_512ms,         /*!< DAC Out Ramp time 512ms        */
    PT8311_DAC_OUT_RAMP_TIME_768ms,         /*!< DAC Out Ramp time 768ms        */
    PT8311_DAC_OUT_RAMP_TIME_1024ms,        /*!< DAC Out Ramp time 1024ms       */
    PT8311_DAC_OUT_RAMP_TIME_1280ms,        /*!< DAC Out Ramp time 1280ms       */
    PT8311_DAC_OUT_RAMP_TIME_1536ms,        /*!< DAC Out Ramp time 1536ms       */
    PT8311_DAC_OUT_RAMP_TIME_1792ms,        /*!< DAC Out Ramp time 1792ms       */
    PT8311_DAC_OUT_RAMP_TIME_2048ms,        /*!< DAC Out Ramp time 2048ms       */
} PT8311_DAC_OutRampTimeETypeDef;

/**
  * @brief PT8311 ADC/DAC Debug Pattern Definition
  */
typedef enum {
    PT8311_DBG_PAT_ZERO_DATA = 0,           /*!< Debug Pattern Zero data        */
    PT8311_DBG_PAT_0dB_Sin = 3,             /*!< Debug Pattern 0dB Sin          */

    // 22bit
    PT8311_DBG_PAT_0x296969 = 1,            /*!< Debug Pattern 0x296969         */
    PT8311_DBG_PAT_0x048D15 = 2,            /*!< Debug Pattern 0x048D15         */

    // 24bit
    PT8311_DBG_PAT_0xA5A5A5 = 1,            /*!< Debug Pattern 0xA5A5A5         */
    PT8311_DBG_PAT_0x123454 = 2,            /*!< Debug Pattern 0x123454         */

    // 20bit
    PT8311_DBG_PAT_0xA5A5A = 1,             /*!< Debug Pattern 0xA5A5A          */
    PT8311_DBG_PAT_0x12345 = 2,             /*!< Debug Pattern 0x12345          */

    // 16bit
    PT8311_DBG_PAT_0xA5A5 = 1,              /*!< Debug Pattern 0xA5A5           */
    PT8311_DBG_PAT_0x1234 = 2,              /*!< Debug Pattern 0x1234           */
} PT8311_DbgPatETypeDef;

/**
  * @}
  */

/** @defgroup PT8311_Exported_Types_Group3 PT8311 User Common Types
  * @brief    PT8311 User Common Types
  * @{
  */

/**
  * @brief PT8311 Status type definition
  */
typedef enum {
    PT8311_OK      = 0x00U,                 /*! PT8311 status OK                */
    PT8311_ERROR   = 0x01U,                 /*! PT8311 status Error             */
    PT8311_BUSY    = 0x02U,                 /*! PT8311 status Busy              */
    PT8311_TIMEOUT = 0x03U,                 /*! PT8311 status Timeout           */
    PT8311_FAILED  = 0x04U,                 /*! PT8311 status Failed            */
    PT8311_INVALID = 0x05U,                 /*! PT8311 status Invalid           */
} PT8311_StatusETypeDef;

/**
  * @brief PT8311 Init Module definition
  */
typedef enum {
    PT8311_INIT_MOD_NONE = 0,                                   /*!< None       */
    PT8311_INIT_MOD_ADC = PT8311_BIT(0),                        /*!< ADC        */
    PT8311_INIT_MOD_DAC = PT8311_BIT(1),                        /*!< DAC        */
    PT8311_INIT_MOD_ADC_DAC = PT8311_BIT(0) | PT8311_BIT(1),    /*!< ADC&DAC    */
} PT8311_InitModETypeDef;


/**
  * @brief PT8311 DAC Output Path Definition
  */
typedef enum {
    PT8311_DAC_OUT_PATH_HEADPHONE = 0,      /*!< DAC Output Path Headphone      */
    PT8311_DAC_OUT_PATH_LINEOUT,            /*!< DAC Output Path Lineout        */
} PT8311_DAC_OutPathETypeDef;

/**
  * @brief PT8311 PDMCLK IO definition
  */
typedef enum {
    PT8311_PDMCLK_IO_MCLK,                  /*!< PDMCLK IO select MCLK          */
    PT8311_PDMCLK_IO_DEV_ID,                /*!< PDMCLK IO select DEV_ID        */
} PT8311_PDMClkIOETypeDef;

/**
  * @brief PT8311 External Clock Source definition
  */
typedef enum {
    PT8311_EXT_CLK_SRC_MCLK = 0,            /*!< External Clock Source MCLK     */
    PT8311_EXT_CLK_SRC_BCLK,                /*!< External Clock Source BCLK     */
} PT8311_ExtClkSrcETypeDef;

/**
  * @brief PT8311 User sample rate definition
  */
typedef enum {
    PT8311_USER_SR_NONE   = 0,              /*!< User sample rate None          */

    PT8311_USER_SR_8K     = 8000,           /*!< User sample rate 8000          */
    PT8311_USER_SR_16K    = 16000,          /*!< User sample rate 16000         */
    PT8311_USER_SR_32K    = 32000,          /*!< User sample rate 32000         */

    PT8311_USER_SR_12K    = 12000,          /*!< User sample rate 12000         */
    PT8311_USER_SR_24K    = 24000,          /*!< User sample rate 24000         */
    PT8311_USER_SR_48K    = 48000,          /*!< User sample rate 48000         */
    PT8311_USER_SR_96K    = 96000,          /*!< User sample rate 96000         */
    PT8311_USER_SR_192K   = 192000,         /*!< User sample rate 192000        */

    PT8311_USER_SR_11025  = 11025,          /*!< User sample rate 11025         */
    PT8311_USER_SR_22050  = 22050,          /*!< User sample rate 22050         */
    PT8311_USER_SR_44100  = 44100,          /*!< User sample rate 44100         */
    PT8311_USER_SR_88200  = 88200,          /*!< User sample rate 88200         */
    PT8311_USER_SR_176400 = 176400,         /*!< User sample rate 176400        */
} PT8311_UserSmpRateETypeDef;

/**
  * @brief PT8311 ADC/DAC Work Mode definition
  */
typedef enum {
    PT8311_ADDA_WORK_MODE_NORMAL = 0,       /*!< Work Mode Normal               */
    PT8311_ADDA_WORK_MODE_DOUBLE_SPEED,     /*!< Work Mode Double Speed         */
} PT8311_ADDA_WorkModeETypeDef;

/**
  * @brief PT8311 I2S Config definition
  */
typedef struct __PT8311_I2SCfgTypeDef {
    bool lrck_invert;                       /*!< LRCK invert flag               */
    bool bclk_invert;                       /*!< BCLK invert flag               */
    PT8311_I2SRoleETypeDef role;            /*!< I2S role                       */
    PT8311_I2S_FmtETypeDef fmt;             /*!< I2S format                     */
    PT8311_I2S_WordLenETypeDef word_len;    /*!< I2S word length                */
} PT8311_I2SCfgTypeDef;

/**
  * @brief PT8311 SYSCLK Config definition
  * @note  i2s_lrck_period param means BCLK numbers in a LRCK period, which equal to slot_width * slot_nums,
  *        and only use in I2S master mode.When in I2S slave mode, please config it to 0.
  */
typedef struct __PT8311_SysclkCfgTypeDef {
    uint32_t extclk_freq;                       /*!< External clock frequence   */
    PT8311_ExtClkSrcETypeDef extclk_src;        /*!< External clock Source      */
    PT8311_UserSmpRateETypeDef play_smp_rate;   /*!< Play sample rate           */
    PT8311_UserSmpRateETypeDef rec_smp_rate;    /*!< Record sample rate         */
    PT8311_ADDA_WorkModeETypeDef work_mode;     /*!< ADC/DAC work mode          */

    // Only use in I2S master mode
    uint16_t i2s_lrck_period;                   /*!< I2S LRCK Period            */
} PT8311_SysclkCfgTypeDef;

/**
  * @brief PT8311 PDM Config definition
  */
typedef struct __PT8311_PDMCfgTypeDef {
    bool timing_invert;                     /*!< PDM timing invert flag         */
    PT8311_PDMClkIOETypeDef clk_io;         /*!< PDMCLK IO, recommend DEVID     */
} PT8311_PDMCfgTypeDef;

/**
  * @}
  */

/**
  * @}
  */


/* Exported Variables --------------------------------------------------------*/
/* Exported Functions --------------------------------------------------------*/
/** @addtogroup PT8311_Exported_Functions
  * @{
  */

/** @addtogroup PT8311_Exported_Functions_Group1
  * @{
  */
int8_t pt8311_update_bits(uint8_t i2c_addr, uint8_t reg, uint8_t mask, uint8_t val);
/**
  * @}
  */

/** @addtogroup PT8311_Exported_Functions_Group2
  * @{
  */
PT8311_StatusETypeDef pt8311_init(uint8_t i2c_addr, PT8311_InitModETypeDef mod);
PT8311_StatusETypeDef pt8311_deinit(uint8_t i2c_addr, PT8311_InitModETypeDef mod);
/**
  * @}
  */

/** @addtogroup PT8311_Exported_Functions_Group3
  * @{
  */
void pt8311_play_start(uint8_t i2c_addr);
void pt8311_play_stop(uint8_t i2c_addr);
void pt8311_record_start(uint8_t i2c_addr);
void pt8311_record_stop(uint8_t i2c_addr);
void pt8311_play_vol_cfg(uint8_t i2c_addr, uint8_t vol);
void pt8311_record_gain_cfg(uint8_t i2c_addr, PT8311_ADC_PGAGainETypeDef gain);
/**
  * @}
  */

/** @addtogroup PT8311_Exported_Functions_Group4
  * @{
  */
PT8311_StatusETypeDef pt8311_i2s_cfg(uint8_t i2c_addr, PT8311_I2SCfgTypeDef *cfg);
PT8311_StatusETypeDef pt8311_sysclk_cfg(uint8_t i2c_addr, PT8311_SysclkCfgTypeDef *cfg);
PT8311_StatusETypeDef pt8311_pdm_cfg(uint8_t i2c_addr, bool en, PT8311_PDMCfgTypeDef *cfg);
void pt8311_play_path_cfg(uint8_t i2c_addr, PT8311_DAC_OutPathETypeDef path);
/**
  * @}
  */

/** @addtogroup PT8311_Exported_Functions_Group5
  * @{
  */
PT8311_StatusETypeDef pt8311_adc_eq_cfg(uint8_t i2c_addr, bool en, const int32_t array_cfg[], uint8_t array_size);
PT8311_StatusETypeDef pt8311_adc_drc_cfg(uint8_t i2c_addr, bool en, const int32_t array_cfg[], uint8_t array_size);
PT8311_StatusETypeDef pt8311_dac_eq_cfg(uint8_t i2c_addr, bool en, const int32_t array_cfg[], uint8_t array_size);
PT8311_StatusETypeDef pt8311_dac_drc_cfg(uint8_t i2c_addr, bool en, const int32_t array_cfg[], uint8_t array_size);
/**
  * @}
  */

/** @addtogroup PT8311_Exported_Functions_Group6
  * @{
  */
void pt8311_all_regs_read(uint8_t i2c_addr, bool print_all);
/**
  * @}
  */

/** @addtogroup PT8311_Exported_Functions_Group7
  * @{
  */
// I2C Read/Write Functions
int8_t pt8311_multichips_read(const uint8_t i2c_addrs[], uint8_t chip_nums, uint8_t reg, uint8_t rt_val[]);
int8_t pt8311_multichips_write(const uint8_t i2c_addrs[], uint8_t chip_nums, uint8_t reg, uint8_t val);
int8_t pt8311_multichips_update_bits(const uint8_t i2c_addrs[], uint8_t chip_nums, uint8_t reg, uint8_t mask, uint8_t val);

// Init and DeInit Functions
PT8311_StatusETypeDef pt8311_multichips_init(const uint8_t i2c_addrs[], uint8_t chip_nums, PT8311_InitModETypeDef mod);
PT8311_StatusETypeDef pt8311_multichips_deinit(const uint8_t i2c_addrs[], uint8_t chip_nums, PT8311_InitModETypeDef mod);

// Play and Record Functions
void pt8311_multichips_play_start(const uint8_t i2c_addrs[], uint8_t chip_nums);
void pt8311_multichips_play_stop(const uint8_t i2c_addrs[], uint8_t chip_nums);
void pt8311_multichips_record_start(const uint8_t i2c_addrs[], uint8_t chip_nums);
void pt8311_multichips_record_stop(const uint8_t i2c_addrs[], uint8_t chip_nums);
void pt8311_multichips_play_vol_cfg(const uint8_t i2c_addrs[], uint8_t chip_nums, uint8_t vol);
void pt8311_multichips_record_gain_cfg(const uint8_t i2c_addrs[], uint8_t chip_nums, PT8311_ADC_PGAGainETypeDef gain);

// Common Config Functions
PT8311_StatusETypeDef pt8311_multichips_i2s_cfg(const uint8_t i2c_addrs[], uint8_t chip_nums, PT8311_I2SCfgTypeDef *cfg);
PT8311_StatusETypeDef pt8311_multichips_i2s_ext_cfg(const uint8_t i2c_addrs[], uint8_t chip_nums);
PT8311_StatusETypeDef pt8311_multichips_sysclk_cfg(const uint8_t i2c_addrs[], uint8_t chip_nums, PT8311_SysclkCfgTypeDef *cfg);
PT8311_StatusETypeDef pt8311_multichips_pdm_cfg(const uint8_t i2c_addrs[], uint8_t chip_nums, bool en, PT8311_PDMCfgTypeDef *cfg);
void pt8311_multichips_play_path_cfg(const uint8_t i2c_addrs[], uint8_t chip_nums, PT8311_DAC_OutPathETypeDef path);

// EQ and DRC Config Functions
PT8311_StatusETypeDef pt8311_multichips_adc_eq_cfg(const uint8_t i2c_addrs[], uint8_t chip_nums, bool en,
        const int32_t array_cfg[], uint8_t array_size);
PT8311_StatusETypeDef pt8311_multichips_adc_drc_cfg(const uint8_t i2c_addrs[], uint8_t chip_nums, bool en,
        const int32_t array_cfg[], uint8_t array_size);
PT8311_StatusETypeDef pt8311_multichips_dac_eq_cfg(const uint8_t i2c_addrs[], uint8_t chip_nums, bool en,
        const int32_t array_cfg[], uint8_t array_size);
PT8311_StatusETypeDef pt8311_multichips_dac_drc_cfg(const uint8_t i2c_addrs[], uint8_t chip_nums, bool en,
        const int32_t array_cfg[], uint8_t array_size);

// Debug Functions
void pt8311_multichips_all_regs_read(const uint8_t i2c_addrs[], uint8_t chip_nums, bool print_all);
/**
  * @}
  */

/**
  * @}
  */


#endif  /* PT8311_MODULE_ENABLED */


/**
  * @}
  */


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _PT8311_H_ */

/***********END OF FILE***********/
