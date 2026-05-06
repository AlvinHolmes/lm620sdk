/**
 *************************************************************************************
 * 版权所有 (C) 2023, 南京创芯慧联技术有限公司
 * 保留所有权利。
 *
 * @file        drv_uart.c
 *
 * @brief       uart驱动实现.
 *
 * @revision
 *
 * 日期           作者              修改内容
 * 2023-04-21     ict team          创建
 ************************************************************************************
 */

/************************************************************************************
 *                                 头文件
 ************************************************************************************/
#include <stdlib.h>
#include <os.h>
#include <os_hw.h>
#include <drv_uart.h>
#if defined(OS_USING_PM)
#include <drv_pcu.h>
#include <drv_psm_sys.h>
#include "psm_sys.h"
#endif
#include <drv_soc.h>
#include <top_ram_config.h>
#if defined(_CPU_AP)
#include "ringbuffer.h"
#endif
#include "drv_pin.h"
#include "nr_micro_shell.h"
/************************************************************************************
 *                                 配置开关
 ************************************************************************************/
/**
 * @brief 是否打开调试
 */
// #define DEBUG_UART

/************************************************************************************
 *                                 宏定义
 ************************************************************************************/

/**
 * @brief 调试打印宏
 */
#ifdef DEBUG_UART
#define UART_dbg(fmt, ...) osPrintf("\033[" "30m" "(%s-%d):" fmt "\r\n" "\033[0m", __func__, __LINE__, ##__VA_ARGS__)
#define UART_err(fmt, ...) osPrintf("\033[" "31m" "(%s-%d):" fmt "\r\n" "\033[0m", __func__, __LINE__, ##__VA_ARGS__)
#else
#define UART_dbg(fmt, ...)
#define UART_err(fmt, ...)
#endif

/*** Bit definition for [vs] register(0x00000000) ****/
#define VERSION_NUMBER_X_Pos              (24)
#define VERSION_NUMBER_X_Msk              (0xFFUL << VERSION_NUMBER_X_Pos)
#define VERSION_NUMBER_X                  VERSION_NUMBER_X_Msk              /*!<Version Number X in VX.Y */
#define VERSION_NUMBER_Y_Pos              (16)
#define VERSION_NUMBER_Y_Msk              (0xFFUL << VERSION_NUMBER_Y_Pos)
#define VERSION_NUMBER_Y                  VERSION_NUMBER_Y_Msk              /*!<Version Number Y in VX.Y */


/*** Bit definition for [dr] register(0x00000004) ****/
#define OVERRUN_ERROR_Pos                 (12)
#define OVERRUN_ERROR_Msk                 (0x1UL << OVERRUN_ERROR_Pos)
#define OVERRUN_ERROR                     OVERRUN_ERROR_Msk                 /*!<the receive FIFO is already full */
#define BREAK_ERROR_Pos                   (11)
#define BREAK_ERROR_Msk                   (0x1UL << BREAK_ERROR_Pos)
#define BREAK_ERROR                       BREAK_ERROR_Msk                   /*!<the received data input was held LOW for longer */
#define PARITY_ERROR_Pos                  (10)
#define PARITY_ERROR_Msk                  (0x1UL << PARITY_ERROR_Pos)
#define PARITY_ERROR                      PARITY_ERROR_Msk                  /*!<parity does not match */
#define FRAMING_ERROR_Pos                 (9)
#define FRAMING_ERROR_Msk                 (0x1UL << FRAMING_ERROR_Pos)
#define FRAMING_ERROR                     FRAMING_ERROR_Msk                 /*!<FRAMING error */
#define DATA_Pos                          (0)
#define DATA_Msk                          (0x1FFUL << DATA_Pos)
#define DATA                              DATA_Msk                          /*!<receive data character */


/*** Bit definition for [sc] register(0x00000008) ****/
#define SPCHAR_Pos                        (0)
#define SPCHAR_Msk                        (0x1FFUL << SPCHAR_Pos)
#define SPCHAR                            SPCHAR_Msk                        /*!<特殊字符 */


/*** Bit definition for [rsrEcr] register(0x00000010) ****/
#define OE_Pos                            (3)
#define OE_Msk                            (0x1UL << OE_Pos)
#define OE                                OE_Msk                            /*!<overrun error */
#define BE_Pos                            (2)
#define BE_Msk                            (0x1UL << BE_Pos)
#define BE                                BE_Msk                            /*!<break error */
#define PE_Pos                            (1)
#define PE_Msk                            (0x1UL << PE_Pos)
#define PE                                PE_Msk                            /*!<parity error */
#define FE_Pos                            (0)
#define FE_Msk                            (0x1UL << FE_Pos)
#define FE                                FE_Msk                            /*!<frame error */


/*** Bit definition for [fr] register(0x00000014) ****/
#define RXF_DATA_CNT_Pos                  (24)
#define RXF_DATA_CNT_Msk                  (0xFFUL << RXF_DATA_CNT_Pos)
#define RXF_DATA_CNT                      RXF_DATA_CNT_Msk                  /*!<Receive fifo data count */
#define TXF_SPACE_CNT_Pos                 (16)
#define TXF_SPACE_CNT_Msk                 (0xFFFFUL << TXF_SPACE_CNT_Pos)
#define TXF_SPACE_CNT                     TXF_SPACE_CNT_Msk                 /*!<Transmit fifo space count */
#define RXF_NOT_EMPTY_Pos                 (11)
#define RXF_NOT_EMPTY_Msk                 (0x1UL << RXF_NOT_EMPTY_Pos)
#define RXF_NOT_EMPTY                     RXF_NOT_EMPTY_Msk                 /*!<Receive fifo not empty */
#define TXF_NOT_FULL_Pos                  (10)
#define TXF_NOT_FULL_Msk                  (0x1UL << TXF_NOT_FULL_Pos)
#define TXF_NOT_FULL                      TXF_NOT_FULL_Msk                  /*!<Transmit fifo not full */
#define RXBUSY_Pos                        (9)
#define RXBUSY_Msk                        (0x1UL << RXBUSY_Pos)
#define RXBUSY                            RXBUSY_Msk                        /*!<rx busy */
#define TXBUSY_Pos                        (8)
#define TXBUSY_Msk                        (0x1UL << TXBUSY_Pos)
#define TXBUSY                            TXBUSY_Msk                        /*!<tx busy */
#define TXFE_Pos                          (7)
#define TXFE_Msk                          (0x1UL << TXFE_Pos)
#define TXFE                              TXFE_Msk                          /*!<tx fifo empty */
#define RXFF_Pos                          (6)
#define RXFF_Msk                          (0x1UL << RXFF_Pos)
#define RXFF                              RXFF_Msk                          /*!<RX FIFO满 */
#define TXFF_Pos                          (5)
#define TXFF_Msk                          (0x1UL << TXFF_Pos)
#define TXFF                              TXFF_Msk                          /*!<TX FIFO满 */
#define RXFE_Pos                          (4)
#define RXFE_Msk                          (0x1UL << RXFE_Pos)
#define RXFE                              RXFE_Msk                          /*!<RX FIFO控 */
#define DSR_Pos                           (3)
#define DSR_Msk                           (0x1UL << DSR_Pos)
#define DSR                               DSR_Msk                           /*!<data set ready */
#define DCD_Pos                           (2)
#define DCD_Msk                           (0x1UL << DCD_Pos)
#define DCD                               DCD_Msk                           /*!<modem status input is 0 */
#define CTS_Pos                           (1)
#define CTS_Msk                           (0x1UL << CTS_Pos)
#define CTS                               CTS_Msk                           /*!<CTS */
#define RI_Pos                            (0)
#define RI_Msk                            (0x1UL << RI_Pos)
#define RI                                RI_Msk                            /*!<RI */


/*** Bit definition for [ilpr] register(0x00000020) ****/
#define IRDA_LPD_Pos                      (24)
#define IRDA_LPD_Msk                      (0xFFUL << IRDA_LPD_Pos)
#define IRDA_LPD                          IRDA_LPD_Msk                      /*!<IRDA  */


/*** Bit definition for [ibrd] register(0x00000024) ****/
#define BAUD_RATE_INTEGER_Pos             (0)
#define BAUD_RATE_INTEGER_Msk             (0xFFFFUL << BAUD_RATE_INTEGER_Pos)
#define BAUD_RATE_INTEGER                 BAUD_RATE_INTEGER_Msk             /*!<波特率分频整数寄存器 */


/*** Bit definition for [fbrd] register(0x00000028) ****/
#define BAUD_RATE_FRACTION_Pos            (0)
#define BAUD_RATE_FRACTION_Msk            (0x3FUL << BAUD_RATE_FRACTION_Pos)
#define BAUD_RATE_FRACTION                BAUD_RATE_FRACTION_Msk            /*!<波特率分频小数寄存器 */


/*** Bit definition for [lcrH] register(0x00000030) ****/
#define WLEN9_Pos                         (8)
#define WLEN9_Msk                         (0x1UL << WLEN9_Pos)
#define WLEN9                             WLEN9_Msk                         /*!<Word Length 9 bit */
#define SPS_Pos                           (7)
#define SPS_Msk                           (0x1UL << SPS_Pos)
#define SPS                               SPS_Msk                           /*!<RX FIFO满 */
#define WLEN_Pos                          (5)
#define WLEN_Msk                          (0x3UL << WLEN_Pos)
#define WLEN                              WLEN_Msk                          /*!<Word Length */
#define WLEN8                             (0x3UL << WLEN_Pos)
#define WLEN7                             (0x2UL << WLEN_Pos)
#define WLEN6                             (0x1UL << WLEN_Pos)
#define WLEN5                             (0x0UL << WLEN_Pos)

#define FEN_Pos                           (4)
#define FEN_Msk                           (0x1UL << FEN_Pos)
#define FEN                               FEN_Msk
#define STP2_Pos                          (3)
#define STP2_Msk                          (0x1UL << STP2_Pos)
#define STP2                              STP2_Msk                          /*!<Two Stop Bits Select */
#define EPS_Pos                           (2)
#define EPS_Msk                           (0x1UL << EPS_Pos)
#define EPS                               EPS_Msk                           /*!<Even Parity Selec */
#define PEN_Pos                           (1)
#define PEN_Msk                           (0x1UL << PEN_Pos)
#define PEN                               PEN_Msk                           /*!<Parity Enable  */
#define BRK_Pos                           (0)
#define BRK_Msk                           (0x1UL << BRK_Pos)
#define BRK                               BRK_Msk                           /*!<Send Break */


/*** Bit definition for [cr] register(0x00000034) ****/
#define CTSEN_Pos                         (15)
#define CTSEN_Msk                         (0x1UL << CTSEN_Pos)
#define CTSEN                             CTSEN_Msk                         /*!<CTS Hardware Flow Control Enable */
#define RTSEN_Pos                         (14)
#define RTSEN_Msk                         (0x1UL << RTSEN_Pos)
#define RTSEN                             RTSEN_Msk                         /*!<RTS Hardware Flow Control Enable */
#define OUT2_Pos                          (13)
#define OUT2_Msk                          (0x1UL << OUT2_Pos)
#define OUT2                              OUT2_Msk                          /*!<Out2 */
#define OUT1_Pos                          (12)
#define OUT1_Msk                          (0x1UL << OUT1_Pos)
#define OUT1                              OUT1_Msk                          /*!<Out1 */
#define RTS_Pos                           (11)
#define RTS_Msk                           (0x1UL << RTS_Pos)
#define RTS                               RTS_Msk                           /*!<Request to Send */
#define DTR_Pos                           (10)
#define DTR_Msk                           (0x1UL << DTR_Pos)
#define DTR                               DTR_Msk                           /*!<Data Transmit Ready */
#define RXE_Pos                           (9)
#define RXE_Msk                           (0x1UL << RXE_Pos)
#define RXE                               RXE_Msk                           /*!<Receive Enable */
#define TXE_Pos                           (8)
#define TXE_Msk                           (0x1UL << TXE_Pos)
#define TXE                               TXE_Msk                           /*!<Transmit Enable */
#define LBE_Pos                           (7)
#define LBE_Msk                           (0x1UL << LBE_Pos)
#define LBE                               LBE_Msk                           /*!<Loop Back Enable */
#define AUTO_BAUD_Pos                     (5)
#define AUTO_BAUD_Msk                     (0x1UL << AUTO_BAUD_Pos)
#define AUTO_BAUD                         AUTO_BAUD_Msk                     /*!<auto adapt bauderate */
#define SAMP_RATE_Pos                     (4)
#define SAMP_RATE_Msk                     (0x1UL << SAMP_RATE_Pos)
#define SAMP_RATE                         SAMP_RATE_Msk                     /*!<sample rate */
#define LP_MODE_Pos                       (3)
#define LP_MODE_Msk                       (0x1UL << LP_MODE_Pos)
#define LP_MODE                           LP_MODE_Msk                       /*!<lp_mode */
#define SIRLP_Pos                         (2)
#define SIRLP_Msk                         (0x1UL << SIRLP_Pos)
#define SIRLP                             SIRLP_Msk                         /*!<IrDA SIR Low Power Mode */
#define SIREN_Pos                         (1)
#define SIREN_Msk                         (0x1UL << SIREN_Pos)
#define SIREN                             SIREN_Msk                         /*!<SIR Enable */
#define UARTEN_Pos                        (0)
#define UARTEN_Msk                        (0x1UL << UARTEN_Pos)
#define UARTEN                            UARTEN_Msk                        /*!<UART Enable */


/*** Bit definition for [ifls ] register(0x00000038) ****/
#define RXIFLSEL_Pos                      (4)
#define RXIFLSEL_Msk                      (0xFUL << RXIFLSEL_Pos)
#define RXIFLSEL                          RXIFLSEL_Msk                      /*!<Receive Interrupt FIFO Level Select */
#define RXIFLSEL1                         (0x0UL << RXIFLSEL_Pos)
#define RXIFLSEL2                         (0x1UL << RXIFLSEL_Pos)
#define RXIFLSEL3                         (0x2UL << RXIFLSEL_Pos)
#define RXIFLSEL4                         (0x3UL << RXIFLSEL_Pos)
#define RXIFLSEL5                         (0x4UL << RXIFLSEL_Pos)
#define RXIFLSEL6                         (0x5UL << RXIFLSEL_Pos)
#define RXIFLSEL7                         (0x6UL << RXIFLSEL_Pos)
#define RXIFLSEL8                         (0x7UL << RXIFLSEL_Pos)
#define RXIFLSEL9                         (0x8UL << RXIFLSEL_Pos)
#define RXIFLSEL10                        (0x9UL << RXIFLSEL_Pos)
#define RXIFLSEL11                        (0xAUL << RXIFLSEL_Pos)
#define RXIFLSEL12                        (0xBUL << RXIFLSEL_Pos)
#define RXIFLSEL13                        (0xCUL << RXIFLSEL_Pos)
#define RXIFLSEL14                        (0xDUL << RXIFLSEL_Pos)
#define RXIFLSEL15                        (0xEUL << RXIFLSEL_Pos)
#define RXIFLSEL16                        (0xFUL << RXIFLSEL_Pos)
#define TXIFLSEL_Pos                      (0)
#define TXIFLSEL_Msk                      (0xFUL << TXIFLSEL_Pos)
#define TXIFLSEL                          TXIFLSEL_Msk                      /*!<Transmit Interrupt FIFO Level Select */
#define TXIFLSEL1                          (0x0UL << TXIFLSEL_Pos)
#define TXIFLSEL2                          (0x1UL << TXIFLSEL_Pos)
#define TXIFLSEL3                          (0x2UL << TXIFLSEL_Pos)
#define TXIFLSEL4                          (0x3UL << TXIFLSEL_Pos)
#define TXIFLSEL5                          (0x4UL << TXIFLSEL_Pos)
#define TXIFLSEL6                          (0x5UL << TXIFLSEL_Pos)
#define TXIFLSEL7                          (0x6UL << TXIFLSEL_Pos)
#define TXIFLSEL8                          (0x7UL << TXIFLSEL_Pos)
#define TXIFLSEL9                          (0x8UL << TXIFLSEL_Pos)
#define TXIFLSEL10                         (0x9UL << TXIFLSEL_Pos)
#define TXIFLSEL11                         (0xAUL << TXIFLSEL_Pos)
#define TXIFLSEL12                         (0xBUL << TXIFLSEL_Pos)
#define TXIFLSEL13                         (0xCUL << TXIFLSEL_Pos)
#define TXIFLSEL14                         (0xDUL << TXIFLSEL_Pos)
#define TXIFLSEL15                         (0xEUL << TXIFLSEL_Pos)
#define TXIFLSEL16                         (0xFUL << TXIFLSEL_Pos)


/*** Bit definition for [imsc] register(0x00000040) ****/
#define ILIM_Pos                          (14)
#define ILIM_Msk                          (0x1UL << ILIM_Pos)
#define ILIM                              ILIM_Msk                          /*!<idle line interrupt mask */
#define WCDRIM_Pos                        (13)
#define WCDRIM_Msk                        (0x1UL << WCDRIM_Pos)
#define WCDRIM                            WCDRIM_Msk                        /*!<work clock domain receive interrupt mask */
#define ACRIM_Pos                         (12)
#define ACRIM_Msk                         (0x1UL << ACRIM_Pos)
#define ACRIM                             ACRIM_Msk                         /*!<Any char received interrupt mask */
#define SCIM_Pos                          (11)
#define SCIM_Msk                          (0x1UL << SCIM_Pos)
#define SCIM                              SCIM_Msk                          /*!<Special Char Received Interrupt Mask */
#define OEIM_Pos                          (10)
#define OEIM_Msk                          (0x1UL << OEIM_Pos)
#define OEIM                              OEIM_Msk                          /*!<Overrun Error Interrupt Mask */
#define BEIM_Pos                          (9)
#define BEIM_Msk                          (0x1UL << BEIM_Pos)
#define BEIM                              BEIM_Msk                          /*!<Break Error Interrupt Mask */
#define PEIM_Pos                          (8)
#define PEIM_Msk                          (0x1UL << PEIM_Pos)
#define PEIM                              PEIM_Msk                          /*!<Parity Error Interrupt */
#define FEIM_Pos                          (7)
#define FEIM_Msk                          (0x1UL << FEIM_Pos)
#define FEIM                              FEIM_Msk                          /*!<Framing Error Interrupt Mask */
#define RTIM_Pos                          (6)
#define RTIM_Msk                          (0x1UL << RTIM_Pos)
#define RTIM                              RTIM_Msk                          /*!<Receive Timeout Interrupt Mask */
#define TXIM_Pos                          (5)
#define TXIM_Msk                          (0x1UL << TXIM_Pos)
#define TXIM                              TXIM_Msk                          /*!<Transmit Interrupt Mask */
#define RXIM_Pos                          (4)
#define RXIM_Msk                          (0x1UL << RXIM_Pos)
#define RXIM                              RXIM_Msk                          /*!<work clock domain receive interrupt mask */
#define DSRMIM_Pos                        (3)
#define DSRMIM_Msk                        (0x1UL << DSRMIM_Pos)
#define DSRMIM                            DSRMIM_Msk                        /*!<nSPUART0DSR Modem Interrupt Mask */
#define DCDMIM_Pos                        (2)
#define DCDMIM_Msk                        (0x1UL << DCDMIM_Pos)
#define DCDMIM                            DCDMIM_Msk                        /*!<nSPUART0DCD Modem Interrupt Mask */
#define CTSMIM_Pos                        (1)
#define CTSMIM_Msk                        (0x1UL << CTSMIM_Pos)
#define CTSMIM                            CTSMIM_Msk                        /*!<nSPUART0CTS Modem Interrupt Mask */
#define RIMIM_Pos                         (0)
#define RIMIM_Msk                         (0x1UL << RIMIM_Pos)
#define RIMIM                             RIMIM_Msk                         /*!<nSPUART0RI Modem Interrupt Mask */


/*** Bit definition for [ris] register(0x00000044) ****/
#define ILIS_Pos                          (14)
#define ILIS_Msk                          (0x1UL << ILIS_Pos)
#define ILIS                              ILIS_Msk                          /*!<idle line interrupt status */
#define WCDRIS_Pos                        (13)
#define WCDRIS_Msk                        (0x1UL << WCDRIS_Pos)
#define WCDRIS                            WCDRIS_Msk                        /*!<work clock domain receive interrupt status */
#define ACRIS_Pos                         (12)
#define ACRIS_Msk                         (0x1UL << ACRIS_Pos)
#define ACRIS                             ACRIS_Msk                         /*!<Any char received interrupt status */
#define SCRIS_Pos                         (11)
#define SCRIS_Msk                         (0x1UL << SCRIS_Pos)
#define SCRIS                             SCRIS_Msk                         /*!<Special Char Received interrupt status  */
#define OERIS_Pos                         (10)
#define OERIS_Msk                         (0x1UL << OERIS_Pos)
#define OERIS                             OERIS_Msk                         /*!<Overrun error interrupt status  */
#define BERIS_Pos                         (9)
#define BERIS_Msk                         (0x1UL << BERIS_Pos)
#define BERIS                             BERIS_Msk                         /*!<Break error interrupt status  */
#define PERIS_Pos                         (8)
#define PERIS_Msk                         (0x1UL << PERIS_Pos)
#define PERIS                             PERIS_Msk                         /*!<Parity error interrupt status */
#define FERIS_Pos                         (7)
#define FERIS_Msk                         (0x1UL << FERIS_Pos)
#define FERIS                             FERIS_Msk                         /*!<Framing error interrupt status */
#define RTRIS_Pos                         (6)
#define RTRIS_Msk                         (0x1UL << RTRIS_Pos)
#define RTRIS                             RTRIS_Msk                         /*!<Receive timeout interrupt status */
#define TXRIS_Pos                         (5)
#define TXRIS_Msk                         (0x1UL << TXRIS_Pos)
#define TXRIS                             TXRIS_Msk                         /*!<Transmit interrupt status */
#define RXRIS_Pos                         (4)
#define RXRIS_Msk                         (0x1UL << RXRIS_Pos)
#define RXRIS                             RXRIS_Msk                         /*!<Receive interrupt status */
#define DSRMRIS_Pos                       (3)
#define DSRMRIS_Msk                       (0x1UL << DSRMRIS_Pos)
#define DSRMRIS                           DSRMRIS_Msk                       /*!<nUARTDSR modem interrupt status  */
#define DCDMRIS_Pos                       (2)
#define DCDMRIS_Msk                       (0x1UL << DCDMRIS_Pos)
#define DCDMRIS                           DCDMRIS_Msk                       /*!<nUARTDCD modem interrupt status  */
#define CTSMRIS_Pos                       (1)
#define CTSMRIS_Msk                       (0x1UL << CTSMRIS_Pos)
#define CTSMRIS                           CTSMRIS_Msk                       /*!<nUARTCTS modem interrupt status  */
#define RIMRIS_Pos                        (0)
#define RIMRIS_Msk                        (0x1UL << RIMRIS_Pos)
#define RIMRIS                            RIMRIS_Msk                        /*!<nUARTRI modem interrupt status */


/*** Bit definition for [mis] register(0x00000048) ****/
#define ILMIS_Pos                         (14)
#define ILMIS_Msk                         (0x1UL << ILMIS_Pos)
#define ILMIS                             ILMIS_Msk                         /*!<idle line interrupt status */
#define WCDRMIS_Pos                       (13)
#define WCDRMIS_Msk                       (0x1UL << WCDRMIS_Pos)
#define WCDRMIS                           WCDRMIS_Msk                       /*!<work clock domain receive interrupt status */
#define ACRMIS_Pos                        (12)
#define ACRMIS_Msk                        (0x1UL << ACRMIS_Pos)
#define ACRMIS                            ACRMIS_Msk                        /*!<Any char received interrupt status */
#define SCMIS_Pos                         (11)
#define SCMIS_Msk                         (0x1UL << SCMIS_Pos)
#define SCMIS                             SCMIS_Msk                         /*!<Special Char Received interrupt status */
#define OEMIS_Pos                         (10)
#define OEMIS_Msk                         (0x1UL << OEMIS_Pos)
#define OEMIS                             OEMIS_Msk                         /*!<Overrun error masked interrupt status. */
#define BEMIS_Pos                         (9)
#define BEMIS_Msk                         (0x1UL << BEMIS_Pos)
#define BEMIS                             BEMIS_Msk                         /*!<Break error masked interrupt status. */
#define PEMIS_Pos                         (8)
#define PEMIS_Msk                         (0x1UL << PEMIS_Pos)
#define PEMIS                             PEMIS_Msk                         /*!<Parity error masked interrupt status. */
#define FEMIS_Pos                         (7)
#define FEMIS_Msk                         (0x1UL << FEMIS_Pos)
#define FEMIS                             FEMIS_Msk                         /*!<Framing error masked  interrupt status. */
#define RTMIS_Pos                         (6)
#define RTMIS_Msk                         (0x1UL << RTMIS_Pos)
#define RTMIS                             RTMIS_Msk                         /*!<Receive timeout masked interrupt status. */
#define TXMIS_Pos                         (5)
#define TXMIS_Msk                         (0x1UL << TXMIS_Pos)
#define TXMIS                             TXMIS_Msk                         /*!<Transmit masked interrupt status. */
#define RXMIS_Pos                         (4)
#define RXMIS_Msk                         (0x1UL << RXMIS_Pos)
#define RXMIS                             RXMIS_Msk                         /*!<Receive masked interrupt status.  */
#define DSRMMIS_Pos                       (3)
#define DSRMMIS_Msk                       (0x1UL << DSRMMIS_Pos)
#define DSRMMIS                           DSRMMIS_Msk                       /*!<nUARTDSR modem masked interrupt status. */
#define DCDMMIS_Pos                       (2)
#define DCDMMIS_Msk                       (0x1UL << DCDMMIS_Pos)
#define DCDMMIS                           DCDMMIS_Msk                       /*!<nUARTDCD modem masked interrupt status. */
#define CTSMMIS_Pos                       (1)
#define CTSMMIS_Msk                       (0x1UL << CTSMMIS_Pos)
#define CTSMMIS                           CTSMMIS_Msk                       /*!<nUARTCTS modem masked interrupt status. */
#define RIMMIS_Pos                        (0)
#define RIMMIS_Msk                        (0x1UL << RIMMIS_Pos)
#define RIMMIS                            RIMMIS_Msk                        /*!<nUARTRI modem masked interrupt status. */


/*** Bit definition for [icr] register(0x0000004c) ****/
#define ILIC_Pos                          (14)
#define ILIC_Msk                          (0x1UL << ILIC_Pos)
#define ILIC                              ILIC_Msk                          /*!<idle line interrupt clear */
#define WCDRIC_Pos                        (13)
#define WCDRIC_Msk                        (0x1UL << WCDRIC_Pos)
#define WCDRIC                            WCDRIC_Msk                        /*!<work clock domain receive interrupt clear */
#define ACRIC_Pos                         (12)
#define ACRIC_Msk                         (0x1UL << ACRIC_Pos)
#define ACRIC                             ACRIC_Msk                         /*!<Any char received interrupt clear */
#define SCIC_Pos                          (11)
#define SCIC_Msk                          (0x1UL << SCIC_Pos)
#define SCIC                              SCIC_Msk                          /*!<Clears the UARTSCINTR interrupt */
#define OEIC_Pos                          (10)
#define OEIC_Msk                          (0x1UL << OEIC_Pos)
#define OEIC                              OEIC_Msk                          /*!<Clears the UARTOEINTR interrupt */
#define BEIC_Pos                          (9)
#define BEIC_Msk                          (0x1UL << BEIC_Pos)
#define BEIC                              BEIC_Msk                          /*!<Clears the UARTBEINTR interrupt */
#define PEIC_Pos                          (8)
#define PEIC_Msk                          (0x1UL << PEIC_Pos)
#define PEIC                              PEIC_Msk                          /*!<Clears the UARTPEINTR interrupt */
#define FEIC_Pos                          (7)
#define FEIC_Msk                          (0x1UL << FEIC_Pos)
#define FEIC                              FEIC_Msk                          /*!<Clears the UARTFEINTR interrupt */
#define RTIC_Pos                          (6)
#define RTIC_Msk                          (0x1UL << RTIC_Pos)
#define RTIC                              RTIC_Msk                          /*!<Clears the UARTRTINTR interrupt */
#define TXIC_Pos                          (5)
#define TXIC_Msk                          (0x1UL << TXIC_Pos)
#define TXIC                              TXIC_Msk                          /*!<Clears the UARTTXINTR interrupt */
#define RXIC_Pos                          (4)
#define RXIC_Msk                          (0x1UL << RXIC_Pos)
#define RXIC                              RXIC_Msk                          /*!<Clears the UARTRXINTR interrupt */
#define DSRMIC_Pos                        (3)
#define DSRMIC_Msk                        (0x1UL << DSRMIC_Pos)
#define DSRMIC                            DSRMIC_Msk                        /*!<Clears the UARTDSRINTR interrupt */
#define DCDMIC_Pos                        (2)
#define DCDMIC_Msk                        (0x1UL << DCDMIC_Pos)
#define DCDMIC                            DCDMIC_Msk                        /*!<Clears the UARTDCDINTR interrupt */
#define CTSMIC_Pos                        (1)
#define CTSMIC_Msk                        (0x1UL << CTSMIC_Pos)
#define CTSMIC                            CTSMIC_Msk                        /*!<Clears the UARTCTSINTR interrupt */
#define RIMIC_Pos                         (0)
#define RIMIC_Msk                         (0x1UL << RIMIC_Pos)
#define RIMIC                             RIMIC_Msk                         /*!<Clears the UARTRIINTR interrupt */


/*** Bit definition for [dmacr] register(0x00000050) ****/
#define DMAONERR_Pos                      (2)
#define DMAONERR_Msk                      (0x1UL << DMAONERR_Pos)
#define DMAONERR                          DMAONERR_Msk                      /*!<DMA on Error */
#define TXDMAE_Pos                        (1)
#define TXDMAE_Msk                        (0x1UL << TXDMAE_Pos)
#define TXDMAE                            TXDMAE_Msk                        /*!<Transmit DMA Enable */
#define RXDMAE_Pos                        (0)
#define RXDMAE_Msk                        (0x1UL << RXDMAE_Pos)
#define RXDMAE                            RXDMAE_Msk                        /*!<Receive DMA Enable */


/*** Bit definition for [to] register(0x00000080) ****/
#define TO_THRES_Pos                      (0)
#define TO_THRES_Msk                      (0xFFFFUL << TO_THRES_Pos)
#define TO_THRES                          TO_THRES_Msk                      /*!<time out threshold */


/*** Bit definition for [silent] register(0x00000084) ****/
#define SILENT_MODE0_Pos                  (13)
#define SILENT_MODE0_Msk                  (0x1UL << SILENT_MODE0_Pos)
#define SILENT_MODE0                      SILENT_MODE0_Msk                  /*!<idle line detect silent mode */
#define SILENT_MODE1_Pos                  (12)
#define SILENT_MODE1_Msk                  (0x1UL << SILENT_MODE1_Pos)
#define SILENT_MODE1                      SILENT_MODE1_Msk                  /*!<address match silent mode */
#define MODE1_ADDR_Pos                    (0)
#define MODE1_ADDR_Msk                    (0x1FFUL << MODE1_ADDR_Pos)
#define MODE1_ADDR                        MODE1_ADDR_Msk                    /*!<silent mode1 address */


/*** Bit definition for [abrSta] register(0x00000088) ****/
#define ABR_DONE_Pos                      (20)
#define ABR_DONE_Msk                      (0x1UL << ABR_DONE_Pos)
#define ABR_DONE                          ABR_DONE_Msk                      /*!<auto baud rate detection complete. */
#define ABR_RESULT_Pos                    (0)
#define ABR_RESULT_Msk                    (0xFFFFFUL << ABR_RESULT_Pos)
#define ABR_RESULT                        ABR_RESULT_Msk                    /*!<baud time.unit is uclk cycle */


/*** Addr for [aon crm uart0] register(0xF100000CUL) ****/
#define AON_CRM_UART0                     (BASE_AON_CRM + 0xC)

/*** Addr for [aon crm uart2] register(0xF100005CUL) ****/
#define AON_CRM_UART2                     (BASE_AON_CRM + 0x5C)

/*** Bit definition for [aon crm uart] register ****/
#define WRESET_Pos                        (8)
#define WRESET_Msk                        (0x1UL << WRESET_Pos)
#define WRESET                            WRESET_Msk                        /*!<Wreset */
#define PRESET_Pos                        (7)
#define PRESET_Msk                        (0x1UL << PRESET_Pos)
#define PRESET                            PRESET_Msk                        /*!<Preset */

//26M工作时钟支持的最大波特率
#define WAKEUP_MAX_BAUDERATE (115200)

//Ringbuffer 模式分配内存大小,该buf的深度必须是2的幂次
#define RINGBUFFER_MAX  (4096)
//wfi预读剩余字节数，用于触发RX中断
#define UART_WFIREAD    16


//波特率设置delay时间。用于读取波特率分频寄存器中的值。在UART_BrateSet函数中配置。
#define BAUDSET_DELAY   20
/************************************************************************************
 *                                 类型定义
 ************************************************************************************/
/**
 * @brief  REG_Uart
 */
typedef struct {
  __IO uint32_t  VS;                               /*!< Offset 0x00000000 */
  __IO uint32_t  DR;                               /*!< Offset 0x00000004 */
  __IO uint32_t  SC;                               /*!< Offset 0x00000008 */
  __IO uint32_t  rsvd0[1];                         /*!< Offset 0x0000000c */
  __IO uint32_t  RSR_ECR;                           /*!< Offset 0x00000010 */
  __IO uint32_t  FR;                               /*!< Offset 0x00000014 */
  __IO uint32_t  rsvd1[2];                         /*!< Offset 0x00000018 */
  __IO uint32_t  ILPR;                             /*!< Offset 0x00000020 */
  __IO uint32_t  IBRD;                             /*!< Offset 0x00000024 */
  __IO uint32_t  FBRD;                             /*!< Offset 0x00000028 */
  __IO uint32_t  rsvd2[1];                         /*!< Offset 0x0000002c */
  __IO uint32_t  LCR_H;                             /*!< Offset 0x00000030 */
  __IO uint32_t  CR;                               /*!< Offset 0x00000034 */
  __IO uint32_t  IFLS ;                            /*!< Offset 0x00000038 */
  __IO uint32_t  rsvd3[1];                         /*!< Offset 0x0000003c */
  __IO uint32_t  IMSC;                             /*!< Offset 0x00000040 */
  __IO uint32_t  RIS;                              /*!< Offset 0x00000044 */
  __IO uint32_t  MIS;                              /*!< Offset 0x00000048 */
  __IO uint32_t  ICR;                              /*!< Offset 0x0000004c */
  __IO uint32_t  DMACR;                            /*!< Offset 0x00000050 */
  __IO uint32_t  rsvd4[11];                        /*!< Offset 0x00000054 */
  __IO uint32_t  TO;                               /*!< Offset 0x00000080 */
  __IO uint32_t  SILENT;                           /*!< Offset 0x00000084 */
  __IO uint32_t  ABRSTA;                           /*!< Offset 0x00000088 */
} REG_Uart;
/************************************************************************************
 *                                 全局变量定义
 ************************************************************************************/
uint32_t supportList[  ] = {0, 2400, 4800, 9600, 14400, 19200, 38400, 57600, 115200, 230400, 460800, 921600,
                            3000000, 6000000, 12000000};

/*对使用的UART进行保存，以便于省电进行检查*/
osList_t g_uartList = OS_LIST_INIT(g_uartList);

/*common dpm ,UART_PORT_LOG和UART_PORT_AT_LOG_COMBINE链接*/
osList_t g_uartCmnDpmList = OS_LIST_INIT(g_uartCmnDpmList);

/*dpm ,UART_PORT_USER，UART_PORT_AT和UART_PORT_CONSOLE链接*/
osList_t g_uartDpmList = OS_LIST_INIT(g_uartDpmList);

/*唤醒功能UART的信息，UART_PORT_AT*/
__IRAM_DATA_PSM_RE UART_RestoreInfo *g_RestoreInfo = (UART_RestoreInfo *)IRAM_BASE_ADDR_LPUART_RESTORE_INFO;

/*WFI 预读UART信息， UART_PORT_USER*/
UART_WFIInfo *g_Uart2_WFIRetore = NULL;
UART_WFIInfo *g_Uart0_WFIRetore = NULL;
UART_WFIInfo *g_Uart1_WFIRetore = NULL;
UART_WFIInfo *g_Uart3_WFIRetore = NULL;
/************************************************************************************
 *                                 函数声明
 ************************************************************************************/
//计算自适应的波特率
static uint32_t prv_getAdaptBauderate(UART_Handle *uart);
//设置波特率
static void prv_setWclkBaud(UART_Handle *uart, uint32_t bauderate);

static int32_t UART_BrateSet(UART_Handle *uart, uint32_t baudval);
/************************************************************************************
 *                                 函数定义
 ************************************************************************************/
/**
 ************************************************************************************
 * @brief           根据function获取链表
 *
 * @param[in]       head            链表头
 * @param[in]       func            功能
 *
 * @return          osList_t *
 * @retval
 ************************************************************************************
*/
static osList_t *prvGetListByFunc(UART_PORT_FUNCTION func)
{

  osList_t *head = OS_NULL;

  switch (func)
  {
    case UART_PORT_LOG:
      head = &g_uartCmnDpmList;
      break;
    case UART_PORT_USER:
    case UART_PORT_AT:
    case UART_PORT_CONSOLE:
      head = &g_uartDpmList;
      break;

    default:
      break;
  }
  return head;
}

/**
 ************************************************************************************
 * @brief           查找链表，并添加到相应的head
 *
 * @param[in]       head            链表头
 * @param[in]       func            功能
 *
 * @return          osList_t *
 * @retval
 ************************************************************************************
*/
static osList_t *prvAddList(UART_Handle *uart, UART_PORT_FUNCTION func)
{
  ubase_t  level;
  osList_t *pos;
  bool_t exist = false;
  UART_Handle *iter = OS_NULL;
  osList_t * head = prvGetListByFunc(func);
  if(head == OS_NULL)
  {
    return head;
  }

  level = osInterruptDisable();
  osListForEach(pos, head)
  {
      iter = osListEntry(pos, UART_Handle, funcNode);
      if (iter->pRes->regBase == uart->pRes->regBase)
      {
        exist = true;
        break;
      }
  }

  if(exist == false)
  {
    osListInsertAfter(head, &(uart->funcNode));
  }

  osInterruptEnable(level);

  return head;
}

/**
 ************************************************************************************
 * @brief           查找链表，并删除
 *
 * @param[in]       head            链表头
 * @param[in]       func            功能
 *
 * @return          osList_t *
 * @retval
 ************************************************************************************
*/
static osList_t *prvDelList(UART_Handle *uart, UART_PORT_FUNCTION func)
{
  ubase_t  level;
  osList_t *pos;
  UART_Handle *iter;
  osList_t * head = prvGetListByFunc(func);
  if(head == OS_NULL)
  {
    return head;
  }

  level = osInterruptDisable();
  osListForEach(pos, head)
  {
      iter = osListEntry(pos, UART_Handle, funcNode);
      if (iter->pRes->regBase == uart->pRes->regBase)
      {
        osListRemove(&(iter->funcNode));
        break;
      }
  }
  osInterruptEnable(level);

  return head;
}

/**
 ************************************************************************************
 * @brief           查找链表,返回查找到的第一个
 *
 * @param[in]       func            功能
 *
 * @return          UART_Handle *
 * @retval
 ************************************************************************************
*/
UART_Handle *UART_findUart(UART_PORT_FUNCTION func)
{
  ubase_t  level;
  osList_t *pos;
  UART_Handle *iter = OS_NULL;

  level = osInterruptDisable();
  osListForEach(pos, &g_uartList)
  {
      iter = osListEntry(pos, UART_Handle, node);
      if (iter->func == func)
      {
        osInterruptEnable(level);
        return iter;
      }
  }
  osInterruptEnable(level);

  return OS_NULL;
}

/**
 ************************************************************************************
 * @brief           添加唤醒UART,只有UART_PORT_AT才具有唤醒功能，具有唤醒功能必须波特率小于等于WAKEUP_MAX_BAUDERATE
 *
 * @param[in]       uart         UART控制器句柄
 *
 * @return          void
 * @retval
 ************************************************************************************
*/
void UART_updateUartWakeUp(UART_Handle *uart)
{
  //UART_PORT_AT(LPUART)添加恢复信息进行唤醒时预读取FIFO，允许进入省电
  if((uart->func == UART_PORT_AT) && (uart->pRes->domain == 0))
  {
    // osPrintf("UART_updateUartWakeUp support26M %d bauderate %d\r\n", g_RestoreInfo->support26M, uart->info.status.setBauderate);
    g_RestoreInfo->addr = (void *)0;

    //支持26M, 波特率小于等于WAKEUP_MAX_BAUDERATE；支持32K(lpuart)，波特率为9600
    if(uart->info.status.setBauderate <= WAKEUP_MAX_BAUDERATE)
    {
      g_RestoreInfo->addr = (void *)(uart->pRes->regBase);
    }
  }

}

/**
 ************************************************************************************
 * @brief          判断是否可以进入省电
 *
 * @param[in]       void
 *
 * @return          boot_t
 * @retval          OS_TURE      系统进入IDLE
 *                  OS_FALSE     系统进入DEEP_SLEEP
 ************************************************************************************
*/
static bool_t prvIsIdle(void)
{
    bool_t ret = OS_FALSE;
    bool_t adaptDone = OS_FALSE;
    REG_Uart    *reg;

    /**
     * UART_PORT_AT存在,需要判断是否符合条件进入睡眠
     * 1.在自适应允许睡眠
     * 2.设置的波特率不符合条件不允许睡眠(g_RestoreInfo->addr == OS_NULL)，
     *   比如设置的不小于115200(26M)或者不是9600(32K)
     *
     * UART_PORT_AT不存在,都可进入睡眠
     */
    UART_Handle *pUart = UART_findUart(UART_PORT_AT);
    if(pUart != OS_NULL)
    {
      reg = (REG_Uart *)(pUart->pRes->regBase);
      adaptDone = (READ_REG(reg->ABRSTA) & ABR_DONE) >> ABR_DONE_Pos ? true : false;

      if((pUart->info.flags & UART_FLAG_AUTO_BAUD))
      {
        // 在自适应允许睡眠
        if(!adaptDone)
        {
          ret = OS_FALSE;
          return ret;
        }
      }

      /*如果不是LPUART作为UART_PORT_AT，允许睡眠*/
      if(pUart->pRes->domain != 0)
      {
          ret = OS_FALSE;
          return ret;
      }

      //设置的波特率不符合睡眠条件或者正在自适应则不允许睡眠
      if(g_RestoreInfo->addr == OS_NULL)
      {
        ret = OS_TRUE;
      }

    }

    return ret;
}

/**
 ************************************************************************************
 * @brief           注册
 *
 * @param[in]       UART            UART控制器句柄
 *
 * @return          int32_t
 * @retval          DRV_OK          成功
 *                  DRV_ERR         失败(已经注册)
 ************************************************************************************
*/
static int prvRegisterUart(UART_Handle *uart)
{
  ubase_t  level;
  osList_t *pos;
  UART_Handle *iter;
  int32_t   ret = DRV_OK;
  bool_t       exist = false;

  level = osInterruptDisable();
  osListForEach(pos, &g_uartList)
  {
      iter = osListEntry(pos, UART_Handle, node);
      if (iter->pRes->regBase == uart->pRes->regBase)
      {
        exist = true;
        ret = DRV_ERR;
        break;
      }
  }

  if(exist == false)
  {
      osListInsertAfter(&g_uartList, &(uart->node));
      prvAddList(uart, uart->func);
      ret = DRV_OK;
  }
  osInterruptEnable(level);

  return ret;
}


/**
 ************************************************************************************
 * @brief           去注册
 *
 * @param[in]       UART            UART控制器句柄
 *
 * @return          void
 * @retval          null
 ************************************************************************************
*/
static void prvDeRegisterUart(UART_Handle *uart)
{
  ubase_t  level;
  osList_t *pos;
  UART_Handle *iter;

  level = osInterruptDisable();
  osListForEach(pos, &g_uartList)
  {
      iter = osListEntry(pos, UART_Handle, node);
      if (iter->pRes->regBase == uart->pRes->regBase)
      {
        osListRemove(&(iter->node));
        prvDelList(uart, uart->func);
        break;
      }
  }
  osInterruptEnable(level);
}

/**
 ************************************************************************************
 * @brief           保存寄存器
 *
 * @param[in]       UART            UART控制器句柄
 * @param[in]       save_addr       保存地址
 *
 * @return          uint32_t
 * @retval
 ************************************************************************************
*/
static uint32_t prvSaveRegs(UART_Handle *uart, uint32_t *save_addr)
{

  REG_Uart    *reg;
  uint32_t    *saveAddr = save_addr;
  uint32_t i = 0;
  OS_ASSERT(uart != OS_NULL);

  reg = (REG_Uart *)(uart->pRes->regBase);
  if (reg != OS_NULL)
  {
    *(saveAddr+(i++)) = READ_REG(reg->CR);
    *(saveAddr+(i++))  = READ_REG(reg->IBRD);
    *(saveAddr+(i++))  = READ_REG(reg->FBRD);
    *(saveAddr+(i++))  = READ_REG(reg->LCR_H);
    *(saveAddr+(i++))  = READ_REG(reg->IMSC);
    *(saveAddr+(i++))  = READ_REG(reg->IFLS);
    *(saveAddr+(i++))  = READ_REG(reg->DMACR);
    *(saveAddr+(i++))  = READ_REG(reg->TO);
  }
  return i*4;
}

/**
 ************************************************************************************
 * @brief           恢复寄存器
 *
 * @param[in]       UART            UART控制器句柄
 * @param[in]       save_addr       恢复地址
 *
 * @return          uint32_t
 * @retval
 ************************************************************************************
*/
static uint32_t prv_restoreRegs(UART_Handle *uart, uint32_t *save_addr)
{

    REG_Uart    *reg;
    uint32_t *saveAddr = save_addr;
    uint32_t i = 0;

    OS_ASSERT(uart != OS_NULL);
    reg = (REG_Uart *)(uart->pRes->regBase);
    if(reg != OS_NULL)
    {
      WRITE_REG(reg->CR, *(saveAddr+(i++)));
      WRITE_REG(reg->IBRD, *(saveAddr+(i++)));
      WRITE_REG(reg->FBRD, *(saveAddr+(i++)));
      WRITE_REG(reg->LCR_H, *(saveAddr+(i++)));
      WRITE_REG(reg->IMSC, *(saveAddr+(i++)));
      WRITE_REG(reg->IFLS, *(saveAddr+(i++)));
      WRITE_REG(reg->DMACR, *(saveAddr+(i++)));
      WRITE_REG(reg->TO, *(saveAddr+(i++)));
    }
    return i*4;
}

static int prv_findBauderate(uint32_t *baudrate_table, int len, uint32_t baudrate)
{
  int i;
  int min = abs(baudrate_table[ 0 ] - baudrate);
  int r   = 0;
  for (i = 0; i < len; i++)
  {
    if (abs(baudrate_table[ i ] - baudrate) < min)
    {
      min = abs(baudrate_table[ i ] - baudrate);
      r   = i;
    }
  }
  return baudrate_table[ r ];
}

//计算自适应的波特率 
static uint32_t prv_getAdaptBauderate(UART_Handle *uart)
{
  REG_Uart *reg = (REG_Uart *)uart->pRes->regBase;
  uint32_t abr_result = 0;
  uint32_t bauderate = 0;

  abr_result = READ_BIT(reg->ABRSTA, ABR_RESULT) >> ABR_RESULT_Pos;
  OS_ASSERT(abr_result != 0);
  bauderate = (uart->info.workClkFreq)/(abr_result);
  bauderate = prv_findBauderate(supportList,9,bauderate);
  return bauderate;
}

//设置波特率
static void prv_setWclkBaud(UART_Handle *uart, uint32_t bauderate)
{
  uint32_t arg = bauderate;
  REG_Uart *reg = (REG_Uart *)uart->pRes->regBase;

  //保存设置的波特率
  uart->info.status.setBauderate = arg;
  /**
   * lpuart && UART_PORT_AT 具有唤醒功能，根据不同波特率设置不同的省电模式
   * 具体配置如下：
   * 1.LPUART 9600      32K   不打开DEEPSLEEP 26M
   * 2.LPUART  <=115200 26M   打开DEEPSLEEP 26M
   * 3.LPUART  其他波特率不允许睡眠
   *
   * 不满足上述条件，则进行如下条件判断：
   * 任意UART作为UART_PORT_CONSOLE，必须配置为26M（省电要求）
   * 任意UART作为UART_PORT_AT，波特率小于等于WAKEUP_MAX_BAUDERATE，必须配置为26M
   * 任意UART除了UART_PORT_CONSOLE && UART_PORT_AT以外使用默认的时钟配置
   */
  if((uart->pRes->domain == 0) && (uart->func == UART_PORT_AT))
  {

    if (arg == 9600)
    {
      if(uart->info.select_Clock == 0)
      {
        uart->info.workClkFreq = FREQ_32KHZ;
        CLK_SetregGenLbAonCrmRegs((CLK_regGenLbAonCrmRegs)(uart->pRes->crmRegCLKSEL), LPUART_32K);
        SET_BIT(reg->CR,  LP_MODE);

        #if defined(_CPU_AP) && defined(OS_USING_PM)
        PSM_SetPsmNv(PSM_DS26MDCXOEN_ID, 0);
        #endif        
      }
      else
      {
        uart->info.workClkFreq = FREQ_26MHZ;
        CLK_SetregGenLbAonCrmRegs((CLK_regGenLbAonCrmRegs)(uart->pRes->crmRegCLKSEL), LPUART_26M);
        CLEAR_BIT(reg->CR,  LP_MODE);

        #if defined(_CPU_AP) && defined(OS_USING_PM)
        PSM_SetPsmNv(PSM_DS26MDCXOEN_ID, 1);
        #endif         
      }

    }
    else if(arg <= WAKEUP_MAX_BAUDERATE)
    {
      uart->info.workClkFreq = FREQ_26MHZ;
      CLK_SetregGenLbAonCrmRegs((CLK_regGenLbAonCrmRegs)(uart->pRes->crmRegCLKSEL), uart->pRes->clk_sel);
      CLEAR_BIT(reg->CR, LP_MODE);

      #if defined(_CPU_AP) && defined(OS_USING_PM)
      PSM_SetPsmNv(PSM_DS26MDCXOEN_ID, 1);
      #endif
    }
    else
    {
      uart->info.workClkFreq = FREQ_156MHZ;
      CLK_SetregGenLbAonCrmRegs((CLK_regGenLbAonCrmRegs)(uart->pRes->crmRegCLKSEL), LPUART_156M);
      CLEAR_BIT(reg->CR, LP_MODE);
      
      #if defined(_CPU_AP) && defined(OS_USING_PM)
      PSM_SetPsmNv(PSM_DS26MDCXOEN_ID, 0);
      #endif
    }

  }

  if(uart->func == UART_PORT_CONSOLE)
  {
    uart->info.workClkFreq = FREQ_26MHZ;
    CLK_SetPdcoreLspCrmRegs((CLK_PdcoreLspCrmRegs)(uart->pRes->crmRegCLKSEL), UART0_WCLK_26M);

    #if defined(_CPU_AP) && defined(OS_USING_PM)
    PSM_SetPsmNv(PSM_DS26MDCXOEN_ID, 1);
    #endif
  }
  
  if((uart->pRes->domain == 0) && (uart->func == UART_PORT_LOG))
  {
      uart->info.workClkFreq = FREQ_156MHZ;
      CLK_SetregGenLbAonCrmRegs((CLK_regGenLbAonCrmRegs)(uart->pRes->crmRegCLKSEL), LPUART_156M);
      CLEAR_BIT(reg->CR, LP_MODE);
  }

  //set Baudrate
  if(arg != 0)
  {
    UART_BrateSet(uart, arg);
  }
}

/**
 ************************************************************************************
 * @brief           轮询读取FIFO数据
 *
 * @param[in]       reg            UART控制器寄存器地址
 * @param[in]       pcChar         数据存放的缓冲
 * @param[out]      count          读取的数据的计数
 * @param[in]       remain_read     剩余的可以读取的个数
 *
 * @return          void
 * @retval          null
 ************************************************************************************
*/
static __IRAM_CODE_PSM_RE void UART_RxPoll(REG_Uart *reg, char *pcChar, uint32_t *count, uint32_t remain_read, uint32_t rsvd_cnt)
{
  uint32_t status;
  uint32_t can_read = 0;

  status            = READ_REG(reg->FR);
  can_read          = (status & RXF_DATA_CNT) >> 24;
  if (can_read > rsvd_cnt)
  {
      can_read -= rsvd_cnt;
  }
  else
  {
      return;
  }
  // if empty or space count is 0 then return
  if (!(status & RXF_NOT_EMPTY) | !(can_read)) return;
  while (can_read)
  {
      *(pcChar++) = READ_REG(reg->DR) & 0xFF;
      (*count)++;
      can_read--;
      
      // 达到需读取数量则返回
      if (*count == remain_read) return;
  }
}
#if defined(_CPU_AP)
static __IRAM_CODE_PSM_RE void UART_RxPoll_Copy(UART_Handle *uart, uint32_t *count)
{
  REG_Uart *reg  = (REG_Uart *)uart->pRes->regBase;
  uint32_t status;
  uint32_t can_read = 0;
  status            = READ_REG(reg->FR);
  can_read          = (status & RXF_DATA_CNT) >> 24;
  

  // if empty or space count is 0 then return
  if (!(status & RXF_NOT_EMPTY) | !(can_read)) return;
  while (can_read)
  {

    uint8_t byte = READ_REG(reg->DR) & 0xFF;
    if(!os_ringbuffer_overwrite_byte(&(uart->ringbuffer), byte))
    {
        break;
    }
    (*count)++;
    can_read--;

  }
}
#endif


static int8_t UART_SW_AUTOBaudCheck(UART_Handle *uart, uint8_t *buf, uint32_t len)
{
    if(uart == NULL || buf == NULL)
    {
        UART_dbg("check AT faild: invalid parameters\r\n");
        return DRV_ERR;
    }

    /* 如果接收到小于2字节，返回error */
    if (len < 2)
    {
        UART_dbg("check AT failed: received < 2 bytes\r\n");
        goto error;
    }
    
    /* 如果接收到大于2个字节，遍历整个rx_buf查找"AT"字符 */
    for (uint32_t i = 0; i < len - 1; i++)
    {
        if ((buf[i] == 'a' || buf[i] == 'A') && (buf[i+1] == 't' || buf[i+1] == 'T'))
        {
            UART_dbg("check AT successful: found 'AT' in rx_buf at position %d\r\n", i);
            return DRV_OK;
        }
    }
    
    /* 如果没有找到"AT"字符，返回error 重新自适应*/
    UART_dbg("check AT failed: 'AT' not found in buffer of length %d\r\n", len);
  
error:
      uart->re_adaptbaud();
      return DRV_ERR;
}

/**
 ************************************************************************************
 * @brief           轮询读取FIFO数据,供PSM读取
 *
 * @param[in]       void            UART控制器寄存器地址
 *
 * @return          void
 * @retval          null
 ************************************************************************************
*/
__IRAM_CODE_PSM_RE void UART_PSMRxPoll(void)
{
    
    if((void *)BASE_LP_UART == g_RestoreInfo->addr)
    {
        // 只有LPUART支持
        g_RestoreInfo->sleep = true;
        REG_Uart *reg = (REG_Uart *)(g_RestoreInfo->addr);

            // UART_PSMRxPoll读UART_RxPoll保留一位不读, 用于产生中断
        UART_RxPoll(reg, (char *)(g_RestoreInfo->store + g_RestoreInfo->size), &(g_RestoreInfo->size), 
                        sizeof(g_RestoreInfo->store), 1);
    }

}

/**
 ************************************************************************************
 * @brief           IDLE 预读取
 *
 * @param[in]       void            UART控制器寄存器地址
 *
 * @return          void
 * @retval          null
 ************************************************************************************
*/
static int8_t UART_WFIRxPoll(UART_WFIInfo *info)
{

      info->wfi = true;
      REG_Uart *reg = (REG_Uart *)(info->addr);
      uint8_t current_size = info->size;
      uint8_t remain_space = sizeof(info->store) - current_size;
      uint8_t uart_read = READ_U32(BASE_UART2 + 0x14) >> 24;

      if(remain_space > UART_WFIREAD && uart_read > UART_WFIREAD)
      {
          uint32_t temp_count = 0;
          UART_RxPoll(reg, (char *)(info->store + info->size), (uint32_t *)&(temp_count), remain_space, UART_WFIREAD);
          info->size += temp_count;
          if(info->size >= sizeof(info->store))
          {
              return DRV_ERR;
          }
          else
          {
              return DRV_OK;
          }
      }
      else
      {
          return DRV_ERR;
      }      
}

void UART_WFIRestore(void)
{ 
    if(g_Uart2_WFIRetore != NULL)
    {
        UART_WFIRxPoll(g_Uart2_WFIRetore);
    }
    if(g_Uart0_WFIRetore != NULL)
    {
        UART_WFIRxPoll(g_Uart0_WFIRetore);
    }
    if(g_Uart1_WFIRetore != NULL)
    {
        UART_WFIRxPoll(g_Uart1_WFIRetore);
    }
    if(g_Uart3_WFIRetore != NULL)
    {
        UART_WFIRxPoll(g_Uart3_WFIRetore);
    }
}

/**
 ************************************************************************************
 * @brief           清空接收FIFO数据
 *
 * @param[in]       reg            UART控制器寄存器地址
 * @param[in]       pcChar         数据存放的缓冲
 * @param[out]      count          读取的数据的计数
 * @param[in]       remain_read     剩余的可以读取的个数
 *
 * @return          void
 * @retval          null
 ************************************************************************************
*/
static void UART_flushRxFIFO(REG_Uart *reg)
{
  uint32_t count = 0;
  uint32_t val;
  count = (READ_REG(reg->FR) & RXF_DATA_CNT) >> 24 ;

  while (count --)
  {
    val = READ_REG(reg->DR) & 0xFF;
    val++; // 去除编译警告
  }
}

/**
 ************************************************************************************
 * @brief           清空ringbuffer数据
 *
 * @param[in]       uart           UART控制器句柄
 *
 * @return          void
 * @retval          null
 ************************************************************************************
*/
#if defined(_CPU_AP)
void UART_FlushRingbuf(UART_Handle *uart)
{
    os_ringbuffer_reset(&(uart->ringbuffer));
}
#endif

/**
 ************************************************************************************
 * @brief          发送数据
 *
 * @param[in]       uart           UART控制器句柄
 * @param[in]       ch             待发送的数据
 *
 * @return          uint32_t
 * @retval          == 0                发送失败
 *                  == 1                发送失败
 ************************************************************************************
*/
static uint32_t UART_DataTx(UART_Handle *uart, uint8_t ch)
{
  REG_Uart *reg = (REG_Uart *)uart->pRes->regBase;
  /*add new bits TXF_NOT_FULL/RXF_NOT_EMPTY/TXF_SPACE_CNT/RXF_DATA_CNT.they are
   * conservative and reliability,soc suggest do not use old bits TXFF/RXFF/TXFE/RXFE
   */
  /*if TXF_NOT_FULL(1 not full   0 full ) is 0 ,then return ,
  or TXF_SPACE_CNT is 0 ,then return.
  */
  if ((READ_REG(reg->FR) & TXF_NOT_FULL) == 0) return 0; /* unable to transmit character */

  WRITE_REG(reg->DR, ch);
  return 1;
}

/**
 ************************************************************************************
 * @brief          CTS中断处理函数
 *
 * @param[in]       irq_id         中断号
 * @param[in]       irq_data       中断参数
 *
 * @return          void
 * @retval          null
 ************************************************************************************
 */
static void UART_CTSIRQHandler(int irq_id, void *irq_data)
{
    osInterruptClrPending(irq_id);
    #if defined(OS_USING_PM)
    PCU_WakeupIrqClrpending(irq_id);
    #endif
    osPrintf("%s\r\n", __FUNCTION__);
}

/**
 ************************************************************************************
 * @brief          WFI唤醒之后，中断读取数据到ringbuffer接口
 *
 * @param[in]       uart           UART控制器句柄
 * 
 * @return          void
 * @retval          null
 ************************************************************************************
 */
static void UART_DataToBuf(UART_Handle *uart)
{
    
    if((uart->wfiInfo !=NULL) && (uart->wfiInfo->wfi == true))
    {
        uart->wfiInfo->wfi = false;
        uint32_t copy_WfiNum = 0;
        if(uart->wfiInfo->size !=0)
        {
            #if defined(_CPU_AP)
              // uint32_t freeSpace = os_ringbuffer_get_free((os_ringbuffer_t *)uart->xfer.rx_buf);
              copy_WfiNum = uart->wfiInfo->size;// > freeSpace ? freeSpace : uart->wfiInfo->size;
              os_ringbuffer_overwrite(&(uart->ringbuffer), uart->wfiInfo->store, copy_WfiNum);
            #endif                 
            uart->xfer.rx_cnt += copy_WfiNum;
            uart->wfiInfo->size = 0;
            memset(uart->wfiInfo->store, 0x0, sizeof(uart->wfiInfo->store));
        }
    }
}
/**
 ************************************************************************************
 * @brief          中断处理函数
 *
 * @param[in]       irq_id         中断号
 * @param[in]       irq_data       中断参数
 *
 * @return          void
 * @retval          null
 ************************************************************************************
 */
static void UART_IRQHandler(int irq_id, void *irq_data)
{
  int32_t       status, count, remain_count;
  uint32_t      event;
  UART_Handle  *uart = (UART_Handle *)irq_data;
  uint32_t      dma_cnt = 0;
  REG_Uart *reg  = (REG_Uart *)uart->pRes->regBase;
  UartDmaInfo *dmaInfo = &uart->info.dmaInfo;
  DMA_ChHandle *rxDmaHandle = dmaInfo->rxDmaHandle;
  uint32_t bauderate = 0;
  uint32_t abr_result = 0;
  uint8_t abr_done = 0;
  bool_t adaptDone = OS_FALSE;

  count        = 0U;
  remain_count = 0U;
  event        = 0U;
  #if defined(OS_USING_PM)
  //每次中断都清除uart的wakeup
  if(uart->capabilities.wakeup == 1)
  {
    PCU_WakeupIrqClrpending(irq_id);
  }
  #endif
  if((uart->info.flags & UART_FLAG_AUTO_BAUD) && uart->func == UART_PORT_AT)
  {
    abr_result = READ_BIT(reg->ABRSTA, ABR_RESULT) >> ABR_RESULT_Pos;
    abr_done = READ_BIT(reg->ABRSTA, ABR_DONE) >> ABR_DONE_Pos;
    if ((abr_result > 0x0) && (abr_result < 0xE0) && (uart->re_adaptbaud) && (abr_done != 1))
    {
      uart->re_adaptbaud();
      return;
    }
  }
  status = (READ_REG(reg->RIS)) & (uart->info.mask);

  if (status)
  {

    WRITE_REG(reg->ICR, status);
    if(status & SCRIS)
    {
      event |= UART_EVENT_RECEIVE_SPECIAL_CHAR;
    }

    if(status & (FEIM | PEIM | BEIM ))
    {
      #ifdef OS_USING_PM
      if(uart->capabilities.wakeup == 1)
      {
        PSM_ClrPeriBusy(BUSY_FLAG_APSS_LP_UART);
      }
      #endif
    }

    if (status & (RXRIS | RTRIS | WCDRIS))
    {
      if(uart->capabilities.rxDMAEnable == 0)
      {
          //不可以进入睡眠
          #ifdef OS_USING_PM
          if(uart->capabilities.wakeup == 1)
          {
            PSM_SetPeriBusy(BUSY_FLAG_APSS_LP_UART);
          }

          //唤醒
          if ((g_RestoreInfo->addr != NULL) && (g_RestoreInfo->addr == reg) && (g_RestoreInfo->sleep == true))
          {
              uint32_t copyNum = 0;
              g_RestoreInfo->sleep = false;
              // osPrintf("wakeup recv size %d\r\n", g_RestoreInfo->size);
              if(g_RestoreInfo->size != 0)
              {
                if(uart->func == UART_PORT_USER)
                {
                  #if defined(_CPU_AP)
                    uint32_t free_space = os_ringbuffer_get_free(&(uart->ringbuffer));
                    copyNum = g_RestoreInfo->size > free_space ? free_space : g_RestoreInfo->size;
                    os_ringbuffer_write(&(uart->ringbuffer), g_RestoreInfo->store, copyNum);
                  #endif                 
                }
                else
                {
                  copyNum = g_RestoreInfo->size > uart->xfer.rx_num ? uart->xfer.rx_num : g_RestoreInfo->size;
                  memcpy(uart->xfer.rx_buf, g_RestoreInfo->store, copyNum);
                }

                remain_count = uart->xfer.rx_num - copyNum;
                uart->xfer.rx_cnt += copyNum;
                g_RestoreInfo->size = 0;
              }

          }

          #endif

          UART_DataToBuf(uart);

          if(uart->func == UART_PORT_USER)
          {
            #if defined(_CPU_AP)        
              UART_RxPoll_Copy(uart, (uint32_t *)&(count));      
            #endif     
          }
          else
          {
              remain_count = uart->xfer.rx_num - uart->xfer.rx_cnt;
              UART_RxPoll(reg, (char *)(uart->xfer.rx_buf + uart->xfer.rx_cnt), (uint32_t *)&(count), remain_count, 0);
          }
          // for (int i = 0; i < count; i++)
          // {
          //   UART_dbg("ch[%d] %d-%c \r\n", i, *(char *)(uart->xfer.rx_buf + uart->xfer.rx_cnt  + i),*(char *)(uart->xfer.rx_buf + uart->xfer.rx_cnt  + i));
          // }
          
          uart->xfer.rx_cnt += count;
  #if defined(OS_USING_PM) && defined(_CPU_AP)
          if (((uint32_t)uart->pRes->regBase == BASE_LP_UART) && (status & RTRIS)) {
            PSM_RcSwitchToDcxo();
          }
  #endif
          //Check if requested amount of data is received

          /**
           * 对接收处理的分类：
           * 1. 实际发送的长度等于配置接收的长度，通过长度比较相等立即发送消息通知上层，屏蔽中断。上层再配置的时候，上次的产生RTRIS也不会通知上层，因为FIFO中没有数据读出,即rx_cnt为0。
           * 2. 实际发送的长度小于配置接收的长度，通过RTRIS通知上层，屏蔽中断。
           * 3. a 实际发送的长度大于配置接收的长度，接收到配置的长度就通知上层，屏蔽中断。
           *    b 上层再配置的时候，会将剩余的数据FIFO中读出(也有可能溢出)，通过rx中断读出长度等于再配置的长度，回到a的情况。
           *    c 超时中断通知上层。
           */
          if ((uart->xfer.rx_cnt == uart->xfer.rx_num) || ((status & RTRIS) && (uart->xfer.rx_cnt != 0)))
          {
            uart->info.mask &= ~(RXIM | RTIM | FEIM | PEIM | BEIM | OEIM | WCDRIM);
            WRITE_REG(reg->IMSC,uart->info.mask);
            uart->info.status.rx_busy = 0U;
            event |= UART_EVENT_RX_ARRIVED;
            event |= UART_EVENT_RECEIVE_COMPLETE;

            if((uart->info.flags & UART_FLAG_AUTO_BAUD) && uart->func == UART_PORT_AT)
            {
              adaptDone = (READ_REG(reg->ABRSTA) & ABR_DONE) >> ABR_DONE_Pos ? true : false;
              uint8_t ret = UART_SW_AUTOBaudCheck(uart, uart->xfer.rx_buf, uart->xfer.rx_cnt);
              if(adaptDone && !ret)
              {
                //自适应结束
                uart->info.flags &=  ~UART_FLAG_AUTO_BAUD;
                //获取自适应的波特率进行时钟切换
                bauderate = prv_getAdaptBauderate(uart);
                uart->info.status.setBauderate = bauderate;
                if(uart->adaptChClock == 1)
                {
                  // osPrintf("adapt bauderate is %d\r\n", bauderate);
                  prv_setWclkBaud(uart, bauderate);
                  UART_updateUartWakeUp(uart);
                }

              }

            }

          }        
      }
      else
      {
        if(status & (RTRIS | RXRIS))
        {
          //如果已经在DMA回调里处理，但是尚未配置接收直接返回不再通知上层
          if(uart->info.status.rx_busy ==  0)
            return;

          uart->info.mask &= ~RXIM;
          WRITE_REG(reg->IMSC,uart->info.mask);
          event |= UART_EVENT_RX_ARRIVED;

          DMA_Stop(rxDmaHandle);
          dma_cnt = DMA_GetCount(rxDmaHandle);
          UART_dbg("dma_cnt %d\r\n", dma_cnt);
          uart->info.status.rx_busy = 0U;
          uart->xfer.rx_cnt += (uart->xfer.rx_num - dma_cnt );
          event |= UART_EVENT_RECEIVE_COMPLETE;
          // 收到数据进行invalidate
          osDCacheInvalidRange(uart->xfer.rx_buf, uart->xfer.rx_cnt);
        }
      }

    }
    // fifo tx
    if (status & TXRIS)
    {

      if (uart->xfer.tx_num != uart->xfer.tx_cnt)
      {

        for (int i = uart->xfer.tx_cnt; i < uart->xfer.tx_num; i++)
        {
          status = UART_DataTx(uart, uart->xfer.tx_buf[ i ]);
          if (status == true)
          {
            uart->xfer.tx_cnt++;
          }
          else
            break;
        }
      }
      if (uart->xfer.tx_num == uart->xfer.tx_cnt)
      {
        uart->info.mask &= ~TXIM;
        WRITE_REG(reg->IMSC ,uart->info.mask);
        uart->xfer.send_active = 0U;
        event |= UART_EVENT_SEND_COMPLETE;
        #if defined(OS_USING_PM) && defined(_CPU_AP)
        PSM_WakeUnlock(&(uart->info.lock));
        #endif

      }
    }

    if(status & OERIS)
    {
      uart->info.mask &= ~OEIM;
      WRITE_REG(reg->IMSC ,uart->info.mask);
      event |= UART_EVENT_RX_OVERFLOW;
    }

    if(uart->func == UART_PORT_USER)
    {
        ubase_t level = osInterruptDisable();

        uart->info.mask |= RXIM | RTIM | FEIM | PEIM | BEIM | OEIM | WCDRIM;
        WRITE_REG(reg->IMSC, uart->info.mask);

        osInterruptEnable(level);      
    }


    // Send Event
    if ((event && uart->info.cb_event) != 0U)
    {
      uart->info.cb_event(uart, event);
    }
  }
  
}

/**
 ************************************************************************************
 * @brief          DMA tx 回调函数
 *
 * @param[in]       para         参数
 *
 * @return          void
 * @retval          void
 ************************************************************************************
 */
static void UART_TxDMACb(void *para)
{
  uint32_t event = 0;
  int32_t  dma_cnt = 0;
  UART_Handle *uart = (UART_Handle *)para;
  UartDmaInfo *dmaInfo = &uart->info.dmaInfo;
  UART_dbg("UART_TxDMACb\r\n");
  uart->xfer.send_active = 0U;
  dma_cnt = DMA_GetCount(dmaInfo->txDmaHandle);
  uart->xfer.tx_cnt += (uart->xfer.tx_num - dma_cnt);

  event |= UART_EVENT_SEND_COMPLETE;
  // Send Event
  if ((event && ((UART_Handle *)para)->info.cb_event) != 0U)
  {
    ((UART_Handle *)para)->info.cb_event(para, event);
  }
}

/**
 ************************************************************************************
 * @brief          获取uart TX物理通道并配置部分通道信息
 *
 * @param[in]       uart           UART控制器句柄
 *
 * @return          int32_t
 * @retval          DRV_ERR      失败
 *                  DRV_OK       成功
 ************************************************************************************
 */

static int32_t UART_TxDmaInit(UART_Handle *uart)
{
  UartDmaInfo *dmaInfo = &uart->info.dmaInfo;
  DMA_ChCfg   *txChCfg = &dmaInfo->txChCfg;
  dmaInfo->txDmaHandle   = DMA_Request(uart->pRes->txLogicID);

  if(dmaInfo->txDmaHandle == NULL)
  {
    OS_ASSERT(0);
    return DRV_ERR;
  }

  dmaInfo->txDmaHandle->callback = UART_TxDMACb;
  dmaInfo->txDmaHandle->para     = (void *)uart;

  txChCfg->DestAddr              = (uint32_t)(&(((REG_Uart *)uart->pRes->regBase)->DR));
  txChCfg->Control.BurstReqMod   = DMA_RMOD_DEV;
  txChCfg->Control.DestMod       = DMA_AMOD_FIFO;
  txChCfg->Control.SrcMod        = DMA_AMOD_RAM;
  txChCfg->Control.SrcBurstSize  = DMA_BSIZE_8BIT;
  txChCfg->Control.DestBurstSize = DMA_BSIZE_8BIT;
  txChCfg->Control.IrqMod        = DMA_IMOD_ALL_ENABLE;
  txChCfg->Control.IntSel        = DMA_INT1;
  return DRV_OK;
}

/**
 ************************************************************************************
 * @brief          释放通道
 *
 * @param[in]       uart           UART控制器句柄
 *
 * @return          int32_t
 * @retval          DRV_ERR      失败
 *                  DRV_OK       成功
 ************************************************************************************
 */

static int32_t UART_TxDmaDeInit(UART_Handle *uart)
{
  UartDmaInfo *dmaInfo = &uart->info.dmaInfo;

  if(dmaInfo->txDmaHandle == NULL)
  {
    OS_ASSERT(0);
    return DRV_ERR;
  }

  DMA_Release(dmaInfo->txDmaHandle);
  return DRV_OK;
}


/**
 ************************************************************************************
 * @brief           设置UART控制器发送数据
 *
 * @param[in]       UART            UART控制器句柄
 * @param[in]       data            发送数据缓存的buffer
 * @param[in]       num             发送数据的个数
 *
 * @return          int32_t
 * @retval          DRV_ERR_PARAMETER             错误的参数
 *                  DRV_ERR                       错误
 *                  DRV_OK                        成功
 ************************************************************************************
*/
static int32_t Uart_txDMA(UART_Handle *uart, const void *data, uint32_t num)
{
  int32_t     ret = 0;
  UartDmaInfo *dmaInfo = &uart->info.dmaInfo;
  DMA_ChCfg   *txChCfg = &dmaInfo->txChCfg;
  DMA_ChHandle *txDmaHandle = dmaInfo->txDmaHandle;


  txChCfg->SrcAddr = (uint32_t)data;
  txChCfg->Control.SrcBurstLen = DMA_BLEN_4;
  txChCfg->Count               = num;
  osDCacheCleanRange((void *)txChCfg->SrcAddr, txChCfg->Count);
  ret = DMA_Start(txDmaHandle, txChCfg);
  return ret;
}
/**
 ************************************************************************************
 * @brief          DMA rx 回调函数
 *
 * @param[in]       para         参数
 *
 * @return          void
 * @retval          void
 ************************************************************************************
 */
static void UART_RxDMACb(void *para)
{
  uint32_t event = 0;
  int32_t  dma_cnt = 0;
  UART_Handle *uart = (UART_Handle *)para;
  UartDmaInfo *dmaInfo = &uart->info.dmaInfo;
  DMA_ChHandle *rxDmaHandle = dmaInfo->rxDmaHandle;
  REG_Uart *reg  = (REG_Uart *)uart->pRes->regBase;

  uart->info.status.rx_busy = 0U;
  uart->info.mask &= ~RXIM;
  WRITE_REG(reg->IMSC,uart->info.mask);
  event |= UART_EVENT_RX_ARRIVED;

  UART_dbg("UART_RxDMACb\r\n");
  dma_cnt = DMA_GetCount(rxDmaHandle);
  UART_dbg("dma_cnt %d\r\n", dma_cnt);
  uart->xfer.rx_cnt += (uart->xfer.rx_num - dma_cnt );
  event |= UART_EVENT_RECEIVE_COMPLETE;
  // 收到数据进行invalidate
  osDCacheInvalidRange(uart->xfer.rx_buf, uart->xfer.rx_cnt);
  // Send Event
  if ((event && uart->info.cb_event) != 0U)
  {
    uart->info.cb_event(uart, event);
  }
}
/**
 ************************************************************************************
 * @brief          获取uart rX物理通道并配置部分通道信息
 *
 * @param[in]       uart           UART控制器句柄
 *
 * @return          int32_t
 * @retval          DRV_ERR      失败
 *                  DRV_OK       成功
 ************************************************************************************
 */

static int32_t UART_RxDmaInit(UART_Handle *uart)
{
  UartDmaInfo *dmaInfo = &uart->info.dmaInfo;
  DMA_ChCfg   *rxChCfg = &dmaInfo->rxChCfg;
  dmaInfo->rxDmaHandle   = DMA_Request(uart->pRes->rxLogicID);

  if(dmaInfo->rxDmaHandle == NULL)
  {
    OS_ASSERT(0);
    return DRV_ERR;
  }

  dmaInfo->rxDmaHandle->callback = UART_RxDMACb;
  dmaInfo->rxDmaHandle->para     = (void *)uart;

  rxChCfg->SrcAddr               = (uint32_t)(&(((REG_Uart *)uart->pRes->regBase)->DR));
  rxChCfg->Control.BurstReqMod   = DMA_RMOD_DEV;
  rxChCfg->Control.SrcMod        = DMA_AMOD_FIFO;
  rxChCfg->Control.DestMod       = DMA_AMOD_RAM;
  rxChCfg->Control.SrcBurstSize  = DMA_BSIZE_8BIT;
  rxChCfg->Control.DestBurstSize = DMA_BSIZE_8BIT;
  rxChCfg->Control.IrqMod        = DMA_IMOD_ALL_ENABLE;
  rxChCfg->Control.IntSel        = DMA_INT1;
  return DRV_OK;
}

/**
 ************************************************************************************
 * @brief         释放RX通道
 *
 * @param[in]       uart           UART控制器句柄
 *
 * @return          int32_t
 * @retval          DRV_ERR      失败
 *                  DRV_OK       成功
 ************************************************************************************
 */

static int32_t UART_RxDmaDeInit(UART_Handle *uart)
{
  UartDmaInfo *dmaInfo = &uart->info.dmaInfo;

  if(dmaInfo->rxDmaHandle == NULL)
  {
    OS_ASSERT(0);
    return DRV_ERR;
  }
  DMA_Release(dmaInfo->rxDmaHandle);

  return DRV_OK;
}

/**
 ************************************************************************************
 * @brief           设置UART控制器接收数据
 *
 * @param[in]       UART            UART控制器句柄
 * @param[in]       data            接收数据缓存的buffer
 * @param[in]       num             接收数据的个数
 *
 * @return          int32_t
 * @retval          DRV_ERR_PARAMETER             错误的参数
 *                  DRV_ERR                       错误
 *                  DRV_OK                        成功
 ************************************************************************************
 */
static int32_t Uart_rxDMA(UART_Handle *uart, void *data, uint32_t num)
{
  int32_t     ret = 0;
  UartDmaInfo *dmaInfo = &uart->info.dmaInfo;
  DMA_ChCfg   *rxChCfg = &dmaInfo->rxChCfg;
  DMA_ChHandle *rxDmaHandle = dmaInfo->rxDmaHandle;

  rxChCfg->DestAddr = (uint32_t)data;
  rxChCfg->Control.SrcBurstLen = DMA_BLEN_1;
  rxChCfg->Count               = num;

  osDCacheCleanAndInvalidRange((void *)rxChCfg->DestAddr, rxChCfg->Count);

  ret = DMA_Start(rxDmaHandle, rxChCfg);
  return ret;
}
/**
 ************************************************************************************
 * @brief          UART控制器初始化
 *
 * @param[in]       uart           UART控制器句柄
 *
 * @return          void
 * @retval          null
 ************************************************************************************
 */
static void UART_HwInit(UART_Handle *uart)
{
  
  UART_CAPABILITIES *capablity = 0;
  REG_Uart          *reg       = 0;

  capablity = &uart->capabilities;
  reg  = (REG_Uart *)uart->pRes->regBase;



  if(uart->func == UART_PORT_USER)
  {

      uart->wfiInfo = osMalloc(sizeof(UART_WFIInfo));
      if(uart->wfiInfo == NULL)
      {
          OS_ASSERT(0);
      }
      osMemset(uart->wfiInfo, 0x0, sizeof(UART_WFIInfo));
      uart->wfiInfo->addr = (void *)reg;

      if(reg == (void*)BASE_UART2)
      {
          g_Uart2_WFIRetore = uart->wfiInfo;
      }
      else if(reg == (void*)BASE_LP_UART)
      {
          g_Uart0_WFIRetore = uart->wfiInfo;
      }
      else if(reg == (void*)BASE_UART0)
      {
          g_Uart1_WFIRetore = uart->wfiInfo;
      }
      else if(reg == (void*)BASE_UART3)
      {
          g_Uart3_WFIRetore = uart->wfiInfo;
      }
  }

#if defined(_CPU_AP)
  static bool_t s_lpuartRestoreInfoInit = OS_FALSE;
  if (!s_lpuartRestoreInfoInit)
  {
    osMemset(g_RestoreInfo, 0x0, sizeof(UART_RestoreInfo));
    s_lpuartRestoreInfoInit = OS_TRUE;
  }
#endif
   
  //lp_uart
  if(uart->pRes->domain == 0)
  {
    // lpuart wclk enable，1：enable，0：disable
    CLK_SetregGenLbAonCrmRegs((CLK_regGenLbAonCrmRegs)(uart->pRes->crmRegWCLK), 1);
    //默认选择32K  0 26M ; 1 32K ;3 156M
    CLK_SetregGenLbAonCrmRegs((CLK_regGenLbAonCrmRegs)(uart->pRes->crmRegCLKSEL), uart->pRes->clk_sel);
    switch(uart->pRes->clk_sel)
    {
        case LPUART_32K:
        uart->info.workClkFreq = 32*1000;
        break;
        case LPUART_26M:
        uart->info.workClkFreq = 26*1000*1000;
        break;
        case LPUART_156M:
        uart->info.workClkFreq = 156*1000*1000;
        break;
        default:
         OS_ASSERT(0);
        }
  }
  else
  {
    //work clk sw gate (0-close clk ,1-open clk)
    CLK_SetPdcoreLspCrmRegs((CLK_regGenLbAonCrmRegs)(uart->pRes->crmRegWCLK), 1);
    //work clk select (0-26Mhz,1-78Mhz,2-104Mhz,3-156Mhz)
    CLK_SetPdcoreLspCrmRegs((CLK_regGenLbAonCrmRegs)(uart->pRes->crmRegCLKSEL), uart->pRes->clk_sel);
    switch(uart->pRes->clk_sel)
    {
        case UART0_WCLK_26M:
        uart->info.workClkFreq =  26*1000*1000;
        break;
        case UART0_WCLK_78M:
        uart->info.workClkFreq = 78*1000*1000;
        break;
        case UART0_WCLK_104M:
        uart->info.workClkFreq = 104*1000*1000;
        break;
        case UART0_WCLK_156M:
        uart->info.workClkFreq = 156*1000*1000;
        break;
        default:
        OS_ASSERT(0);
    }
  }

  //clear pending error and rx intterrupts
  if(capablity->rxDMAEnable || capablity->txDMAEnable)
  {
    WRITE_REG(reg->ICR,OEIC | BEIC | PEIC | FEIC | RTIC);
  }
  else
  {
    WRITE_REG(reg->ICR,OEIC | BEIC | PEIC | FEIC | RTIC | RXIC);
  }

  //unmask intterupt
  if(capablity->rxDMAEnable || capablity->txDMAEnable)
  {
    uart->info.mask &= ~(RTIM | FEIM | PEIM | BEIM | OEIM);
  }
  else
  {
    uart->info.mask &= ~(RXIM | RTIM | FEIM | PEIM | BEIM | OEIM | TXIM);
  }

  if(capablity->rxDMAEnable)
  {
    UART_RxDmaInit(uart);
  }

  if(capablity->txDMAEnable && (capablity->useForHDLC != 1))
  {
    UART_TxDmaInit(uart);
  }

  if(uart->func == UART_PORT_USER)
  {
      uart->info.mask |= RXIM | RTIM | FEIM | PEIM | BEIM | OEIM | WCDRIM;
      WRITE_REG(reg->IMSC, uart->info.mask);
  }
  else
  {
      WRITE_REG(reg->IMSC, uart->info.mask);
  }

  uart->info.mask = READ_REG(reg->IMSC);

#if defined(OS_USING_PM)
  if(capablity->wakeup == 1)
  {

    PCU_WakeupIrqRegister(OS_EXT_IRQ_TO_IRQ(uart->pRes->intNum),ICT_PCU_HIGH_LEVEL);
  }
  #endif
  osInterruptConfig(OS_EXT_IRQ_TO_IRQ(uart->pRes->intNum), 1, IRQ_HIGH_LEVEL);
  osInterruptInstall(OS_EXT_IRQ_TO_IRQ(uart->pRes->intNum), UART_IRQHandler, uart);
  osInterruptUnmask(OS_EXT_IRQ_TO_IRQ(uart->pRes->intNum));

  if(uart->pRes->ctsNum != 0)
  {
    #if defined(OS_USING_PM)
    PCU_WakeupIrqRegister(OS_EXT_IRQ_TO_IRQ(uart->pRes->ctsNum),ICT_PCU_NEGATIVE_EDGE);
    osInterruptConfig(OS_EXT_IRQ_TO_IRQ(uart->pRes->ctsNum), 1, IRQ_HIGH_LEVEL);
    #else
    osInterruptConfig(OS_EXT_IRQ_TO_IRQ(uart->pRes->ctsNum), 1, IRQ_NEGATIVE_EDGE);
    #endif
    osInterruptInstall(OS_EXT_IRQ_TO_IRQ(uart->pRes->ctsNum), UART_CTSIRQHandler, uart);
    osInterruptUnmask(OS_EXT_IRQ_TO_IRQ(uart->pRes->ctsNum));
  }

}

/**
 ************************************************************************************
 * @brief          轮询发送数据
 *
 * @param[in]       uart           UART控制器句柄
 * @param[in]       ch             待发送的数据
 *
 * @return          void
 * @retval          null
 ************************************************************************************
*/
static void UART_DataTxPoll(UART_Handle *uart, uint8_t ch)
{
  REG_Uart *reg = (REG_Uart *)uart->pRes->regBase;
  // make sure there is space in FIFO
  while (1)
  {
    if (READ_REG(reg->FR) & TXF_NOT_FULL)
    {
      break;
    }
  }
  WRITE_REG(reg->DR, ch);
}

/**
 ************************************************************************************
 * @brief          波特率设置
 *
 * @param[in]       uart               UART控制器句柄
 * @param[in]       baudval            波特率
 *
 * @return          int32_t
 * @retval          DRV_OK             设置成功
 *                  DRV_ERR_PARAMETER  参数错误
 ************************************************************************************
*/
static int32_t UART_BrateSet(UART_Handle *uart, uint32_t baudval)
{
  int32_t       result   = DRV_OK;
  uint32_t      uart_clk = 0;
  uint32_t      baudrate;
  int32_t       ibrd, fbrd;
  uint32_t      timecnt = 10;
  REG_Uart *reg = (REG_Uart *)uart->pRes->regBase;

#ifdef USE_TOP_ASIC
  uart_clk = uart->info.workClkFreq;
#endif
#ifdef USE_TOP_FPGA
  uart_clk          = 100 * 1000 * 1000;
#endif
  baudrate = baudval;

  if(baudrate == 9600)
  {
      WRITE_REG(reg->TO, 0x3FF);
  }

  ibrd = uart_clk / (baudrate << 3);
  if ((ibrd > 0xFFFF) || (uart_clk < (baudrate << 3)))
  {
    result = DRV_ERR_PARAMETER;
    return result;
  }
  OS_ASSERT(ibrd != 0);
  fbrd      = ((uart_clk - ibrd * 8 * baudrate) * 64 + 4 * baudrate) / (8 * baudrate);
  WRITE_REG(reg->IBRD, ibrd);
  WRITE_REG(reg->FBRD, fbrd);
  WRITE_REG(reg->TO, TO_THRES/ibrd);

  uart->info.status.baudRate = uart_clk / (READ_BIT(reg->IBRD, BAUD_RATE_INTEGER) * 8);
  //配置波特率，载入分频器delay一段时间，使其生效。如果配置完成立刻发数据，有概率出现乱码
  while(timecnt--)
  {    
      int32_t R_ibrd = READ_REG(reg->IBRD);
      int32_t R_fbrd = READ_REG(reg->FBRD);

      if((R_ibrd == ibrd) && (R_fbrd == fbrd))
      {
          result = DRV_OK;
          break;
      }

      osUsDelay(BAUDSET_DELAY);
  }

  if(timecnt == 0)
  {
      result = DRV_ERR;
  }

  return result;
}


/**
 ************************************************************************************
 * @brief           获取支持的能力
 *
 * @param[in]       UART            UART控制器句柄
 *
 * @return          UART_CAPABILITIES
 * @retval          支持DMA以及流控
 ************************************************************************************
*/
UART_CAPABILITIES UART_GetCapabilities(UART_Handle *uart)
{
  return uart->capabilities;
}

/**
 ************************************************************************************
 * @brief           初始化UART控制器
 *
 * @param[in]       UART            UART控制器句柄
 *
 * @return          int32_t
 * @retval          DRV_OK          成功
 ************************************************************************************
*/
int32_t UART_Initialize(UART_Handle *uart, UART_Callback cb_event)
{
  if (uart->info.flags & UART_FLAG_INITIALIZED)
  {
    // Driver is already initialized
    return DRV_OK;
  }

  // Initialize UART Run-time Resources
  uart->info.cb_event = cb_event;

  uart->info.status.tx_busy      = 0U;

  uart->info.status.rx_busy          = 0U;

  uart->xfer.send_active = 0U;

  // Clear transfer information
  uart->info.flags = UART_FLAG_INITIALIZED;
#if defined(_CPU_AP)
  if(uart->func == UART_PORT_USER)
  {
      uart->uart_ringbuf = osMalloc(RINGBUFFER_MAX);
      if (uart->uart_ringbuf == NULL) {
        return DRV_ERR;
      }
      uint8_t ret = os_ringbuffer_init(&(uart->ringbuffer), uart->uart_ringbuf, RINGBUFFER_MAX);
      if(ret == 0)
      {
          UART_dbg("ringbuffer create sucessfull\r\n");
      }
      else 
      {
          UART_dbg("ringbuffer create failed\r\n");
          return DRV_ERR;
      }
      uart->xfer.rx_buf = (uint8_t *)uart->uart_ringbuf;

      uart->xfer.rx_cnt = 0U;
      uart->xfer.rx_num = RINGBUFFER_MAX;
  }
#endif
  //register
  prvRegisterUart(uart);

  return DRV_OK;
}

/**
 ************************************************************************************
 * @brief           去初始化UART控制器
 *
 * @param[in]       UART            UART控制器句柄
 *
 * @return          int32_t
 * @retval          DRV_OK          成功
 ************************************************************************************
*/
int32_t UART_Uninitialize(UART_Handle *uart)
{
  uart->info.cb_event = NULL;
  // Reset UART status flags
  uart->info.flags = 0U;
#if defined(_CPU_AP)
  if(uart->func == UART_PORT_USER)
  {
      // 释放 RingBuffer 资源
      if (uart->uart_ringbuf != NULL) {
          osFree(uart->uart_ringbuf);
      }
      
      os_ringbuffer_reset(&(uart->ringbuffer));
      uart->ringbuffer.mask = 0;
      uart->ringbuffer.pool = NULL;  

      if(uart->wfiInfo != NULL)
      {
          osFree(uart->wfiInfo);
      }
  }
#endif
  //register
  prvDeRegisterUart(uart);

  return DRV_OK;
}

/**
 ************************************************************************************
 * @brief           设置UART控制器的供电
 *
 * @param[in]       UART            UART控制器句柄
 * @param[in]       state           设置控制的状态
 *
 * @return          int32_t
 * @retval          DRV_ERR_PARAMETER             错误的参数
 *                  DRV_ERR_UNSUPPORTED           不支持
 *                  DRV_ERR                       错误
 *                  DRV_OK                        成功
 ************************************************************************************
*/
int32_t UART_PowerControl(UART_Handle *uart, DRV_POWER_STATE state)
{
  UART_CAPABILITIES *capablity = 0;
  if ((state != DRV_POWER_OFF) &&
      (state != DRV_POWER_FULL) &&
      (state != DRV_POWER_LOW))
  {
    return DRV_ERR_PARAMETER;
  }

  switch (state)
  {
  case DRV_POWER_OFF:

    if ((uart->info.flags & UART_FLAG_POWERED) == 0U)
    {
      return DRV_ERR;
    }
    //复位
    if(uart->pRes->domain == 0)
    {
      CLK_SetregGenLbAonCrmRegs((CLK_regGenLbAonCrmRegs)(CLK_LPUART_SW_PRST_N), 0);
      CLK_SetregGenLbAonCrmRegs((CLK_regGenLbAonCrmRegs)(CLK_LPUART_SW_WRST_N), 0);
    }

    capablity = &uart->capabilities;
    // Clear Status flags
    uart->info.status.tx_busy          = 0U;
    uart->info.status.rx_busy          = 0U;
    uart->xfer.send_active             = 0U;
    #if defined(OS_USING_PM)
    if(capablity->wakeup == 1)
    {
      PCU_WakeupIrqUnregister(OS_EXT_IRQ_TO_IRQ(uart->pRes->intNum));
    }
    #endif
    osInterruptUninstall(OS_EXT_IRQ_TO_IRQ(uart->pRes->intNum));

    if(uart->pRes->ctsNum != 0)
    {
      #if defined(OS_USING_PM)
      PCU_WakeupIrqUnregister(OS_EXT_IRQ_TO_IRQ(uart->pRes->ctsNum));
      #endif
      osInterruptUninstall(OS_EXT_IRQ_TO_IRQ(uart->pRes->ctsNum));
    }
    if(capablity->txDMAEnable && (capablity->useForHDLC != 1))
    {
      UART_TxDmaDeInit(uart);
    }
    if(capablity->rxDMAEnable)
    {
      UART_RxDmaDeInit(uart);
    }
    uart->info.flags &= ~UART_FLAG_POWERED;
    break;

  case DRV_POWER_LOW:
    return DRV_ERR_UNSUPPORTED;

  case DRV_POWER_FULL:
    if ((uart->info.flags & UART_FLAG_INITIALIZED) == 0U)
    {
      return DRV_ERR;
    }
    if ((uart->info.flags & UART_FLAG_POWERED) != 0U)
    {
      return DRV_OK;
    }

    //释放复位
    if(uart->pRes->domain == 0)
    {

      CLK_SetregGenLbAonCrmRegs((CLK_regGenLbAonCrmRegs)(CLK_LPUART_SW_PRST_N), 1);
      CLK_SetregGenLbAonCrmRegs((CLK_regGenLbAonCrmRegs)(CLK_LPUART_SW_WRST_N), 1);
    }

    // Clear Status flags
    uart->info.status.tx_busy          = 0U;
    uart->info.status.rx_busy          = 0U;

    uart->xfer.send_active  = 0U;
    uart->xfer.break_flag   = 0U;
    uart->info.flow_control = 0U;

    uart->info.flags = UART_FLAG_POWERED | UART_FLAG_INITIALIZED;
    UART_HwInit(uart);
    break;
  }
  return DRV_OK;
}

/**
 ************************************************************************************
 * @brief           判断硬件发送FIFO是否全部发送完数据
 *
 * @param[in]       UART            UART控制器句柄
 *
 * @return          int8_t
 * @retval          DRV_ERR_BUSY                  硬件FIFO中数据还未全部发送完成
 *                  DRV_OK                        数据全部发送完毕
 ************************************************************************************
*/
int8_t UART_SendComplete(UART_Handle *uart)
{
    REG_Uart *reg = (REG_Uart *)uart->pRes->regBase;
    uint8_t value = 0;

    value = (READ_REG(reg->FR) & TXF_SPACE_CNT) >> 16;

    if(value == 0x20)
    {
       return DRV_OK;
    }
    else
    {
       return DRV_ERR_BUSY;
    }
}

/**
 ************************************************************************************
 * @brief           设置UART控制器发送数据
 *
 * @param[in]       UART            UART控制器句柄
 * @param[in]       data            发送数据缓存的buffer
 * @param[in]       num             发送数据的个数
 *
 * @return          int32_t
 * @retval          DRV_ERR_PARAMETER             错误的参数
 *                  DRV_ERR                       错误
 *                  DRV_ERR_BUSY                  设备忙
 *                  DRV_OK                        成功
 ************************************************************************************
*/
int32_t UART_Send(UART_Handle *uart, const void *data, uint32_t num)
{
  uint32_t      status;
  ubase_t       level;
  REG_Uart *reg = (REG_Uart *)uart->pRes->regBase;
  if ((data == NULL) || (num == 0U))
  {
    // Invalid parameters
    return DRV_ERR_PARAMETER;
  }

  if ((uart->info.flags & UART_FLAG_CONFIGURED) == 0U)
  {
    // UART is not configured
    return DRV_ERR;
  }

  if (uart->xfer.send_active != 0U)
  {
    // Send is not completed yet
    return DRV_ERR_BUSY;
  }

  if(uart->capabilities.useForHDLC == 1)
  {
    //HDLC不使用此接口
    return DRV_ERR_UNSUPPORTED;
  }

  if((uart->info.flags & UART_FLAG_AUTO_BAUD) && uart->func == UART_PORT_AT && (uart->capabilities.txDMAEnable == 0))
  {
    //AT uart is not ready
    return DRV_ERR;
  }
  // Set Send active flag
  uart->xfer.send_active = 1U;

  // Save transmit buffer info
  uart->xfer.tx_buf = (uint8_t *)((uint32_t)data);
  uart->xfer.tx_num = num;
  uart->xfer.tx_cnt = 0U;
  if(uart->capabilities.txDMAEnable == 0){
    for (int i = 0; i < uart->xfer.tx_num; i++)
    {
      status = UART_DataTx(uart, uart->xfer.tx_buf[ i ]);
      if (status == true)
      {
        uart->xfer.tx_cnt++;
      }
      else
        break;
    }
    #if defined(OS_USING_PM) && defined(_CPU_AP)
    PSM_WakeLock(&(uart->info.lock));
    #endif
    /*in order to stop handler,add enable TX*/
    level = osInterruptDisable();
    uart->info.mask |= TXIM;
    WRITE_REG(reg->IMSC, uart->info.mask);
    osInterruptEnable(level);

  } else
  {
    Uart_txDMA(uart, data, num);
  }
  return DRV_OK;
}

/**
 ************************************************************************************
 * @brief           设置UART控制器轮询发送数据
 *
 * @param[in]       UART            UART控制器句柄
 * @param[in]       data            发送数据缓存的buffer
 * @param[in]       num             发送数据的个数
 *
 * @return          int32_t
 * @retval          DRV_ERR_PARAMETER             错误的参数
 *                  DRV_ERR                       错误
 *                  DRV_ERR_BUSY                  设备忙
 *                  DRV_OK                        成功
 ************************************************************************************
*/
int32_t UART_PollSend(UART_Handle *uart, const void *data, uint32_t num)
{

  if ((data == NULL) || (num == 0U))
  {
    // Invalid parameters
    return DRV_ERR_PARAMETER;
  }

  if ((uart->info.flags & UART_FLAG_CONFIGURED) == 0U)
  {
    // UART is not configured
    return DRV_ERR;
  }
  //轮询发送不管之前是否发送结束，继续新的发送
  // if (uart->xfer.send_active != 0U)
  // {
  //   // Send is not completed yet
  //   return DRV_ERR_BUSY;
  // }

  // Set Send active flag
  uart->xfer.send_active = 1U;

  // Save transmit buffer info
  uart->xfer.tx_buf = (uint8_t *)((uint32_t)data);
  uart->xfer.tx_num = num;
  uart->xfer.tx_cnt = 0U;
  for (int i = 0; i < uart->xfer.tx_num; i++)
  {
    UART_DataTxPoll(uart, uart->xfer.tx_buf[ i ]);
    uart->xfer.tx_cnt++;
  }
  uart->xfer.send_active = 0U;
  return DRV_OK;
}

/**
 ************************************************************************************
 * @brief           设置UART控制器接收数据
 *                  注意：UART控制器工作在DMA方式，
 *                  函数调用者传入的参数data是cacheline(32)对齐的，num是cacheline(32)的整数倍
 * @param[in]       UART            UART控制器句柄
 * @param[in]       data            接收数据缓存的buffer
 * @param[in]       num             接收数据的个数
 *
 * @return          int32_t
 * @retval          DRV_ERR_PARAMETER             错误的参数
 *                  DRV_ERR                       错误
 *                  DRV_ERR_BUSY                  设备忙
 *                  DRV_OK                        成功
 ************************************************************************************
 */
int32_t UART_Receive(UART_Handle *uart, void *data, uint32_t num)
{

  ubase_t level;
  REG_Uart *reg = (REG_Uart *)uart->pRes->regBase;
  if ((data == NULL) || (num == 0U))
  {
    // Invalid parameters
    return DRV_ERR_PARAMETER;
  }

  if ((uart->info.flags & UART_FLAG_CONFIGURED) == 0U)
  {
    // UART is not configured (mode not selected)
    return DRV_ERR;
  }

  // Check if receiver is busy
  if (uart->info.status.rx_busy == 1U)
  {
    return DRV_ERR_BUSY;
  }

  // Save number of data to be received
  uart->xfer.rx_num = num;
  // Save receive buffer info
  uart->xfer.rx_buf = (uint8_t *)data;
  uart->xfer.rx_cnt = 0U;

  // Set RX busy flag
  uart->info.status.rx_busy = 1U;
  if(uart->capabilities.rxDMAEnable == 1)
  {
    Uart_rxDMA(uart, data, num);
  }
  level = osInterruptDisable();
  if(uart->capabilities.rxDMAEnable != 1)
  {
    uart->info.mask |= RXIM | RTIM | FEIM | PEIM | BEIM | OEIM | WCDRIM;
  } else
  {
    uart->info.mask |= RXIM | RTIM | FEIM | PEIM | BEIM | OEIM;
  }

  WRITE_REG(reg->IMSC, uart->info.mask);
  osInterruptEnable(level);
  #ifdef OS_USING_PM
  if(uart->capabilities.wakeup == 1)
  {
    PSM_ClrPeriBusy(BUSY_FLAG_APSS_LP_UART);
  }
  #endif

  return DRV_OK; 
}

/**
 ************************************************************************************
 * @brief           设置UART控制器接收数据
 *                  此接口接收数据使用的是RingBuffer方案
 *                  UART控制器初始化的时候会将RX中断打开，接收到数据之后存储到RingBuffer中，读取数据从RingBuffer中读取
 * @param[in]       UART            UART控制器句柄
 * @param[in]       data            接收数据缓存的buffer
 * @param[in]       num             应用buf大小
 *
 * @return          int32_t
 * @retval          DRV_ERR_PARAMETER             错误的参数
 *                  DRV_ERR                       错误
 *                  DRV_ERR_BUSY                  设备忙
 *                  DRV_OK                        成功
 *                  read_count                    返回接收到数据的个数
 ************************************************************************************
 */
int32_t UART_Receive_Copy(UART_Handle *uart, void *data, uint32_t num)
{

  ubase_t level;
  uint32_t data_cnt = 0;
  uint32_t rxCount = 0;
  if ((data == NULL) || (num == 0U))
  {
    // Invalid parameters
    return DRV_ERR_PARAMETER;
  }

  if ((uart->info.flags & UART_FLAG_CONFIGURED) == 0U)
  {
    // UART is not configured (mode not selected)
    return DRV_ERR;
  }

  // Check if receiver is busy
  if (uart->info.status.rx_busy == 1U)
  {
    return DRV_ERR_BUSY;
  }
#if defined(_CPU_AP)
  if(uart->ringbuffer.in == uart->ringbuffer.out)
  {
      return DRV_ERR_EMPTY;
  }
#endif
  #if defined(_CPU_AP)
    data_cnt = os_ringbuffer_get_used(&(uart->ringbuffer));
    rxCount = data_cnt > num ? num : data_cnt;

    uart->info.status.rx_busy = 1U;
  
    uint32_t read_count = os_ringbuffer_peek(&(uart->ringbuffer), data, rxCount);
    UART_dbg("receive read_count %d data_cnt %d rxCount %d\r\n", read_count, data_cnt, rxCount);
    if(read_count > 0)
    {
        level = osInterruptDisable();
        os_ringbuffer_drop(&(uart->ringbuffer), read_count);
        osInterruptEnable(level);
    }
    uart->info.status.rx_busy = 0U;
    return (read_count >= 0) ? read_count : DRV_ERR;   
  #endif
}

/**
 ************************************************************************************
 * @brief           获取UART控制器发送的字节的个数.
 *
 * @param[in]       UART            UART控制器句柄
 *
 * @return          uint32_t
 * @retval          >=0             发送的字节的个数
 *
 ************************************************************************************
*/
uint32_t UART_GetTxCount(UART_Handle *uart)
{
  return uart->xfer.tx_cnt;
}

/**
 ************************************************************************************
 * @brief           获取UART控制器收到的字节的个数.
 *
 * @param[in]       UART            UART控制器句柄
 *
 * @return          uint32_t
 * @retval          >=0             收到的字节的个数
 *
 ************************************************************************************
 */
uint32_t UART_GetRxCount(UART_Handle *uart)
{
#if defined(_CPU_AP)
  if(uart->func == UART_PORT_USER)
  {
      return os_ringbuffer_get_used(&(uart->ringbuffer));
  }
  else
#endif  
  {
      return uart->xfer.rx_cnt;
  }
}

/**
 ************************************************************************************
 * @brief           控制UART控制器.
 *
 * @param[in]       UART            UART控制器句柄
 * @param[in]       control         控制命令
 * @param[in]       control         控制命令参数
 *
 * @return          初始化返回值.
 * @retval          ==DRV_OK        执行成功
 *                  ==DRV_ERR       执行错误
 *                  ==DRV_ERR_BUSY  设备忙
 *                  == UART_ERROR_DATA_BITS 错误的数据位
 *                  ==UART_ERROR_PARITY     错误的校验位
 *                  ==UART_ERROR_STOP_BITS  错误的停止位
 *                  ==UART_ERROR_FLOW_CONTROL 错误的流控
 *                  ==UART_ERROR_BAUDRATE     错误的波特率
 ************************************************************************************
 */
int32_t UART_Control(UART_Handle *uart, uint32_t control, uint32_t arg)
{
  uint32_t      flow_control;
  UART_CAPABILITIES *capablity = 0;
  REG_Uart *reg = (REG_Uart *)uart->pRes->regBase;
  UartSupportedBauderate *bauderateSupport;

  capablity = &uart->capabilities;
  if ((uart->info.flags & UART_FLAG_POWERED) == 0U)
  {
    // UART not powered
    return DRV_ERR;
  }

  switch (control & UART_CONTROL_Msk)
  {
  // Abort Send
    case UART_ABORT_SEND:

    /*
    * FIFO模式下，去使能发送中断。HDLC下不使用UART_Send接口，所以不需要释放wakelock锁
    * DMA模式下并且没有使能HDLC，则关闭DMA。
    */
    if(uart->capabilities.txDMAEnable == 0)
    {
        if((uart->capabilities.useForHDLC == 0) && (uart->xfer.send_active == 1))
        {
            #if defined(OS_USING_PM) && defined(_CPU_AP)
            PSM_WakeUnlock(&(uart->info.lock));
            #endif
        }

      // Disable TX interrupt
      uart->info.mask &= ~(TXIM);
      WRITE_REG(reg->IMSC, uart->info.mask);
    }else if(uart->capabilities.useForHDLC != 1)
    {
      DMA_Stop(uart->info.dmaInfo.txDmaHandle);
    }

    // Clear Send active flag
    uart->xfer.send_active = 0U;
    return DRV_OK;

  // Abort receive
  case UART_ABORT_RECEIVE:
  
    if(uart->capabilities.rxDMAEnable != 1)
    {
      // Disable RX interrupt
      uart->info.mask &= ~(RXIM | RTIM | FEIM | PEIM |BEIM |OEIM);
      WRITE_REG(reg->IMSC, uart->info.mask);
    }
    else
    {
      uart->info.mask &= ~(RTIM | FEIM | PEIM |BEIM |OEIM);
       DMA_Stop(uart->info.dmaInfo.rxDmaHandle);
    }

    // Clear RX busy status
    uart->info.status.rx_busy = 0U;

    return DRV_OK;
  // Control TX
  case UART_CONTROL_TX:
    if (arg)
    {
      uart->info.flags |= UART_FLAG_TX_ENABLED;
      SET_BIT(reg->CR, TXE);
    }
    else
    {
      CLEAR_BIT(reg->CR, TXE);
      uart->info.flags &= ~UART_FLAG_TX_ENABLED;
    }

    return DRV_OK;

  // Control RX
  case UART_CONTROL_RX:
    if (arg)
    {
      uart->info.flags |= UART_FLAG_RX_ENABLED;
      SET_BIT(reg->CR, RXE);
    }
    else
    {
      CLEAR_BIT(reg->CR, RXE);
      uart->info.flags &= ~UART_FLAG_RX_ENABLED;
    }

    return DRV_OK;

  case UART_CONTROL_AUTO_BAUD:
    if(uart->func != UART_PORT_AT)
    {
      return DRV_ERR;
    }

    if (arg)
    {
      WRITE_REG(reg->IBRD, 0x1);
      uart->info.flags |= UART_FLAG_AUTO_BAUD;
      SET_BIT(reg->CR, AUTO_BAUD);
      CLEAR_BIT(reg->CR, LP_MODE);
      WRITE_REG(reg->TO, 0x3FF);
    }
    else
    {
      CLEAR_BIT(reg->CR, AUTO_BAUD);
      uart->info.flags &= ~UART_FLAG_AUTO_BAUD;
    }
    return DRV_OK;
    case UART_SET_TX_FIFO_SEL:
    {
      //arg = TXIFLSEL8
      MODIFY_REG(reg->IFLS, TXIFLSEL, arg);
    }
    return DRV_OK;

    case UART_SET_SPECIAL_CHAR:
    if (arg <= 0x1ff)
    {
      WRITE_REG(reg->SC, arg);
      uart->info.mask |= SCIM;
      WRITE_REG(reg->IMSC, uart->info.mask);
    }
    else
    {
      WRITE_REG(reg->SC, 0);
      uart->info.mask &= ~SCIM;
      WRITE_REG(reg->IMSC, uart->info.mask);
    }
    return DRV_OK;
    case UART_UART0_RESET:
    {
      CLEAR_BIT_U32(AON_CRM_UART0, PRESET | WRESET);
      SET_BIT_U32(AON_CRM_UART0, PRESET | WRESET);
    }
    return DRV_OK;
    case UART_UART2_RESET:
    {
      CLEAR_BIT_U32(AON_CRM_UART2, PRESET | WRESET);
      SET_BIT_U32(AON_CRM_UART2, PRESET | WRESET);
    }
    return DRV_OK;
    case UART_CONTROL_LOOPBACK:
    {
      SET_BIT(reg->CR, LBE);
    }
      return DRV_OK;
    case UART_GET_SUPPORTED_BAUDERATE_LIST:
    bauderateSupport = (UartSupportedBauderate *)arg;
    bauderateSupport->bauderateList = supportList;
    bauderateSupport->bauderateNum = sizeof(supportList)/sizeof(supportList[0]);
    return DRV_OK;
    default:
      break;
  }

  // Check if busy
  if ((uart->info.status.rx_busy != 0U) || (uart->xfer.send_active != 0U))
  {
    return DRV_ERR_BUSY;
  }

  // UART Data bits
  switch (control & UART_DATA_BITS_Msk)
  {
  case UART_DATA_BITS_6:
    MODIFY_REG(reg->LCR_H, WLEN_Msk, WLEN6);
    break;
  case UART_DATA_BITS_7:
    MODIFY_REG(reg->LCR_H, WLEN_Msk, WLEN7);
    break;
  case UART_DATA_BITS_8:
    MODIFY_REG(reg->LCR_H, WLEN_Msk, WLEN8);
    break;
  case UART_DATA_BITS_5:
    MODIFY_REG(reg->LCR_H, WLEN_Msk, WLEN5);
    break;
  default:
    return UART_ERROR_DATA_BITS;
  }
  uart->info.status.wordLen = READ_BIT(reg->LCR_H,WLEN) >> 5;


  // UART Parity
  switch (control & UART_PARITY_Msk)
  {
  case UART_PARITY_NONE:
    break;
  case UART_PARITY_EVEN:
    SET_BIT(reg->LCR_H, (PEN | EPS));
    break;
  case UART_PARITY_ODD:
    SET_BIT(reg->LCR_H, PEN);
    break;
  default:
    return UART_ERROR_PARITY;
  }
  //Use UART_GetStatus get parity
  switch((READ_REG(reg->LCR_H) >> 1) & 0x3)
  {
      case 0x0:
        uart->info.status.parity = UART_PARITY_NONE >> UART_PARITY_Pos;
        break;
      case 0x1:
        uart->info.status.parity = UART_PARITY_ODD >> UART_PARITY_Pos;
        break;
      case 0x3:
        uart->info.status.parity = UART_PARITY_EVEN >> UART_PARITY_Pos;
        break;
      default:
      return UART_ERROR_PARITY;  
  }

  // UART Stop bits
  switch (control & UART_STOP_BITS_Msk)
  {
  case UART_STOP_BITS_1:
    CLEAR_BIT(reg->LCR_H, STP2);
    break;
  case UART_STOP_BITS_2:
    SET_BIT(reg->LCR_H, STP2);
    break;
  default:
    return UART_ERROR_STOP_BITS;
  }
  uart->info.status.stopBit = READ_BIT(reg->LCR_H,STP2) >> 3;

  // UART Flow control
  switch (control & UART_FLOW_CONTROL_Msk)
  {
  case UART_FLOW_CONTROL_NONE:
    flow_control = UART_FLOW_CONTROL_NONE;
    break;
  case UART_FLOW_CONTROL_RTS:
    if (uart->capabilities.flow_control_rts)
    {
      flow_control = UART_FLOW_CONTROL_RTS;
      // RTS Enable
      SET_BIT(reg->CR, RTSEN);
    }
    else
    {
      // RTS Enable
      CLEAR_BIT(reg->CR, RTSEN);
    }
    return DRV_OK;
  case UART_FLOW_CONTROL_CTS:
    if (capablity->flow_control_cts){
      flow_control = UART_FLOW_CONTROL_CTS;
      // CTS Enable, CTS interrupt enable
      SET_BIT(reg->CR, CTSEN);
    } else
    {
      CLEAR_BIT(reg->CR, CTSEN);
    }
    return DRV_OK;
  case UART_FLOW_CONTROL_RTS_CTS:
    if ((capablity->flow_control_rts != 0U) &&
        (capablity->flow_control_cts != 0U)){
      flow_control = UART_FLOW_CONTROL_RTS_CTS;
      // RTS and CTS Enable, CTS interrupt enable
      SET_BIT(reg->CR, (RTSEN | CTSEN));
    } else
    {
      CLEAR_BIT(reg->CR, (RTSEN | CTSEN));
    }
    return DRV_OK;
  default:
    return UART_ERROR_FLOW_CONTROL;
  }
  uart->info.status.flowControl = READ_BIT(reg->CR,RTSEN | CTSEN) >> 14;

  // UART Enable

  prv_setWclkBaud(uart, arg);

  CLEAR_BIT(reg->DMACR, (RXDMAE | TXDMAE));

  if(capablity->rxDMAEnable){
    SET_BIT(reg->DMACR, RXDMAE);
  }

  if(capablity->txDMAEnable){
    SET_BIT(reg->DMACR, TXDMAE);
  }
  else if(!capablity->useForHDLC)
  {
    #if defined(OS_USING_PM) && defined(_CPU_AP)
    PSM_WakelockInit(&(uart->info.lock),PSM_DEEP_SLEEP);
    #endif
  }

  SET_BIT(reg->LCR_H,  FEN);

  if(uart->func == UART_PORT_USER)
  {
    MODIFY_REG(reg->IFLS, (RXIFLSEL | TXIFLSEL), (RXIFLSEL16 | TXIFLSEL4));
  }
  else
  {
    MODIFY_REG(reg->IFLS, (RXIFLSEL | TXIFLSEL), (RXIFLSEL1 | TXIFLSEL4));    
  }

  // Save flow control mode
  uart->info.flow_control = flow_control;

  // Configure UART control registers
  //8分频
  SET_BIT(reg->CR, SAMP_RATE);

  SET_BIT(reg->CR, (TXE | RXE | UARTEN));

  // Set configured flag
  uart->info.flags |= UART_FLAG_CONFIGURED;

  //添加唤醒UART
  UART_updateUartWakeUp(uart);
  return DRV_OK;
}

/**
 ************************************************************************************
 * @brief           获取UART控制器状态
 *
 * @param[in]       UART            UART控制器句柄
 * @param[in]       status          uart状态指针
 *
 * @return          UART控制器状态.
 * @retval          0      获取成功
 ************************************************************************************
 */
int32_t UART_GetStatus(UART_Handle *uart, UART_STATUS *status)
{
  REG_Uart *reg = (REG_Uart *)uart->pRes->regBase;

  if (uart->xfer.send_active != 0U)
  {
    status->tx_busy = 1U;
  }
  else
  {
    status->tx_busy = ((READ_REG(reg->FR) & TXBUSY) ? (1U) : (0U));
  }
  status->rx_busy          = uart->info.status.rx_busy;
  //查询自适应波特率返回0，标识为自适应
  status->setBauderate     = ((uart->func == UART_PORT_AT) && ((READ_REG(reg->ABRSTA) & ABR_DONE) >> ABR_DONE_Pos)) ? 0 : uart->info.status.setBauderate;
  status->wordLen          = uart->info.status.wordLen;
  status->stopBit          = uart->info.status.stopBit;
  status->flowControl      = uart->info.status.flowControl;
  status->parity           = uart->info.status.parity;
  status->baudRate         = uart->info.status.baudRate;
  return 0;
}

/**
 ************************************************************************************
 * @brief           获取当前UART波特率
 *
 * @param[in]       UART            UART控制器句柄
 *
 * @return          UART波特率.
 ************************************************************************************
 */
int32_t UART_GetCurrentBaudRate(UART_Handle *uart)
{

    return uart->info.status.setBauderate;
}

/**
 ************************************************************************************
 * @brief           获取LPUART的发送状态
 *
 * @param[in]       bool_t
 *
 * @return          bool_t
 * @retval          true            空闲状态
                    false           发送忙
 ************************************************************************************
 */
bool_t UART_GetLpuartIdleState(void)
{
  uint8_t busy = 0;

  REG_Uart *reg = (REG_Uart *) (g_UART_Res[0].regBase);
  busy = ((READ_REG(reg->FR) & (TXBUSY | RXBUSY)) ? (1U) : (0U));
  if(busy == 1)
    return false;
  return true;
}

uint32_t UART_GetCTS(UART_Handle *uart)
{
    REG_Uart *reg = (REG_Uart *)(uart->pRes->regBase);
    uint32_t cts = ((READ_REG(reg->FR)) & (CTS)) >> CTS_Pos;

    return cts;
}

#ifdef OS_USING_PM
static int UART_dpmSuspend(void *param, PSM_Mode mode, uint32_t *save_addr)
{
  osList_t *pos;
  int i = 0;
  int sum = 0;
  UART_Handle *iter;
  osList_t * head = &g_uartDpmList;
  if ((mode == PSM_DEEP_SLEEP) && (osListLen(head) != 0))
  {
    osListForEach(pos, head)
    {
      iter = osListEntry(pos, UART_Handle, funcNode);
      if(iter->pRes->regBase == g_RestoreInfo->addr)
      {
        continue;
      }
      i = prvSaveRegs(iter, (save_addr + sum));
      sum = sum + i;
    }
  }
  return sum;
}

static int UART_dpmResume(void *param, PSM_Mode mode,uint32_t *save_addr)
{
  osList_t *pos;
  int i = 0;
  int sum = 0;
  UART_Handle *iter;
  osList_t * head = &g_uartDpmList;
  if ((mode == PSM_DEEP_SLEEP) && (osListLen(head) != 0))
  {
    osListForEach(pos, head)
    {
      iter = osListEntry(pos, UART_Handle, funcNode);
      if(iter->pRes->regBase == g_RestoreInfo->addr)
      {
        continue;
      }

      i = prv_restoreRegs(iter, (save_addr + sum));
      sum = sum + i;
    }
  }
  return sum;
}

static PSM_DpmOps dpmops = {
    .PsmSuspendNoirq = UART_dpmSuspend,
    .PsmResumeNoirq = UART_dpmResume,
};

PSM_DPM_INFO_DEFINE(uart_dpm, dpmops, OS_NULL, PSM_LEVEL_HIGH);


static int UART_cmnSuspend(void *param, PSM_Mode mode, uint32_t *save_addr)
{
  osList_t *pos;
  int i = 0;
  int sum = 0;
  UART_Handle *iter;
  osList_t * head = &g_uartCmnDpmList;

  if ((mode == PSM_DEEP_SLEEP) && (osListLen(head) != 0))
  {
    osListForEach(pos, head)
    {
      iter = osListEntry(pos, UART_Handle, funcNode);
      i = prvSaveRegs(iter, (save_addr + sum));
      sum = sum + i;
      CLEAR_BIT(((REG_Uart*)(iter->pRes->regBase))->CR, RXE);
      // 进入深睡流程中关闭uart流控, 防止HDLC在后续睡眠流程中堵塞
      CLEAR_BIT(((REG_Uart*)(iter->pRes->regBase))->CR, RTSEN);
      CLEAR_BIT(((REG_Uart*)(iter->pRes->regBase))->CR, CTSEN);
    }
  }
  return sum;
}

static int UART_cmnResume(void *param, PSM_Mode mode,uint32_t *save_addr)
{
  osList_t *pos;
  int i = 0;
  int sum = 0;
  UART_Handle *iter;
  osList_t * head = &g_uartCmnDpmList;
  if ((mode == PSM_DEEP_SLEEP) && (osListLen(head) != 0))
  {
    osListForEach(pos, head)
    {
      iter = osListEntry(pos, UART_Handle, funcNode);

      i = prv_restoreRegs(iter, (save_addr + sum));
      sum = sum + i;
    }
  }
  return sum;
}

static PSM_DpmOps cmndpmops = {
    .PsmSuspendNoirq = UART_cmnSuspend,
    .PsmResumeNoirq = UART_cmnResume,
};

PSM_CMNDPM_INFO_DEFINE(uart_cmndpm, cmndpmops, OS_NULL, PSM_CMNDEV_UART1);

#ifdef _CPU_AP
#if 0
/**
 * @brief check  uart idle
 *
 */
static void UART_prvPsmCheck(PSM_Mode *mode, uint64_t *sleeptime)
{
    bool_t value = OS_FALSE;
    static PSM_Mode mode_pre = 0;

    value = prvIsIdle();
    if (value == OS_TRUE)
    {
      *mode = PSM_IDLE;
    }
    else
    {
      *mode = PSM_IGNORE;
    }
    *sleeptime = PSM_SLEEPTIME_MAX;
    if(mode_pre != *mode)
    {
      mode_pre = *mode;
      #if 0
      #ifdef _CPU_AP
      osPrintf("AP UART_prvSbyCheck mode %d\r\n", *mode);
      #else
      osPrintf("CP UART_prvSbyCheck mode %d\r\n", *mode);
      #endif
      #endif
    }

}

PSM_IDLE_CALLBACK_DEFINE(UART_prvPsmCheck, NULL, PSM_LEVEL_HIGH);
#endif

bool_t UART_CheckUartIdleSupport(void)
{
    bool_t ret = OS_TRUE;
    
    /**
     * UART_PORT_AT存在,需要判断是否符合条件进入IDLE睡眠
     * 1.正在自适应允许睡眠
     * 2.设置的波特率不符合条件不允许睡眠(>WAKEUP_MAX_BAUDERATE)
     *
     * UART_PORT_AT不存在,都可进入睡眠
     */
    UART_Handle *pUart = UART_findUart(UART_PORT_AT);
    UART_Handle *lUart = UART_findUart(UART_PORT_LOG);
    UART_Handle *cUart = UART_findUart(UART_PORT_CONSOLE);
    UART_Handle *rUart = UART_findUart(UART_PORT_USER);
    if(pUart != OS_NULL)
    {
        bool_t adaptDone = OS_FALSE;
        REG_Uart *reg = (REG_Uart *)(pUart->pRes->regBase);
        adaptDone = (READ_REG(reg->ABRSTA) & ABR_DONE) >> ABR_DONE_Pos ? OS_TRUE : OS_FALSE;

        if((pUart->info.flags & UART_FLAG_AUTO_BAUD))
        {
            // 正在自适应允许进入idle睡眠
            if(!adaptDone)
            {
                ret = OS_TRUE;
            }
        }
        
        // 设置的波特率不符合IDLE睡眠条件
        if(pUart->info.status.setBauderate > WAKEUP_MAX_BAUDERATE)
        {
            ret = OS_FALSE;
            return ret;
        }

    }
    if(lUart != OS_NULL)
    {
        REG_Uart *l_reg = (REG_Uart *)(lUart->pRes->regBase);
        uint32_t rx_countl = (READ_REG(l_reg->FR) & RXF_DATA_CNT) >> RXF_DATA_CNT_Pos;
        
        if(rx_countl > 0)
        {   
            ret = OS_FALSE;
            return ret;
        }
        else
        {
            ret = OS_TRUE;
        }
    }
    if(cUart != OS_NULL)
    {
        REG_Uart *c_reg = (REG_Uart *)(cUart->pRes->regBase);
        uint32_t rx_countc = (READ_REG(c_reg->FR) & RXF_DATA_CNT) >> RXF_DATA_CNT_Pos;

        if(rx_countc > 0)
        {
            ret = OS_FALSE;
            return ret;
        }
        else
        {
            ret = OS_TRUE;
        }

    }
    if(rUart != OS_NULL)
    {
        REG_Uart *r_reg = (REG_Uart *)(rUart->pRes->regBase);
        uint32_t rx_countr = (READ_REG(r_reg->FR) & RXF_DATA_CNT) >> RXF_DATA_CNT_Pos;

        if(rx_countr >0)
        {
            ret = OS_FALSE;
            return ret;
        }
        else
        {
            ret = OS_TRUE;
        }
    }

    return ret;
}

#endif
#endif

