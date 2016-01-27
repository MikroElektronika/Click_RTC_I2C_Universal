/*    - Turn on PORTE LED's at switch SW15
     - Pull down PortD10 pin (PortD three state pin)
     - Set Button Press Level for PortD on Vcc */
#include "rtc.h"



// TFT module connections
/*unsigned int TFT_DataPort at GPIOE_ODR;
sbit TFT_RST at GPIOE_ODR.B8;
sbit TFT_RS at GPIOE_ODR.B12;
sbit TFT_CS at GPIOE_ODR.B15;
sbit TFT_RD at GPIOE_ODR.B10;
sbit TFT_WR at GPIOE_ODR.B11;
sbit TFT_BLED at GPIOE_ODR.B9;*/
// End TFT module connections


//sbit RTC_CS_PIN at GPIOB_ODR.B0;
rtc_time_t time_test;
rtc_time_t *time;
rtc_time_t *local_time;
char txt[10];

static void DisplayInit()
{

    /*TFT_Init_ILI9341_8bit(320, 240);
    TFT_BLED = 1;
    TFT_FILL_SCREEN(CL_AQUA);
    TFT_SET_FONT(TFT_defaultFont,CL_BLACK, FO_HORIZONTAL);
    tft_write_text("GMT Time", 100, 10);
    tft_write_text("Local Time", 200, 10);
    tft_write_text("seconds", 10,40);
    tft_write_text("minutes", 10,60);
    tft_write_text("hours", 10,80);
    tft_write_text("monthday", 10,100);
    tft_write_text("month", 10,120);
    Tft_write_text("year", 10,140);
    tft_write_text("Current time zone : GMT - 2", 10,200);
    tft_set_pen(CL_AQUA,1);
    TFT_SET_BRUSH(1,CL_AQUA, 0, 0, 0,0);*/

}

display_values()
{
  TFT_RECTANGLE(100,40, 240,180);
  inttostr(time->seconds, txt);
  tft_write_text(txt, 100,40);
  inttostr(time->minutes, txt);
  tft_write_text(txt, 100,60);
  inttostr(time->hours, txt);
  tft_write_text(txt, 100,80);
  inttostr(time->monthday, txt);
  tft_write_text(txt, 100,100);
  inttostr(time->month, txt);
  tft_write_text(txt, 100,120);
  inttostr(time->year, txt);
  tft_write_text(txt, 100,140);


  inttostr(local_time->seconds, txt);
  tft_write_text(txt, 200,40);
  inttostr(local_time->minutes, txt);
  tft_write_text(txt, 200,60);
  inttostr(local_time->hours, txt);
  tft_write_text(txt, 200,80);
  inttostr(local_time->monthday, txt);
  tft_write_text(txt, 200,100);
  inttostr(local_time->month, txt);
  tft_write_text(txt, 200,120);
  inttostr(local_time->year, txt);
  tft_write_text(txt, 200,140);
}


void main() {

   //DisplayInit();

  //GPIO_Digital_Input(&GPIOD_BASE, _GPIO_PINMASK_10);


  time_test.seconds = 0;
  time_test.minutes = 15;
  time_test.hours = 15;
  time_test.weekday = 0;
  time_test.monthday = 1;
  time_test.month = 1;
  time_test.year = 15;


  //I2C1_Init_Advanced( 100000, &_GPIO_MODULE_I2C1_PB67 );
  rtc_init( RTC6_MCP7941X, -1 );
  delay_ms(1000);
  rtc_set_gmt_time(time_test);


  while(1)                             // Infinite loop
  {       Delay_ms( 200 );
          time = rtc_get_gmt_time();
          local_time = rtc_get_local_time();
//          display_values();
  }

}