#include <Arduino.h>
#include "Common.h"

#ifndef CONTROLLERCOMM_H
#define CONTROLLERCOMM_H


class ControllerComm
/**
 * @class ControllerComm
 * @brief Manages communication and control logic for the stage driver controller.
 *
 * The ControllerComm class implements a singleton pattern to ensure a single instance
 * manages all communication with the controller hardware. It provides methods for
 * sending and receiving updates, handling button interrupts, and managing control modes
 * for multiple motors. The class supports both remote and joystick control modes,
 * and includes mechanisms for command processing, checksum validation, and encoder reference reset.
 *
 * Private members include configuration for the number of motors, control mode tracking,
 * and button press state. Public methods allow for initialization, communication,
 * input mode checks, and interrupt handling.
 */
{
private:
  int8_t isRemoteControlled[MAX_NUM_MOTORS] = {0}; // flag whether axis is remotely controlled
  int8_t isJoystickControlled[MAX_NUM_MOTORS] = {0}; // flag whether axis is controlled by joystick

  /**
   * @brief Private constructor to enforce the singleton pattern.
   *
   * This constructor is private to prevent instantiation from outside the class.
   * Use the static getInstance() method to access the singleton instance.
   */
  ControllerComm();

  /**
   * @brief Sends a command to the controller.
   *
   * This method formats the command string, appends a checksum, and sends it over UART.
   *
   * @param cmd The command string to send.
   */
  void sendCommand(const char *cmd);

  /**
   * @brief Processes a command received from the controller.
   *
   * This method interprets the command string and performs the corresponding action.
   *
   * @param cmd The command string to process.
   * @return An integer indicating the result of the command processing (0 for success, -1 for failure).
   */
  int8_t processCommand(const char *cmd);

  /**
   * @brief Resets the encoder reference position for a specified channel.
   *
   * This method sets the reference position for the specified encoder channel to the last displayed position.
   *
   * @param idx The index of the encoder channel (0 to MAX_NUM_MOTORS-1).
   */
  void resetEncoderReference(int8_t idx);

  /**
   * @brief Validates the checksum of the input string.
   *
   * This method checks if the checksum in the input string matches the calculated checksum.
   * The expected format of the input string is "<command|checksum>".
   *
   * @param data The input string containing the command and checksum.
   * @return 0 if the checksum is valid, -1 if it is invalid.
   */
  int8_t validateChecksum(char *data);

  /**
   * @brief Calculates the checksum for a given data string.
   *
   * This method computes an 8-bit checksum by summing the ASCII values of the characters in the data string.
   * The checksum is used to verify the integrity of the command received from the controller.
   *
   * @param data The input string for which to calculate the checksum.
   * @return The calculated checksum as an 8-bit unsigned integer.
   */
  uint8_t calculateChecksum(const char *data);

public:

  /**
   * @brief Retrieves the singleton instance of the ControllerComm class.
   *
   * This method ensures that only one instance of ControllerComm exists throughout
   * the application's lifetime. The instance is created on the first call and reused
   * for all subsequent calls.
   *
   * @return Reference to the singleton ControllerComm instance.
   */
  static ControllerComm& getInstance();   // Access the singleton

  /**
   * @brief Initializes the controller communication.
   *
   * Sets up the necessary resources and configurations for communication.
   *
   * @param timeout_ms Timeout in milliseconds for initialization (default is 1000 ms).
   */
  void init(long timeout_ms = 1000);

  /**
   * @brief Sends a list of command strings to the controller.
   *
   * This method is periodically called to communicate with the controller.
   */
  void sendUpdatesToController(void);

  /**
   * @brief Receives updates from the controller.
   *
   * This method checks for incoming commands from the controller and processes them.
   */
  void receiveUpdatesFromController(void);

  /**
   * @brief Checks for a switch in input mode (e.g., joystick vs. encoder).
   *
   * This method monitors the input mode and updates the control modes accordingly.
   */
  void inputModeCheck(void);

};

#endif // CONTROLLERCOMM_H

