#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <Arduino.h>
#include "Common.h"

#include "Motors.h"
#include "RemoteComm.h"


// *************************************************************************************
// forward declarations
// *************************************************************************************
class Motors;
class RemoteComm;


// *************************************************************************************
// Structure definitions
// *************************************************************************************
/**
 * @struct HWParamStruct
 * @brief Structure to hold hardware motor parameters and configuration settings.
 * 
 * This structure contains the number of motors defined, their types, and the chip select pins for each motor driver.
 * It is used to store and manage the configuration of motors in the system.
 */
struct HWParamStruct {
  MotorType motorType[MAXNUMMOTORS] = MOTORS_DEFAULT_DEV_TYPE;
  int8_t DRIVER_CS[MAXNUMMOTORS] = MOTORS_DEFAULT_DRIVER_CS;
  AxisType axisType[MAXNUMMOTORS] = MOTORS_DEFAULT_AX_TYPE;
};


// *************************************************************************************
// Parameters class
// *************************************************************************************


/**
 * @class Parameters
 * @brief Manages configuration parameters for motor controllers and remote communication.
 *
 * This class encapsulates the storage, retrieval, and management of parameters related to motor drivers and remote communication interfaces.
 * It provides methods to initialize, configure, and persist parameter sets, as well as to access and modify individual parameter values.
 *
 * @note Parameter identifiers for motors and remote communication are defined as static constexpr arrays for easy reference.
 *
 */
class Parameters
{
private:
  char errorMsg[MAXERRORSTRINGSIZE]; // error message buffer for storing parameter-related errors
  Motors *motors; // pointer to the Motors class instance, which manages multiple motors
  RemoteComm *remote; // pointer to the RemoteComm class instance, which handles remote communication

public:

  /**
   * @brief List of parameter IDs for motor configuration.
   *
   * This array defines string identifiers for each configurable parameter of the motor driver.
   * The array size is defined by MOTORS_NUM_PARAMS in Common.h.
   */
  static constexpr const char* motParamsIDList[MOTORS_NUM_PARAMS] = {
      // CurrentParams
      "CSCA",    // Scale: overall scale factor [0 (full scale, or 32..255)]
      "CRAN",    // Range: 0->1A, 1->2A, 2-> 3A, 3-> 3A
      "CRUN",    // Run: scale factor for operating current [0..31]
      "CHOL",    // Hold: scale factor for holding current [0..31]
      // ModeParams
      "MMIC",    // MICrosteps: step size in  2^MMIC MS (0-> native 256 MS, 8-> full step)
      "MINV",    // INVert direction: inverts the axis direction, sets the shaft parameter
      "MTOF",    // TOff: off time (0-> driver disabled, set to 5 otherwise)
      "MSGE",    // SG Enable: flag to enable stallGuard2
      "MSGT",    // SG Threshold for stallguard
      "MTCT",    // TCoolThres (lower limit for stallguard velocity)
      // HomingParams
      "HMOD",    // MODe: Mode for the homing search (0->disabled, 1->limits, 2->index)
      "HDIR",    // DIRection: determines the homing direction (-1->neg, 1->pos)
      "HVEL",    // VELocity
      "HSST",    // Use Soft STop
      "HNEV",    // index(N) EVent: 0->event during pos N, 1-> event on N rising edge, 2-> falling, 3->any edge
      // RateParams
      "RSEV",    // SEt Vel: units are internal (~4/3 microsteps/s)
      "RMXV",    // MaX Vel: units are internal (~4/3 microsteps/s)
      "RSEA",    // SEt Acc: internal units (~(4/3)^2 microsteps/s^2)
      "RMXA",    // MaX Acc: internal units (~(4/3)^2 microsteps/s^2)
      // EncoderParams
      "ECON",    // CONstant: sets the encoder resolution, sign sets the direction (0 not present)
      "EDEV",    // DEViation: max deviation between encoder and motor position before flag is raised
      "ETOL",    // TOLerance: tolerance window for closed-loop operation
      "EMAX",    // MAX iterations : max number of pull-in tries for closed-loop operation
      "ERST",    // ReSeT X after closed-loop move
      // SwitchParams
      "SLEN",    // Left ENabled 
      "SREN",    // Right ENabled 
      "SLPO",    // Left POlarity: polarity: 1-> high level stops motor; 0-> low level stops motor
      "SRPO",    // RightPOlarity: polarity: 1-> high level stops motor; 0-> low level stops motor
      "SSWP",    // Swap: 1->swap left and right role
      // LimitsParam
      "LENC",    // ENCoder mode (use encoder, rather than XACT for the virtual limits)  
      "LLEN",    // Left ENabled  
      "LREN",    // Right ENabled 
      "LLPS",    // Left PoSition 
      "LRPS"     // Right PoSition 
  };

  /**
   * @brief List of parameter IDs for remote communication configuration.
   *
   * This array defines string identifiers for each configurable parameter of the remote controller.
   * The array size is defined by REMOTE_NUM_PARAMS in Common.h.
   */
  static constexpr const char* remoteIDList[REMOTE_NUM_PARAMS] = {
      "ENAB",     // remote enabled
      "JDIR",     // joystick direction
      "JMAX",     // joystick max val
      "EDIR",     // encoder direction
      "ESTP"      // encoder step size
  };

  int8_t errorFlag; // error flag to indicate if an error has occurred
  HWParamStruct hwParameters; // structure to hold motor parameters and configuration settings
  int32_t motorParamArr[MAXNUMMOTORS][MOTORS_NUM_PARAMS]; // array to hold motor parameters for each motor board
  int32_t remoteParamArr[MAXNUMMOTORS][REMOTE_NUM_PARAMS]; // array to hold remote parameters for each motor board

  /**
   * @brief Default constructor for the Parameters class.
   */
  Parameters();

  /**
   * @brief Destructor for the Parameters class.
   */
  ~Parameters();
  
  /**
   * @brief Initializes the Parameters instance with motors and remote communication pointers.
   * @param motorPtr Pointer to the Motors instance containing motor configurations.
   * @param remotePtr Pointer to the RemoteComm instance managing remote communication.
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t Init(Motors *motorPtr, RemoteComm *remotePtr);

  /**
   * @brief Configures the parameters based on the specified configuration type.
   * 
   * This method initializes the parameters based on the configuration type provided.
   * It can load default values, reconfigure existing settings, or load from flash memory.
   * 
   * @param confType The type of configuration to apply (CONFIG_DEFAULT, CONFIG_RECONFIG, CONFIG_LOAD_FROM_FLASH).
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t Config(ConfigType confType);

  /**
   * @brief Saves the current configuration to flash memory.
   * 
   * This method persists the current motor and remote parameters to flash memory for later retrieval.
   * 
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t SaveConfigToFlash(void);

  /**
   * @brief Sets the type of device for a specific motor board.
   * 
   * This method configures the type of motor driver (TMC, SIM, or None) for the specified board.
   * 
   * @param board The index of the motor board to configure (0 to MAXNUMMOTORS-1).
   * @param value The device type to set (0 for None, 1 for SIM, 2 for TMC).
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t SetDeviceType(int8_t board, int32_t value);

  /**
   * @brief Gets the type of device for a specific motor board.
   * 
   * This method retrieves the type of motor driver configured for the specified board.
   * 
   * @param board The index of the motor board to query (0 to MAXNUMMOTORS-1).
   * @param value Reference to an integer to store the retrieved device type.
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t GetDeviceType(int8_t board, int32_t &value);

  /**
   * @brief Sets the type of axis for a specific motor board.
   * 
   * This method configures the type of axis (UNDEF, X, Y, Z, or Aux) for the specified board.
   * These values are used to identify the role of the motor in the system (used with Micro-Manager).
   * 
   * @param board The index of the motor board to configure (0 to MAXNUMMOTORS-1).
   * @param value The device type to set (0->Undef, 1->X, 2->Y, 3->Z, 4->Aux).
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t SetAxisType(int8_t board, int32_t value);

  /**
   * @brief Gets the axis type of device for a specific motor board.
   * 
   * This method retrieves the type of axis (UNDEF, X, Y, Z, or Aux) for the specified board.
   * These values are used to identify the role of the motor in the system (used with Micro-Manager).
   * 
   * @param board The index of the motor board to query (0 to MAXNUMMOTORS-1).
   * @param value Reference to an integer to store the retrieved axis type.
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t GetAxisType(int8_t board, int32_t &value);

  /**
   * @brief Checks if a motor board is valid.
   * @param board The index of the motor board to check (0 to MAXNUMMOTORS-1).
   */
  int8_t IsValidMotor(int8_t board);

  /**
   * @brief Checks if a motor board is active (within the defined number of motors).
   * @param board The index of the motor board to check (0 to MAXNUMMOTORS-1).
   * @param raiseError 0 (default)-> no error message if not an active motor, 1->raise err.
   * @return int8_t Returns 1 if the board is active, or 0 if it is not.
   */
  int8_t IsActiveMotor(int8_t board, int8_t raiseError=0);

  /**
   * @brief Sets a specific motor parameter for a given board.
   * 
   * This method updates the motor parameters for the specified board index.
   * 
   * @param board The index of the motor board to configure (0 to MAXNUMMOTORS-1).
   * @param index The index of motParamsIDList[index] of the parameter to set.
   * @param value The value to set for the specified parameter.
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t SetMotorParams(int8_t board, int8_t index, int32_t value);

  /**
   * @brief Gets the current motor parameters for a specific board.
   * 
   * This method retrieves the motor parameters for the specified board index.
   * 
   * @param board The index of the motor board to query (0 to MAXNUMMOTORS-1).
   * @param index The index of motParamsIDList[index] of the parameter to retrieve.
   * @param value Reference to an integer to store the retrieved parameter value.
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t GetMotorParams(int8_t board, int8_t index, int32_t &value);

  /**
   * @brief Sets a remote parameter for a specific board.
   * 
   * This method updates the remote parameters for the specified board index.
   * 
   * @param board The index of the motor board to configure (0 to MAXNUMMOTORS-1).
   * @param index The index of remoteIDList[index] of the remote parameter to set.
   * @param value The value to set for the specified remote parameter.
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t SetRemoteParams(int8_t board, int8_t index, int32_t value);

  /**
   * @brief Gets the current remote parameters for a specific board.
   * 
   * This method retrieves the remote parameters for the specified board index.
   * 
   * @param board The index of the motor board to query (0 to MAXNUMMOTORS-1).
   * @param index The index of remoteIDList[index] of the remote parameter to retrieve.
   * @param value Reference to an integer to store the retrieved remote parameter value.
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t GetRemoteParams(int8_t board, int8_t index, int32_t &value);

  /**
   * @brief Reteieve a pointer to the motor parameters for a specific board.
   * @param board The index of the motor board to query (0 to MAXNUMMOTORS-1).
   */
  int32_t* GetMotorParamPtr(int8_t board);

  /**
   * @brief Retrieve a pointer to the hardware parameters.
   * 
   * This method provides access to the HWParamStruct containing motor parameters.
   * It allows other components to directly manipulate or read the motor configuration settings.
   * This way we don't have to call the getter function for each parameter
   *
   * @return Pointer to the HWParamStruct containing motor parameters.
   */
  HWParamStruct* GetHWParamPtr();

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

};

#endif // PARAMETERS_H
