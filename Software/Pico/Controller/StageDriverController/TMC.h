#ifndef TMC_H
#define TMC_H

#include "Common.h"
#include "Parameters.h"


// *************************************************************************************
// defines
// *************************************************************************************
#define TMC_HOMING_STANDSTILL_TIMEOUT_MS  1000 // max time to wait for motor to stop after switch is reached during homing  
#define TMC_OVERTEMP_PREWARN              0xB92 // ADC value corresponding to 120 deg C
                                                // ADC = 7.7 * temp_C + 2038    


// *************************************************************************************
// forward declarations
// *************************************************************************************
class Motors; // forward declaration


// *************************************************************************************
// Structure definitions
// *************************************************************************************
/**
 * @struct TMCSimStatus
 * @brief Represents the simulation status of a TMC motor controller.
 *
 * This structure holds the current state and parameters for a simulated TMC motor,
 * including parameter values, actual and target positions, encoder position, velocity,
 * and the timestamp of the last velocity calculation.
 *
 */
struct TMCSimStatus {
//  int32_t params[MOTORS_NUM_PARAMS];
  int32_t xact;
  int32_t xtar;
  int32_t xenc;
  int32_t xmin;
  int32_t xmax;
  int32_t vel;
  unsigned long lastVelCalcTime = 0;
};


// *************************************************************************************
// TMC class
// *************************************************************************************

/**
 * @class TMC
 * @brief Controller class for managing TMC motor drivers.
 *
 * This class provides an interface for initializing, configuring, and controlling TMC motor drivers.
 * It supports operations such as moving the motor, querying and setting parameters, handling homing routines,
 * and accessing driver status and registers.
 * The class is designed to work with a specified number of motors and their associated parameters.
 */
class TMC
{
private:
  int8_t board; // ech instance gets this board number (from 0 to MAXNUMMOTORS-1) to pass into the TMC functions 
  Motors* motors; // pointer to the Motors class instance (parent), which manages multiple motors
  HWParamStruct *hwParam; // pointer to the HW parameters structure, which contains general settings for the motors
  int32_t *motorParam; // pointer to the motor parameters array, which is defined in the Parameters class
  // a subset of motorParam is copied here for faster access
  TMCSimStatus simValues; // simulation values for TMC motor controller

/**
 * @brief Sets an error message for the TMC controller.
 * 
 * This function passes the error message to the Motors class instance.
 * @param msg The error message to set.
 */
  void SetErrorMsg(const char *msg);

public:
  int32_t encConst, maxIterations, tolerance, resetXafterCL; // closed-loop parameters

  /**
   * @brief Default constructor for the TMC class.
   */
  TMC();

  /**
   * @brief Destructor for the TMC class.
   */
  ~TMC();

  /**
   * @brief Initializes the TMC controller but doesn't configre the boards.
   * 
   * @param tNum The motor number to initialize (0 to MAXNUMMOTORS-1).
   * @param motorsPtr Pointer to the Motors class instance managing the motors.
   * @param mParam Pointer to the array of motor parameters.
   * @param param Pointer to the HWParamStruct containing general settings for the motors.
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t Init(int8_t tNum, Motors *motorsPtr, int32_t *mParam, HWParamStruct *param);

  /**
   * @brief Configures the TMC controller based on the current motor parameters.
   * 
   * This function sets up the TMC driver with the parameters defined in the motorParam array.
   * 
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t Config(void);

  /**
   * @brief Clears the status registers.
   * 
   * This function clears the GSTAT, ENC_STATUS, and RAMPSTAT registers.
   * 
   * @return int8_t Returns 0 (for consistency).
   */
  int8_t ClearStatusRegs(void);

  /**
   * @brief Moves the motor at a specified velocity.
   * 
   * This function sets the motor to move at a given velocity. The velocity is specified in microsteps per second,
   * and the sign of the velocity determines the direction of movement.
   * 
   * @param velocity The desired velocity in microsteps per second (negative for reverse direction).
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t MoveAtVel(int32_t velocity);

  /**
   * @brief Moves the motor to a specified position.
   * 
   * This function sets the motor to move to a specific position. The position is specified in microsteps.
   * 
   * @param pos The target position in microsteps.
   * @param setVel Flag to indicate whether the velocity should be set (as opposed to just the position).
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t MoveToPos(int32_t pos, int8_t setVel);

  /**
   * @brief Sets the X positions to the specified position without moving the motor.
   * 
   * This function sets the X positions to a specific position. The position is specified in microsteps.
   * This functionality is used to reset the positions to the encoder positions after closed-loop moves.
   * 
   * @param pos The target position in microsteps.
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t SetXPos(int32_t pos);

  /**
   * @brief Gets the current position X_ACT of the motor.
   * 
   * @param pos Reference to an integer for the current position in microsteps.
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t GetPos(int32_t &pos);

  /**
   * @brief Gets the current encoder position (X_ENC) of the motor.
   * 
   * @param pos Reference to an integer for the current position in microsteps.
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t GetEnc(int32_t &pos);

  /**
   * @brief Sets the enable state of the motor driver.
   * 
   * This function enables or disables the motor driver by setting the TOFF register.
   * 
   * @param mode If non-zero, enables the motor; if zero, disables it.
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t SetEnable(int32_t mode);

  /**
   * @brief Starts the homing process for the motor.
   * 
   * This function initiates the homing routine for the motor, moving it towards a defined home position.
   * The direction of movement is determined by the DHOM parameter.
   * DHOM: -1 -> neg (towards the L switch, lower positions) , +1 -> pos (R switch, higher pos)
   * 
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t StartHoming(void);

  /**
   * @brief Cancels the homing process for the motor.
   * 
   * This function stops the homing routine if it is currently in progress.
   * It resets the homing state and clears any relevant status flags.
   *
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t CancelHoming(void);

  /**
   * @brief Ends the homing process for the motor.
   * 
   * This function finalizes the homing routine, adjusting the motor's position and resetting relevant parameters.
   * It ensures that the motor is stopped and its position is correctly set after homing.
   * 
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t EndHoming(void);

  /**
   * @brief Gets the current status of the motor driver status parameters.
   *
   * @param index Index for the array motStatIDList, defined in Motors.h.
   * This array is a list of acepted status IDs (short strings), also used during serial comm.
   * @param value Integer to set the status to.
   * @return int8_t Returns 0 on success, or a negative error code on failure. 
   */
  int8_t SetStatusValue(int32_t index, int32_t value);

  /**
   * @brief Gets the current value of a specific status parameter.
   *
   * @param index Index for the array motStatIDList, defined in Motors.h.
   * This array is a list of accepted status IDs (short strings), also used during serial comm.
   * @param value Reference to an integer to store the retrieved status value.
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t GetStatusValue(int32_t index, int32_t &value);

  /**
   * @brief Sets a specific register value in the TMC driver.
   * @param address The register address to set.
   * @param value The value to set in the register.
   */
  int8_t SetRegisterValue(uint8_t address, int32_t value);

  /**
   * @brief Gets the value of a specific register in the TMC driver.
   * @param address The register address to read.
   * @param value Reference to an integer to store the retrieved register value.
   */
  int8_t GetRegisterValue(uint8_t address, int32_t &value);

  /**
   * @brief Checks for errors in the TMC driver.
   * 
   * This function reads the GSTAT register and checks for any error flags.
   * This should be done occasionally, not necessarily during a move
   * If an error is detected, it disables the motor and sets an error message.
   * 
   * @return int8_t Returns 0 if no errors are found, or a negative error code on failure.
   */
  int8_t CheckError(void);

  /**
   * @brief Checks the status of the TMC driver.
   * 
   * This function reads the status flags and updates the motor's moving state.
   * It also checks if the motor has completed its motion. This should be done during a move.
   * 
   * @param isMotionDone Reference to an integer to indicate if the motion is done (1 for done, 0 for not done).
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t CheckStatus(int32_t &isMotionDone);

  /**
   * @brief Gets the current status flags of the TMC driver.
   * 
   * This function retrieves the status flags from the RAMP_STAT register.
   * It indicates various states such as motion done, stop conditions, and errors.
   * Format:
   * bit:      11   |   10   |   9    |   8    |   7    |   6    |   5    |   4    |   3    |   2    |   1    |   0    |
   * flag:  enabled |  atPos |  isMov |latch_R |latch_L | encDev |SG_evnt |SG_stat | virt_R | virt_L | stop_R | stop_L |
   * 
   * @param status Reference to an integer to store the status flags.
   * @return int8_t Returns 0 on success, or a negative error code on failure.
   */
  int8_t GetStatusFlags(int32_t &status);

  /**
   * @brief Finds the index and value of a specific parameter by its name.
   * 
   * This function searches for a parameter by its name in the motor parameters array.
   * If found, it sets the index and value accordingly.
   * 
   * @param name The name of the parameter to find.
   * @param index Reference to an integer to store the index of the parameter.
   * @param value Reference to an integer to store the value of the parameter.
   * @return int8_t Returns 0 on success, or a negative error code if the parameter is not found.
   */
  int8_t FindParamIndexVal(const char *name, int32_t &index, int32_t &value);

  /**
   * @brief Checks if a parameter is within a specified range.
   * 
   * This function checks if the value of a parameter motorParam[index] is within the specified range.
   * 
   * @param index The index of the parameter to check (motorParam[index]).
   * @param min The minimum acceptable value for the parameter.
   * @param max The maximum acceptable value for the parameter.
   * @return int8_t Returns 1 if the parameter is in range, or 0 if it is out of range.
   */
  int8_t IsParamInRange(int32_t index, int32_t min, int32_t max);

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

#endif // TMC_H
