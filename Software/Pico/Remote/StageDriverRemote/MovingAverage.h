#ifndef MOVINGAVERAGE_H
#define MOVINGAVERAGE_H

#include <Arduino.h>

/**
 * @class MovingAverage
 * @brief Implements a moving average filter with a fixed-size window.
 *
 * This class maintains a running average of the last N values, where N is a power of two
 * determined by the sizeBase parameter. It efficiently updates the average as new values
 * are added, using a circular buffer and a running sum.
 * This class is optimized for speed: It avoids floating points and divisions
 * and does not adjust for partial averaging. It is designed for unsigned integers.
 * The running sum of the integers cannot exceed 16 bits (uint16).
 * Inspired by the post by Santanu Das ("Moving Average from Data Stream", 
 * (https://medium.com/root-node/moving-average-from-data-stream-774aefb72a2c)  
 *
 * Members:
 * - sizeBase: The base-2 exponent for the window size (i.e., window size = 2^sizeBase).
 * - size: The actual window size (number of values to average).
 * - head: Index of the current position in the circular buffer.
 * - windowSum: The sum of the values currently in the window.
 * - queue: Pointer to the dynamically allocated buffer storing the window values.
 *
 * Methods:
 * - MovingAverage(uint8_t sizeBase): Constructor. Initializes the moving average with the specified window size.
 * - ~MovingAverage(): Destructor. Cleans up allocated resources.
 * - void addNewValue(uint16_t val): Adds a new value to the moving average window.
 * - uint16_t getCurrentValue(): Returns the current average of the values in the window.
 */
class MovingAverage
{
	uint8_t sizeBase; // base-2 exponent for the window size
  uint8_t size; // the actual number of averages (2^sizeBase)
  uint8_t head = 0; // pointer to the current queue position 
	uint16_t windowSum = 0; // running sum
	uint16_t *queue; // queue to store the values
	
public:
  /**
   * @brief Constructs a MovingAverage instance with a specified window size.
   *
   * @param sizeBase The base-2 exponent for the window size (i.e., window size = 2^sizeBase).
   *                 Must be between 1 and 8 (inclusive) to ensure the window size does not exceed 256.
   */
  MovingAverage(uint8_t sizeBase);
  
  /**
   * @brief Destructor for the MovingAverage class.
   *
   * Cleans up the dynamically allocated buffer used for storing the moving average values.
   */
  ~MovingAverage();
  
  /**
   * @brief Adds a new value to the moving average window.
   *
   * This method updates the circular buffer with the new value, adjusts the running sum,
   * and updates the head pointer to maintain the circular nature of the buffer.
   *
   * @param val The new value to be added to the moving average.
   */
  void addNewValue(uint16_t val);
  
  /**
   * @brief Retrieves the current average value from the moving average window.
   */
  uint16_t getCurrentValue();
};

#endif // MOVINGAVERAGE_H