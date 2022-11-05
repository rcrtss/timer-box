
#include <stdlib.h>
/*************************************************************************/

#include "DHT.h"

#define DHTPIN 5     // Digital pin connected to the DHT sensor

#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

/**************************************************************************/

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
#define DISPLAY_TIME 3000
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


//////////////////////////////////////////////////////////////
// ************************* SETUP ***************************
//////////////////////////////////////////////////////////////

const byte interruptPin = 2;
bool sw1=0;

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
  
  //Servo attach
  myservo.attach(9);  
  myservo.write(100); 
  
  //SW1 Interrupt
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), awake, FALLING );
  
  //********************* initializing the rtc ********************
  if(!rtc.begin()) {
      Serial.println("Couldn't find RTC!");
      Serial.flush();
      abort();
  }
  
  /* Uncomment block below to set time when power loss
  if(rtc.lostPower()) {
      // this will adjust to the date and time at compilation
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }*/
    
  //we don't need the 32K Pin, so disable it
  rtc.disable32K();
  // stop oscillating signals at SQW Pin
  rtc.writeSqwPinMode(DS3231_OFF);

  // ***************** OLED WELCOME**********************
    char txt1[10] = "Hola";
  char txt2[10] = "Como tas?";
  
  DisplayText(30, 10, txt1, 4, 0, 2, false);
  DisplayText(0, 40, txt2, 9, DISPLAY_TIME, 2, true);
  display.clearDisplay();
  delay(1000);
  display.display();
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
  
  DisplayText(0, 10, "Humidity", 5, 0, 2, false);
  DisplayText(0,40, h_str, 5, 0, 2, true);
  DisplayText(80,40, " %", 5, DISPLAY_TIME, 2, true);
  
  char t_str[8]; // Buffer big enough for 7-character float
  dtostrf(t, 6, 2, t_str); // Leave room for too large numbers!
  
  DisplayText(0, 10, "Temp", 5, 0, 2, false);
  DisplayText(0,40, t_str, 5, 0, 2, true);
  DisplayText(80,40, " C", 5, DISPLAY_TIME, 2, true);
  
}

void DisplayText(int16_t curX, int16_t curY, char* text, uint8_t textLen, unsigned long displayTime, uint8_t textSize, bool holdPrevious)
{

  if(!holdPrevious)
    display.clearDisplay();
  display.setTextSize(textSize);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(curX,curY);
  display.print(text);
  display.display();
  delay(displayTime);
}

//******************************************************


//******************* RTC ******************************
void DisplayDateAndTime()
{
  // print current time
  char timeStr[10] = "hh:mm AP";
  char dateStr[9] = "DD/MMM/YY";
  uint8_t day;
  rtc.now().toString(timeStr);
  rtc.now().toString(dateStr);
  day = rtc.now().dayOfTheWeek();
  DisplayText(0, 10, dateStr, 9, 0, 2, false);
  DisplayText(0, 40, weekDay[day], 3, 0, 2, true);
  DisplayText(60, 47, timeStr, 5, 2*DISPLAY_TIME, 1, true);
}
//******************************************************


//******************* SERVO ****************************
//******************************************************

void onAlarm() 
{
  //DisplayText(0, 10, "ITS OPEN!", 0, DISPLAY_TIME, 2, false);
}

void awake()
{
  sw1 = !sw1;
} 

void loop() {

  if(sw1) //To awake screen
  {
    for(int i=0; i<3; i++)
    {
      DisplayDateAndTime();
      DisplayTempAndHumidity();
    }
    sw1=!sw1;
    display.clearDisplay();
    display.display();
  }

  if(rtc.now().dayOfTheWeek()>1 && rtc.now().hour()>=16 && rtc.now().minute()>=19)
  {
    bool button1 = 0;
    myservo.write(0);   
    while(!button1)
    { 
      button1 = !digitalRead(2);
      DisplayText(0, 47, "OPEN", 5, 0, 2, false);
    }
      DisplayText(0, 47, "CLOSED", 5, DISPLAY_TIME, 2, false);
  }
   
}
