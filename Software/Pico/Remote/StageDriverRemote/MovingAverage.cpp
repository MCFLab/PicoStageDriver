#include <Arduino.h>
#include "MovingAverage.h"


// *************************************************************************************
// MovingAverage class
// *************************************************************************************


// ----------------------------
// Initializes the moving average with the specified window size.
// ----------------------------

MovingAverage::MovingAverage(uint8_t sizeBase) : sizeBase{sizeBase}
{
  size = 1<<sizeBase;
  queue = new uint16_t[size]{0};
}


// ----------------------------
// Destructor for the MovingAverage class.
// ----------------------------

MovingAverage::~MovingAverage()
{
  delete[] queue;
}


// ----------------------------
// This method updates the circular buffer with the new value and adjusts the running sum
// ----------------------------

void MovingAverage::addNewValue(uint16_t val) {
  // calculate the new sum by shifting the window
  uint8_t tail = (head + 1) % size;
  windowSum = windowSum - queue[tail] + val;
  // move the head
  head = (head + 1) % size;
  queue[head] = val;
}


// ----------------------------
// Retrieves the current average value from the moving average window.
// ----------------------------

uint16_t MovingAverage::getCurrentValue()
{
  return windowSum>>sizeBase;
}
