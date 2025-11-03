#ifndef DISPLAY_H
#define DISPLAY_H


#include <Arduino.h>
#include "Common.h"
#include <Adafruit_SSD1306.h>

class Display
/**
 * @class Display
 * @brief Singleton class for managing an Adafruit_SSD1306 display and tracking motor positions.
 *
 * This class provides an interface to initialize and interact with an SSD1306 display,
 * as well as to store and retrieve the last known positions for multiple motors.
 *
 * Private Members:
 * - lastPosition: Array storing the last position for each motor channel.
 * - disp: Pointer to the Adafruit_SSD1306 display object.
 * - Display(): Private constructor to enforce singleton pattern.
 *
 * Public Methods:
 * - getInstance(): Returns the singleton instance of the Display class.
 * - init(): Initializes the display.
 * - setPosition(channel, position): Sets the position for a specific motor channel.
 * - getPosition(channel): Retrieves the last known position for a specific motor channel.
 * - clear(): Clears the display.
 */
{
private:
  int32_t lastPosition[MAX_NUM_MOTORS] = {0}; // last position displayed for each channel
  Adafruit_SSD1306 *disp; // Pointer to the display object

  /**
   * @brief Private constructor to enforce the singleton pattern.
   */
  Display();


public:
  /**
   * @brief Retrieves the singleton instance of the Display class.
   *
   * This method ensures that only one instance of Display exists throughout
   * the application's lifetime. The instance is created on the first call and reused
   * for all subsequent calls.
   *
   * @return Reference to the singleton Display instance.
   */
  static Display& getInstance();
  
  /**
   * @brief Initializes the display.
   *
   * This method sets up the Adafruit_SSD1306 display, clears it, and configures
   * the text size and color.
   */
  void init(void);

  /**
   * @brief Sets the position for a specific motor channel.
   *
   * @param channel The motor channel (0 to MAX_NUM_MOTORS-1).
   * @param position The position value to set.
   */
  void setPosition(uint8_t channel, int32_t position);
  
  /**
   * @brief Retrieves the last known position for a specific motor channel.
   *
   * @param channel The motor channel (0 to MAX_NUM_MOTORS-1).
   * @return The last known position for the specified channel.
   */
  int32_t getPosition(uint8_t channel);

  /**
   * @brief Clears the display.
   *
   * This method clears the entire display and resets the cursor position.
   */
  void clear(void);
};

#endif // DISPLAY_H