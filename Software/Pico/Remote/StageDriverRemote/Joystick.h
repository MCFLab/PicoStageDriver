#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Arduino.h>
#include "Common.h"

#include "JoystickAxis.h"


class Joystick
/**
 * @class Joystick
 * @brief Singleton class to manage multiple joystick axes and buttons.
 *
 * This class provides an interface for handling up to MAX_NUM_MOTORS joystick axes and their associated buttons.
 * It manages analog-to-digital conversion (ADC) pins for axis readings and digital pins for button presses.
 * The class supports calibration, direction setting, and value retrieval for each axis, as well as button press detection.
 *
 * Private Members:
 * - JoystickAxis* axis[MAX_NUM_MOTORS]: Array of pointers to JoystickAxis objects, one for each channel.
 * - const int pins[MAX_NUM_MOTORS]: ADC pin numbers for each joystick channel.
 * - const int buttonPin[MAX_NUM_MOTORS]: Digital pin numbers for each joystick button.
 * - volatile int8_t buttonPressed[MAX_NUM_MOTORS]: Flags indicating button press state for each channel.
 * - static struct repeating_timer timer: Timer for periodic updates.
 *
 * Private Methods:
 * - Joystick(): Private constructor for singleton pattern.
 * - static void buttonISR(): Interrupt Service Routine for button presses.
 * - static bool repeating_timer_callback(struct repeating_timer *t): Timer callback for periodic updates.
 * - void update(): Updates joystick state.
 *
 * Public Methods:
 * - static Joystick& getInstance(): Returns the singleton instance.
 * - void init(void): Initializes the joystick hardware and state.
 * - void setMaxValue(uint8_t channel, int32_t value): Sets the maximum value for a given axis.
 * - void setCenterMargin(uint8_t channel, int16_t value): Sets the center margin for a given axis.
 * - void setDirection(uint8_t channel, int32_t dir): Sets the direction for a given axis.
 * - void updateCalibration(uint8_t channel): Updates calibration for a given axis.
 * - int8_t getUpdatedValue(uint8_t channel, int32_t &newValue): Retrieves the latest value for a given axis.
 * - int8_t getButtonPressed(uint8_t channel): Checks if the button for a given channel is pressed.
 * - void resetButtonPressed(uint8_t channel): Resets the button pressed state for a given channel.
 */
{
private:
  JoystickAxis* axis[MAX_NUM_MOTORS]; // JoystickAxis objects for each channel
  const int pins[MAX_NUM_MOTORS] = {JOYSTICK_CH0_PIN_ADC, JOYSTICK_CH1_PIN_ADC, // pins for the ADC
                                    JOYSTICK_CH2_PIN_ADC, JOYSTICK_CH3_PIN_ADC}; // pins for the ADC
  const int buttonPin[MAX_NUM_MOTORS] = {JOYSTICK_CH0_PIN_BUTTON, JOYSTICK_CH1_PIN_BUTTON, // pins for the button
                                         JOYSTICK_CH2_PIN_BUTTON, JOYSTICK_CH3_PIN_BUTTON}; // pins for the button
  volatile int8_t buttonPressed[MAX_NUM_MOTORS] = {0}; // button pressed flag for each channel
  static struct repeating_timer timer; // Timer for periodic updates

  /**
   * @brief Private constructor to enforce the singleton pattern.
   *
   * This constructor is private to prevent instantiation from outside the class.
   * Use the static getInstance() method to access the singleton instance.
   */
  Joystick();
 
  /**
   * @brief Static interrupt service routine for handling button presses.
   */
  static void buttonISR();
 
  /**
   * @brief Timer callback for periodic updates.
   */
  static bool repeating_timer_callback(struct repeating_timer *t); 
 
  /**
   * @brief Updates the joystick state by reading ADC values and updating the axes.
   *
   * This method is called periodically by the timer ISR.
   */
  void update();

public:
 
  /**
   * @brief Retrieves the singleton instance of the Joystick class.
   *
   * This method ensures that only one instance of Joystick exists throughout
   * the application's lifetime. The instance is created on the first call and reused
   * for all subsequent calls.
   *
   * @return Reference to the singleton Joystick instance.
   */
  static Joystick& getInstance();

  /**
   * @brief Initializes the joystick hardware and state.
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
   * @brief Sets the maximum value for a given axis.
   *
   * @param channel The joystick channel (0 to MAX_NUM_MOTORS-1).
   * @param value The maximum value to set.
   */
  void setMaxValue(uint8_t channel, int32_t value);

  /**
   * @brief Sets the center margin for a given axis.
   *
   * Values within this margin will be treated as zero.
   *
   * @param channel The joystick channel (0 to MAX_NUM_MOTORS-1).
   * @param value The margin value to set.
   */
  void setCenterMargin(uint8_t channel, int16_t value);

  /**
   * @brief Sets the direction for a given axis.
   *
   * A value of 1 means normal direction, -1 means inverted.
   *
   * @param channel The joystick channel (0 to MAX_NUM_MOTORS-1).
   * @param dir The direction multiplier (1 or -1).
   */
  void setDirection(uint8_t channel, int32_t dir);

  /**
   * @brief Updates the calibration for a given axis.
   *
   * This method sets the center position based on the current ADC reading.
   *
   * @param channel The joystick channel (0 to MAX_NUM_MOTORS-1).
   */
  void updateCalibration(uint8_t channel);

  /**
   * @brief Retrieves the latest value for a given axis.
   * 
   * @param channel The joystick channel (0 to MAX_NUM_MOTORS-1).
   * @param newValue Reference to store the new value.
   * @return 0 if the value has not changed, 1 if it has changed.
   */
  int8_t getUpdatedValue(uint8_t channel, int32_t &newValue);

  /**
   * @brief Checks if the button for a given channel is pressed.
   *
   * @param channel The joystick channel (0 to MAX_NUM_MOTORS-1).
   * @return 1 if the button is pressed, 0 otherwise.
   */
  int8_t getButtonPressed(uint8_t channel);

  /**
   * @brief Resets the button pressed state for a given channel.
   *
   * @param channel The joystick channel (0 to MAX_NUM_MOTORS-1).
   */
  void resetButtonPressed(uint8_t channel);
};

#endif // JOYSTICK_H