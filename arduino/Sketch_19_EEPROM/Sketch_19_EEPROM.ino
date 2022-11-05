#include <stdlib.h>
#include <EEPROM.h>
/*************************************************************************/

#include "DHT.h"

#define DHTPIN 5     // Digital pin connected to the DHT sensor

#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


//************************* OLED ******************************************
// Test for minimum program size.
// Edit AVRI2C_FASTMODE in SSD1306Ascii.h to change the default I2C frequency.

#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

// Define proper RST_PIN if required.
#define RST_PIN -1

SSD1306AsciiAvrI2c oled;
#define DISPLAY_TIME 2000


/**************************************************************************/

#include <RTClib.h>

RTC_DS3231 rtc;

// the pin that is connected to SQW of DS3231
#define CLOCK_INTERRUPT_PIN 3
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
char buff[32];
uint8_t openDay;
char openClock[10] = "hh:mm AP";
char openCalendar[10] = "DD/MMM/YY";
char weekDay[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
//********EEPROM VARS**********
int eeAddress=0;
bool wasSet=0;
uint16_t yearEEPROM;
uint8_t timerEEPROM[5];
#define BLUE_BUTTON     2
#define WHITE_BUTTON    3

void setup() 
{
  Serial.begin(9600);
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

  //Uncomment block below to set time when power loss
  /*
  if(rtc.lostPower()) {
      // this will adjust to the date and time at compilation
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  */
  
  //we don't need the 32K Pin, so disable it
  rtc.disable32K();
  // stop oscillating signals at SQW Pin
  rtc.writeSqwPinMode(DS3231_OFF);


  //////////////////OLED/////////////////
#if RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif // RST_PIN >= 0
  // Call oled.setI2cClock(frequency) to change from the default frequency.


  ////////////////////////////////////////
  
  //////////////////READ EEPROM///////////////// 
  eeAddress=0;
  EEPROM.get(eeAddress,wasSet);
  //wasSet=0;//Manually set wasSet. change to 0 to reset timer by code, then comment again
  Serial.print("Was Set Status = ");
  Serial.println(wasSet);
  if(wasSet)
  {
    eeAddress += sizeof(bool);
    EEPROM.get(eeAddress,yearEEPROM);
    eeAddress += sizeof(uint16_t);
    EEPROM.get(eeAddress,timerEEPROM[0]);
    eeAddress += sizeof(uint8_t);
    EEPROM.get(eeAddress,timerEEPROM[1]);
    eeAddress += sizeof(uint8_t);
    EEPROM.get(eeAddress,timerEEPROM[2]);
    eeAddress += sizeof(uint8_t);
    EEPROM.get(eeAddress,timerEEPROM[3]);
    eeAddress += sizeof(uint8_t);
    EEPROM.get(eeAddress,timerEEPROM[4]);
    rtc.clearAlarm(1);
    DateTime dt(yearEEPROM,timerEEPROM[0],timerEEPROM[1],timerEEPROM[2],timerEEPROM[3],timerEEPROM[4]);
    rtc.setAlarm1(dt,DS3231_A1_Date);
    dt.toString(openClock);
    dt.toString(openCalendar);
    openDay = dt.dayOfTheWeek();
    menuLevel=2; //Door closed
    Serial.print("Date that was set: ");
    Serial.print(yearEEPROM);
    Serial.print("/");
    Serial.print(timerEEPROM[0]);
    Serial.print("/");
    Serial.print(timerEEPROM[1]);
    Serial.print("  ");
    Serial.print(timerEEPROM[2]);
    Serial.print(":");
    Serial.print(timerEEPROM[3]);
    Serial.print(":");
    Serial.println(timerEEPROM[4]);
  }
  
  // ***************** OLED WELCOME**********************
  oled.clear();
  oled.setFont(Cooper19);
  oled.print("  R-Cortes \n  Inc.");
  delay(2000);
  oled.clear();
}

//////////////////////FUNCTIONS//////////////////////////////////////////////////FUNCTIONS////////////////////////////
////////////////////////////////////////////////////////FUNCTIONS/////////////////////////////////////////////////////
//////////////////////FUNCTIONS//////////////////////////////////////////////////FUNCTIONS////////////////////////////

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
      oled.clear();
      oled.setFont(TimesNewRoman16_italic);
      oled.print("\n Humidity: ");
      oled.print(h_str);
      oled.print(" %\n\n");
  
      char t_str[8]; // Buffer big enough for 7-character float
      dtostrf(t, 6, 2, t_str); // Leave room for too large numbers!
      oled.print(" Temp:      ");
      oled.print(t_str);
      oled.print(" C");
      delay(2*DISPLAY_TIME);  
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
    
    oled.clear();
    oled.setFont(Arial_bold_14);
    oled.println(" Time:"); 
    oled.setFont(TimesNewRoman16_italic);
    oled.print("\n");
    oled.println(dateStr); 
    oled.print("\n");
    oled.print(weekDay[day]);
    oled.print("    ");
    oled.print(timeStr);
    delay(2*DISPLAY_TIME);
}
void DisplayOpeningDate()
{
    oled.clear();
    oled.setFont(Arial_bold_14);
    oled.println(" Will open on:"); 
    oled.setFont(TimesNewRoman16_italic);
    oled.print("\n");
    oled.println(openCalendar);
    oled.print(weekDay[openDay]);
    oled.print("    ");
    oled.print(openClock);
    delay(2*DISPLAY_TIME);
}
//******************************************************


//******************* SERVO ****************************

void openDoor()
{
      myservo.attach(9);
      myservo.write(0);
      oled.clear();
      oled.print("Timer ended");
      delay(2000);
      myservo.detach();
      oled.clear();
      wasSet=0;
      EEPROM.put(0,wasSet);
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
  
      oled.clear();
      oled.setFont(Arial_bold_14);
      oled.println("Lets Start\n");
      oled.setFont(System5x7);
      oled.println("Later (White)\n");
      oled.print("Continue (Blue)");
      delay(2000);
      
      bool blue=0, white=0;
      while(menuLevel==0)
      {
            blue = digitalRead(BLUE_BUTTON);
            white = digitalRead(WHITE_BUTTON);
            if(!white)
            {
                  menuLevel = 3; //Door open
                  oled.clear();
                  oled.print("You say when then");
                  delay(2000);
            }
            else if(!blue)
            {
                  menuLevel = 1;
                  oled.clear();
                  oled.setFont(Arial_bold_14);
                  oled.println("Staring:\n");
                  oled.setFont(System5x7);
                  oled.print("Timer mode");
                  delay(2000);
            }
      }
      oled.clear();
  
}

void setParameter(char* title,byte i)
{
    bool okFlag = 1; //Normally open button, 1 = not pushed, 0 = pushed
    bool increment = 0;
    oled.clear();
    oled.setFont(Arial_bold_14);
    oled.print("\n\n    ");
    oled.print(title);
    oled.print("  0");
    while (okFlag == 1)
    {
        delay(350);
        increment = !digitalRead(WHITE_BUTTON);
        okFlag = digitalRead(BLUE_BUTTON);

        if (increment)
        {
            if(i==0 && timerTarget[i]==42)
            {
              timerTarget[i]=0;
            }
            else if(i==1 && timerTarget[i]==23)
            {
              timerTarget[i]=0;
            }
            else if(i==2 && timerTarget[i]==59)
            {
              timerTarget[i]=0;
            }
            else
            {
              timerTarget[i]++;
            }
        }
        oled.clear();
        oled.setFont(Arial_bold_14);
        oled.print("\n\n    ");
        oled.print(title);
        oled.print("  ");
        oled.print(timerTarget[i]);
    }
}

void menu1() //Program mode 1: Timer -> Set timer
{
      bool okFlag=1; //Normally open button, 1 = not pushed, 0 = pushed
      bool increment=0;
      
      oled.clear();
      oled.setFont(Arial_bold_14);
      oled.println("\n\n      Set timer..");
      delay(DISPLAY_TIME/2);

      for(uint8_t i = 0; i<3;i++)
      {
            timerTarget[i] = 0;
      }

      setParameter("   Days  :", 0);
      setParameter("   Hours:", 1);
      setParameter("   Mins  :", 2);

      delay(500);
      oled.clear();
      oled.setFont(Arial_bold_14);
      oled.println("Timer\n");
      oled.setFont(System5x7);
      oled.println("Close door and");
      oled.print("press Blue");
      delay(DISPLAY_TIME/2);
      
      bool blue=1;
      while(blue)
      {
        blue = digitalRead(BLUE_BUTTON);
      }
      delay(500);
      oled.clear();
      oled.setFont(Arial_bold_14);
      oled.println("\n\n           ok");
      delay(DISPLAY_TIME/2);
      oled.clear();
      oled.println("\n\n          bye.");
      delay(DISPLAY_TIME/2);
      oled.clear();
      closeDoor();
      menuLevel = 2; //Menu = 2 -> door closed, 3 -> door open
      rtc.clearAlarm(1);
      DateTime openTime = rtc.now() + TimeSpan((uint16_t)timerTarget[0],timerTarget[1],timerTarget[2],0);
      rtc.setAlarm1(openTime,DS3231_A1_Date);

      openTime.toString(openClock);
      openTime.toString(openCalendar);
      openDay = openTime.dayOfTheWeek();

      //Write vars for EEPROM
      eeAddress = 0;
      wasSet=1;
      yearEEPROM=openTime.year();
      Serial.print("The year set was: ");
      Serial.println(yearEEPROM);
      timerEEPROM[0]=openTime.month();
      timerEEPROM[1]=openTime.day();
      timerEEPROM[2]=openTime.hour();
      timerEEPROM[3]=openTime.minute();
      timerEEPROM[4]=0;
      EEPROM.put(eeAddress,wasSet);
      eeAddress += sizeof(bool);
      EEPROM.put(eeAddress,yearEEPROM);
      eeAddress += sizeof(uint16_t);
      EEPROM.put(eeAddress,timerEEPROM[0]);
      eeAddress += sizeof(uint8_t);
      EEPROM.put(eeAddress,timerEEPROM[1]);
      eeAddress += sizeof(uint8_t);
      EEPROM.put(eeAddress,timerEEPROM[2]);
      eeAddress += sizeof(uint8_t);
      EEPROM.put(eeAddress,timerEEPROM[3]);
      eeAddress += sizeof(uint8_t);
      EEPROM.put(eeAddress,timerEEPROM[4]);
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
  oled.clear();
  oled.setFont(Arial_bold_14);
  oled.println("Start Again? (W)");
  delay(DISPLAY_TIME/3);
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
//------------------------------------------------------------------------------
void loop() 
{
  
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
              {
                restart();
                break;
              }
              else
              {
                DisplayDateAndTime();
                DisplayOpeningDate();
                DisplayTempAndHumidity();
              }
        }
        sw1=!sw1;
        oled.clear();
  }
}
