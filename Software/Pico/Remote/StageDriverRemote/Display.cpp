#include <Arduino.h>
#include "Common.h"

#include "Display.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SERIAL_DEBUG  1
#include "Serial_Debug.h"

#define OLED_RESET     -1 // No reset pin 

// *************************************************************************************
// Display class
// *************************************************************************************
// ----------------------------
// // (Private) constructor enforces singleton
// ----------------------------

Display::Display() {}


// ----------------------------
// Return the singleton instance
// ----------------------------

Display& Display::getInstance() {
  static Display instance;  // Only created once
  return instance;
}


// ----------------------------
// Initialize the display
// ----------------------------

void Display::init(void)
{
  D_println("Display Init.");
  disp = new Adafruit_SSD1306(DISPLAY_SCREEN_WIDTH, DISPLAY_SCREEN_HEIGHT, &Wire, OLED_RESET);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!disp->begin(SSD1306_SWITCHCAPVCC, DISPLAY_I2C_ADDRESS)) { // calls Wire.begin with default Wire parameters
    D_println("SSD1306 allocation failed");
    for(;;); // Don't proceed, loop forever
  }
  disp->clearDisplay();
  disp->setTextSize(1);
  disp->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
}


// ----------------------------
// Set the position for a specific motor channel
// ----------------------------

void Display::setPosition(uint8_t channel, int32_t position)
{
  if (channel >= MAX_NUM_MOTORS) return; // unsuppored chanel
  lastPosition[channel] = position; // store the last position for this channel
  disp->setCursor(0, 8*channel);
  for (int i = 0; i < (disp->width() / 6); ++i) disp->print(' ');
  disp->setCursor(0, 8*channel);
  disp->printf("Ch%hhu: %i", channel, position);
  disp->display();
}


// ----------------------------
// Get the last known position for a specific motor channel
// ----------------------------

int32_t Display::getPosition(uint8_t channel)
{
  if (channel >= MAX_NUM_MOTORS) return 0; // unsuppored chanel
  return (lastPosition[channel]);
}


// ----------------------------
// Clear the display
// ----------------------------

void Display::clear(void)
{
  disp->clearDisplay();
}

