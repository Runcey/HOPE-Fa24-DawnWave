/***************************************************
  This is our GFX example for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Adafruit_FT6206.h>
#include <WiFi.h>
#include "time.h"

const char* ssid     = "DawnWave";
const char* password = "1234";

// For the Adafruit shield, these are the default.
#define TFT_DC 15
#define TFT_CS 16
#define TFT_MOSI 22
#define TFT_CLK 11
#define TFT_RST 17
#define TFT_MISO 23

#define SDA0 6
#define SCL0 7

#define SEALEVELPRESSURE_HPA (1013.25)


// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
// Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// If using the breakout, change pins as desired
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
Adafruit_BME680 bme(&Wire); // I2C

Adafruit_FT6206 ts = Adafruit_FT6206();

bool includeMore;
int more_x;
int more_y;
int lightMenu_x;
int lightMenu_y;
int height;
int width;
int lightBrightness;

void setup() { 
  Serial.begin(9600);
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
  
  Wire.begin(SDA0, SCL0);

  if (!bme.begin(0x76)) {
    tft.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  // Setup ESP32 as a Wifi access point
  //WiFi.softAP(ssid, password);
  setenv("TZ", "PST8PDT,M3.2.0,M11.1.0",1);
  tzset();
  // configTime( -3600, 1*3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org" );
  configTime( 0, 0, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org" );


  ts.begin();
  tft.fillScreen(ILI9341_BLACK);

  more_x = tft.width() / 12;
  more_y = tft.height() / 2;
  lightMenu_x = tft.width() / 12 * 7;
  lightMenu_y = more_y;
  height = tft.height() / 8;
  width = tft.width() / 3;
  mainMenu();
  // Serial.println("orig x: ");
  // Serial.print(x);
  // Serial.print(", orig y: ");
  // Serial.println(y);

  /*
    NOTE: tft and ts have different coordinate systems
      tft (display): (0,0) is top left
      ts (touchscreen control): (0,0) is bottom right
  */
  lightBrightness = 5;
}


void loop(void) {
  Serial.println("Hi!");
  if (! bme.performReading()) {
    tft.println("Failed to perform reading :(");
    return;
  }

  TS_Point p;
  if (ts.touched()) {
    p = ts.getPoint();
    // Serial.println("x: ");
    // Serial.print(p.x);
    // Serial.print(", y: ");
    // Serial.println(p.y);

    // This is for more/less button (just trust me)
    if ((p.x >= lightMenu_x) && (p.x <= lightMenu_x+width) && (p.y <= lightMenu_y) && (p.y >= lightMenu_y-height)) {
      clearButton(more_x, more_y, width, height);
      String text;
      includeMore = !includeMore;
      if (includeMore) {
        text = "Less";
      } else {
        text = "More";
      }
      createButton(more_x, more_y, width, height, text);
    }
    // This is for light settings button
    else if ((p.x >= more_x) && (p.x <= more_x+width) && (p.y <= more_y) && (p.y >= more_y-height)) {
      lightMenu();
    }
  }

  printTime();
  showBME(includeMore);

  delay(3000);
  tft.fillRect(0, 0, tft.width(), tft.height()/2, ILI9341_BLACK);
}

void printTime() {
  tft.setTextColor(ILI9341_WHITE); tft.setTextSize(5);
  tft.setCursor(0, 0);
  struct tm timeinfo;
  getLocalTime(&timeinfo, 0);
  tft.println(&timeinfo, "%H:%M");
}

void showBME(bool include) {
  tft.setTextColor(ILI9341_WHITE); tft.setTextSize(2);
  tft.setCursor(0, 40);

  tft.print("Temperature: ");
  tft.print(bme.temperature);
  tft.println("*C");

  tft.print("Pressure: ");
  tft.print(bme.pressure / 100.0);
  tft.println("hPa");

  tft.print("Humidity: ");
  tft.print(bme.humidity);
  tft.println(" %");

  if (include) {
    tft.print("Gas: ");
    tft.print(bme.gas_resistance / 1000.0);
    tft.println(" KOhms");

    tft.print("Altitude.: ");
    tft.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    tft.println(" m");
  }
  
}

void createButton(int x, int y, int width, int height, String text) {
  
  tft.drawRect(x, y, width, height, ILI9341_WHITE);

  // each character is (size) * 5x8 pixels
  int text_size = (text.length()+1)*10; //length in pixels
  int x_offset = (width-text_size)/2;

  tft.setTextColor(ILI9341_WHITE); 
  tft.setTextSize(2);
  tft.setCursor(x + x_offset, y + height/2 - 8);
  tft.println(text);
}


void clearButton(int x, int y, int width, int height) {
  tft.fillRect(x, y, width, height, ILI9341_BLACK);
}

void mainMenu() {
  includeMore = false;
  createButton(more_x, more_y, width, height, "More");
  createButton(lightMenu_x, lightMenu_y, width, height, "Light");
}

void lightMenu() {
  tft.fillScreen(ILI9341_BLACK);
  bool exit = false;
  TS_Point p;
  int up_x = tft.width() / 4; // top left
  int up_y = tft.height() / 3;
  int down_x = up_x;
  int down_y = tft.height() / 3 * 2; // top left
  int side = tft.width() / 4;

  // set up screen
  tft.fillTriangle(up_x+side/2, up_y, //peak
                  up_x, up_y+side, //bot left
                  up_x+side, up_y+side, ILI9341_WHITE);
  tft.drawRect(up_x, up_y, side, side, ILI9341_WHITE);
  tft.fillTriangle(down_x+side/2, down_y+side, //peak
                  down_x, down_y, //bot left
                  down_x+side, down_y, ILI9341_WHITE);
  tft.drawRect(down_x, down_y, side, side, ILI9341_WHITE);
  createButton(0, 0, tft.width()/4, tft.height()/8, "Back");

  updateBrightnessText();
  while (!exit) {
    if (ts.touched()) {
      p = ts.getPoint();
      if ((p.x >= tft.width()/4*3) && (p.x <= tft.width()) && (p.y <= tft.height()) && (p.y >= tft.height()/8*7)) {
        exit = true;
      //bruh idk the x and y axis are flipped btw display and touchscreen just trust me on this
      } else if ((p.x >= 2*up_x) && (p.x <= 2*up_x+side) && (p.y <= up_y) && (p.y >= up_y-side)) {
        updateBrightness(false);
        delay(500);
      } else if ((p.x >= 2*down_x) && (p.x <= 2*down_x+side) && (p.y <= down_y) && (p.y >= down_y-side)) {
        updateBrightness(true);
        delay(500);
      }
    }
  }
  tft.fillScreen(ILI9341_BLACK);
  mainMenu();
}

void updateBrightness(bool increase) {
  if (increase && (lightBrightness < 10)) {
    lightBrightness++;
    updateBrightnessText();
  } else if (!increase && lightBrightness > 0) {
    lightBrightness--;
    updateBrightnessText();
  }
}

void updateBrightnessText() {
  tft.fillRect(tft.width() / 2, tft.height() / 3, tft.width() / 2, tft.height() / 3, ILI9341_BLACK);
  tft.setCursor(tft.width() / 4 * 3, tft.height()/3 * 2);
  tft.setTextColor(ILI9341_WHITE); 
  tft.setTextSize(5);
  tft.println(lightBrightness);
}

// void writeText() {
//   // int x = tft.width() / 4;
//   // int y = tft.height() / 2;
//   // int height = tft.height() / 4;
//   // int width = tft.width() / 2;

//   tft.setCursor(x + width/4, y + height/2 - 6);
//   tft.println("Boo!");
//   // delay(3000);
// }

unsigned long testTriangles() {
  unsigned long start;
  int           n, i, cx = tft.width()  / 2 - 1,
                      cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  n     = min(cx, cy);
  start = micros();
  for(i=0; i<n; i+=5) {
    tft.drawTriangle(
      cx    , cy - i, // peak
      cx - i, cy + i, // bottom left
      cx + i, cy + i, // bottom right
      tft.color565(i, i, i));
  }

  return micros() - start;
}

unsigned long testFilledTriangles() {
  unsigned long start, t = 0;
  int           i, cx = tft.width()  / 2 - 1,
                   cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  start = micros();
  for(i=min(cx,cy); i>10; i-=5) {
    start = micros();
    tft.fillTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
      tft.color565(0, i*10, i*10));
    t += micros() - start;
    tft.drawTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
      tft.color565(i*10, i*10, 0));
    yield();
  }

  return t;
}