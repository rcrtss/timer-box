//IDEA: Instead of passing attribute to DisplayText, do not recieve char and print what is in buff (global Var)
#include <stdlib.h>
/*************************************************************************/

#include "DHT.h"

#define DHTPIN 5     // Digital pin connected to the DHT sensor

#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

/**************************************************************************/
//************************* OLED ******************************************
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define DISPLAY_TIME 2000
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


/**************************************************************************/

#include <RTClib.h>

RTC_DS3231 rtc;

// the pin that is connected to SQW of DS3231
#define CLOCK_INTERRUPT_PIN 3
char weekDay[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
/**************************************************************************/


#include <Servo.h>

Servo myservo;  // create servo object to control a servo


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ************************* SETUP **************************************************** SETUP ******************************
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const byte interruptPin = 2;
bool sw1=0;
byte menuLevel = 0;
uint8_t timerTarget[3];
char buff[10];
#define BLUE_BUTTON     2
#define WHITE_BUTTON    3

void setup()
{
      Serial.begin(9600);
      //Begin I2C
      Wire.begin();
      //Begin OLED
      if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
      }
      //Begin humidity sensor
      dht.begin();
  
      //BLUE (Interrupt)
      pinMode(interruptPin, INPUT_PULLUP);
      attachInterrupt(digitalPinToInterrupt(interruptPin), awake, FALLING );
      //WHITE
      pinMode(WHITE_BUTTON, INPUT_PULLUP);
  
      //********************* initializing the rtc ********************
      if(!rtc.begin()) {
          Serial.println("Couldn't find RTC!");
          Serial.flush();
          abort();
      }
  
      /*Uncomment block below to set time when power loss
      if(rtc.lostPower()) {
          // this will adjust to the date and time at compilation
          rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      }*/
    
      //we don't need the 32K Pin, so disable it
      rtc.disable32K();
      // stop oscillating signals at SQW Pin
      rtc.writeSqwPinMode(DS3231_OFF);

      // ***************** OLED WELCOME**********************
      sprintf(buff, "RC");
      DisplayText(60, 29, DISPLAY_TIME/2, 1, false);
      //testdrawbitmap(DISPLAY_TIME);
      testdrawrect();
}

//***************** OLED+Temp **************************
void DisplayTempAndHumidity()
{
      // Reading temperature or humidity takes about 250 milliseconds!
      // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
      float h = dht.readHumidity();
      // Read temperature as Celsius (the default)
      float t = dht.readTemperature();

      char h_str[8]; // Buffer big enough for 7-character float
      dtostrf(h, 6, 2, h_str); // Leave room for too large numbers!
      sprintf(buff, "Humidity");
      DisplayText(0, 10, 0, 2, false);
      sprintf(buff, h_str);
      DisplayText(0,40, 0, 2, true);
      sprintf(buff, " %");
      DisplayText(80,40, DISPLAY_TIME, 2, true);
  
      char t_str[8]; // Buffer big enough for 7-character float
      dtostrf(t, 6, 2, t_str); // Leave room for too large numbers!
  
      sprintf(buff, "Temp");
      DisplayText(0, 10, 0, 2, false);
      sprintf(buff, t_str);
      DisplayText(0,40, 0, 2, true);
      sprintf(buff, " C");
      DisplayText(80,40, DISPLAY_TIME, 2, true);
  
}

void DisplayText(int16_t curX, int16_t curY, unsigned long displayTime, uint8_t textSize, bool holdPrevious)
{

    if(!holdPrevious)
        display.clearDisplay();
    display.setTextSize(textSize);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(curX,curY);
    display.print(buff);
    display.display();
    delay(displayTime);
}

void testdrawrect() {
  
      for(int16_t i=0; i<display.height()/2-6; i+=2) {
        display.drawRect(i, i, display.width()-2*i, display.height()-2*i, SSD1306_WHITE);
        display.display(); // Update screen with each newly-drawn rectangle
        delay(1);
      }

      delay(2000);
}
//******************************************************


//******************* OLED+RTC ******************************
void DisplayDateAndTime()
{
    // print current time
    char timeStr[10] = "hh:mm AP";
    char dateStr[10] = "DD/MMM/YY";
    uint8_t day;
    rtc.now().toString(timeStr);
    rtc.now().toString(dateStr);
    day = rtc.now().dayOfTheWeek();
    sprintf(buff, dateStr);
    DisplayText(0, 10, 0, 2, false);
    sprintf(buff, weekDay[day]);
    DisplayText(0, 40, 0, 2, true);
    sprintf(buff, timeStr);
    DisplayText(60, 47, DISPLAY_TIME, 1, true);
}
void DisplayOpeningDate()
{
    /*char buff[9];
    DisplayText(0, 10, "Time left:", 0, 0, 2, false);
    sprintf_P(buff, (PGM_P)F("%02d days %02d hrs %02d min"), timerTarget[0], timerTarget[1],timerTarget[2]);
    DisplayText(0, 40, buff, 0, 2 * DISPLAY_TIME, 1, true);*/
}
//******************************************************


//******************* SERVO ****************************

void openDoor()
{
      myservo.attach(9);
      myservo.write(0);
      display.clearDisplay();
      sprintf(buff, "Timer ended");
      DisplayText(0, 30, 300, 1, true);
      delay(1000);
      myservo.detach();
      display.clearDisplay();
      display.display();
}

void closeDoor()
{
      myservo.attach(9);
      myservo.write(100);
      delay(1000);
      myservo.detach();
}
//******************************************************

//******************* INTERRUPTS ****************************
void awake()
{
    sw1 = !sw1;
} 
//***********************************************************

//************************ MENUS ****************************
void menu0()
{
  
      sprintf(buff, "LETS START");
      DisplayText(0, 10, 0, 1, false);
      sprintf(buff, "Later W");
      DisplayText(0, 30, 0, 1, true);
      sprintf(buff,"Continue B");
      DisplayText(0, 45, DISPLAY_TIME, 1, true);
      bool blue=0, white=0;
      while(menuLevel==0)
      {
            blue = digitalRead(BLUE_BUTTON);
            white = digitalRead(WHITE_BUTTON);
            if(!white)
            {
                  menuLevel = 4; //Door open
                  sprintf(buff,"You say when then");
                  DisplayText(0, 10, DISPLAY_TIME, 1, false);
            }
            else if(!blue)
            {
                  menuLevel = 1;
                  sprintf(buff,"Staring:");
                  DisplayText(0, 10, 0, 1, false);
                  sprintf(buff,"Timer mode");
                  DisplayText(0, 40, DISPLAY_TIME, 2, true);
            }
      }
      display.clearDisplay();
      display.display();
  
}

void setParameter(char* title,byte i)
{
    bool okFlag = 1; //Normally open button, 1 = not pushed, 0 = pushed
    bool increment = 0;
    sprintf(buff,title);
    DisplayText(20, 10, 0, 2, false);
    sprintf(buff,"00");
    DisplayText(20, 40, 0, 2, true);
    while (okFlag == 1)
    {
        delay(350);
        increment = !digitalRead(WHITE_BUTTON);
        okFlag = digitalRead(BLUE_BUTTON);

        if (increment)
            timerTarget[i]++;
        sprintf(buff,title);
        DisplayText(20, 10, 0, 2, false);
        sprintf(buff, "%0d", timerTarget[i]);
        DisplayText(20, 40, 0, 2, true);

    }
}

void menu1() //Program mode 1: Timer -> Set timer
{
      bool okFlag=1; //Normally open button, 1 = not pushed, 0 = pushed
      bool increment=0;
      
      sprintf(buff,"TIMER");
      DisplayText(20, 10, 100, 2, false);
      sprintf(buff,"Set timer");
      DisplayText(0, 47, DISPLAY_TIME/2, 1, true);

      for(uint8_t i = 0; i<3;i++)
      {
            timerTarget[i] = 0;
      }

      setParameter("DAYS:", 0);
      setParameter("HOURS:", 1);
      setParameter("MINUTES:", 2);

      delay(500);
      sprintf(buff,"TIMER");
      DisplayText(20, 10, 100, 2, false);
      sprintf(buff,"Close door");
      DisplayText(0, 47, 0, 1, true);
      bool blue=1;
      while(blue)
      {
        blue = digitalRead(BLUE_BUTTON);
      }
      delay(500);
      sprintf(buff,"OK");
      DisplayText(0, 10, DISPLAY_TIME/3, 1, false);
      sprintf(buff,"Bye");
      DisplayText(15, 20, DISPLAY_TIME/3, 1, false);
      display.clearDisplay();
      display.display();
      closeDoor();
      menuLevel = 2; //Menu = 2 -> door closed, 3 -> door open
      rtc.clearAlarm(1);
      rtc.setAlarm1(rtc.now() + TimeSpan((uint16_t)timerTarget[0],timerTarget[1],timerTarget[2],0),DS3231_A1_Date);
}

void checkTimer()
{
  if(rtc.alarmFired(1))
  {
    openDoor();
    menuLevel=3;
  }
}

void restart()
{
  sprintf(buff, "Star");
  DisplayText(15, 20, DISPLAY_TIME/3, 1, false);
  bool white=1;
  for(int i=0; i<10; i++)
  {
    delay(300);
    white = digitalRead(WHITE_BUTTON);
    if(!white)
      break;
  }
  if(!white)
    menuLevel=0;
}

//************************************************************

/////////////////LOOP/////////////////////////////LOOOP//////////////////
/////////////////LOOP/////////////////////////////LOOOP//////////////////
/////////////////LOOP/////////////////////////////LOOOP//////////////////

void loop() {
      //Menu 1: Timer(blue) Weekend(white)
      if(menuLevel==0) //MENU Start
            menu0();
      if(menuLevel==1) //MENU Program mode 1: Timer -> Set timer
            menu1();

      if(menuLevel==2) //Door closed
            checkTimer();

    
      if(sw1) //To awake screen
      {
            for(int i=0; i<2; i++)
            {
                  if(menuLevel==3)
                    restart();
                  else
                  {
                    DisplayDateAndTime();
                    DisplayTempAndHumidity();                    
                  }
            }
            sw1=!sw1;
            display.clearDisplay();
            display.display();
      }

  
   
}
