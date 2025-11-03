#include <Arduino.h>
#include "Common.h"

#include "JoystickAxis.h"
#include "SensAdjust.h"

#define SERIAL_DEBUG  1
#include "Serial_Debug.h"


// *************************************************************************************
// JoystickAxis class
// *************************************************************************************

// ----------------------------
// Constructor for the JoystickAxis class.
// ----------------------------

JoystickAxis::JoystickAxis(uint8_t ADCPin) : ADCPin{ADCPin}
{
  pinMode(ADCPin, INPUT);
}


// ----------------------------
// Sets the maximum output value for this axis.
// ----------------------------

void JoystickAxis::setMaxValue(int32_t value)
{
  maxValue = value;
}


// ----------------------------
// Sets the dead zone margin around the center ADC value.
// ----------------------------

void JoystickAxis::setCenterMargin(int16_t value)
{
  centerMargin = value;
}


// ----------------------------
// Sets the direction multiplier for this axis.
// ----------------------------

void JoystickAxis::setDirection(int32_t dir)
{
  if (dir != 1 && dir != -1) {
    D_println("Invalid direction value, must be 1 or -1.");
    return; // invalid direction
  }
  direction = dir;
}


// ----------------------------
// Sets the current ADC value as the Calibration for this axis.
// ----------------------------

void JoystickAxis::updateCalibration()
{
  centerADCValue = adcAverage.getCurrentValue();
}


// ----------------------------
// Updates the ADC value and adds it to the moving average.
// ----------------------------

void JoystickAxis::update()
{
  if (ADCPin < 0) return; // not enabled
  adcAverage.addNewValue(analogRead(ADCPin));
}


// ----------------------------
// Gets the updated value from the channel
// ----------------------------

int8_t JoystickAxis::getUpdatedValue(int32_t &newValue)
{
  int32_t r; // ADC distance from center
  int32_t sensVal;

  uint16_t currentADCValue = adcAverage.getCurrentValue();
  r = currentADCValue - centerADCValue;
  if (abs(r) < centerMargin) {
    r=0;
  }
  // scale by the sensitivity
  SensAdjust::getInstance().getUpdatedValue(sensVal);
  newValue = 2*direction*maxValue*r/(1<<COMMON_ADC_RESOLUTION)
              *sensVal/(1<<COMMON_ADC_RESOLUTION);

  if (newValue==lastVelValue) {
    return 0; // no update
  } else {
    lastVelValue = newValue;
    return 1; // value has changed
  }
}
