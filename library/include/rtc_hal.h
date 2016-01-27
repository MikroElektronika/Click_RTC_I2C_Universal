/****************************************************************************
* Title                 :   RTC HAL
* Filename              :   rtc_hal.h
* Author                :   RBL
* Origin Date           :   08/12/2015
* Notes                 :   None
*****************************************************************************/
/**************************CHANGE LIST **************************************
*
*    Date    Software Version    Initials   Description
*  08/12/15         1              RBL     Interface Created.
*
*****************************************************************************/
/**
 * @file rtc_hal.h
 * @brief Abstraction layer for I2C access
 *
 * @date 08 Dec 2015
 * @author Richard Lowe
 * @copyright GNU Public License
 *
 * @version 1 - Initial testing and verification
 *
 * @note Test configuration:
 *   MCU:             STM32F107VC
 *   Dev.Board:       EasyMx Pro v7
 *   Oscillator:      72 Mhz internal
 *   Ext. Modules:    GPS Click
 *   SW:              ARM 4.5.2
 *
 */
#ifndef RTC_HAL_H_
#define RTC_HAL_H_

/******************************************************************************
* Includes
*******************************************************************************/
#include <stdint.h>

/******************************************************************************
* Preprocessor Constants
*******************************************************************************/


/******************************************************************************
* Configuration Constants
*******************************************************************************/


/******************************************************************************
* Macros
*******************************************************************************/


/******************************************************************************
* Typedefs
*******************************************************************************/


/******************************************************************************
* Variables
*******************************************************************************/


/******************************************************************************
* Function Prototypes
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the proper i2c slave address and sets appropriate
 * function pointers
 *
 * @param address_id - Desired i2c slave address
 */
void rtc_hal_init( uint8_t address_id );

/**
 * @brief Changes the i2c slave address
 *
 * @param address_id - Desired i2c slave address
 */
void rtc_hal_set_slave( uint8_t address_id );

/**
 * @brief Sends data through the i2c bus
 *
 * @param address[IN] - Desired slave register address to write to
 * @param data_in[IN] - Desired data to be written
 * @param num_bytes[IN] - Number of bytes to write
 */
void rtc_hal_write( uint8_t address, void *data_in, size_t num_bytes );

/**
 * @brief Reads data from slave through i2c
 *
 * @param address[IN] - Desired register address inside the i2c slave
 * @param data_out[OUT] - Buffer to store the read data to
 * @param num_bytes[IN] - Number of bytes to be read
 */
void rtc_hal_read ( uint8_t address, void *data_out, size_t num_bytes );

#ifdef __cplusplus
} // extern "C"
#endif

#endif /*RTC_HAL_H_*/

/*** End of File **************************************************************/
