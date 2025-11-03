#ifndef ENCODERS_H
#define ENCODERS_H

#include <Arduino.h>
#include "Common.h"


/**
 * @class Encoders
 * @brief Singleton class to manage multiple rotary encoders and their associated buttons.
 *
 * This class provides an interface for initializing and interacting with up to MAX_NUM_MOTORS rotary encoders,
 * including reading their positions, handling button presses via interrupts, and configuring step size and direction.
 *
 * Private Members:
 * - pio: The PIO instance used for encoder input.
 * - sm: Array of state machines assigned to each encoder channel.
 * - pins: Array of pin numbers for encoder A channel (B channel needs to the next pin number).
 * - buttonPins: Array of pin numbers for encoder button inputs.
 * - buttonPressed: Flags indicating button press state for each channel (volatile for ISR use).
 * - refPosition: Reference positions for each encoder channel (used when switching between encoder and joystick mode).
 * - stepSize: Step size for position increments per channel.
 * - direction: Direction of counting for each channel (1 or -1).
 *
 * Private Methods:
 * - Encoders(): Private constructor to enforce singleton pattern.
 * - buttonISR(): Static interrupt service routine for handling button presses.
 *
 * Public Methods:
 * - getInstance(): Returns the singleton instance of the Encoders class.
 * - init(): Initializes the encoders and configures hardware resources.
 * - getChangedPosition(): Returns the change in position for a given channel.
 * - resetEncoderReference(): Resets the reference position for a specified channel.
 * - getButtonPressed(): Returns the button pressed state for a specified channel.
 * - resetButtonPressed(): Resets the button pressed flag for a specified channel.
 * - setDirection(): Sets the counting direction for a specified channel.
 * - setStepSize(): Sets the step size for position increments for a specified channel.
 */
class Encoders
{
private:
  PIO pio = pio0; // Pico PIO to use (deafult pio0)
  const uint sm[MAX_NUM_MOTORS] = {0, 1, 2, 3}; // state machines to use
  const int pins[MAX_NUM_MOTORS] = {ENCODER_CH0_PIN_ENCA, ENCODER_CH1_PIN_ENCA, // pins for EncA
                                    ENCODER_CH2_PIN_ENCA, ENCODER_CH3_PIN_ENCA}; // pins for EncA
  const int buttonPins[MAX_NUM_MOTORS] = {ENCODER_CH0_PIN_BUTTON, ENCODER_CH1_PIN_BUTTON, // pins for the button
                                          ENCODER_CH2_PIN_BUTTON, ENCODER_CH3_PIN_BUTTON}; // pins for the button
  volatile int8_t buttonPressed[MAX_NUM_MOTORS] = {0}; // button pressed flag for each channel
  int32_t refPosition[MAX_NUM_MOTORS] = {0}; // reference position for each channel
  int32_t stepSize[MAX_NUM_MOTORS] = {1, 1, 1, 1}; // step size for the position
  int32_t direction[MAX_NUM_MOTORS] = {1, 1, 1, 1}; // step direction (1 or -1)

  /**
   * @brief Private constructor to enforce the singleton pattern.
   *
   * This constructor is private to prevent instantiation from outside the class.
   * Use the static getInstance() method to access the singleton instance.
   */
  Encoders();   // Private constructor enforces singleton
  
  /**
   * @brief Static interrupt service routine for handling button presses.
   */
  static void buttonISR0();  // interrupt servoce routine for button press

  /**
   * @brief Static interrupt service routine for handling button presses.
   */
  static void buttonISR1();  // interrupt servoce routine for button press

  /**
   * @brief Static interrupt service routine for handling button presses.
   */
  static void buttonISR2();  // interrupt servoce routine for button press

  /**
   * @brief Static interrupt service routine for handling button presses.
   */
  static void buttonISR3();  // interrupt servoce routine for button press


public:

  /**
   * @brief Retrieves the singleton instance of the Encoders class.
   *
   * This method ensures that only one instance of Encoders exists throughout
   * the application's lifetime. The instance is created on the first call and reused
   * for all subsequent calls.
   *
   * @return Reference to the singleton Encoders instance.
   */
  static Encoders& getInstance();   // Access to the singleton

  /**
   * @brief Initializes the encoders and configures hardware resources.
   *
   * This method sets up the PIOs for each encoder axis, initializes the state machines,
   * and configures the button pins for interrupt handling.
   */
  void init(void);

  /**
   * @brief Checks if the channel is defined (if the corresponding pins are defined)
   * 
   * @param channel The quieried channel (0 to MAX_NUM_MOTORS-1).
   * @return 1 if the channel is present, 0 if not.
   */
  int8_t isChannelPresent(uint8_t channel);

  /**
   * @brief Gets the changed position of the specified encoder channel.
   *
   * @param channel The encoder channel to check (0 to MAX_NUM_MOTORS-1).
   * @param pos Reference to store the current position.
   * @return 0 if unchanged, 1 if changed.
   */
  int8_t getChangedPosition(uint8_t channel, int32_t &pos);

  /**
   * @brief Resets the reference position for the specified encoder channel.
   *
   * @param channel The encoder channel to reset (0 to MAX_NUM_MOTORS-1).
   */
  void resetEncoderReference(uint8_t channel);

  /**
   * @brief Checks if the button for the specified encoder channel was pressed.
   * 
   * @param channel The encoder channel to check (0 to MAX_NUM_MOTORS-1).
   * @return 1 if the button was pressed, 0 otherwise.
   */
  int8_t getButtonPressed(uint8_t channel);

  /**
   * @brief Resets the button pressed flag for the specified encoder channel.
   *
   * @param channel The encoder channel to reset (0 to MAX_NUM_MOTORS-1).
   */
  void resetButtonPressed(uint8_t channel);

  /**
   * @brief Sets the counting direction for the specified encoder channel.
   *
   * @param channel The encoder channel to set (0 to MAX_NUM_MOTORS-1).
   * @param dir The direction to set (1 for forward, -1 for reverse).
   */
  void setDirection(uint8_t channel, int32_t dir);

  /**
   * @brief Sets the step size for position increments for the specified encoder channel.
   *
   * @param channel The encoder channel to set (0 to MAX_NUM_MOTORS-1).
   * @param stepSize The step size to set (positive integer).
   */
  void setStepSize(uint8_t channel, int32_t stepSize);

};

#endif // ENCODERS_H
