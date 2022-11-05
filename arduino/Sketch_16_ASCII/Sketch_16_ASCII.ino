// Test for minimum program size.
// Edit AVRI2C_FASTMODE in SSD1306Ascii.h to change the default I2C frequency.

#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

// Define proper RST_PIN if required.
#define RST_PIN -1

SSD1306AsciiAvrI2c oled;
//------------------------------------------------------------------------------

char buff[32];

void setup() {
  int n=14;
#if RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif // RST_PIN >= 0
  // Call oled.setI2cClock(frequency) to change from the default frequency.

  oled.clear();
  
  oled.setFont(Cooper19);
  sprintf(buff,"  R-Cortes \n  Inc.");
  DisplayText(2000,true);
  
  oled.setFont(Arial_bold_14);
  sprintf(buff,"   Timer");
  DisplayText(2000,true);
  
  oled.setFont(System5x7);
  sprintf(buff,"\n\nSet Timer");
  DisplayText(2000,false);
  sprintf(buff,"\n\nTime = %d",n);
  DisplayText(2000,false);
}

void DisplayText(uint16_t disTime,bool clearDisplay)
{
  if(clearDisplay)oled.clear();
  oled.print(buff);
  delay(disTime);
}
//------------------------------------------------------------------------------
void loop() {}
