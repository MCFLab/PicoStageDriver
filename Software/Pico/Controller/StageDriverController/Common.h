#ifndef COMMON_H
#define COMMON_H

#define VERSION 1 // integer to indicate the version number; used to make sure the flash info matches
                  // increment if any changes are made to the parameters structure

// *************************************************************************************
// Typedefs
// *************************************************************************************

/**
 * @enum ErrorType
 * @brief Enumeration of error types for the motor controller.
 */
typedef enum {
  ERR_None = 0,
  ERR_Serial = -1,
  ERR_Motor = -2,
  ERR_TMC = -3,
  ERR_Parameter = -4,
  ERR_Remote = -5
} ErrorType;

/**
 * @enum ConfigType
 * @brief Enumeration of configuration types for the motor controller.
 */
typedef enum {
  CONFIG_DEFAULT = 0,
  CONFIG_RECONFIG,
  CONFIG_LOAD_FROM_FLASH
} ConfigType;

/**
 * @enum MotorType
 * @brief Enumeration of motor types for the motor controller.
 */
typedef enum {
  MOTOR_NONE = 0,
  MOTOR_SIM,
  MOTOR_TMC
} MotorType;

/**
 * @enum AxisType
 * @brief Enumeration of axis types for the motor controller.
 */
typedef enum {
  AXIS_UNDEF = 0,
  AXIS_X,
  AXIS_Y,
  AXIS_Z,
  AXIS_AUX
} AxisType;

// *************************************************************************************
// General defines
// *************************************************************************************

#define MAXNUMMOTORS                      4 // maximum number of motors supported
#define MAXERRORSTRINGSIZE                200 // maximum size of the error string
#define DEFAULT_STARTUP_PIN               D15 // pin for the button to start the controller with default parameters
                                             // press the button during startup to use default parameters


// *************************************************************************************
// Module-specific defines
// *************************************************************************************

#define PRARAMETERS_FLASH_SIZE            1024 // size of the flash memory for the parameters in bytes

#define SERIAL_BAUDRATE                   115200 // baudrate for the serial communication
#define SERIAL_TERMCHAR                   0xA  // termination char can be 0xA (LF) or 0xD (CR)
#define SERIAL_ID_STRING                  "Stage Driver Pico" // ID string for the serial communication
#define SERIAL_CHECK_INTERVAL_MS          20 // check interval for serial commands in ms

#define REMOTE_ENABLED                    1 // set to 0 if no remote present (turns off UART communication)
#define REMOTE_NUM_PARAMS                 5 // number of parameters in Parameters::remoteIDList
//#define REMOTE_BAUDRATE                   115200 // (just for testing)
#define REMOTE_BAUDRATE                   921600 // UART baudrate for the remote communication
#define REMOTE_UART_BUFFER_SIZE           1024 // size of the UART buffer for the remote communication
#define REMOTE_PIN_TX                     PIN_SERIAL1_TX // TX pin for the remote communication
#define REMOTE_PIN_RX                     PIN_SERIAL1_RX // RX pin for the remote communication
#define REMOTE_SEND_INTERVAL_MS           200 // interval in ms to send commands to the remote controller
#define REMOTE_RECEIVE_INTERVAL_MS        10 // interval in ms to receive commands from the remote controller

#define MOTORS_NUM_PARAMS                 34 // number of parameters in Parameters::motParamsIDList
#define MOTORS_NUM_STATUS                 8 // number of status parameters in Motors::motStatIDList
#define MOTORS_CHECK_ERROR_INTERVAL_MS    50 // check interval in ms for motor errors
#define MOTORS_CHECK_STATUS_INTERVAL_MS   10 // check interval in ms for motor status
#define MOTORS_DEFAULT_DEV_TYPE           {MOTOR_SIM, MOTOR_SIM, MOTOR_NONE, MOTOR_NONE} // default device types for the motors
#define MOTORS_DEFAULT_AX_TYPE            {AXIS_X, AXIS_Y, AXIS_Z, AXIS_AUX} // default axis types for the motors
#define MOTORS_DEFAULT_DRIVER_CS          {22, 21, 20, 17} // default CS pins for the motors, -1 means no driver

#endif // COMMON_H