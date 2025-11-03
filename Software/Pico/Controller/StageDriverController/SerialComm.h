#ifndef SERIALCOMM_H
#define SERIALCOMM_H

#include "Arduino.h"
#include "Common.h"

#include "Parameters.h"
#include "Motors.h"
#include "RemoteComm.h"


// *************************************************************************************
// SerialComm class
// *************************************************************************************

/**
 * @class SerialComm
 * @brief Handles serial communication, command processing, and error reporting for the stage driver controller.
 *
 * This class manages the interface between the controller and external devices via serial communication.
 * It processes incoming commands, interacts with parameter and motor objects, and handles error reporting.
 */
class SerialComm
{
private:
  Parameters *params; // pointer to the Parameters class instance, which holds motor and remote parameters
  Motors *motors; // pointer to the Motors class instance, which manages multiple motors
  RemoteComm *remote; // pointer to the RemoteComm class instance, which handles remote communication
  char errorMsg[MAXERRORSTRINGSIZE]; // error message buffer for storing serial communication errors

public:
  int8_t errorFlag; // error flag to indicate if there is an error in the serial communication

  /**
   * @brief Default constructor for the SerialComm class.
   */
  SerialComm();

  /**
   * @brief Initializes the SerialComm instance with parameters, motors, and remote communication.
   *
   * This method sets up the SerialComm instance by linking it to the provided
   * Parameters, Motors, and RemoteComm instances, allowing it to access motor configurations
   * and remote control settings.
   *
   * @param paramPtr Pointer to the Parameters instance containing motor and remote settings.
   * @param motorPtr Pointer to the Motors instance managing motor operations.
   * @param remotePtr Pointer to the RemoteComm instance handling remote communication.
   * @param timeout_ms Timeout for initialization in milliseconds (default is 1000 ms).
   */
  void Init(Parameters *paramPtr, Motors *motorPtr, RemoteComm *remotePtr, long timeout_ms = 1000);

  /**
   * @brief Checks for incoming serial commands and processes them.
   *
   * This method reads data from the serial port, checks for valid commands, and processes
   * them accordingly. It handles command parsing and command execution.
   */
  void CheckSerialCommand(void);

  /**
   * @brief Helper function to check if a motor is under remote control.
   */
  int8_t CheckRemoteControl(int8_t board);

  /**
   * @brief Sets an error message and flags the error.
   *
   * This method stores an error message in the errorMsg buffer and sets the
   * errorFlag to indicate that an error has occurred in serial communication.
   *
   * @param msg The error message to set.
   */
  void SetErrorMsg(const char *msg);

  /**
   * @brief Prints the error message to serial and clears the error flag.
   *
   * This method checks if an error has occurred, prints the error message to the serial output,
   * and resets the error flag.
   *
   * @return int8_t Returns 1 if there was an error, or 0 if no error is present.
   */
  int8_t PrintErrorMsg(void);

  /**
   * @brief Reports the current error code by printing it to serial.
   *
   * This method prints the provided error code to the serial output in the format "ERROR=<error>".
   * Returned codes are of type ErrorType as defined in Common.h. ERR_None (0) means no error.
   * The error code indocates in which module the error was raised:
   * Serial (-1), Motor (-2), TMC (-3), Parameter (-4), and Remote (-5).
   * 
   * @param error The error code to print. Output format is "ERROR=<error>".
   */
  void ReportErrorCode(int8_t error);

  /**
   * @brief Reports the current error status by printing error messages to serial.
   *
   * This method checks for errors in the SerialComm, Parameters, Motors, and RemoteComm instances,
   * and prints any error messages to the serial output.
   */
  void ReportErrorMsg(void);


};

#endif // SERIALCOMM_H

