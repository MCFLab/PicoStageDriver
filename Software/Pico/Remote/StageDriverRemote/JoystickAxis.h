#ifndef JOYSTICKAXIS_H
#define JOYSTICKAXIS_H

#include <Arduino.h>
#include "MovingAverage.h"

/**
 * @brief Represents a joystick axis with ADC input, calibration, and value processing.
 *
 * This class handles reading from an ADC pin, applying a moving average filter,
 * calibrating the center position, applying a dead zone (center margin), and
 * scaling/inverting the output value as needed.
 *
 * Members:
 * - ADCPin: The pin number connected to the ADC for this axis.
 * - maxValue: The maximum value the axis can output after scaling.
 * - direction: Direction multiplier (1 or -1) to invert the axis if needed.
 * - centerMargin: Margin around the center ADC value to treat as zero (dead zone).
 * - centerADCValue: The calibrated center ADC value.
 * - adcAverage: Moving average filter for smoothing ADC readings.
 *
 * Methods:
 * - JoystickAxis(uint8_t ADCPin): Constructor, initializes the axis with the given ADC pin.
 * - void setMaxValue(int32_t value): Sets the maximum output value for the axis.
 * - void setCenterMargin(int16_t value): Sets the dead zone margin around the center.
 * - void setDirection(int32_t dir): Sets the direction multiplier (1 or -1).
 * - void update(): Reads and processes the latest ADC value.
 * - void updateCalibration(): Calibrates the center ADC value.
 * - int8_t getUpdatedValue(int32_t &newValue): Returns the processed value if updated.
 */
class JoystickAxis
{
	uint8_t ADCPin; // pin of the ADC
  int32_t maxValue = 0; // maximum value of the controller parameter
  int32_t direction = 1; // direction multiplier (1 or -1)
  int16_t centerMargin = 0; // ADC readings within this margin yields zero (to avoid imprecise offsets)
  uint16_t centerADCValue = 0; // center ADC value (calibrated)
  int32_t lastVelValue = 0; // last velocity value

  MovingAverage adcAverage = MovingAverage(ADC_AVERAGING_BASE); // moving average of the ADC readings
	
public:
  /**
   * @brief Constructs a JoystickAxis instance with the specified ADC pin.
   *
   * @param ADCPin The pin number connected to the ADC for this axis.
   */
  JoystickAxis(uint8_t ADCPin);

  /**
   * @brief Sets the maximum output value for this axis.
   */
  void setMaxValue(int32_t value);

  /**
   * @brief Sets the dead zone margin around the center ADC value.
   *
   * Values within this margin will be treated as zero.
   *
   * @param value The margin value to set.
   */
  void setCenterMargin(int16_t value);

  /**
   * @brief Sets the direction multiplier for this axis.
   *
   * A value of 1 means normal direction, -1 means inverted.
   *
   * @param dir The direction multiplier (1 or -1).
   */
  void setDirection(int32_t dir);

  /**
   * @brief Reads the ADC value and updates the moving average.
   *
   * This method should be called periodically to keep the moving average updated.
   */
  void update();

  /**
   * @brief Calibrates the center ADC value for this axis.
   *
   * This method should be called once to set the center position based on the current ADC reading.
   */
  void updateCalibration();

  /**
   * @brief Retrieves the current processed value for this axis.
   *
   * This method checks if the ADC value has changed since the last call,
   * applies the dead zone, scales, and inverts the value as necessary.
   *
   * @param newValue Reference to an int32_t where the updated value will be stored.
   * @return 1 if the value has changed, 0 if it remains the same.
   */
  int8_t getUpdatedValue(int32_t &newValue);
};

#endif // ifndef JOYSTICKAXIS