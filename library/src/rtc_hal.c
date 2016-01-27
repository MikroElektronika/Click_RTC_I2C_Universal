/*******************************************************************************
* Title                 :   Hal I2C
* Filename              :   hal_i2c.c
* Author                :   MSV
* Origin Date           :   02/11/2015
* Notes                 :   None
*******************************************************************************/
/*************** MODULE REVISION LOG ******************************************
*
*    Date    Software Version    Initials   Description
*  02/11/15    XXXXXXXXXXX         MSV      Module Created.
*
*******************************************************************************/
/** @file I2C_HAL_template.c
 *
 *  @brief I2C template.
 */
/******************************************************************************
* Includes
*******************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rtc_hal.h"
/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/

/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/
#define WRITE 0
#define READ  1
/******************************************************************************
* Module Typedefs
*******************************************************************************/
#if defined( __MIKROC_PRO_FOR_ARM__ )


#if defined( STM32 )
static unsigned int( *i2c_start_p )( void );
static unsigned int( *i2c_write_p )( unsigned char slave_address,
                                     unsigned char *buffer,
                                     unsigned long count,
                                     unsigned long end_mode );
static void( *i2c_read_p )( unsigned char slave_address,
                            unsigned char *buffer,
                            unsigned long count,
                            unsigned long end_mode );

#elif defined( TIVA )
static void( *i2c_enable_p )( void );
static void( *i2c_disable_p )( void );
static void( *i2c_set_slave_address_p )( unsigned char slave_address,
        unsigned char dir );
static unsigned char( *i2c_write_p )( unsigned char data_out,
                             unsigned char mode );
static unsigned char( *i2c_read_p )( unsigned char *data_in,
                            unsigned char mode );
#endif

#elif  defined( __MIKROC_PRO_FOR_AVR__ )
static unsigned char( *i2c_busy_p )( void );
static unsigned char( *i2c_status_p )( void );
static unsigned char( *i2c_start_p )( void );
static void( *i2c_stop_p )( void );
static void( *i2c_close_p )( void );
static void( *i2c_write_p )( unsigned char data_out );
static unsigned char( *i2c_read_p )( unsigned char ack );
static void ( *write_spi_p )( unsigned char data_out );
static unsigned char ( *read_spi_p )( unsigned char dummy );

#elif  defined( __MIKROC_PRO_FOR_PIC__ )
static unsigned char( *i2c_idle_p )( void );
static unsigned char( *i2c_start_p )( void );
static void( *i2c_stop_p )( void );
static void( *i2c_restart_p )( void );
static unsigned char( *i2c_write_p )( unsigned char data_out );
static unsigned char( *i2c_read_p )( unsigned char ack );
static void ( *write_spi_p )( unsigned char data_out );
static unsigned char ( *read_spi_p )( unsigned char dummy );

#elif defined( __MIKROC_PRO_FOR_PIC32__ )
static unsigned int( *i2c_idle_p )( void );
static unsigned int( *i2c_start_p )( void );
static void( *i2c_stop_p )( void );
static unsigned int( *i2c_restart_p )( void );
static unsigned int( *i2c_write_p )( unsigned char data_out );
static unsigned char( *i2c_read_p )( unsigned int ack );
static void ( *write_spi_p )( unsigned long data_out );
static unsigned long ( *read_spi_p )( unsigned long buffer );

#elif defined( __MIKROC_PRO_FOR_DSPIC__ )
static unsigned int( *i2c_idle_p )( void );
static unsigned int( *i2c_start_p )( void );
static void( *i2c_stop_p )( void );
static void( *i2c_restart_p )( void );
static unsigned int( *i2c_write_p )( unsigned char data_out );
static unsigned char( *i2c_read_p )( unsigned int ack );
static void ( *write_spi_p )( unsigned int data_out );
static unsigned int ( *read_spi_p )( unsigned int buffer );

#elif defined( __MIKROC_PRO_FOR_8051__ )
static unsigned char( *i2c_busy_p )( void );
static unsigned char ( *i2c_status_p )( void );
static unsigned char( *i2c_start_p )( void );
static void( *i2c_stop_p )( void );
static void( *i2c_close_p )( void );
static void( *i2c_write_p )( unsigned char data_out );
static unsigned char( *i2c_read_p )( unsigned char ack );

#elif defined( __MIKROC_PRO_FOR_FT90x__ )
static void( *i2c_soft_reset_p )( void );
static void( *i2c_set_slave_address_p )( unsigned char slave_address );
static unsigned char( *i2c_write_p )( unsigned char data_out );
static unsigned char( *i2c_read_p )( unsigned char *data_in );
static unsigned char( *i2c_write_bytes_p )( unsigned char *buffer,
        unsigned int count );
static unsigned char( *i2c_read_bytes_p )( unsigned char *buffer,
        unsigned int count );
static void ( *write_spi_p )( unsigned char data_out );
static unsigned char ( *read_spi_p )( unsigned char dummy );
static void ( *write_bytes_spi_p )( unsigned char *data_out,
                                    unsigned int count );
static void ( *read_bytes_spi_p )( unsigned char *buffer,
                                   unsigned int count );
#endif
/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
static uint8_t _i2c_address;

#define DUMMY                                                           0x00
#if   defined( __MIKROC_PRO_FOR_ARM__ )
#elif defined( __MIKROC_PRO_FOR_AVR__ )
#elif defined( __MIKROC_PRO_FOR_PIC__ )
#elif defined( __MIKROC_PRO_FOR_PIC32__ )
#elif defined( __MIKROC_PRO_FOR_DSPIC__ )
#elif defined( __MIKROC_PRO_FOR_8051__ )
#elif defined( __MIKROC_PRO_FOR_FT90x__ )
//extern sfr sbit CLICK_PIN_1;
#endif

//#if   defined( __MIKROC_PRO_FOR_ARM__ )
//#elif defined( __MIKROC_PRO_FOR_AVR__ )
//#elif defined( __MIKROC_PRO_FOR_PIC__ )
//#elif defined( __MIKROC_PRO_FOR_PIC32__ )
//#elif defined( __MIKROC_PRO_FOR_DSPIC__ )
//#elif defined( __MIKROC_PRO_FOR_8051__ )
//#elif defined( __MIKROC_PRO_FOR_FT90x__ )
//
//#endif
/******************************************************************************
* Function Prototypes
*******************************************************************************/
//static void advanced_init( uint8_t interface );

/******************************************************************************
* Function Definitions
*******************************************************************************/

void rtc_hal_init( uint8_t address_id )
{
#if defined( __MIKROC_PRO_FOR_ARM__ )

#if defined( STM32 )
    i2c_start_p = I2C_Start_Ptr;
    i2c_write_p = I2C_Write_Ptr;
    i2c_read_p = I2C_Read_Ptr;
#elif defined( TIVA )
    i2c_enable_p = I2C_Enable_Ptr;
    i2c_disable_p = I2C_Disable_Ptr;
    i2c_set_slave_address_p = I2C_Master_Slave_Addr_Set_Ptr;
    i2c_write_p = I2C_Write_Ptr;
    i2c_read_p = I2C_Read_Ptr;
#endif

#elif defined( __MIKROC_PRO_FOR_AVR__ )
    i2c_busy_p = TWIC_Busy;
    i2c_status_p = TWIC_Status;
    i2c_close_p = TWIC_Close;
    i2c_start_p = TWIC_Start;
    i2c_stop_p = TWIC_Stop;
    i2c_write_p = TWIC_Write;
    i2c_read_p = TWIC_Read;

#elif defined( __MIKROC_PRO_FOR_PIC__ )
    i2c_start_p = I2C1_Start;
    i2c_stop_p = I2C1_Stop;
    i2c_write_p = I2C1_Wr;
    i2c_read_p = I2C1_Rd;


#elif defined( __MIKROC_PRO_FOR_PIC32__ )
    i2c_idle_p = I2C_Is_Idle_Ptr;
    i2c_start_p = I2C_Start_Ptr;
    i2c_stop_p = I2C_Stop_Ptr;
    i2c_restart_p = I2C_Restart_Ptr;
    i2c_write_p = I2C_Write_Ptr;

#elif defined( __MIKROC_PRO_FOR_DSPIC__ )
    i2c_start_p = I2C1_Start;
    i2c_stop_p = I2C1_Stop;
    i2c_write_p = I2C1_Write;
    i2c_read_p = I2C1_Read;

#elif defined( __MIKROC_PRO_FOR_8051__ )
    i2c_busy_p = TWI_Busy;
    i2c_status_p = TWI_Status;
    i2c_close_p = TWI_Close;
    i2c_start_p = TWI_Start;
    i2c_stop_p = TWI_Stop;
    i2c_write_p = TWI_Write;
    i2c_read_p = TWI_Read;
    write_spi_p = SPI_Wr_Ptr;
    read_spi_p = SPI_Rd_Ptr;

#elif defined( __MIKROC_PRO_FOR_FT90x__ )
    i2c_soft_reset_p = I2CM_Soft_Reset_Ptr;
    i2c_set_slave_address_p = I2CM_Set_Slave_Address_Ptr;
    i2c_write_p = I2CM_Write_Ptr;
    i2c_read_p = I2CM_Read_Ptr;
    i2c_write_bytes_p = I2CM_Write_Bytes_Ptr;
    i2c_read_bytes_p = I2CM_Read_Bytes_Ptr;
#endif

#if defined( __MIKROC_PRO_FOR_ARM__ ) || defined(__MIKROC_PRO_FOR_FT90x__)
    _i2c_address = address_id;
#else
    _i2c_address = ( address_id << 1 );
#endif

}

void rtc_hal_set_slave( uint8_t address_id )
{
    _i2c_address = address_id;
}


void rtc_hal_write( uint8_t address, void *data_in, size_t num_bytes )
{
    uint8_t buffer[10];
    buffer[0] = address;
    memcpy( &buffer[1], data_in, num_bytes );
#if defined( __MIKROC_PRO_FOR_ARM__ )
    #if defined( TIVA )
    i2c_set_slave_address_p( _i2c_address, _I2C_DIR_MASTER_TRANSMIT );
    i2c_write_p( address, _I2C_MASTER_MODE_BURST_SEND_START );
    
    if( num_bytes > 1 )
    {
        while( num_bytes-- > 1 )
            i2c_write_p( *( ( uint8_t* )data_in++ ), _I2C_MASTER_MODE_BURST_SEND_CONT );

        i2c_write_p( *( uint8_t* )data_in, _I2C_MASTER_MODE_BURST_SEND_FINISH );
    } else {
        i2c_write_p( *(uint8_t* )data_in, _I2C_MASTER_MODE_SINGLE_SEND );
    }

    #else
    i2c_start_p();
    i2c_write_p( _i2c_address, buffer, num_bytes + 1, END_MODE_STOP );
    #endif
#elif defined (__MIKROC_PRO_FOR_FT90x__)
    i2cm1_set_slave_address ( _i2c_address );
    i2c_write_p ( address );
    i2c_write_bytes_p ( buffer, num_bytes );
#elif defined(__MIKROC_PRO_FOR_AVR__) || \
          defined(__MIKROC_PRO_FOR_8051__)  || \
          defined(__MIKROC_PRO_FOR_DSPIC__) || \
          defined(__MIKROC_PRO_FOR_PIC32__)

    i2c_start_p();
    i2c_write_p( _i2c_address | WRITE );
    i2c_write_p( address );
    while ( num_bytes >= 1 )
    {
        i2c_write_p ( *( ( uint8_t * ) data_in++ ) );
        num_bytes--;
    }

    i2c_stop_p();

#elif  defined(__MIKROC_PRO_FOR_PIC__)
    i2c_start_p();
    i2c_write_p( _i2c_address + WRITE );
    i2c_write_p( address );
    while ( num_bytes >= 1 )
    {
        i2c_write_p ( *( ( uint8_t * ) data_in++ ) );
        num_bytes--;
    }

    i2c_stop_p();

#endif

}


void rtc_hal_read( uint8_t address, void *data_out, size_t num_bytes )
{
#if defined( __MIKROC_PRO_FOR_ARM__ )
    #if defined( TIVA )
    i2c_set_slave_address_p( _i2c_address, _I2C_DIR_MASTER_TRANSMIT );
    i2c_write_p( address, _I2C_MASTER_MODE_SINGLE_SEND );
    i2c_set_slave_address_p( _i2c_address, _I2C_DIR_MASTER_RECEIVE );

    if( num_bytes > 1 )
    {
        i2c_read_p( ( uint8_t * )data_out++, _I2C_MASTER_MODE_BURST_RECEIVE_START );
        num_bytes--;
        
        while( num_bytes > 1 )
            i2c_read_p( ( uint8_t * )data_out++, _I2C_MASTER_MODE_BURST_RECEIVE_CONT );

        i2c_read_p( ( uint8_t * )data_out, _I2C_MASTER_MODE_BURST_RECEIVE_FINISH );
    } else {
        i2c_read_p( ( uint8_t * )data_out, _I2C_MASTER_MODE_SINGLE_RECEIVE );
    }
    #else
    i2c_start_p();
    i2c_write_p ( _i2c_address, &address, 1, END_MODE_RESTART );
    i2c_read_p  ( _i2c_address, ( uint8_t * )data_out, num_bytes, END_MODE_STOP );
    #endif
    
#elif defined (__MIKROC_PRO_FOR_FT90x__)
    i2c_set_slave_address_p( _i2c_address );
    i2c_write_p ( address );
    i2c_read_bytes_p ( ( uint8_t * )data_out , num_bytes );

#elif       defined(__MIKROC_PRO_FOR_8051__)  || \
              defined(__MIKROC_PRO_FOR_DSPIC__) || \
              defined(__MIKROC_PRO_FOR_PIC32__)


#elif defined(__MIKROC_PRO_FOR_AVR__)
    i2c_start_p ();
    i2c_write_p ( _i2c_address + WRITE );
    i2c_write_p ( address );
    i2c_start_p ();
    i2c_write_p ( _i2c_address + READ );

    while( num_bytes >= 2 )
    {
        *( ( uint8_t * ) data_out++ ) = i2c_read_p( 1 );
        num_bytes--;
    }

    *( ( uint8_t * ) data_out ) = i2c_read_p( 0 );

    i2c_stop_p();

#elif  defined(__MIKROC_PRO_FOR_PIC__)
    uint8_t buffer;
    i2c_start_p ();
    i2c_write_p ( _i2c_address + WRITE );
    i2c_write_p ( address );
    i2c_start_p ();
    i2c_write_p ( _i2c_address + READ );

    while( num_bytes >= 2 )
    {
        *( ( uint8_t * ) data_out++ ) = i2c_read_p( 1 );
        num_bytes--;
    }

    *( ( uint8_t * ) data_out ) = i2c_read_p( 0 );

    i2c_stop_p();


#endif


}



/*************** END OF FUNCTIONS *********************************************/