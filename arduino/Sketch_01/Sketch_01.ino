#include "DHT.h"

#define DHTPIN 2     // Digital pin connected to the DHT sensor

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
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


//***************** OLED+Temp **************************
void DisplayTempAndHumidity()
{
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  display.clearDisplay();
  
  display.setTextSize(2);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,10);
  display.print("Humidity");
  display.setCursor(0,40);
  display.print(h);
  display.print(" %");
  display.display();
  delay(5000);
  
  display.clearDisplay();
  display.setCursor(0,10);
  display.print("Temp");
  display.setCursor(0,40);
  display.print(t);
  display.print(" C");
  display.display();
  delay(5000);
}
//******************************************************


//******************* RTC ******************************
//******************************************************


//******************* SERVO ****************************
//******************************************************

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
  
}


void loop() {
  DisplayTempAndHumidity();
}
