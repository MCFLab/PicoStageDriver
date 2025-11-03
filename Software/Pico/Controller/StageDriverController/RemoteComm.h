#ifndef REMOTECOMM_H
#define REMOTECOMM_H

#include <Arduino.h>
#include "Common.h"

#include "Parameters.h"
#include "Motors.h"

// *************************************************************************************
// forward declarations
// *************************************************************************************
class Parameters;
class Motors;


// *************************************************************************************
// RemoteComm class
// *************************************************************************************

/**
 * @class RemoteComm
 * @brief Handles remote communication for motor control, including command processing,
 *        position updates, error handling, and serial communication.
 *
 * This class manages the interface between the controller and remote devices,
 * providing methods for initialization, configuration, command transmission,
 * position reporting, and error management. It ensures safe serial communication
 * and validates data integrity using checksums.
 */
class RemoteComm
{
private:
  Parameters *params; // pointer to the Parameters class instance, which holds motor and remote parameters
  Motors *motors; // pointer to the Motors class instance, which manages multiple motors
  char errorMsg[MAXERRORSTRINGSIZE]; // error message buffer for storing remote error messages
  volatile bool serial1Busy = false; // flag to indicate if Serial1 is busy with a write operation

public:
  int8_t errorFlag; // error flag to indicate if there is an error in the remote communication

  /**
   * @brief Default constructor for the RemoteComm class.
   */
  RemoteComm();

  /**
   * @brief Initializes the RemoteComm instance with parameters and motors.
   *
   * This method sets up the RemoteComm instance by linking it to the provided
   * Parameters and Motors instances, allowing it to access motor configurations
   * and remote control settings.
   *
   * @param paramPtr Pointer to the Parameters instance containing motor and remote settings.
   * @param motorPtr Pointer to the Motors instance managing motor operations.
   * @param timeout_ms Timeout for initialization in milliseconds (default is 1000 ms).
   */
  void Init(Parameters *paramPtr, Motors *motorPtr, long timeout_ms = 1000);

  /**
   * @brief Configures the remote communication settings.
   *
   * This method sends the necessary parameters to the remote device to configure
   * it for operation. It initializes the number of motors and sets up remote parameters.
   *
   * @param motor The motor number to config (-1 for all)
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t Config(int8_t motor);

  /**
   * @brief Sends position updates to the remote device.
   *
   * This method retrieves the current positions of all motors and sends them
   * to the remote device for display.
   */
  void SendPositionUpdates(void);

  /**
   * @brief Sends a remote command to the specified channel with a value.
   *
   * This method constructs a command string and sends it to the remote device
   * via Serial1. It includes a checksum for data integrity.
   *
   * @param cmd The command string to send.
   * @param channel The channel number (motor index) for the command.
   * @param value The value associated with the command.
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t SendRemoteCommand(const char *cmd, int8_t channel, int32_t value);

  /**
   * @brief Checks for incoming remote commands and processes them.
   *
   * This method reads data from Serial1, checks for valid commands, and processes
   * them accordingly. It handles command parsing, checksum validation, and command execution.
   */
  void CheckRemoteCommands(void);
  
  /**
   * @brief Processes a remote command received from Serial1.
   *
   * This method interprets the command string, executes the corresponding action,
   * and updates motor states or parameters as needed.
   *
   * @param cmd The command string to process.
   */  
  void processRemoteCommand(const char *cmd);

   /**
   * @brief Transmits a remote command.
   *
   * This method sends the formatted command string to the remote device
   *
   * @param cmd The command string to transmit.
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t transmitRemoteCommand(const char *cmd);

  /**
   * @brief Safely writes data to Serial1, ensuring no concurrent writes.
   *
   * This method checks if Serial1 is busy before writing data to it. It uses a
   * volatile flag to prevent concurrent write operations, ensuring thread safety.
   *
   * @param data Pointer to the data to write.
   * @param len Length of the data to write.
   */  
  void safeSerial1Write(const char* data, size_t len);

  /**
   * @brief Sets an error message and flags the error.
   *
   * This method stores an error message in the errorMsg buffer and sets the
   * errorFlag to indicate that an error has occurred in remote communication.
   *
   * @param msg The error message to set.
   */
  void SetErrorMsg(const char *msg);
  
  /**
   * @brief Prints the current error message to Serial and clears the error flag.
   *
   * This method outputs the stored error message to Serial for debugging purposes
   * and resets the errorFlag to indicate that the error has been handled.
   *
   * @return int8_t Returns 1 if there was an error, or 0 if no error is present.
   */  
  int8_t PrintErrorMsg(void);

  /**
   * @brief Calculates a checksum for the given data string.
   *
   * This method computes a simple checksum by summing the ASCII values of the characters
   * in the data string. The checksum is used to verify data integrity during communication.
   *
   * @param data Pointer to the data string for which to calculate the checksum.
   * @return uint8_t Returns the calculated checksum value.
   */  
  uint8_t calculateChecksum(const char *data);
  
  /**
   * @brief Validates the checksum of the received data string.
   *
   * This method checks if the checksum in the data string matches the calculated checksum
   * for the data portion of the string. It returns an error code if the checksums do not match.
   *
   * @param data Pointer to the data string containing the checksum to validate.
   * @return int8_t Returns 0 if the checksum is valid, or a negative error code if invalid.
   */  
  int8_t validateChecksum(char *data);

  /**
   * @brief Finds the index and value of a parameter by its name.
   *
   * This method searches for a parameter by its name in the motor parameters array
   * and retrieves its index and value if found.
   *
   * @param board The index of the motor board to search (0 to MAXNUMMOTORS-1).
   * @param name The name of the parameter to find.
   * @param value Reference to an integer to store the found value.
   * @return int8_t Returns 0 on success, or a negative error code if not found.
   */
  int8_t FindParamVal(int8_t board, const char *name, int32_t &value);


  /**
   * @brief Finds the index of a remote parameter by its name.
   * 
   * This function searches for a remote parameter by its name in the remote parameters array
   * for a specific motor board and retrieves its index if found.
   * 
   * @param board The index of the motor board to search (0 to MAXNUMMOTORS-1).
   * @param name The name of the remote parameter to find.
   * @param index Reference to an integer to store the found index.
   * @return int8_t Returns 0 on success, or a negative error code if not found.
   */
  int8_t FindRemoteParamIndex(int8_t board, const char *name, int32_t &index);


  /**
   * @brief Checks if a value is within a specified range.
   * 
   * This function checks if a given value is within the specified minimum and maximum range.
   * 
   * @param value The value to check.
   * @param name The name of the parameter for error reporting.
   * @param min The minimum acceptable value.
   * @param max The maximum acceptable value.
   * @return int8_t Returns 1 if the value is in range, or 0 if it is out of range.
   */
  int8_t IsValueInRange(int32_t value, const char* name, int32_t min, int32_t max);

};

#endif // REMOTECOMM_H

