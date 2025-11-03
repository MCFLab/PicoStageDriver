#ifndef MOTORS_H
#define MOTORS_H

#include "Common.h"
#include "Parameters.h"


// *************************************************************************************
// forward declarations
// *************************************************************************************
class TMC; // forward declaration
class Parameters; // forward declaration


// *************************************************************************************
// Motors class
// *************************************************************************************

/**
 * @class Motors
 * @brief Manages and controls multiple stepper motors, providing configuration, status, and motion control.
 *
 * This class encapsulates the logic for initializing, configuring, and controlling an array of stepper motors.
 * It provides methods for moving motors, querying and setting status, handling errors, and managing remote control state.
 *
 */
class Motors
{
private:
  Parameters *params; // pointer to the Parameters class instance
  char errorMsgGeneral[MAXERRORSTRINGSIZE]; // genral error message string
  char errorMsgBoard[MAXNUMMOTORS][MAXERRORSTRINGSIZE]; // board-related error message string

public:
  TMC *tmcArr; // array of TMC objects, one for each motor

  /**
   * @brief List of motor status identifier strings.
   *
   * This static constexpr array contains string identifiers for various motor status parameters:
   * - "XACT": Actual position of the motor.
   * - "XTAR": Target position of the motor.
   * - "XENC": Encoder position of the motor.
   * - "VELO": Current velocity of the motor.
   * - "ACCE": Current acceleration of the motor.
   * - "ENAB": Motor enable status.
   * - "TEMP": Motor temperature [in deg C].
   * - "PULL": last number of pull-in tries.
   *
   * The array size is determined by MOTORS_NUM_STATUS in Common.h.
   */
  static constexpr const char* motStatIDList[MOTORS_NUM_STATUS] = {
      "XACT",     // actual position
      "XTAR",     // target position
      "XENC",     // encoder position
      "VELO",     // current velocity
      "ACCE",     // current acceleration
      "ENAB",     // motor enable
      "TEMP",     // motor temperature [in deg C]
      "PULL"      // last number of pull-in tries
    };

  int8_t errorFlag = 0; // error flag to indicate if any error has occurred
  int8_t errorFlagGeneral = 0; // error flag to indicate if a general error has occurred
  int8_t errorFlagBoard[MAXNUMMOTORS] = {0}; // error flag to indicate if a board-specific error has occurred
  int8_t isMotorEnabled[MAXNUMMOTORS] = {0}; // array to track if each motor is enabled (1) or not (0)
  int8_t isRemoteControlled[MAXNUMMOTORS] = {0}; // array to track if each motor is controlled remotely (1) or serial (0, default)
  
  int8_t isMotorMoving[MAXNUMMOTORS] = {0}; // array to track if each motor is currently moving (1) or not (0)
  int8_t isMotorHoming[MAXNUMMOTORS] = {0}; // array to track if each motor is currently homing (1) or not (0)
  
  // The following vars are for closed loop operation. Some are just copies of parameters, but storing them here saves time
  int8_t isMotorSearching[MAXNUMMOTORS] = {0}; // array to track if each motor is currently in closed loop searching mode (1) or not (0)
  int32_t targetPosition[MAXNUMMOTORS] = {0}; // target position for closed loop motion
  int32_t setPosition[MAXNUMMOTORS] = {0}; // current set position for closed loop motion
  int32_t iterationsLeft[MAXNUMMOTORS] = {0}; // iteration counter for closed loop motion, counts backwards to 0

  // int32_t encConst[MAXNUMMOTORS] = {0}; // encoder constant (0 means no encoder present)
  // int32_t maxIterations[MAXNUMMOTORS] = {0}; // maximum number of iterations
  // int32_t tolerance[MAXNUMMOTORS] = {0}; // tolerance for closed loop motion
  // int32_t resetXafterCL[MAXNUMMOTORS] = {0}; // flag whether to reset X after a closed-loop move

  
  /**
   * @brief Default constructor for the Motors class.
   */
  Motors();

  /**
   * @brief Destructor for the Motors class.
   */
  ~Motors();

  /**
   * @brief Initializes the Motors class with a pointer to the Parameters instance.
   *
   * This method sets up the Motors class with the provided Parameters instance,
   * which contains configuration and status information for the motors.
   *
   * @param paramPtr Pointer to the Parameters instance.
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t Init(Parameters *paramPtr);

  // /**
  //  * @brief Configures the motors based on the current parameters.
  //  *
  //  * This method initializes and configures each motor according to the parameters defined in the Parameters class.
  //  * It sets up the TMC drivers and prepares the motors for operation.
  //  *
  //  * @return int8_t Returns 0 on success, or a negative error code on failure.
  //  */
  // //int8_t Config(void);

  /**
   * @brief Configures a motor board.
   * 
   * @param board The index of the motor board to configure (-1 for all).
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t ConfigBoard(int8_t board);

  /**
   * @brief Processes updates and changes for all motors.
   *
   * This method is called periodically to check the status of the motors, handle errors,
   * and update the motor states based on their current configuration and motion.
   */
  void ProcessUpdateChanges(void);

  /**
   * @brief Clears the status registers.
   * 
   * This function clears the GSTAT, ENC_STATUS, and RAMPSTAT registers.
   * 
   * @param board The index of the motor board to move (0 to MAXNUMMOTORS-1).
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t ClearStatusRegs(int8_t board);

  /**
   * @brief Moves a motor at a specified velocity.
   *
   * @param board The index of the motor board to move (0 to MAXNUMMOTORS-1).
   * @param velocity The desired velocity for the motor.
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t MoveAtVel(int8_t board, int32_t velocity);

  /**
   * @brief Moves a motor to a specified position.
   *
   * @param board The index of the motor board to move (0 to MAXNUMMOTORS-1).
   * @param pos The target position in microsteps.
   * @param setVel Flag whether the velocity should be set.
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t MoveToPos(int8_t board, int32_t pos, int setVel);

  /**
   * @brief Gets the current position of a motor.
   *
   * @param board The index of the motor board to query (0 to MAXNUMMOTORS-1).
   * @param pos Reference to an integer to store the current position in microsteps.
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t GetPos(int8_t board, int32_t &pos);

  /**
   * @brief Sets whether a motor is controlled remotely or via serial.
   *
   * @param board The index of the motor board to set (-1 for all motors).
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t SetRemoteEnabled(int8_t board, int8_t state);

  /**
   * @brief Starts the homing process for a specific motor.
   *
   * @param board The index of the motor board to home (0 to MAXNUMMOTORS-1).
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t StartHoming(int8_t board);

  /**
   * @brief Gets the current status of the motor driver status parameters.
   *
   * @param board The index of the motor board to set (0 to MAXNUMMOTORS-1).
   * @param idx Index for the array motStatIDList, defined in Motors.h.
   * This array is a list of acepted status IDs (short strings), also used during serial comm.
   * @param intVal Integer to set the status to.
   * @return int8_t Returns 0 on success, or a negative error code on failure. 
   */
  int8_t SetStatusValue(int8_t board, int idx, int32_t intVal);

  /**
   * @brief Gets the current status of the motor driver status parameters.
   *
   * @param board The index of the motor board to query (0 to MAXNUMMOTORS-1).
   * @param idx Index for the array motStatIDList, defined in Motors.h.
   * This array is a list of acepted status IDs (short strings), also used during serial comm.
   * @param intVal Reference to an integer to store the retrieved status.
   * @return int8_t Returns 0 on success, or a negative error code on failure. 
   */
  int8_t GetStatusValue(int8_t board, int idx, int32_t &intVal);

  /**
   * @brief Sets a specific register value in the TMC driver.
   *
   * @param board The index of the motor board to set (0 to MAXNUMMOTORS-1).
   * @param address The register address to set.
   * @param value The value to set in the register.
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t SetRegisterValue(int8_t board, uint8_t address, int32_t value);

  /**
   * @brief Gets the value of a specific register in the TMC driver.
   *
   * @param board The index of the motor board to query (0 to MAXNUMMOTORS-1).
   * @param address The register address to read.
   * @param value Reference to an integer to store the retrieved register value.
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t GetRegisterValue(int8_t board, uint8_t address, int32_t &value);

  /**
   * @brief Checks the status of the TMC driver.
   *
   * This function reads the status flags and updates the motor's moving state.
   * It also checks if the motor has completed its motion. This should be done during a move.
   *
   * @param board The index of the motor board to check (0 to MAXNUMMOTORS-1).
   * @param isMotionDone Reference to an integer to indicate if the motion is done (1 for done, 0 for not done).
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t GetStatusFlags(int8_t board, int32_t &status);

  /**
   * @brief Checks if the motion is complete for a specific motor or all motors.
   * 
   * @param board The index of the motor board to check (0 to MAXNUMMOTORS-1) or -1 for all motors.
   * @param done Reference to an integer to indicate if the motion is done (1 for done, 0 for not done).
   */
  int8_t IsMotionDone(int8_t board, int32_t &done);

//TODO
  /**
   * @brief Checks if a motor board is valid.
   * @param board The index of the motor board to check (0 to MAXNUMMOTORS-1).
   */
//  int8_t IsValidMotor(int8_t board);

  /**
   * @brief Checks if a motor board is active (within the defined number of motors).
   * @param board The index of the motor board to check (0 to MAXNUMMOTORS-1).
   * @return int8_t Returns 1 if the board is active, or 0 if it is not.
   */
//  int8_t IsActiveMotor(int8_t board);

  /**
   * @brief Sets an error message and flags the error.
   *
   * This method stores an error message in the errorMsg buffer and sets the
   * errorFlag to indicate that an error has occurred in motor module.
   *
   * @param source The source of the error: "Board"->Motors.cpp or "TMC"->TMC.cpp.
   * @param num The motor number for the error (or -1 for general).
   * @param msg The error message to set.
   */

  void SetErrorMsg(const char *source, int8_t num, const char *msg);

  /**
   * @brief Prints the error message to serial and clears the error flag.
   *
   * This method checks if an error has occurred, prints the error message to the serial output,
   * and resets the error flag.
   *
   * @return int8_t Returns 1 if there was an error, or 0 if no error is present.
   */
  int8_t PrintErrorMsg(void);
};

#endif // MOTORS_H
