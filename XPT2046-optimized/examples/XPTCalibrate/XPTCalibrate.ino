/*
 * Copyright (c) 2015-2016  Spiros Papadimitriou
 *
 * This file is part of github.com/spapadim/XPT2046 and is released
 * under the MIT License: https://opensource.org/licenses/MIT
 *
 * This software is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied.
 */

#include <Arduino.h>
#include <SPI.h>

#include <ILI9341_t3.h>
#include <XPT2046-t.h>

#include <EEPROM.h>

// Modify the following two lines to match your hardware
#define TFT_DC   15
#define TFT_CS   10
#define TFT_MOSI 11
#define TFT_MISO 12
#define TFT_CLK  13
#define TFT_RST  4
#define TFT_LED  19
#define TOUCH_CS  16
#define TOUCH_IRQ 0

#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0 
#define WHITE    0xFFFF

//Create Display and Touch object
ILI9341_t3 display = ILI9341_t3(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_CLK, TFT_MISO);
XPT2046 touch(TOUCH_CS, TOUCH_IRQ);

static void calibratePoint(uint16_t x, uint16_t y, uint16_t &vi, uint16_t &vj) {
  // Draw cross
  display.drawFastHLine(x - 8, y, 16, WHITE);
  display.drawFastVLine(x, y - 8, 16, WHITE);

  //Wait until touched
  while (!touch.isTouching()) {
    delay(10);
  }

  //Get uncallibrated Raw touchpoint
  touch.getRaw(vi, vj);

  // Erase by overwriting with black
  display.drawFastHLine(x - 8, y, 16, BLACK);
  display.drawFastVLine(x, y - 8, 16, BLACK);
}

void calibrate() {
  uint16_t x1, y1, x2, y2;
  uint16_t vi1, vj1, vi2, vj2;

  //Get calibration-Points
  touch.getCalibrationPoints(x1, y1, x2, y2);
  //Draw and get RAW-Refference
  calibratePoint(x1, y1, vi1, vj1);
  delay(1000);

  //The same again but with new points
  calibratePoint(x2, y2, vi2, vj2);
  touch.setCalibration(vi1, vj1, vi2, vj2);

  //Write results into a buffer
  char buf[80];
  snprintf(buf, sizeof(buf), "%d,%d,%d,%d", (int)vi1, (int)vj1, (int)vi2, (int)vj2);

  //Print it to the Display
  display.setCursor(0, 25);
  display.setTextColor(WHITE);
  display.print("setCalibration params:");
  
  display.setCursor(0, 50);
  display.print(buf);
  
  display.setCursor(0,75);
  display.print("Writing to EEPROM...");

  //Save to EEPROM
  int eepromAdress = 0;
  EEPROM.put(eepromAdress, vi1);
  eepromAdress += sizeof(vi1);

  EEPROM.put(eepromAdress, vj1);
  eepromAdress += sizeof(vj1);

  EEPROM.put(eepromAdress, vi2);
  eepromAdress += sizeof(vi2);

  EEPROM.put(eepromAdress, vj2);

  display.setCursor(0,100);
  display.print("Finish!");

  display.setCursor(0,125);
  display.print("Start drawing");

  delay(3000);
}

void setup() {
  //Activate backlight
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);
  
  delay(1000);

  //Start display and touch
  display.begin();
  touch.begin(display.width(), display.height());

  //Draw 
  display.fillScreen(BLACK);

  //Calibrate
  calibrate();  // No rotation!!

  //Clear Screen
  display.fillScreen(BLACK);
}

void loop() {
  static uint16_t lastX, lastY;

  //If touched than draw
  if(touch.isTouching()){
    uint16_t x, y;

    //Get touched and calibrated position
    touch.getPosition(x, y);

    //Draw a 3x3 cube
    for(int i = -1; i <= 1; i++){
      for(int j = -1; i <= 1; i++){
        display.drawPixel(x+i, y+i, WHITE);
      }
    }

    //Draw line between last touchpoint and current touchpoint
    display.drawLine(x, y, lastX, lastY, WHITE);

    lastX = x;
    lastY = y;
  }
}
