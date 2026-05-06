/**
  ******************************************************************************
  * @file    pt8311_user_cfg.h
  * @brief   Header file for pt8311 driver user config
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _PT8311_USER_CFG_H_
#define _PT8311_USER_CFG_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include "slog_print.h"
#include "os.h"


/** @addtogroup PT8311_Driver
  * @{
  */


/* Exported Constants --------------------------------------------------------*/
/** @addtogroup PT8311_Exported_Constants
  * @{
  */

/** @defgroup PT8311_User_Config_Constants PT8311 User Config Constants
  * @brief    PT8311 User Config Constants
  * @{
  */

#define PT8311_MODULE_ENABLED               /*!< PT8311 Module Enable   */

/**
  * @brief PT8311 debug level macro definition
  */
#define PT8311_DBG_NONE            (0)      /*!< Debug None             */
#define PT8311_DBG_ERROR           (1)      /*!< Debug Error            */
#define PT8311_DBG_WARNING         (2)      /*!< Debug Warning          */
#define PT8311_DBG_INFO            (3)      /*!< Debug Information      */
#define PT8311_DBG_LOG             (4)      /*!< Debug Log              */

/**
  * @brief PT8311 debug level user select
  * @note The debug information which is less than or equal to this debug level will be printed
  */
#define PT8311_DBG_LVL             (PT8311_DBG_LOG)

/**
  * @}
  */

/**
  * @}
  */


#ifdef PT8311_MODULE_ENABLED

/* Exported Macros -----------------------------------------------------------*/
/** @addtogroup PT8311_Exported_Macros
  * @{
  */

/** @defgroup PT8311_User_Config_Macros PT8311 User Config Macros
  * @brief    PT8311 User Config Macros
  * @{
  */

#ifndef PT8311_USER_DBG_LOG_ENABLE
#   define PT8311_USER_DBG_LOG_ENABLE    1
#endif
 
#if PT8311_USER_DBG_LOG_ENABLE
#   define PT8311_USER_DBG_LOG(format, ...) \
        slogPrintf(SLOG_LEVEL_INFO, SLOG_PRINT_SUBMDL_APP, "[pt8311][%s:%d] " format "\r\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#   define PT8311_USER_DBG_LOG(format, ...)   ((void)0)
#endif

/**
  * @brief PT8311 debug print user macro definition
  */
#define PT8311_DBG_PRINT_USER(fmt, ...)             PT8311_USER_DBG_LOG(fmt, ##__VA_ARGS__)


/**
  * @brief PT8311 delay ms user macro definition
  */
#define PT8311_DELAY_MS_USER(ms)                    pt8311_delay_ms(ms)

/**
  * @brief  PT8311 I2C read byte function macro definition
  * @param  i2c_addr PT8311 I2C address
  * @param  reg PT8311 register address to read
  * @return register read value
  */
#define PT8311_I2C_READ_BYTE(i2c_addr, reg)         pt8311_i2c_read_byte(i2c_addr, reg)

/**
  * @brief  PT8311 I2C write byte function macro definition
  * @param  i2c_addr PT8311 I2C address
  * @param  reg PT8311 register address to write
  * @param  val register value
  * @retval 0      Register write Success
  * @retval others Register write Failed
  */
#define PT8311_I2C_WRITE_BYTE(i2c_addr, reg, val)   pt8311_i2c_write_byte(i2c_addr, reg, val)

/**
  * @}
  */

/**
  * @}
  */


/* Exported Types ------------------------------------------------------------*/
/** @addtogroup PT8311_Exported_Types
  * @{
  */

/** @defgroup PT8311_User_Config_Types PT8311 User Config Types
  * @brief    PT8311 User Config Types
  * @{
  */

// 项目中已包含stdint.h，注释掉typedef定义
#if 0
/**
  * @brief PT8311 data type definition
  */
typedef   signed       char int8_t;         /*!< int8_t typedef         */
typedef   signed short  int int16_t;        /*!< int16_t typedef        */
typedef   signed        int int32_t;        /*!< int32_t typedef        */
typedef unsigned       char uint8_t;        /*!< uint8_t typedef        */
typedef unsigned short  int uint16_t;       /*!< uint16_t typedef       */
typedef unsigned        int uint32_t;       /*!< uint32_t typedef       */
#endif

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

/** @defgroup PT8311_User_Config_Functions PT8311 User Config Functions
  * @brief    PT8311 User Config Functions
  * @{
  */

/**
  * @brief  PT8311 delay ms
  * @param  ms millisecond
  */
void pt8311_delay_ms(int ms);

/**
  * @brief  PT8311 I2C read byte
  * @param  i2c_addr PT8311 I2C address
  * @param  reg PT8311 register address to read
  * @return register read value
  */
unsigned char pt8311_i2c_read_byte(unsigned char i2c_addr, unsigned char reg);

/**
  * @brief  PT8311 I2C write byte
  * @param  i2c_addr PT8311 I2C address
  * @param  reg PT8311 register address to write
  * @param  val register value
  * @retval 0      Register write Success
  * @retval others Register write Failed
  */
signed char pt8311_i2c_write_byte(unsigned char i2c_addr, unsigned char reg, unsigned char val);

/**
  * @brief  PT8311 I2C config
  * @param  busNo I2C bus number
  */
int pt8311_i2c_config(uint8_t busNo);

/**
  * @brief  PT8311 I2C deconfig
  * @param  busNo I2C bus number
  */
void pt8311_i2c_deconfig(uint8_t busNo);

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

#endif /* _PT8311_USER_CFG_H_ */

/***********END OF FILE***********/