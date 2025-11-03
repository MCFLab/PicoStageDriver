#include <Arduino.h>
#include "SensAdjust.h"
#include "pico/stdlib.h"

#define SERIAL_DEBUG  1
#include "Serial_Debug.h"


// *************************************************************************************
// SensAdjust class
// *************************************************************************************

// ----------------------------
// (Private) constructor enforces singleton
// ----------------------------

SensAdjust::SensAdjust() {}


// ----------------------------
// Set up the ADC pin and the moving average filter
// Note: the ADC pin is defined in Common.h as SENSADJUST_PIN_ADC
// ----------------------------

void SensAdjust::init(void)
{
  D_println("SensAdjust Init.");
  analogReadResolution(COMMON_ADC_RESOLUTION);
  pinMode(pin, INPUT);
  // set up the timer
  add_repeating_timer_ms(ADC_UPDATE_INTERVAL_MS, SensAdjust::repeating_timer_callback, NULL, &SensAdjust::timer);
}


// ----------------------------
// Return the singleton instance
// ----------------------------

SensAdjust& SensAdjust::getInstance() {
  static SensAdjust instance;  // Only created once
  return instance;
}


// ----------------------------
// Struct used by the timer callback
// ----------------------------

struct repeating_timer SensAdjust::timer;


// ----------------------------
// Static callback function for the repeating timer
// ----------------------------

bool SensAdjust::repeating_timer_callback(struct repeating_timer *t) {
  SensAdjust::getInstance().update();
  return true;
}


// ----------------------------
// Update the sensor value by reading from the ADC and applying the moving average
// ----------------------------

void SensAdjust::update()
{
  adcAverage.addNewValue(analogRead(pin));
}


// ----------------------------
// Get the updated sensor value
// ----------------------------

int8_t SensAdjust::getUpdatedValue(int32_t &newValue)
{
  if (pin<0) {
    newValue = (1<<COMMON_ADC_RESOLUTION);
    return 0;
  } else {
    uint16_t currentADCValue = adcAverage.getCurrentValue();
    newValue = currentADCValue;
    if (direction < 0) {
      newValue = (1<<COMMON_ADC_RESOLUTION) - newValue; // invert the direction
    }
    if (currentADCValue == lastADCValue) {
      return 0; // no update
    } else {
      lastADCValue = currentADCValue;
      return 1; // value has changed
    }
  }
}
