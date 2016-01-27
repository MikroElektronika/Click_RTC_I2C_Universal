/*******************************************************************************
* Title                 :   RTC Implementation
* Filename              :   rtc.c
* Author                :   RBL
* Origin Date           :   08/12/2015
* Notes                 :   None
*******************************************************************************/
/*************** MODULE REVISION LOG ******************************************
*
*    Date    Software Version    Initials   Description
*  08/12/15         .1              RBL      Module Created.
*
*******************************************************************************/
/**
 * @file rtc.c
 * @brief This module contains the
 */
/******************************************************************************
* Includes
*******************************************************************************/
#include "rtc.h"
#include "rtc_hal.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/
/**
 * @def Common Registry
 */
#define RTC_SECONDS_ADDR   0x00
#define RTC_SECONDS_MASK( value ) ( value & 0x7f )
#define RTC_SECONDS_CLEAR( value ) ( value & 0x80 )

#define RTC_MINUTES_ADDR   0x01
#define RTC_MINUTES_MASK( value ) ( value & 0x7f )
#define RTC_MINUTES_CLEAR( value ) ( value & 0x80 )

#define RTC_HOURS_ADDR     0x02
#define RTC_HOURS_MASK( value ) ( value & 0x3f )
#define RTC_HOURS_CLEAR( value ) ( value & 0xc0 )

#define RTC_DAY_ADDR       0x03
#define RTC_DAY_MASK( value ) ( value & 0x07 )
#define RTC_DAY_CLEAR( value ) ( value & 0xf8 )

#define RTC_DATE_ADDR      0x04
#define RTC_DATE_MASK( value ) ( value & 0x3f )
#define RTC_DATE_CLEAR( value ) ( value & 0xc0 )

#define RTC_MONTH_ADDR     0x05
#define RTC_MONTH_MASK( value ) ( value & 0x1f )
#define RTC_MONTH_CLEAR( value ) ( value & 0xe0 )

#define RTC_YEAR_ADDR      0x06
#define RTC_YEAR_MASK( value ) ( value & 0xff )
#define RTC_YEAR_CLEAR( value ) ( value & 0x00 )

#define RTC_CONFIG_ADDR    0x07

#define RTC_START_OSC_BIT  7
#define RTC_START_OSC_MASK ( 1 << RTC_START_OSC_BIT )

#define RTC_TIMEDATE_BYTES 7
#define RTC_SECONDS_BYTE   0
#define RTC_MINUTES_BYTE   1
#define RTC_HOUR_BYTE      2
#define RTC_DAY_BYTE       3
#define RTC_DATE_BYTE      4
#define RTC_MONTH_BYTE     5
#define RTC_YEAR_BYTE      6

/**
  * @def RTC Slave Addresses
  */
#define RTC_PCF8583_SLAVE           0x50

#define RTC2_DS1307_SLAVE           0x68
#define RTC2_RAM_SIZE               56
#define RTC2_RAM_START              0x08
#define RTC2_RAM_END                0x3F

#define RTC3_BQ32000_SLAVE          0x68

#define RTC6_MCP7941X_SLAVE         0x6F
#define RTC6_MCP7941X_SRAM_SLAVE    0xDE
#define RTC6_MCP7941X_EEPROM_SLAVE  0xAE
#define RTC6_RAM_SIZE               64
#define RTC6_RAM_START              0x20
#define RTC6_RAM_END                0x5f
#define RTC6_EEPROM_SIZE            128 * 8
#define RTC6_EEPROM_START           0
#define RTC6_EEPROM_END             RTC6_EEPROM_SIZE
#define RTC6_EEPROM_PAGE_SIZE       8
#define RTC6_EEPROM_STATUS          0xFF

#define JULIAN_DAY_1970 2440588 // julian day calculation for jan 1 1970
#define TIME_SEC_IN_MIN             60                     // seconds per minute
#define TIME_SEC_IN_HOUR            (TIME_SEC_IN_MIN * 60) // seconds per hour
#define TIME_SEC_IN_24_HOURS        (TIME_SEC_IN_HOUR * 24)// seconds per day

/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/
#define BCD2BIN(val) ( ( ( val ) & 15 ) + ( ( val ) >> 4 ) * 10 )
#define BIN2BCD(val) ( ( ( ( val ) / 10 ) << 4 ) + ( val ) % 10 )

/******************************************************************************
* Module Typedefs
*******************************************************************************/

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
static rtc_time_t current_local_time;
static rtc_type_t current_type;
static int8_t     current_time_zone;
static bool       dst_enabled;


/******************************************************************************
* Function Prototypes
*******************************************************************************/
static void get_dst( uint8_t year, rtc_time_t *start, rtc_time_t *end );
static long time_date_to_epoch( rtc_time_t *ts );
static void time_epoch_to_date( long e, rtc_time_t *ts );
/******************************************************************************
* Function Definitions
*******************************************************************************/
static void time_epoch_to_date( long e, rtc_time_t *ts )
{
    /*
     * WARNING :
     * due to PIC16 optimizer problems, this variables have to be volatile
     */
    long julian_day; // julian day

    long J;
    long K;
    long L;
    long N;
    long NN;
    long I;

    /*
     * julian day conversion
     */
    julian_day = e;
    julian_day /= TIME_SEC_IN_24_HOURS;
    julian_day += JULIAN_DAY_1970;

    if( e < 0 )
        julian_day--;

    ts->weekday = julian_day % 7;       // get day in week number

    /*
     * conversion to gregorian calendar
     */
    L = julian_day + 68569;
    N = 4 * L;
    N /= 146097;
    NN = 146097 * N;
    NN += 3;
    NN /= 4;
    L -= NN;
    I = 4000 * ( L + 1 );
    I /= 1461001;
    NN = 1461 * I;
    NN /= 4;
    NN -= 31;
    L -= NN;
    J = 80 * L;
    J /= 2447;
    K = L - ( 2447 * J ) / 80;
    L = J / 11;
    J = J + 2 - 12 * L;
    I = ( 100 * ( N - 49 ) + I + L ) - 2000;

    /*
     * now we have the date
     */
    ts->year = I;
    ts->month = J;
    ts->monthday = K;

    /*
     * compute time
     */
    if( e < 0 )
    {
        /*
         * if epoch is before 1970,
         * extract correct hours, minutes, seconds
         */
        e *= -1;
        e %= TIME_SEC_IN_24_HOURS;
        e = TIME_SEC_IN_24_HOURS - e;
    }

    L = e % TIME_SEC_IN_24_HOURS;
    K = L / TIME_SEC_IN_HOUR;
    ts->hours = K;

    K = L;
    K -= ( long )ts->hours * TIME_SEC_IN_HOUR;
    K /= TIME_SEC_IN_MIN;
    ts->minutes = K;

    K = L % TIME_SEC_IN_MIN;
    ts->seconds = K;
    ts->weekday++;
}


/*
 * USA starting in 2007, extended DST to begin
 * on the 2nd Sunday in March (Day of month = 14 - (1+Y*5/4)mod 7) and
 * end on the 1st Sunday in November (Day of month = 7 - (1 + 5*Y/4) mod 7).
 */

static void get_dst( uint8_t year, rtc_time_t *start, rtc_time_t *end )
{
    start->seconds = 0;
    start->minutes = 0;
    start->hours = 2;
    start->weekday = SUNDAY;
    start->monthday = 14 - ( ( 1 + ( ( year * 5 ) / 4 ) ) % 7 );
    start->month = MARCH;
    start->year = year;

    end->seconds  = 0;
    end->minutes  = 0;
    end->hours  = 2;
    end->weekday  = SUNDAY;
    end->monthday  = 7 - ( ( 1 + ( ( year * 5 ) / 4 ) ) % 7 );
    end->month  = NOVEMBER;
    end->year  = year;
}

static long time_date_to_epoch( rtc_time_t *ts )
{
    /*
           * WARNING :
           * due to PIC16 optimizer problems, this variables have to be volatile
           */
    long   ji;
    long   julian_day;
    unsigned long   sec;
    long   e;

    /*
     * julian day calculation
     */
    ji = ts->year + 2000 + 4800;
    ji *= 12;
    ji += ts->month - 3;

    julian_day = ( 2 * ( ji % 12 ) + 7 + 365 * ji ) / 12;
    julian_day += ts->monthday + ( ji / 48 ) - 32083;
    if( julian_day > 2299171L )
        julian_day += ji / 4800 - ji / 1200 + 38;
    ts->weekday = ( julian_day % 7 );            // and now the day of the week

    /*
     * number of seconds elapsed since the beginning of the julian day
     */
    sec = ts->hours;
    sec *= TIME_SEC_IN_HOUR;
    sec += ts->minutes * TIME_SEC_IN_MIN;
    sec += ts->seconds;

    /*
     * number of seconds since 1970
     */
    e = ( julian_day - JULIAN_DAY_1970 );
    e *= TIME_SEC_IN_24_HOURS;
    e += sec;
    return( e );
}


/****************************************
 ********* RTC Settings *****************
 ***************************************/

int rtc_init( rtc_type_t type, int8_t time_zone )
{
    uint8_t status;

    if( type > RTC6_MCP7941X || time_zone > 14 || time_zone < -12 )
        return -1;
    current_type = type;
    current_time_zone = time_zone;

    switch( current_type )
    {
        case RTC_PCF8583:
            rtc_hal_init ( RTC_PCF8583_SLAVE );
            break;
        case RTC2_DS1307:
            rtc_hal_init ( RTC2_DS1307_SLAVE );
            break;
        case RTC3_BQ32000:
            rtc_hal_init( RTC3_BQ32000_SLAVE );
            break;
        case RTC6_MCP7941X:
            rtc_hal_init( RTC6_MCP7941X_SLAVE );
            break;
        default:
            return -1;
    }

    rtc_hal_read( RTC_SECONDS_ADDR, ( void * )&status, 1 );
    status |= RTC_START_OSC_MASK;
    rtc_hal_write( RTC_SECONDS_ADDR, ( void * )&status, 1 );

    if( ( !status ) & 0x80 )
    {
        status |= RTC_START_OSC_MASK;
        rtc_hal_write( RTC_SECONDS_ADDR, ( void * )&status, 1 );
    }

    return 0;
}

void rtc_enable_dst()
{
    dst_enabled = true;
}

void rtc_disable_dst()
{
    dst_enabled = false;
}

void rtc_enable_swo( rtc_swo_t swo )
{
    switch( current_type )
    {
        case RTC_PCF8583:
            // SWO is always on, 1 Hz, 50% duty cycle
            break;
        case RTC2_DS1307:
        {
            uint8_t temp;
            rtc_hal_read( 0x07, &temp, 1 );
            temp |= ( 1 << 4 );
            switch ( swo )
            {
                case RTC_1HZ:
                    temp &= ~( 0b00000011 );
                    break;
                case RTC_4_096KHZ:
                    temp &= ~( 0b00000010 );
                    temp |=  ( 0b00000001 );
                    break;
                case RTC_8_192KHZ:
                    temp |= ( 1 << 1 );
                    break;
                case RTC_32_768KHZ:
                    temp |= ( 0b00000011 );
                    break;
                default:
                    break;
            }
            rtc_hal_write( 0x07, &temp, 1 );
            break;
        }
        case RTC6_MCP7941X:
        {
            uint8_t temp;
            rtc_hal_read( 0x07, &temp, 1 );
            temp |= ( 1 << 6 );

            switch( swo )
            {
                case RTC_1HZ:
                    temp &= ~( 0b00000111 );
                    break;
                case RTC_4_096KHZ:
                    temp &= ~( 0b00000111 );
                    temp |=  ( 0b00000001 );
                    break;
                case RTC_8_192KHZ:
                    temp &= ~( 0b00000111 );
                    temp |= ( 1 << 1 );
                    break;
                case RTC_32_768KHZ:
                    temp &= ~( 0b00000111 );
                    temp |= ( 0b00000011 );
                    break;
                default:
                    break;
            }
            rtc_hal_write( 0x07, &temp, 1 );
            break;
        }
    }
}

void rtc_disable_swo()
{
    uint8_t temp;
    switch( current_type )
    {
        case RTC_PCF8583:
            // SWO is always on, 1 Hz, 50% duty cycle
            break;
        case RTC2_DS1307:
            rtc_hal_read( 0x07, &temp, 1 );
            temp &= ~( 1 << 7 );
            rtc_hal_write( 0x07, &temp, 1 );
            break;
        case RTC6_MCP7941X:
            rtc_hal_read( 0x07, &temp, 1 );
            temp &= ~( 1 << 6 );
            rtc_hal_write( 0x07, &temp, 1 );
            break;
    }
}

void rtc_battery_enable()
{
    uint8_t temp = 0;

    switch( current_type )
    {
        case RTC6_MCP7941X:
            rtc_hal_read( 0x03, &temp, 1 );
            temp |= ( 1 << 3 );
            rtc_hal_write( 0x03, &temp, 1 );
            break;
    }
}




/****************************************
 ********* General Usage ****************
 ***************************************/
rtc_time_t *rtc_get_gmt_time()
{
    static rtc_time_t gmt_time;
    uint8_t buffer[RTC_TIMEDATE_BYTES];

    memset( buffer, 0, sizeof( buffer ) );

    switch ( current_type )
    {
        case RTC_PCF8583:
#define PCF_SECONDS 0x02
            rtc_hal_read( PCF_SECONDS, buffer, 5 );
            gmt_time.seconds = BCD2BIN( buffer[RTC_SECONDS_BYTE] );
            gmt_time.minutes = BCD2BIN( buffer[RTC_MINUTES_BYTE] );
            gmt_time.hours = BCD2BIN( RTC_HOURS_MASK( buffer[RTC_HOUR_BYTE] ) );
            gmt_time.weekday = ( buffer[RTC_DATE_BYTE] >> 5 ) + 1;
            gmt_time.monthday =  BCD2BIN( RTC_DATE_MASK( buffer[RTC_DAY_BYTE] ) );
            gmt_time.month = BCD2BIN( RTC_MONTH_MASK( buffer[RTC_DATE_BYTE] ) );
            gmt_time.year = 0;
            break;
        case RTC3_BQ32000:
        case RTC2_DS1307:
            rtc_hal_read( RTC_SECONDS_ADDR, buffer, RTC_TIMEDATE_BYTES );
            gmt_time.seconds = BCD2BIN( RTC_SECONDS_MASK( buffer[RTC_SECONDS_BYTE] ) );
            gmt_time.minutes = BCD2BIN( RTC_MINUTES_MASK( buffer[RTC_MINUTES_BYTE] ) );
            gmt_time.hours = BCD2BIN( RTC_HOURS_MASK( buffer[RTC_HOUR_BYTE] ) );
            gmt_time.weekday = BCD2BIN( RTC_DAY_MASK( buffer[RTC_DAY_BYTE] ) );
            gmt_time.monthday = BCD2BIN( RTC_DATE_MASK( buffer[RTC_DATE_BYTE] ) );
            gmt_time.month = BCD2BIN( RTC_MONTH_MASK( buffer[RTC_MONTH_BYTE] ) );
            gmt_time.year = BCD2BIN( RTC_YEAR_MASK( buffer[RTC_YEAR_BYTE] ) );
            break;
        case RTC6_MCP7941X:
            rtc_hal_read( RTC_SECONDS_ADDR, buffer, RTC_TIMEDATE_BYTES );
            gmt_time.seconds = BCD2BIN( RTC_SECONDS_MASK( buffer[RTC_SECONDS_BYTE] ) );
            gmt_time.minutes = BCD2BIN( RTC_MINUTES_MASK( buffer[RTC_MINUTES_BYTE] ) );
            gmt_time.hours = BCD2BIN( RTC_HOURS_MASK( buffer[RTC_HOUR_BYTE] ) );
            gmt_time.weekday = BCD2BIN( RTC_DAY_MASK( buffer[RTC_DAY_BYTE] ) );
            gmt_time.monthday = BCD2BIN( RTC_DATE_MASK( buffer[RTC_DATE_BYTE] ) );
            gmt_time.month = BCD2BIN( RTC_MONTH_MASK( buffer[RTC_MONTH_BYTE] ) );
            gmt_time.year = BCD2BIN( RTC_YEAR_MASK( buffer[RTC_YEAR_BYTE] ) );
            break;

    }

    return &gmt_time;
}



rtc_time_t *rtc_get_local_time()
{
    static rtc_time_t local_time;

    uint32_t temp_time_unix = rtc_get_gmt_unix_time();

    temp_time_unix += ( current_time_zone * 60 *
                        60 ); // for now withouts miliseconds
    // TODO: Convert unix time to date, return address of local_time. DONE: check below

    time_epoch_to_date ( temp_time_unix, &local_time ); // converting to date

    if( dst_enabled )
    {
        rtc_time_t start, end;
        get_dst ( local_time.year, &start, &end );


        // here we check if our current date is in the period where dst adds one hour
        // if it is a month between the start and end, not including the end, not including the start, we surely need to add one hour

        if( ( local_time.month > start.month ) && ( local_time.month < end.month ) )
        {
            temp_time_unix = time_date_to_epoch( &local_time );
            temp_time_unix += 60 * 60 * 1000;
        }
        // or if it is the end month, but before the last day of DST, we still need to add one hour
        // or if it is the last day,but before 02:00 hrs
        else if( ( ( local_time.month = end.month ) &&
                   ( local_time.monthday < end.monthday ) ) ||
                 ( ( local_time.month = end.month ) && ( local_time.hours < 2 ) ) )
        {
            temp_time_unix = time_date_to_epoch( &local_time );
            temp_time_unix += 60 * 60 * 1000;
        }

        // or if it's the starting month, passed the starting monthday
        // if it is the starting monthday, we check if it is passed 02:00
        else if
        (
            ( ( local_time.month == start.month ) &&
              ( local_time.monthday > start.monthday ) )
            ||
            ( ( local_time.month == start.month ) &&
              ( local_time.monthday == start.monthday ) && ( local_time.hours >= 2 ) )
        )
        {
            temp_time_unix = time_date_to_epoch( &local_time );
            temp_time_unix += 60 * 60 * 1000;
        }

        // in other cases, we subtract one hour
        else temp_time_unix -= 60 * 60 * 1000;
    }

    // converting back to date, returning the address
    time_epoch_to_date( temp_time_unix, &local_time );
    return &local_time;

}


int rtc_set_gmt_time( rtc_time_t time )
{
    uint8_t buffer[RTC_TIMEDATE_BYTES];
    uint8_t temp;
    
    if( time.seconds > 59 ||
            time.minutes > 59 ||
            time.hours > 24 ||
            time.weekday > 7 ||
            time.monthday > 31 ||
            time.month > 12 ||
            time.year > 99 )
        return -1;

    switch ( current_type )
    {
        case RTC_PCF8583:
        {
            temp = 0x80;
            rtc_hal_write( 0, &temp, 1 );
            temp = BIN2BCD ( time.seconds );
            rtc_hal_write( 0x02, &temp , 1 );
            rtc_hal_read( 0x02, &temp , 1 );
            temp = BIN2BCD ( time.minutes );
            rtc_hal_write( 0x03, &temp , 1 );
            rtc_hal_read( 0x03, &temp , 1 );
            temp = 0;
            temp = BIN2BCD ( time.hours );
            rtc_hal_write( 0x04, &temp , 1 );
            rtc_hal_read( 0x04, &temp , 1 );
            temp = 0;
            temp = BIN2BCD ( time.monthday );
            rtc_hal_write( 0x05, &temp , 1 );
            rtc_hal_read( 0x05, &temp , 1 );
            temp = 0;
            temp = BIN2BCD( time.month );
            temp |= ( time.weekday - 1 ) << 5;
            rtc_hal_write( 0x06, &temp , 1 );
            rtc_hal_read( 0x06, &temp , 1 );
            temp = 0;
            rtc_hal_write( 0, &temp, 1 );
            break;
        }

        case RTC3_BQ32000:
            rtc_hal_read( RTC_SECONDS_ADDR, &buffer, RTC_TIMEDATE_BYTES );
            // Set seconds
            buffer[RTC_SECONDS_BYTE] = RTC_SECONDS_CLEAR( buffer[RTC_SECONDS_BYTE] );
            buffer[RTC_SECONDS_BYTE] |= BIN2BCD( time.seconds );

            buffer[RTC_MINUTES_BYTE] = RTC_MINUTES_CLEAR( buffer[RTC_MINUTES_BYTE] );
            buffer[RTC_MINUTES_BYTE] |= BIN2BCD( time.minutes );

            buffer[RTC_HOUR_BYTE] = RTC_HOURS_CLEAR( buffer[RTC_HOUR_BYTE] );
            buffer[RTC_HOUR_BYTE] |= BIN2BCD( time.hours );

            buffer[RTC_DAY_BYTE] = RTC_DAY_CLEAR( buffer[RTC_DAY_BYTE] );
            buffer[RTC_DAY_BYTE] |= BIN2BCD( time.weekday );

            buffer[RTC_DATE_BYTE] = RTC_DATE_CLEAR( buffer[RTC_DATE_BYTE] );
            buffer[RTC_DATE_BYTE] |= BIN2BCD( time.monthday );

            buffer[RTC_MONTH_BYTE] = RTC_MONTH_CLEAR( buffer[RTC_MONTH_BYTE] );
            buffer[RTC_MONTH_BYTE] |= BIN2BCD( time.month );

            buffer[RTC_YEAR_BYTE] = RTC_YEAR_CLEAR( buffer[RTC_YEAR_BYTE] );
            buffer[RTC_YEAR_BYTE] |= BIN2BCD( time.year );

            rtc_hal_write( RTC_SECONDS_ADDR, buffer, RTC_TIMEDATE_BYTES );
            rtc_hal_read( RTC_SECONDS_ADDR, &buffer, RTC_TIMEDATE_BYTES );
            break;

        default:

            rtc_hal_read( RTC_SECONDS_ADDR, &buffer, RTC_TIMEDATE_BYTES );
            // Set seconds
            buffer[RTC_SECONDS_BYTE] = RTC_SECONDS_CLEAR( buffer[RTC_SECONDS_BYTE] );
            buffer[RTC_SECONDS_BYTE] |= BIN2BCD( time.seconds );

            buffer[RTC_MINUTES_BYTE] = RTC_MINUTES_CLEAR( buffer[RTC_MINUTES_BYTE] );
            buffer[RTC_MINUTES_BYTE] |= BIN2BCD( time.minutes );

            buffer[RTC_HOUR_BYTE] = RTC_HOURS_CLEAR( buffer[RTC_HOUR_BYTE] );
            buffer[RTC_HOUR_BYTE] |= BIN2BCD( time.hours );

            buffer[RTC_DAY_BYTE] = RTC_DAY_CLEAR( buffer[RTC_DAY_BYTE] );
            buffer[RTC_DAY_BYTE] |= BIN2BCD( time.weekday );

            buffer[RTC_DATE_BYTE] = RTC_DATE_CLEAR( buffer[RTC_DATE_BYTE] );
            buffer[RTC_DATE_BYTE] |= BIN2BCD( time.monthday );

            buffer[RTC_MONTH_BYTE] = RTC_MONTH_CLEAR( buffer[RTC_MONTH_BYTE] );
            buffer[RTC_MONTH_BYTE] |= BIN2BCD( time.month );

            buffer[RTC_YEAR_BYTE] = RTC_YEAR_CLEAR( buffer[RTC_YEAR_BYTE] );
            buffer[RTC_YEAR_BYTE] |= BIN2BCD( time.year );

            rtc_hal_write( RTC_SECONDS_ADDR, buffer, RTC_TIMEDATE_BYTES );
            rtc_hal_read( RTC_SECONDS_ADDR, buffer, RTC_TIMEDATE_BYTES );
            break;

    }

    return 0;
}


uint32_t rtc_get_gmt_unix_time()
{
    rtc_time_t *temp_time;
    uint32_t temp;
    temp_time = rtc_get_gmt_time();

    temp =  time_date_to_epoch( temp_time );

    return temp;
}


uint32_t rtc_get_local_unix_time()
{
    uint32_t temp;
    rtc_time_t *temp_time = rtc_get_local_time();

    temp =  time_date_to_epoch( temp_time );
    return temp;
}

bool rtc_is_leap_year()
{
    switch( current_type )
    {
        case RTC_PCF8583:
            // NOT SUPPORTED
            return false;
            break;

        case RTC2_DS1307:
            if ( current_local_time.year % 4 != 0 )
                return false;
            else if ( current_local_time.year % 100 != 0 )
                return true;
            else if ( current_local_time.year % 400 == 0 )
                return true;
            else return false;
            break;

        case RTC3_BQ32000:
            if ( current_local_time.year % 4 != 0 )
                return false;
            else if ( current_local_time.year % 100 != 0 )
                return true;
            else if ( current_local_time.year % 400 == 0 )
                return true;
            else return false;
            break;

        case RTC6_MCP7941X:
        {
            uint8_t temp;
            rtc_hal_read( 0x05, &temp, 1 );
            temp &= 0b00100000;
            if ( temp  == 0 )
                return false;
            else return true;
            break;
        }
    }

    return false;
}


bool rtc_is_power_failure()
{
    uint8_t temp;
    switch( current_type )
    {
        case RTC_PCF8583:
            // not supported
            return false;
            break;
        case RTC2_DS1307:
            // Not supported
            return false;
            break;
        case RTC3_BQ32000:

            rtc_hal_read( 0x01, &temp, 1 );
            temp &= ( 1 << 7 );
            if ( temp == 0 ) return false;
            else return true;
            break;

        case RTC6_MCP7941X:
            rtc_hal_read( 0x03, &temp, 1 );
            temp &= ( 1 << 4 );
            if ( temp == 0 ) return false;
            else return true;
            break;
    }

    return false;
}


rtc_time_t *rtc_get_last_power_failure()
{
    static rtc_time_t stamp = {0};

    switch( current_type )
    {
        case RTC_PCF8583:
            // not supported
            return &stamp;
            break;
        case RTC2_DS1307:
            // Not supported
            return &stamp;
            break;
        case RTC3_BQ32000:
            return &stamp;
            break;
        case RTC6_MCP7941X:
        {
            uint8_t buffer[4];

            rtc_hal_read( 0x18, &buffer, 4 );
            stamp.minutes  = BCD2BIN( RTC_MINUTES_MASK( buffer[0] ) );
            stamp.hours    = BCD2BIN( RTC_HOURS_MASK( buffer[1] ) );
            stamp.monthday = BCD2BIN( RTC_DATE_MASK( buffer[2] ) );
            stamp.weekday  = BCD2BIN ( ( buffer[3] & 0b11100000 ) );
            stamp.month    = BCD2BIN( ( buffer[3] & 0b00011111 ) );

            return &stamp;
            break;
        }
    }

    return 0;
}

/****************************************
 ********* Alarms ***********************
 ***************************************/

void rtc_set_alarm( rtc_alarm_t alarm, rtc_alarm_trigger_t trigger,
                    rtc_time_t time )
{
    uint8_t buffer[6];
    uint8_t temp;

    if ( current_type == RTC_PCF8583 )
    {
        rtc_hal_read( 0, &temp, 1 );
        temp |= ( 1 << 2 );
        rtc_hal_write( 0, &temp, 1 );

        buffer[0] = time.seconds;
        buffer[1] = time.minutes;
        buffer[2] = time.hours;
        buffer[3] = time.monthday; // date
        buffer[4] = time.month;

        rtc_hal_write( 0x10, buffer, 5 );


        rtc_hal_read( 8, &temp, 1 );
        temp &= ~( 1 << 4 );
        temp &= ~( 1 << 5 );
        temp |= ( 1 << 7 );

        switch ( trigger )
        {
            case RTC_ALARM_DAY:
                temp |= ( 1 << 4 );
                break;
            case RTC_ALARM_WEEKDAY:
                temp |= ( 1 << 5 );
                break;
            case RTC_ALARM_DATE:
                temp |= ( 1 << 4 );
                temp |= ( 1 << 5 );
                break;
        }
        rtc_hal_write( 8, &temp, 1 );
    }

    else if ( current_type == RTC6_MCP7941X )
    {
        switch ( alarm )
        {
            case RTC_ALARM_0:

                rtc_hal_read( 0x0A, buffer, 6 );
                // Set time values for the alarm
                buffer[RTC_SECONDS_BYTE] = RTC_SECONDS_CLEAR( buffer[RTC_SECONDS_BYTE] );
                buffer[RTC_SECONDS_BYTE] |= BIN2BCD( time.seconds );

                buffer[RTC_MINUTES_BYTE] = RTC_MINUTES_CLEAR( buffer[RTC_MINUTES_BYTE] );
                buffer[RTC_MINUTES_BYTE] |= BIN2BCD( time.minutes );

                buffer[RTC_HOUR_BYTE] = RTC_HOURS_CLEAR( buffer[RTC_HOUR_BYTE] );
                buffer[RTC_HOUR_BYTE] |= BIN2BCD( time.hours );

                buffer[RTC_DAY_BYTE] = RTC_DAY_CLEAR( buffer[RTC_DAY_BYTE] );
                buffer[RTC_DAY_BYTE] |= BIN2BCD( time.weekday );

                buffer[RTC_DATE_BYTE] = RTC_DATE_CLEAR( buffer[RTC_DATE_BYTE] );
                buffer[RTC_DATE_BYTE] |= BIN2BCD( time.monthday );

                buffer[RTC_MONTH_BYTE] = RTC_MONTH_CLEAR( buffer[RTC_MONTH_BYTE] );
                buffer[RTC_MONTH_BYTE] |= BIN2BCD( time.month );

                rtc_hal_write( 0x0A, buffer, 6 );

                // set the trigger
                rtc_hal_read( 0x0D, &temp, 1 );
                switch ( trigger )
                {
                    case RTC_ALARM_SECONDS:
                        temp &= ~( 0b01111000 );
                        break;
                    case RTC_ALARM_MINUTES:
                        temp &= ~( 0b01111000 );
                        temp |= ( 1 << 4 );
                        break;
                    case RTC_ALARM_HOURS:
                        temp &= ~( 0b01111000 );
                        temp |= ( 1 << 5 );
                        break;
                    case RTC_ALARM_DAY:
                        temp &= ~( 0b01111000 );
                        temp |= ( 1 << 4 );
                        temp |= ( 1 << 5 );
                        break;
                    case RTC_ALARM_DATE:
                        temp &= ~( 0b01111000 );
                        temp |= ( 1 << 6 );
                        break;
                    case RTC_ALARM_SEC_MIN_HOUR_DAY_DATE_MONTH:
                        temp &= ~( 0b01111000 );
                        temp |=  ( 0b01110000 );
                    default:
                        break;
                }
                temp |= ( 1 << 7 ); // set the polarity to one
                rtc_hal_write( 0x0D, &temp, 1 );
                rtc_hal_read( 0x03, &temp, 1 ); // enable battery
                temp |= ( 1 << 3 );
                rtc_hal_write( 0x03, &temp, 1 );

                rtc_hal_read( 0x07, &temp, 1 );
                //              temp |= (1<<7);
                temp &= ~( 1 << 6 ); // disable SQWO
                temp |= ( 1 << 4 ); // activate alarm 0
                rtc_hal_write( 0x07, &temp, 1 );
                break;

            case RTC_ALARM_1:

                rtc_hal_read( 0x11, buffer, 6 );
                // Set time values for the alarm
                buffer[RTC_SECONDS_BYTE] = RTC_SECONDS_CLEAR( buffer[RTC_SECONDS_BYTE] );
                buffer[RTC_SECONDS_BYTE] |= BIN2BCD( time.seconds );

                buffer[RTC_MINUTES_BYTE] = RTC_MINUTES_CLEAR( buffer[RTC_MINUTES_BYTE] );
                buffer[RTC_MINUTES_BYTE] |= BIN2BCD( time.minutes );

                buffer[RTC_HOUR_BYTE] = RTC_HOURS_CLEAR( buffer[RTC_HOUR_BYTE] );
                buffer[RTC_HOUR_BYTE] |= BIN2BCD( time.hours );

                buffer[RTC_DAY_BYTE] = RTC_DAY_CLEAR( buffer[RTC_DAY_BYTE] );
                buffer[RTC_DAY_BYTE] |= BIN2BCD( time.weekday );

                buffer[RTC_DATE_BYTE] = RTC_DATE_CLEAR( buffer[RTC_DATE_BYTE] );
                buffer[RTC_DATE_BYTE] |= BIN2BCD( time.monthday );

                buffer[RTC_MONTH_BYTE] = RTC_MONTH_CLEAR( buffer[RTC_MONTH_BYTE] );
                buffer[RTC_MONTH_BYTE] |= BIN2BCD( time.month );

                rtc_hal_write( 0x11, buffer, 6 );

                rtc_hal_read( 0x14, &temp, 1 );
                switch ( trigger )
                {
                    case RTC_ALARM_SECONDS:
                        temp &= ~( 0b01111000 );
                        break;
                    case RTC_ALARM_MINUTES:
                        temp &= ~( 0b01111000 );
                        temp |= ( 1 << 4 );
                        break;
                    case RTC_ALARM_HOURS:
                        temp &= ~( 0b01111000 );
                        temp |= ( 1 << 5 );
                        break;
                    case RTC_ALARM_DAY:
                        temp &= ~( 0b01111000 );
                        temp |= ( 1 << 4 );
                        temp |= ( 1 << 5 );
                        break;
                    case RTC_ALARM_DATE:
                        temp &= ~( 0b01111000 );
                        temp |= ( 1 << 6 );
                        break;
                    case RTC_ALARM_SEC_MIN_HOUR_DAY_DATE_MONTH:
                        temp &= ~( 0b01111000 );
                        temp |=  ( 0b01110000 );
                    default:
                        break;
                }
                temp |= ( 1 << 7 ); // set the polarity to one
                rtc_hal_write( 0x14, &temp, 1 );

                rtc_hal_read( 0x03, &temp, 1 ); // enable battery
                temp |= ( 1 << 3 );
                rtc_hal_write( 0x03, &temp, 1 );

                rtc_hal_read( 0x07, &temp, 1 );
                //              temp |= (1<<7);
                temp &= ~( 1 << 6 ); // disable SQWO
                temp |= ( 1 << 5 ); // activate alarm 1
                rtc_hal_write( 0x07, &temp, 1 );
                break;
        }
    }


}


void rtc_disable_alarm( rtc_alarm_t alarm )
{
    uint8_t temp;
    switch( current_type )
    {
        case RTC_PCF8583:
            rtc_hal_read( 0x00, &temp, 1 );
            temp |= ( 1 << 2 );
            rtc_hal_write( 0x00, &temp, 1 );
            break;
        case RTC2_DS1307:
            // Not supported
            break;
        case RTC3_BQ32000:
            // not supported
            break;
        case RTC6_MCP7941X:
            rtc_hal_read( 0x07, &temp, 1 );
            switch( alarm )
            {
                case RTC_ALARM_0:
                    temp &= ~( 1 << 4 );
                    rtc_hal_write( 0x07, &temp, 1 );
                    break;
                case RTC_ALARM_1:
                    temp &= ~( 1 << 5 );
                    rtc_hal_write( 0x07, &temp, 1 );
                    break;
            }
            break;
    }
}


rtc_time_t *rtc_read_alarm( rtc_alarm_t alarm )
{
    uint8_t buffer[6];
    static rtc_time_t temp_time = {0};

    switch( current_type )
    {
        case RTC_PCF8583:
        {
            rtc_hal_read( 0x0A, &temp_time.seconds, 1 );
            rtc_hal_read( 0x0B, &temp_time.minutes, 1 );
            rtc_hal_read( 0x0C, &temp_time.hours, 1 );
            rtc_hal_read( 0x0D, &temp_time.monthday, 1 );
            rtc_hal_read( 0x0E, &temp_time.month, 1 );

            return &temp_time;
            break;
        }
        case RTC2_DS1307:
            // Not supported
            return &temp_time;
            break;
        case RTC3_BQ32000:
            // not supported
            return &temp_time;
            break;
        case RTC6_MCP7941X:
            switch( alarm )
            {
                case RTC_ALARM_0:
                    rtc_hal_read( 0x0A, &buffer, 6 );
                    temp_time.seconds = BCD2BIN( RTC_SECONDS_MASK( buffer[RTC_SECONDS_BYTE] ) );
                    temp_time.minutes = BCD2BIN( RTC_MINUTES_MASK( buffer[RTC_MINUTES_BYTE] ) );
                    temp_time.hours = BCD2BIN( RTC_HOURS_MASK( buffer[RTC_HOUR_BYTE] ) );
                    temp_time.weekday = BCD2BIN( RTC_DAY_MASK( buffer[RTC_DAY_BYTE] ) );
                    temp_time.monthday = BCD2BIN( RTC_DATE_MASK( buffer[RTC_DATE_BYTE] ) );
                    temp_time.month = BCD2BIN( RTC_MONTH_MASK( buffer[RTC_MONTH_BYTE] ) );
                    break;
                case RTC_ALARM_1:
                    rtc_hal_read( 0x11, &buffer, 6 );
                    temp_time.seconds = BCD2BIN( RTC_SECONDS_MASK( buffer[RTC_SECONDS_BYTE] ) );
                    temp_time.minutes = BCD2BIN( RTC_MINUTES_MASK( buffer[RTC_MINUTES_BYTE] ) );
                    temp_time.hours = BCD2BIN( RTC_HOURS_MASK( buffer[RTC_HOUR_BYTE] ) );
                    temp_time.weekday = BCD2BIN( RTC_DAY_MASK( buffer[RTC_DAY_BYTE] ) );
                    temp_time.monthday = BCD2BIN( RTC_DATE_MASK( buffer[RTC_DATE_BYTE] ) );
                    temp_time.month = BCD2BIN( RTC_MONTH_MASK( buffer[RTC_MONTH_BYTE] ) );
                    break;
            }
            return &temp_time;
            break;
    }

    return 0;
}


/****************************************
 ********* Memory ***********************
 ***************************************/
void rtc_write_sram( uint8_t addr, uint8_t data_in )
{
    switch( current_type )
    {
        case RTC2_DS1307:
            if( addr + RTC2_RAM_START < RTC2_RAM_END )
                rtc_hal_write( RTC2_RAM_START + addr, &data_in, 1 );
            break;
        case RTC6_MCP7941X:
            if( addr + RTC6_RAM_START < RTC6_RAM_END )
            {
                rtc_hal_set_slave( RTC6_MCP7941X_SRAM_SLAVE );
                rtc_hal_write( RTC6_RAM_START + addr, &data_in, 1 );
                rtc_hal_set_slave( RTC6_MCP7941X_SLAVE );
            }
            break;
    }
}

void rtc_write_sram_bulk( uint8_t addr, void *data_in, size_t data_size )
{
    switch( current_type )
    {       
        case RTC2_DS1307:
            if( addr + RTC2_RAM_START + data_size < RTC2_RAM_END )
            {
                rtc_hal_write( RTC2_RAM_START + addr, data_in, data_size );
            }
            break;
        case RTC6_MCP7941X:
            if( addr + RTC6_RAM_START + data_size < RTC6_RAM_END )
            {
                rtc_hal_set_slave( RTC6_MCP7941X_SRAM_SLAVE );
                rtc_hal_write( RTC6_RAM_START + addr, data_in, data_size );
                rtc_hal_set_slave( RTC6_MCP7941X_SLAVE );
            }
            break;
    }
}


uint8_t rtc_read_sram( uint8_t addr )
{
    uint8_t temp = 0;

    switch( current_type )
    {        
        case RTC2_DS1307:
            if( addr + RTC2_RAM_START < RTC2_RAM_END )
                rtc_hal_read( RTC2_RAM_START + addr, &temp, 1 );
        break;
        case RTC6_MCP7941X:
            if( addr + RTC6_RAM_START < RTC6_RAM_END )
            {
                rtc_hal_set_slave( RTC6_MCP7941X_SRAM_SLAVE );
                rtc_hal_read ( RTC6_RAM_START + addr, &temp, 1 );
                rtc_hal_set_slave( RTC6_MCP7941X_SLAVE );
            }
            break;
    }

    return temp;
}


void rtc_read_sram_bulk( uint8_t addr, void *data_out, uint8_t data_size )
{
    switch( current_type )
    {
        case RTC2_DS1307:
            if( addr + RTC2_RAM_START + data_size < RTC2_RAM_END )
                rtc_hal_read( RTC2_RAM_START + addr, data_out, data_size );
            break;
        case RTC6_MCP7941X:
            if( addr + RTC6_RAM_START + data_size < RTC6_RAM_END )
            {
                rtc_hal_set_slave( RTC6_MCP7941X_SRAM_SLAVE );
                rtc_hal_read( RTC6_RAM_START + addr, data_out, data_size );
                rtc_hal_set_slave( RTC6_MCP7941X_SLAVE );
            }
            break;
    }
}

void rtc_eeprom_write_protect_on()
{
    if( current_type == RTC6_MCP7941X )
    {
        uint8_t temp;
        rtc_hal_set_slave( RTC6_MCP7941X_EEPROM_SLAVE );
        rtc_hal_read( 0xff, &temp, 1 );
        temp |= ( 1 << 3 ) | ( 1 << 2 );
        rtc_hal_write( 0xff, &temp, 1 );
        rtc_hal_set_slave( RTC6_MCP7941X_SLAVE );
    }
}

void rtc_eeprom_write_protect_off()
{
    if( current_type == RTC6_MCP7941X )
    {
        uint8_t temp;
        rtc_hal_set_slave( RTC6_MCP7941X_EEPROM_SLAVE );
        rtc_hal_read( 0xff, &temp, 1 );
        temp &= ~( 1 << 3 ) | ( 1 << 2 );
        rtc_hal_write( 0xff, &temp, 1 );
        rtc_hal_set_slave( RTC6_MCP7941X_SLAVE );
    }
}

bool rtc_eeprom_is_locked()
{
    if( current_type == RTC6_MCP7941X )
    {
        uint8_t temp;
        rtc_hal_set_slave( RTC6_MCP7941X_EEPROM_SLAVE );
        rtc_hal_read( 0xff, &temp, 1 );
        rtc_hal_set_slave( RTC6_MCP7941X_SLAVE );
        return ( temp & ( 1 << 3 ) | ( 1 << 2 ) ) ? true : false;
    }
    
    return false;
}

bool rtc_write_eeprom( uint8_t addr, void *data_in, uint8_t data_size )
{
    if( current_type == RTC6_MCP7941X )
    {
        if( ( addr >= RTC6_EEPROM_START ) &&
            ( addr + data_size ) < RTC6_EEPROM_END )
        {
            rtc_hal_set_slave( RTC6_MCP7941X_EEPROM_SLAVE );
                
            if( data_size <= RTC6_EEPROM_PAGE_SIZE )
                rtc_hal_write( addr, data_in, data_size );
            else {
                int i;
                uint8_t count = data_size / RTC6_EEPROM_PAGE_SIZE;
                uint8_t rem = data_size % RTC6_EEPROM_PAGE_SIZE;
                    
                for( i = 0; i < count; i++ )
                    rtc_hal_write( addr + ( i * 8 ), data_in + ( i * 8 ), RTC6_EEPROM_PAGE_SIZE );

                if( rem )
                    rtc_hal_write( addr + ( count * 8 ), data_in + ( i * 8 ), rem );
            }
                
                rtc_hal_set_slave( RTC6_MCP7941X_SLAVE );
        }
    }

    return true;
}


void rtc_read_eeprom( uint8_t addr, void *data_out, uint8_t data_size )
{
    if( current_type == RTC6_MCP7941X )
    {
        if( ( data_size < RTC6_EEPROM_SIZE ) &&
            ( ( addr + data_size ) < RTC6_EEPROM_END ) &&
            ( addr >= RTC6_EEPROM_START ) )
        {
            rtc_hal_set_slave( RTC6_MCP7941X_EEPROM_SLAVE );
            rtc_hal_read( addr, data_out, data_size );
            rtc_hal_set_slave( RTC6_MCP7941X_SLAVE );
        }
    }
}

uint8_t *rtc_read_unique_id()
{
    if( current_type == RTC6_MCP7941X )
    {
        static uint8_t unique_id[8];
        rtc_hal_set_slave( RTC6_MCP7941X_EEPROM_SLAVE );
        rtc_hal_read( 0xF0, unique_id, 8 );
        rtc_hal_set_slave( RTC6_MCP7941X_SLAVE );
        
        return unique_id;
    }
    
    return 0;
}

void rtc_write_unique_id( uint8_t *id )
{
    #define RTC6_EEPROM_ID_UNLOCK_ADR 0x09
    if( current_type == RTC6_MCP7941X )
    {
        uint8_t temp = 0x55;
        
        if( id == NULL )
            return;
        
        rtc_hal_set_slave( RTC6_MCP7941X_EEPROM_SLAVE );
        // Unlock EEPROM Unique ID
        rtc_hal_write( RTC6_EEPROM_ID_UNLOCK_ADR, &temp, 1 );
        temp = 0xAA;
        rtc_hal_write( RTC6_EEPROM_ID_UNLOCK_ADR, &temp, 1 );
        // Write ID
        rtc_hal_write( 0xF0, id, 8 );
        rtc_hal_set_slave( RTC6_MCP7941X_SLAVE );
    }
}

/*************** END OF FUNCTIONS ***************************************************************************/