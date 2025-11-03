#ifndef SENSADJUST_H
#define SENSADJUST_H

#include <Arduino.h>
#include "Common.h"
#include "MovingAverage.h"


/**
 * @class SensAdjust
 * @brief Singleton class for handling sensor adjustment via ADC input.
 *
 * This class manages reading from an ADC pin, applies a moving average filter,
 * and provides updated sensor values. It supports direction inversion and uses
 * a repeating timer for periodic updates.
 *
 * Private Members:
 * - pin: ADC pin number (constant).
 * - adcAverage: Moving average filter for ADC readings.
 * - direction: Direction multiplier (1 or -1).
 * - lastADCValue: Last returned ADC value.
 * - timer: Static repeating timer for periodic updates.
 *
 * Private Methods:
 * - SensAdjust(): Private constructor for singleton pattern.
 * - repeating_timer_callback(): Static callback for the repeating timer.
 * - update(): Updates the sensor value.
 *
 * Public Methods:
 * - getInstance(): Returns the singleton instance.
 * - init(): Initializes the sensor adjustment system.
 * - getUpdatedValue(): Retrieves the latest sensor value and indicates if it has changed.
 */
class SensAdjust
{
private:
  const int pin = SENSADJUST_PIN_ADC; // pin for the ADC
  MovingAverage adcAverage = MovingAverage(ADC_AVERAGING_BASE); // moving average filter for the ADC readings
  int32_t direction = 1; // direction multiplier (1 or -1)
  int16_t lastADCValue = 0; // last returned ADC value
  static struct repeating_timer timer; // repeating timer for periodic updates

  /**
   * @brief Private constructor to enforce the singleton pattern.
   */
  SensAdjust();

  /**
   * @brief Static callback function for the repeating timer.
   *
   * This function is called periodically to update the sensor value.
   *
   * @param t Pointer to the repeating timer structure.
   * @return true if the timer should continue, false otherwise.
   */
  static bool repeating_timer_callback(struct repeating_timer *t); 

  /**
   * @brief Updates the sensor value by reading from the ADC and applying the moving average.
   *
   * This method reads the current ADC value, adds it to the moving average filter,
   * and updates the lastADCValue. It is called periodically by the timer ISR.
   */
  void update();

public:
  /**
   * @brief Retrieves the singleton instance of the SensAdjust class.
   *
   * This method ensures that only one instance of SensAdjust exists throughout
   * the application's lifetime. The instance is created on the first call and reused
   * for all subsequent calls.
   *
   * @return Reference to the singleton SensAdjust instance.
   */
  static SensAdjust& getInstance();   // Access the singleton

  /**
   * @brief Initializes the sensor adjustment system.
   *
   * This method sets up the repeating timer for periodic updates and initializes
   * the moving average filter.
   */
  void init(void);

  /**
   * @brief Retrieves the updated sensor value.
   *
   * This method reads the current ADC value, applies the direction multiplier,
   * and returns the updated value. It also indicates whether the value has changed
   * since the last call.
   *
   * @param newValue Reference to store the updated sensor value.
   * @return 1 if the value has changed, 0 if not.
   */
  int8_t getUpdatedValue(int32_t &newValue);

};

#endif // SENSADJUST_H