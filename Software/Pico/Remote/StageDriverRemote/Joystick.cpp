#include <Arduino.h>
#include "Joystick.h"
#include "pico/stdlib.h"

#define SERIAL_DEBUG  1
#include "Serial_Debug.h"


// *************************************************************************************
// Joystick class
// *************************************************************************************

// ----------------------------
// Joystick class constructor
// ----------------------------

Joystick::Joystick() {}


// ----------------------------
// Initializes the joystick hardware and state
// ----------------------------

void Joystick::init(void)
{
  size_t idx;

  D_println("Joystick Init.");
  analogReadResolution(COMMON_ADC_RESOLUTION);

  // set up the axes 
  for (idx=0; idx<MAX_NUM_MOTORS; idx++) {
    if (pins[idx]<0) continue; // skip if not enabled
    axis[idx] = new JoystickAxis(pins[idx]);
  }
  // set up the buttons (only every other channel has a button)
  for (idx=0; idx<MAX_NUM_MOTORS/2; idx++) {
    if (buttonPin[2*idx]<0) continue; // skip if not enabled
    pinMode(buttonPin[2*idx], INPUT_PULLUP);  // Use pull-up to default pin HIGH
    attachInterrupt(digitalPinToInterrupt(buttonPin[2*idx]), Joystick::buttonISR, FALLING);  
  }
  // set up the timer
  add_repeating_timer_ms(ADC_UPDATE_INTERVAL_MS, Joystick::repeating_timer_callback, NULL, &Joystick::timer);
}


// ----------------------------
// Returns the singleton instance of the Joystick class
// ----------------------------

Joystick& Joystick::getInstance() {
  static Joystick instance;  // Only created once
  return instance;
}


// ----------------------------
// Returns a flag indicating the presence of te channel
// ----------------------------

int8_t Joystick::isChannelPresent(uint8_t channel)
{
  return (pins[channel]>=0 ? 1 : 0);
}


// ----------------------------
// Static interrupt service routine for handling button presses
// ----------------------------

void Joystick::buttonISR(void)
{
  Joystick& instance = Joystick::getInstance();
  for (int idx=0; idx<MAX_NUM_MOTORS/2; idx++) {
    if (instance.buttonPin[2*idx]>=0) { // only if the button is enabled
      if (digitalRead(instance.buttonPin[2*idx]) == LOW) {
        instance.buttonPressed[2*idx] = 1;
        instance.buttonPressed[2*idx+1] = 1;
      }
    }
  }
}


// ----------------------------
// Struct for the timer calls
// ----------------------------

struct repeating_timer Joystick::timer;


// ----------------------------
// Timer callback for periodic updates
// ----------------------------

bool Joystick::repeating_timer_callback(struct repeating_timer *t) {
  Joystick& instance = Joystick::getInstance();
  instance.update();
//  D_print("x");
  return true;
}


// ----------------------------
// Sets the maximum value for a given axis
// ----------------------------

void Joystick::setMaxValue(uint8_t channel, int32_t value)
{
  if (channel>=MAX_NUM_MOTORS) return; // unsupported channel
  if (pins[channel]>=0) axis[channel]->setMaxValue(value);
}


// ----------------------------
// Sets the center margin for a given axis
// ----------------------------

void Joystick::setCenterMargin(uint8_t channel, int16_t value)
{
  if (channel>=MAX_NUM_MOTORS) return; // unsupported channel
  if (pins[channel]>=0) axis[channel]->setCenterMargin(value);
}


// ----------------------------
// Sets the direction for a given axis
// ----------------------------

void Joystick::setDirection(uint8_t channel, int32_t dir)
{
  if (channel>=MAX_NUM_MOTORS) return; // unsupported channel
  if (pins[channel]>=0) axis[channel]->setDirection(dir);
}


// ----------------------------
// Updates the calibration for a given axis
// ----------------------------

void Joystick::updateCalibration(uint8_t channel)
{
  if (channel>=MAX_NUM_MOTORS) return; // unsupported channel
  if (pins[channel]>=0) axis[channel]->updateCalibration();
}


// ----------------------------
// Updates the joystick state by reading ADC values and updating the axes
// ----------------------------

void Joystick::update()
{
  for (int idx=0; idx<MAX_NUM_MOTORS; idx++) {
    if (pins[idx]>=0) axis[idx]->update();
  }
}


// ----------------------------
// Retrieves the latest value for a given axis
// ----------------------------

int8_t Joystick::getUpdatedValue(uint8_t channel, int32_t &newValue)
{
  if (channel>=MAX_NUM_MOTORS) return 0; // unsupported channel
  if (pins[channel]>=0) return axis[channel]->getUpdatedValue(newValue);
  return 0;
}


// ----------------------------
// Checks if the button for a given channel is pressed
// ----------------------------

int8_t Joystick::getButtonPressed(uint8_t channel)
{
  if (channel>=MAX_NUM_MOTORS) return 0; // unsupported channel
  if (buttonPin[channel]<0) return 0; // not enabled
  return buttonPressed[channel];
}


// ----------------------------
// Resets the button pressed state for a given channel
// ----------------------------

void Joystick::resetButtonPressed(uint8_t channel)
{
  if (channel>=MAX_NUM_MOTORS) return; // unsupported channel
  if (buttonPin[channel]<0) return; // not enabled
  buttonPressed[channel] = 0;
}
