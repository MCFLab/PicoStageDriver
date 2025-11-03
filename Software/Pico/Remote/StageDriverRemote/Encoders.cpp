#include <Arduino.h>
#include "Common.h"

#include "Encoders.h"
#include "quadrature_encoder.pio.h"
#include "SensAdjust.h"
#include "Display.h"

#define SERIAL_DEBUG  1
#include "Serial_Debug.h"


// *************************************************************************************
// Encoders class
// *************************************************************************************
// ----------------------------
// (Private) constructor enforces singleton
// ----------------------------

Encoders::Encoders() {}


// ----------------------------
// Set up the PIOs for each encoder axis and a shared ISR for the buttons
// ----------------------------

void Encoders::init(void)
{
  size_t idx;

  D_println("Encoders Init.");
  // set up the state machines
  // no need for an offset, program loaded at offset 0
  pio_add_program(pio, &quadrature_encoder_program);
  for (idx=0; idx<MAX_NUM_MOTORS; idx++) {
    if (pins[idx]>=0)
    	quadrature_encoder_program_init(pio, sm[idx], pins[idx], 0);
  }

  // set up the buttons
  if (buttonPins[0]>=0) {
    pinMode(buttonPins[0], INPUT_PULLUP);  // Use pull-up to default pin HIGH
    attachInterrupt(digitalPinToInterrupt(buttonPins[0]), Encoders::buttonISR0, FALLING);  
  }
  if (buttonPins[1]>=0) {
    pinMode(buttonPins[1], INPUT_PULLUP);  // Use pull-up to default pin HIGH
    attachInterrupt(digitalPinToInterrupt(buttonPins[1]), Encoders::buttonISR1, FALLING);  
  }
  if (buttonPins[2]>=0) {
    pinMode(buttonPins[2], INPUT_PULLUP);  // Use pull-up to default pin HIGH
    attachInterrupt(digitalPinToInterrupt(buttonPins[2]), Encoders::buttonISR2, FALLING);  
  }
  if (buttonPins[3]>=0) {
    pinMode(buttonPins[3], INPUT_PULLUP);  // Use pull-up to default pin HIGH
    attachInterrupt(digitalPinToInterrupt(buttonPins[3]), Encoders::buttonISR3, FALLING);  
  }

}


// ---------------------------------
// Return the singleton instance
// ---------------------------------

Encoders& Encoders::getInstance() {
  static Encoders instance;  // Only created once
  return instance;
}


// ---------------------------------
// Static interrupt service routines for handling button presses
// ---------------------------------

void Encoders::buttonISR0(void) { Encoders::getInstance().buttonPressed[0] = 1; }
void Encoders::buttonISR1(void) { Encoders::getInstance().buttonPressed[1] = 1; }
void Encoders::buttonISR2(void) { Encoders::getInstance().buttonPressed[2] = 1; }
void Encoders::buttonISR3(void) { Encoders::getInstance().buttonPressed[3] = 1; }


// ----------------------------
// Returns a flag indicating the presence of te channel
// ----------------------------

int8_t Encoders::isChannelPresent(uint8_t channel)
{
  return (pins[channel]>=0 ? 1 : 0);
}


// ---------------------------------
// Set the step size for the encoder
// ---------------------------------

void Encoders::setDirection(uint8_t channel, int32_t dir)
{
  if (channel >= MAX_NUM_MOTORS) return; // unsupported channel
  if (dir != 1 && dir != -1) return; // invalid direction
  direction[channel] = dir;
}


// ---------------------------------
// Set the step size for the encoder
// ---------------------------------

void Encoders::setStepSize(uint8_t channel, int32_t size)
{
  if (channel >= MAX_NUM_MOTORS) return; // unsupported channel
  stepSize[channel] = size;
}


// -----------------------------
// Gets the changed position of the specified encoder channel
// ------------------------------

int8_t Encoders::getChangedPosition(uint8_t channel, int32_t &pos)
{
  static int32_t lastEncPos[MAX_NUM_MOTORS] = {0};
  static int32_t lastPos[MAX_NUM_MOTORS] = {0};
  int32_t encPosition;
  int32_t encChange, posChange;
  int32_t sensVal;

  if (channel >= MAX_NUM_MOTORS) return 0; // unsupported channel
  if (pins[channel]<0) return 0; // not enabled
  // Note: the encoder position is read from the PIO state machine. However, the sensitivity scaling
  // below should only affect the current step size, not the absolute difference from the reference position.
  // Hence, I need to keep track of the last encoder position. 

  encPosition = quadrature_encoder_get_count(pio, sm[channel]);

  SensAdjust::getInstance().getUpdatedValue(sensVal);
  encChange = encPosition - lastEncPos[channel]; // calculate the change in position
  posChange = direction[channel] * stepSize[channel] * encChange
        * sensVal / (1<<COMMON_ADC_RESOLUTION); // scale by the sensitivity    
  pos = lastPos[channel] + posChange + refPosition[channel]; // add the reference position and the last position
  if (posChange) {
    lastEncPos[channel]+=encChange;
    lastPos[channel]+=posChange; // do not include refPosition
    return 1; // value has changed
  } else {
    return 0;
  }
}


// --------------------------------
// Resets the button pressed flag
// --------------------------------

void Encoders::resetEncoderReference(uint8_t channel)
{
  if (channel >= MAX_NUM_MOTORS) return; // unsupported channel
  refPosition[channel] = 0;
  int32_t encPos;
  getChangedPosition(channel, encPos);
  int32_t displayedPos = Display::getInstance().getPosition(channel);
  refPosition[channel] = displayedPos - encPos;
}


// -----------------------------
// Checks if the button for the specified encoder channel was pressed
// -----------------------------

int8_t Encoders::getButtonPressed(uint8_t channel)
{
  if (channel >= MAX_NUM_MOTORS) return 0; // unsupported channel
  if (buttonPins[channel]<0) return 0; // not enabled
  return buttonPressed[channel];
}


// -----------------------------
// Resets the button pressed flag for the specified encoder channel
// -----------------------------

void Encoders::resetButtonPressed(uint8_t channel)
{
  if (channel >= MAX_NUM_MOTORS) return; // unsupported channel
  if (buttonPins[channel]<0) return; // not enabled
  buttonPressed[channel] = 0;
}
