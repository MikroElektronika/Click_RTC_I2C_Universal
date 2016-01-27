/****************************************************************************
* Title                 :   RTC Library
* Filename              :   rtc.h
* Author                :   RBL
* Origin Date           :   08/12/2015
* Notes                 :   A multi-chip library, functions are generic.
*****************************************************************************/
/**************************CHANGE LIST **************************************
*
*    Date    Software Version   Initials      Description
*  08/12/15         .1              RBL     Interface Created.
*
*****************************************************************************/
/**
 * @file rtc.h
 * @brief Generic RTC library that supports multiple manufacturers chips.
 *
 * @date 08 Dec 2015
 * @author Richard Lowe
 * @copyright GNU Public License
 *
 * @version .1 - Initial testing and verification
 *
 * @note Test configuration:
 *   MCU:             STM32F107VC
 *   Dev.Board:       EasyMx Pro v7
 *   Oscillator:      72 Mhz internal
 *   Ext. Modules:    GPS Click
 *   SW:              ARM 4.5.2
 *
 */

/**
 * @mainpage
 *  <h1>RTC Library</h1>
 * @par Introduction
 *
 * This library provides functions for working with all RTC clicks which
 * use i2c communication. Functions include getting and setting GMT time,
 * as well as local time. Calculating UNIX timestamp for both local and GMT
 * time. Setting square wave output, checking for power failure, and many more.
 * NOTE: Not all functions are provided for all RTCs. As some RTCs do not
 * support square wave output or alarms.
 *
 * @par Example on STM :
 * @ref example_arm
 *
 * @par Example on PIC :
 * @ref example_pic
 *
 *
 * @par Example on FT90x :
 * @ref example_ft90
 *
 *
 * @note You will need to install this library via the
 * <a href="http://www.mikroe.com/package-manager/">Package Manager</a> to use
 *  it.
 */

#ifndef RTC_H_
#define RTC_H_

/******************************************************************************
* Includes
*******************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "rtc_hal.h"

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
/**
 * @enum Supported Device Types
 */
typedef enum
{
    RTC_PCF8583,   /**< RTC1 Click module */
    RTC2_DS1307,   /**< RTC2 Click module */
    RTC3_BQ32000,  /**< RTC3 Click module */
    RTC6_MCP7941X  /**< RTC6 Click module */
} rtc_type_t;

/**
 * @struct Time definition of time elements
 *
 */
typedef struct
{
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t weekday;
    uint8_t monthday;
    uint8_t month;
    uint8_t year;
} rtc_time_t;

/**
 * @enum Output Modes
 */
typedef enum
{
    RTC_1HZ,      /**< 1 HZ Square wave output*/
    RTC_4_096KHZ, /**< 4096 HZ Square wave output*/
    RTC_8_192KHZ, /**< 8192 HZ Square wave output*/
    RTC_32_768KHZ /**< 32768 HZ Square wave output*/
} rtc_swo_t;

/**
 * @enum Alarm Selection
 *
 * @note Some models do not support alarms
 */
typedef enum
{
    RTC_ALARM_0,
    RTC_ALARM_1,
} rtc_alarm_t;

/**
 * @enum Alarm Trigger Time Settings
 */
typedef enum
{
    RTC_ALARM_SECONDS, /**< Alarm triggers when seconds match*/
    RTC_ALARM_MINUTES, /**< Alarm triggers when minutes match*/
    RTC_ALARM_HOURS,   /**< Alarm triggers when hours match*/
    RTC_ALARM_DAY,     /**< Alarm triggers when days match*/
    RTC_ALARM_DATE,    /**< Alarm triggers when date matches*/
    RTC_ALARM_SEC_MIN_HOUR_DAY_DATE_MONTH, /**< Alarm triggers when all above matches*/
    RTC_ALARM_WEEKDAY /**< Alarm triggers when weekday matches*/
} rtc_alarm_trigger_t;

/**
 * @enum Month Definitions
 */
typedef enum
{
    JANUARY = 1,
    FEBRUARY,
    MARCH,
    APRIL,
    MAY,
    JUNE,
    JULY,
    AUGUST,
    SEPTEMBER,
    OCTOBER,
    NOVEMBER,
    DECEMBER
} rtc_month_t;

/**
 * @enum Day of Week Definition
 */
typedef enum
{
    MONDAY = 1,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY,
    SUNDAY,
} rtc_day_t;

/******************************************************************************
* Variables
*******************************************************************************/


/******************************************************************************
* Function Prototypes
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


/****************************************
 ********* RTC Settings *****************
 ***************************************/
/**
 * @brief Initializes RTC based on type and time zone
 *
 * @param rtc_type_t type - type of RTC supported
 * @param time_zone - gmt offset of the zone
 *
 * @return
 *  @retval 0 - successful
 *  @retval -1 - error occured
 *
 * @code
 * if( rtc_init( RTC2_DS1307, -8 ) )
 *     return error;
 * @endcode
 */
int rtc_init( rtc_type_t type, int8_t time_zone );

/**
 * @brief Enables Daylight Savings Time compensation
 *
 */
void rtc_enable_dst( void );

/**
 * @brief Disables Daylight Savings Time compensation
 *
 */
void rtc_disable_dst( void );

/**
 * @brief Enable Square Wave Output
 *
 * @param swo - Frequency of output
 *
 * @code
 * rtc_enable_swo( RTC_1HZ );
 * @endcode
 */
void rtc_enable_swo( rtc_swo_t swo );

/**
 * @brief Disables Square Wave Output
 *
 * @note Not supported by all models
 */
void rtc_disable_swo( void );

/**
 * @brief Enables automatic switch to battery on VCC failure
 *
 * @note Not supported by all models
 */
void rtc_battery_enable( void );

/**
 * @brief Disables automatic switch to battery on VCC failure
 *
 * @note Not supported by all models
 */
void rtc_battery_disable( void );


/****************************************
 ********* General Usage ****************
 ***************************************/
/**
 * @brief Gets the current gmt time set in the RTC
 *
 * @return Returns gmt time
 */
rtc_time_t *rtc_get_gmt_time( void );

/**
 * @brief Calculates the current local time
 *
 * @return Returns local time
 */
rtc_time_t *rtc_get_local_time( void );

/**
 * @brief Sets the desired gmt time in the RTC
 *
 * @param time - desired gmt time to be set
 * @return int
 * @retval -1 failed
 * @retval  0 successful
 */
int rtc_set_gmt_time( rtc_time_t time );

/**
 * @brief Sets desired local time
 *
 * @param time - desired local time to be set
 * @retval -1 failed
 * @retval  0 successful
 */
int rtc_set_local_time( rtc_time_t time );

/**
 * @brief Calculates the current gmt time in UNIX epoch time
 *
 * @return uint32_t
 * @retval gmt time converted to UNIX epoch time
 */
uint32_t rtc_get_gmt_unix_time( void );

/**
 * @brief Calculates the current local time in UNIX epoch time
 * @return uint32_t
 * @retval Returns local time converted to UNIX epoch time
 */
uint32_t rtc_get_local_unix_time( void );

/**
 * @brief Checks if the current year is a leap one
 *
 * @return bool
 * @retval true on a leap year
 * @retval false if not a leap year
 */
bool rtc_is_leap_year( void );

/**
 * @brief Checks if a power failure has occured
 *
 * @return bool
 * @retval true if a power failure detected
 * @retval false if no power failure was detected
 *
 * @note Not supported by all models
 */
bool rtc_is_power_failure( void );

/**
 * @brief Gets the time stamp of the last power failure
 *
 * @return rtc_time_t - time stamp of the power failure
 *
 * @note Not supported by all models
 */
rtc_time_t *rtc_get_last_power_failure( void );

/****************************************
 ********* Alarms ***********************
 ***************************************/
/**
 * @brief Sets the desired alarm
 *
 * @param alarm[IN] - Desired alarm to be set (alarm 0, alarm 1...)
 * @param trigger[IN] - Desired trigger for the alarm
 * @param time[IN] - Desired alarm time to be set
 *
 * @note Not supported by all models
 */
void rtc_set_alarm( rtc_alarm_t alarm, rtc_alarm_trigger_t trigger,
                    rtc_time_t time );

/**
 * @brief Disables the alarm
 *
 * @param alarm - Desired alarm to be disabled (alarm 0, alarm 1...)
 *
 * @note Not supported by all models
 */
void rtc_disable_alarm( rtc_alarm_t alarm );

/**
 * @brief Reads the alarm time
 *
 * @param alarm - Desired alarm to be read (alarm 0, alarm 1)
 * @return rtc_time_t - alarm time
 *
 * @note Not supported by all models
 */
rtc_time_t *rtc_read_alarm( rtc_alarm_t alarm );


/****************************************
 ********* Memory ***********************
 ***************************************/
/**
 * @brief Writes data to the SRAM registers of the RTC
 *
 * @param addr[IN]  - Address of the desired register
 * @param data_in[IN]  - Data to be written
 *
 * Valid Addresses: 
 * RTC2 0x00 to 0x38
 * RTC6 0x00 to 0x40
 *
 * @note Not supported by all models
 */
void rtc_write_sram( uint8_t addr, uint8_t data_in );

/**
 * @brief Writes several bytes to SRAM registers of the RTC
 *
 * @param addr[IN] - Address from where to start the writing
 * @param data_in[IN] - Data to be written to SRAM
 * @param data_size[IN] - Number of bytes to be sent
 *
 * @note Not supported by all models
 */
void rtc_write_sram_bulk( uint8_t addr, void *data_in, size_t data_size );

/**
 * @brief Reads data from an SRAM register
 *
 * @param addr - Register address
 * @return uint8_t - read byte
 *
 * @note Not supported by all models
 */
uint8_t rtc_read_sram( uint8_t addr );

/**
 * @brief Reads several bytes from SRAM registers
 *
 * @param addr[IN] - Address from where to start the reading
 * @param data_out[OUT] - Pointer to the variable which will hold the read bytes
 * @param data_size[IN] - Number of bytes to be read
 *
 * @note Not supported by all models
 */
void rtc_read_sram_bulk( uint8_t addr, void *data_out, uint8_t data_size );


void rtc_eeprom_write_protect_on( void );

void rtc_eeprom_write_protect_off( void );

bool rtc_eeprom_is_locked( void );

/**
 * @brief Writes data to EEPROM registers of RTC
 *
 * @param addr[IN] - Address from where to start writing
 * @param data_in[IN] - Data to be written
 * @param data_size[IN]  - Number of bytes to be written
 *
 * @return bool
 * @retval true - writing was successful
 * @retval false - not successful
 *
 * @note Not supported by all models
 */
bool rtc_write_eeprom( uint8_t addr, void *data_in, uint8_t data_size );

/**
 * @brief Reads data from EEPROM registers of the RTC
 *
 * @param addr[IN] - Address from where to start reading
 * @param data_out[OUT] - Variable which will hold the read data
 * @param data_size[IN] - Number of bytes to be read
 *
 * @note Not supported by all models
 */
void rtc_read_eeprom( uint8_t addr, void *data_out, uint8_t data_size );

/**
 * @brief Reads unique ID from EEPROM registers of the RTC
 *
 * @returns uint8_t* - 8 bytes
 *
 * @note Not supported by all models
 */
uint8_t *rtc_read_unique_id( void );

/**
 * @brief Writes unique ID to EEPROM registers
 *
 * @param uint8_t * id - array of 8 bytes
 *
 * @note Not supported by all models
 */
void rtc_write_unique_id( uint8_t *id );

#ifdef __cplusplus
} // extern "C"
#endif

#endif /*File_H_*/

/*** End of File **************************************************************/

/**
 * @page example_ft90 Example on FT90x
 * @par Example created using the EasyFT90x v7
 * @par FT900
 * @code
 *
 * #include "haptic_hw.h"
 *
 * #include "rtc.h"
 * #include <stdint.h>
 * #include <built_in.h>
 *
 * void Auxiliary_Code(void);
 * void Set_Index(unsigned short index);
 * void Write_Command(unsigned short cmd);
 * void Write_Data(unsigned int _data);
 *
 * rtc_time_t time_test;
 * rtc_time_t *time_m;
 * rtc_time_t *local_time;
 * char txt[10];
 *
 * char TFT_DataPort at GPIO_PORT_32_39;
 * sbit TFT_RST at GPIO_PIN42_bit;
 * sbit TFT_RS at GPIO_PIN40_bit;
 * sbit TFT_CS at GPIO_PIN41_bit;
 * sbit TFT_RD at GPIO_PIN46_bit;
 * sbit TFT_WR at GPIO_PIN47_bit;
 * sbit TFT_BLED at GPIO_PIN58_bit;
 *
 * void Auxiliary_Code() {
 *  TFT_WR = 0;
 *  asm nop;
 *  TFT_WR = 1;
 * }
 *
 * void Set_Index(unsigned short index) {
 *  TFT_RS = 0;
 *  asm nop;
 *  TFT_DataPort = index;
 *  asm nop; asm nop;
 *  Auxiliary_Code();
 * }
 *
 * void Write_Command(unsigned short cmd) {
 *  TFT_RS = 1;
 *  asm nop;
 *  TFT_DataPort = cmd;
 *  asm nop; asm nop;
 *  Auxiliary_Code();
 * }
 *
 * void Write_Data(unsigned int _data) {
 *   TFT_RS = 1;
 *   asm nop;
 *   TFT_DataPort = Hi(_data);
 *   asm nop; asm nop;
 *   Auxiliary_Code();
 *   asm nop;
 *   TFT_DataPort = Lo(_data);
 *   asm nop; asm nop;
 *   Auxiliary_Code();
 * }
 *
 * static void DisplayInit()
 * {
 *     TFT_Set_Active(Set_Index, Write_Command, Write_Data);
 *     TFT_Init_ILI9341_8bit(320, 240);
 *     TFT_BLED = 1;
 *     TFT_FILL_SCREEN(CL_AQUA);
 *     TFT_SET_FONT(TFT_defaultFont,CL_BLACK, FO_HORIZONTAL);
 *     tft_write_text("GMT Time", 100, 10);
 *     tft_write_text("Local Time", 200, 10);
 *     tft_write_text("seconds", 10,40);
 *     tft_write_text("minutes", 10,60);
 *     tft_write_text("hours", 10,80);
 *     tft_write_text("monthday", 10,100);
 *     tft_write_text("month", 10,120);
 *     Tft_write_text("year", 10,140);
 *     tft_write_text("Current time zone : GMT - 2", 10,200);
 *     tft_set_pen(CL_AQUA,1);
 *     TFT_SET_BRUSH(1,CL_AQUA, 0, 0, 0,0);
 *
 * }
 *
 * display_values()
 * {
 *   TFT_RECTANGLE(100,40, 240,180);
 *   inttostr(time_m->seconds, txt);
 *   tft_write_text(txt, 100,40);
 *   inttostr(time_m->minutes, txt);
 *   tft_write_text(txt, 100,60);
 *   inttostr(time_m->hours, txt);
 *   tft_write_text(txt, 100,80);
 *   inttostr(time_m->monthday, txt);
 *   tft_write_text(txt, 100,100);
 *   inttostr(time_m->month, txt);
 *   tft_write_text(txt, 100,120);
 *   inttostr(time_m->year, txt);
 *   tft_write_text(txt, 100,140);
 *
 *
 *   inttostr(local_time->seconds, txt);
 *   tft_write_text(txt, 200,40);
 *   inttostr(local_time->minutes, txt);
 *   tft_write_text(txt, 200,60);
 *   inttostr(local_time->hours, txt);
 *   tft_write_text(txt, 200,80);
 *   inttostr(local_time->monthday, txt);
 *   tft_write_text(txt, 200,100);
 *   inttostr(local_time->month, txt);
 *   tft_write_text(txt, 200,120);
 *   inttostr(local_time->year, txt);
 *   tft_write_text(txt, 200,140);
 * }
 *
 *
 *
 * void main()
 * {
 *
 *      time_test.seconds = 0;
 *      time_test.minutes = 15;
 *      time_test.hours = 15;
 *      time_test.weekday = 4;
 *      time_test.monthday = 1;
 *      time_test.month = 1;
 *      time_test.year = 15;
 *       displayinit();
 *
 *      I2CM1_Init( _I2CM_SPEED_MODE_STANDARD, _I2CM_SWAP_DISABLE );
 *      Delay_ms( 100 );
 *      rtc_init( RTC6_MCP7941X, -2 );
 *
 *      rtc_set_gmt_time(time_test);
 *      time_m = rtc_get_gmt_time();
 *
 *
 *
 *
 *      while( 1 )
 *      {
 *         Delay_ms( 200 );
 *         time_m = rtc_get_gmt_time();
 *         local_time = rtc_get_local_time();
 *         display_values();
 *      }
 * }
 * @endcode
 */

/**
 * @page example_pic Example on PIC
 * @par Example created using the EasyPIC v7 Connectivity
 * @par PIC18F87K22
 * @code
 * #include <stdint.h>
 * #include "rtc.h"
 *
 * // Lcd module connections
 * sbit LCD_RS at LATB4_bit;
 * sbit LCD_EN at LATB5_bit;
 * sbit LCD_D4 at LATB0_bit;
 * sbit LCD_D5 at LATB1_bit;
 * sbit LCD_D6 at LATB2_bit;
 * sbit LCD_D7 at LATB3_bit;
 *
 * sbit LCD_RS_Direction at TRISB4_bit;
 * sbit LCD_EN_Direction at TRISB5_bit;
 * sbit LCD_D4_Direction at TRISB0_bit;
 * sbit LCD_D5_Direction at TRISB1_bit;
 * sbit LCD_D6_Direction at TRISB2_bit;
 * sbit LCD_D7_Direction at TRISB3_bit;
 * // End Lcd module connections
 *
 * sbit LCD_BCK at LATG3_bit;
 * sbit LCD_BCK_Direction at TRISG3_bit;
 *
 *
 * rtc_time_t time_test;
 * rtc_time_t *local_time;
 * rtc_time_t *gmt_time;
 * char txt[10];
 *
 * static void DisplayInit()
 * {
 *   LCD_BCK_Direction = 0;
 *   LCD_BCK = 1;
 *   Lcd_Init();                        // Initialize Lcd
 *   Lcd_Cmd(_LCD_CLEAR);               // Clear display
 *   Lcd_Cmd(_LCD_CURSOR_OFF);          // Cursor off
 * }
 *
 * display_values()
 * {
 *   Lcd_Chr(1, 1, (gmt_time->seconds/10)+48);
 *   Lcd_Chr(1, 2, (gmt_time->seconds%10)+48);
 *   Lcd_Chr(1, 4, (gmt_time->minutes/10)+48);
 *   Lcd_Chr(1, 5, (gmt_time->minutes%10)+48);
 *   Lcd_Chr(1, 7, (gmt_time->hours/10)+48);
 *   Lcd_Chr(1, 8, (gmt_time->hours%10)+48);
 *   Lcd_Chr(1, 10, (gmt_time->monthday%10)+48);
 *   Lcd_Chr(1, 12, (gmt_time->month%10)+48);
 *   Lcd_Chr(1, 15, (gmt_time->year%10)+48);
 *   Lcd_Chr(1, 14, (gmt_time->year/10)+48);
 *
 *   Lcd_Chr(2, 1, (local_time->seconds/10)+48);
 *   Lcd_Chr(2, 2, (local_time->seconds%10)+48);
 *   Lcd_Chr(2, 4, (local_time->minutes/10)+48);
 *   Lcd_Chr(2, 5, (local_time->minutes%10)+48);
 *   Lcd_Chr(2, 7, (local_time->hours/10)+48);
 *   Lcd_Chr(2, 8, (local_time->hours%10)+48);
 *   Lcd_Chr(2, 10, (local_time->monthday%10)+48);
 *   Lcd_Chr(2, 12, (local_time->month%10)+48);
 *   Lcd_Chr(2, 15, (local_time->year%10)+48);
 *   Lcd_Chr(2, 14, (local_time->year/10)+48);
 *
 *
 *
 * }
 *
 * void main() {
 *
 *      ANCON0 = 0;                        // Configure PORTB pins as digital
 *      ANCON1 = 0;
 *      ANCON2 = 0;
 *
 *      DisplayInit();
 *      delay_ms(1000);
 *      Lcd_Cmd(_LCD_CLEAR);
 *      time_test.seconds = 0;
 *      time_test.minutes = 15;
 *      time_test.hours = 15;
 *      time_test.weekday = 0;
 *      time_test.monthday = 1;
 *      time_test.month = 1;
 *      time_test.year = 15;
 *
 *      I2C1_Init(100000);
 *      rtc_init( RTC6_MCP7941X, -2 );
 *      delay_ms(1000);
 *      rtc_set_gmt_time(time_test);
 *
 *
 *    while(1)
 *    {
 *       Delay_ms( 200 );
 *       gmt_time = rtc_get_gmt_time();
 *       local_time = rtc_get_local_time();
 *       display_values();
 *    }
 *
 * }
 *
 *
 *
 * @endcode
 */

/**
 * @page example_arm Example on ARM
 * @par Example created using the EasyMX Pro for STM32
 * @par STM32F107
 * @code
 *
 * #include "rtc.h"
 *
 *
 *
 * // TFT module connections
 * unsigned int TFT_DataPort at GPIOE_ODR;
 * sbit TFT_RST at GPIOE_ODR.B8;
 * sbit TFT_RS at GPIOE_ODR.B12;
 * sbit TFT_CS at GPIOE_ODR.B15;
 * sbit TFT_RD at GPIOE_ODR.B10;
 * sbit TFT_WR at GPIOE_ODR.B11;
 * sbit TFT_BLED at GPIOE_ODR.B9;
 * // End TFT module connections
 *
 *
 * rtc_time_t time_test;
 * rtc_time_t *time;
 * rtc_time_t *local_time;
 * char txt[10];
 *
 * static void DisplayInit()
 * {
 *
 *     TFT_Init_ILI9341_8bit(320, 240);
 *     TFT_BLED = 1;
 *     TFT_FILL_SCREEN(CL_AQUA);
 *     TFT_SET_FONT(TFT_defaultFont,CL_BLACK, FO_HORIZONTAL);
 *     tft_write_text("GMT Time", 100, 10);
 *     tft_write_text("Local Time", 200, 10);
 *     tft_write_text("seconds", 10,40);
 *     tft_write_text("minutes", 10,60);
 *     tft_write_text("hours", 10,80);
 *     tft_write_text("monthday", 10,100);
 *     tft_write_text("month", 10,120);
 *     Tft_write_text("year", 10,140);
 *     tft_write_text("Current time zone : GMT - 2", 10,200);
 *     tft_set_pen(CL_AQUA,1);
 *     TFT_SET_BRUSH(1,CL_AQUA, 0, 0, 0,0);
 *
 * }
 *
 * display_values()
 * {
 *   TFT_RECTANGLE(100,40, 240,180);
 *   inttostr(time->seconds, txt);
 *   tft_write_text(txt, 100,40);
 *   inttostr(time->minutes, txt);
 *   tft_write_text(txt, 100,60);
 *   inttostr(time->hours, txt);
 *   tft_write_text(txt, 100,80);
 *   inttostr(time->monthday, txt);
 *   tft_write_text(txt, 100,100);
 *   inttostr(time->month, txt);
 *   tft_write_text(txt, 100,120);
 *   inttostr(time->year, txt);
 *   tft_write_text(txt, 100,140);
 *
 *
 *   inttostr(local_time->seconds, txt);
 *   tft_write_text(txt, 200,40);
 *   inttostr(local_time->minutes, txt);
 *   tft_write_text(txt, 200,60);
 *   inttostr(local_time->hours, txt);
 *   tft_write_text(txt, 200,80);
 *   inttostr(local_time->monthday, txt);
 *   tft_write_text(txt, 200,100);
 *   inttostr(local_time->month, txt);
 *   tft_write_text(txt, 200,120);
 *   inttostr(local_time->year, txt);
 *   tft_write_text(txt, 200,140);
 * }
 *
 *
 * void main() {
 *
 *    DisplayInit();
 *
 *   GPIO_Digital_Input(&GPIOD_BASE, _GPIO_PINMASK_10);
 *
 *
 *   time_test.seconds = 0;
 *   time_test.minutes = 15;
 *   time_test.hours = 15;
 *   time_test.weekday = 0;
 *   time_test.monthday = 1;
 *   time_test.month = 1;
 *   time_test.year = 15;
 *
 *
 *   I2C1_Init_Advanced( 100000, &_GPIO_MODULE_I2C1_PB67 );
 *   rtc_init( RTC6_MCP7941X, -1 );
 *   delay_ms(1000);
 *   rtc_set_gmt_time(time_test);
 *
 *
 *
 *   while(1)                             // Infinite loop
 *   {       Delay_ms( 200 );
 *           time = rtc_get_gmt_time();
 *           local_time = rtc_get_local_time();
 *           display_values();
 *   }
 *
 * }
 *
 * @endcode
 */