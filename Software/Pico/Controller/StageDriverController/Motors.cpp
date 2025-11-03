#include <Arduino.h>
#include "Common.h"

#include "SPI.h"
#include "Motors.h"
#include "TMC.h"

#define SERIAL_DEBUG  0
#include "Serial_Debug.h"



// *************************************************************************************
// TMC class
// *************************************************************************************


// ----------------------------
// Constructor
// ----------------------------

Motors::Motors() {}


// ----------------------------
// Destructor
// ----------------------------

Motors::~Motors()
{
  delete[] tmcArr;
  tmcArr = nullptr;
  SPI.end();
}

// ----------------------------
// Initialize stuff (arrays and things, don't actually configure the boards yet)
// ----------------------------

int8_t Motors::Init(Parameters *paramPtr)
{
  D_println("Motors::Init");
  params = paramPtr;
  SPI.begin();
  SPI.beginTransaction(SPISettings(115200, MSBFIRST, SPI_MODE3));
  delay(10);

  // disable CS pins by default
  for (int8_t z=0; z<MAXNUMMOTORS; z++) {
    if (params->hwParameters.DRIVER_CS[z]>=0) {
      pinMode(params->hwParameters.DRIVER_CS[z], INPUT_PULLUP);
    }
  }

  tmcArr = new TMC[MAXNUMMOTORS];
  for (int8_t z=0; z<MAXNUMMOTORS; z++) {
    tmcArr[z].Init(z, this, params->GetMotorParamPtr(z), params->GetHWParamPtr());
  }
  return ERR_None;
}


// ----------------------------
// Configure all allowed boards
// ----------------------------

// int8_t Motors::Config(void)
// {
//   D_println("Motors::Config");
//   for (int8_t z=0; z<MAXNUMMOTORS; z++) {
//     if (!params->IsActiveMotor(z)) continue; // silently skip if not defined
//     if (ConfigBoard(z)){
//       SetErrorMsg("Board", z, "Could not configure board");
//       return ERR_Motor;
//     }
//   }
//   return ERR_None;
// }


// ----------------------------
// Configure -> do the entire config sequence
// ----------------------------

int8_t Motors::ConfigBoard(int8_t board)
{
  int32_t index;
//  int32_t eMaxValue, encConstValue, toleranceValue, resetXafterCLValue;

  if (board == -1) { // all motors
    for (int8_t z=0; z<MAXNUMMOTORS; z++) {
      if (!params->IsActiveMotor(z)) continue; // silently skip if not defined
      if (tmcArr[z].Config() != ERR_None) {
        SetErrorMsg("Board", z, "Could not configure board");
        return ERR_Motor;
      } 
    }
  } else if (params->IsActiveMotor(board, 1)) { 
    // transfer the closed-loop parameters into private arrays for speed
    // tmcArr[board].FindParamIndexVal("ECON", index, encConstValue); // 0 if not present
    // tmcArr[board].FindParamIndexVal("EMAX", index, eMaxValue); // max iterations
    // tmcArr[board].FindParamIndexVal("ETOL", index, toleranceValue); // tolerance
    // tmcArr[board].FindParamIndexVal("ERST", index, resetXafterCLValue); // reset flag
    // encConst[board] = encConstValue;
    // maxIterations[board] = eMaxValue;
    // tolerance[board] = toleranceValue;
    // resetXafterCL[board] = resetXafterCLValue;

    if (tmcArr[board].Config() != ERR_None) {
      SetErrorMsg("Board", board, "Could not configure board");
      return ERR_Motor;
    }
  } else {
    SetErrorMsg("Board", -1, "Invalid motor number");
    return ERR_Motor;
  }
  return ERR_None;
}


// ----------------------------
// Function to periodically check the motors and process changes. Called from the loop control in the main controller
// ----------------------------

void Motors::ProcessUpdateChanges(void)
{
  int8_t err;
  int32_t isMotionDone;
  unsigned long currentTime = millis();
  static unsigned long lastErrorCheckTime = 0; // only one instance of the class, so it is static
  static unsigned long lastStatusCheckTime = 0; // only one instance of the class, so it is static
  int32_t currPos, deviation;

  // check for errors occasionally
  if (currentTime - lastErrorCheckTime > MOTORS_CHECK_ERROR_INTERVAL_MS) {
    // check all drivers, whether enabled or not
    for (int8_t z=0; z<MAXNUMMOTORS; z++) {
      if (!params->IsActiveMotor(z)) continue; // silently skip if not defined    
      tmcArr[z].CheckError(); // check all boards
    }
    lastErrorCheckTime = currentTime;
  } // if (currentTime - lastErrorCheckTime > MOTORS_CHECK_ERROR_INTERVAL_MS)

  // check for status updates that occur during motion
  if (currentTime - lastStatusCheckTime > MOTORS_CHECK_STATUS_INTERVAL_MS) {
    // check all drivers that are currently moving
    for (int8_t z=0; z<MAXNUMMOTORS; z++) {
      if (!params->IsActiveMotor(z) || !isMotorEnabled[z]) continue; // skip if not enabled
      
      if ( isMotorHoming[z] ) { // homing in progress

        err = tmcArr[z].CheckStatus(isMotionDone);
        if (isMotionDone || err) isMotorMoving[z]=0;

      } else if (isMotorSearching[z] ) { // closed loop search in progress

        err = tmcArr[z].CheckStatus(isMotionDone);
        if (err) {
          isMotorMoving[z]=0;
          isMotorSearching[z]=0;
          SetErrorMsg("Board", z, "Error during closed loop mode");
        } else if (isMotionDone) { 
          isMotorMoving[z]=0;
          // get the encoder position
          err = tmcArr[z].GetEnc(currPos);
          deviation = currPos - targetPosition[z];
          if (abs(deviation)>tmcArr[z].tolerance) { // still not at the right spot
            if (iterationsLeft[z]==-1 || iterationsLeft[z]>0) { // keep adjusting
              if (tmcArr[z].maxIterations>1) iterationsLeft[z]--;
              isMotorMoving[z]=1;
              setPosition[z] -= deviation; 
              err = tmcArr[z].MoveToPos(setPosition[z], 0);
            } else { // iterationsLeft==0, i.e. not there but no more tries left
              SetErrorMsg("Board", z, "Closed loop motion did not converge");
              isMotorMoving[z]=0;
              isMotorSearching[z]=0;
            }
          } else { // reached the target
            isMotorMoving[z]=0;
            if (iterationsLeft[z]!=-1) isMotorSearching[z]=0;
            if (tmcArr[z].resetXafterCL) tmcArr[z].SetXPos(currPos); // set X positions to encoder
          }
        }

      } else if (isMotorMoving[z]) { // open loop motion in progress

        err = tmcArr[z].CheckStatus(isMotionDone);
        if (isMotionDone || err) isMotorMoving[z]=0;

      }
    }
    lastStatusCheckTime = currentTime;
  } // if (currentTime - lastStatusCheckTime > MOTORS_CHECK_STATUS_INTERVAL_MS)
}

// ----------------------------
// Move to position
// ----------------------------

int8_t Motors::MoveToPos(int8_t board, int32_t pos, int setVel)
{
  int8_t err;

  if(!params->IsActiveMotor(board, 1)) return ERR_Motor;
  if (!isMotorEnabled[board]) {
    SetErrorMsg("Board", board, "Driver is not enabled");
    return ERR_Motor;
  }
  if (isMotorHoming[board]) {
    SetErrorMsg("Board", board, "Motor is homing");
    return ERR_Motor;
  }

  if ( (tmcArr[board].maxIterations==0 || tmcArr[board].maxIterations>1) && tmcArr[board].encConst!=0 ) { // closed loop
    targetPosition[board] = pos; // store the desired position
    iterationsLeft[board] = tmcArr[board].maxIterations - 1; // reset iteration counter
    isMotorMoving[board]=1;
    isMotorSearching[board]=1;
    setPosition[board] = targetPosition[board];
    err = tmcArr[board].MoveToPos(targetPosition[board], setVel);
  } else { // open loop
    iterationsLeft[board] = 0;
    isMotorMoving[board]=1;
    isMotorSearching[board]=0;
    err = tmcArr[board].MoveToPos(pos, setVel);
  }
  if (err) {
    isMotorMoving[board]=0;
    SetErrorMsg("Board", board, "Error setting position target");
    return ERR_Motor;
  }
  return ERR_None;
}

// ----------------------------
// Move at a specified velocity
// ----------------------------

int8_t Motors::MoveAtVel(int8_t board, int32_t vel)
{
  int8_t err;

  if(!params->IsActiveMotor(board, 1)) return ERR_Motor;
  if (!isMotorEnabled[board]) {
    SetErrorMsg("Board", board, "Driver is not enabled");
    return ERR_Motor;
  }
  if (isMotorHoming[board]) {
    // if (vel==0) {
    //   return ConfigBoard(board); // abort motion -> reconfig
    // } else {  
      SetErrorMsg("Board", board, "Motor is homing");
      return ERR_Motor;
    // }
  }

  err = tmcArr[board].MoveAtVel(vel);
  isMotorMoving[board] = ( vel==0 ? 0 : 1 );
  if (err!=ERR_None) {
    SetErrorMsg("Board", board, "Error setting velocity mode");
    isMotorMoving[board]=0;
    return ERR_Motor;
  }
  return ERR_None;
}


// ----------------------------
// Get the current position of the motor
// ----------------------------

int8_t Motors::GetPos(int8_t board, int32_t &pos)
{
  if(!params->IsActiveMotor(board, 1)) return ERR_Motor;
  return tmcArr[board].GetPos(pos);
}


// ----------------------------
// Set whether the motor is controlled by serial or remote
// ----------------------------

int8_t Motors::SetRemoteEnabled(int8_t board, int8_t state)
{
  if (board == -1) { // all motors
    for (int8_t z=0; z<MAXNUMMOTORS; z++) {
      if (!params->IsActiveMotor(z)) continue; // silently skip if not defined
      isRemoteControlled[z]=state;
    }
  } else {
    if (!params->IsActiveMotor(board)) return ERR_None; // do nothing
    isRemoteControlled[board]=state;
  }
  return ERR_None;
}


// ----------------------------
// Clears the status registers
// ----------------------------

int8_t Motors::ClearStatusRegs(int8_t board)
{
  if (board == -1) { // all motors
    for (int8_t z=0; z<MAXNUMMOTORS; z++) {
      if (!params->IsActiveMotor(z)) continue; // silently skip if not defined
      tmcArr[z].ClearStatusRegs();
    }
  } else {
    if (!params->IsActiveMotor(board, 1)) return ERR_Motor;
    tmcArr[board].ClearStatusRegs();
  }
  return ERR_None;
}


// ----------------------------
// Start the homing procedure
// ----------------------------

int8_t Motors::StartHoming(int8_t board)
{
  if (!params->IsActiveMotor(board, 1)) return ERR_Motor;
  if (!isMotorEnabled[board]) {
    SetErrorMsg("Board", board, "Driver is not enabled");
    return ERR_Motor;
  }
  return tmcArr[board].StartHoming();
}


// ----------------------------
// Set the status value
// ----------------------------

int8_t Motors::SetStatusValue(int8_t board, int index, int32_t value)
{
  int8_t status;

  // ENAB is a special case, it can be set for all motors at once
  if ( (strncmp(motStatIDList[index], "ENAB", 4)==0) && (board == -1) ) { 
    for (int8_t z=0; z<MAXNUMMOTORS; z++) {
      if (!params->IsActiveMotor(z)) continue; // silently skip if not defined
      status = tmcArr[z].SetStatusValue(index, value);
      if (status != ERR_None) {
        SetErrorMsg("Board", z, "Error enabling/disabling the motor");
        return status;
      }
    }
    return ERR_None;
  }

  if (!params->IsActiveMotor(board, 1)) return ERR_Motor;
  return tmcArr[board].SetStatusValue(index, value);
}


// ----------------------------
// Get the status value
// ----------------------------

int8_t Motors::GetStatusValue(int8_t board, int index, int32_t &value)
{
  int32_t idx, max;

  if (!params->IsActiveMotor(board, 1)) return ERR_Motor;
  if ( (strncmp(motStatIDList[index], "PULL", 4)==0)) {
    tmcArr[board].FindParamIndexVal("EMAX", idx, max);
    value = max - iterationsLeft[board]; 
    return ERR_None;
  } else { 
    return tmcArr[board].GetStatusValue(index, value);
  }
}


// ----------------------------
// Set a register value
// ----------------------------

int8_t Motors::SetRegisterValue(int8_t board, uint8_t address, int32_t value)
{
  if (!params->IsActiveMotor(board, 1)) return ERR_Motor;
  return tmcArr[board].SetRegisterValue(address, value);
}


// ----------------------------
// Get a register value
// ----------------------------

int8_t Motors::GetRegisterValue(int8_t board, uint8_t address, int32_t &value)
{
  if (!params->IsActiveMotor(board, 1)) return ERR_Motor;
  return tmcArr[board].GetRegisterValue(address, value);
}


// ----------------------------
// Get the status flags
// ----------------------------

int8_t Motors::GetStatusFlags(int8_t board, int32_t &status)
{
  if (!params->IsActiveMotor(board, 1)) return ERR_Motor;
  return tmcArr[board].GetStatusFlags(status);
}


// ----------------------------
// Check if motion is complete
// ----------------------------

int8_t Motors::IsMotionDone(int8_t board, int32_t &done)
{
  done = 1;
  if (board == -1) { // all motors
    for (int8_t z=0; z<MAXNUMMOTORS; z++) {
      if (!params->IsActiveMotor(z)) continue; // silently skip if not defined
      if (isMotorMoving[z] || isMotorSearching[z]) {
        done = 0;
        return ERR_None;
      }
    }
  } else {
    if (!params->IsActiveMotor(board, 1)) return ERR_Motor;
    done = ((isMotorMoving[board] || isMotorSearching[board]) ? 0 : 1);
  }
  return ERR_None;
}


// ----------------------------
// Set the error message and throw the error flag
// ----------------------------

void Motors::SetErrorMsg(const char *source, int8_t num, const char *msg)
{
  if(num==-1) { // general error
    if (errorFlagGeneral) return; // do not overwrite the previous entry
    snprintf(errorMsgGeneral, MAXERRORSTRINGSIZE, "%s error: %s", source, msg);
    errorFlagGeneral = 1;
  } else if (params->IsValidMotor(num)) {
    if (errorFlagBoard[num]) return; // do not overwrite the previous entry
    snprintf(errorMsgBoard[num], MAXERRORSTRINGSIZE, "%s error in board %hhi: %s", source, num, msg);
    errorFlagBoard[num] = 1;
  }
  errorFlag = 1;
}


// ----------------------------
// Print the error to serial and clear the error flag
// ----------------------------

int8_t Motors::PrintErrorMsg(void)
{
  int8_t firstError = 1;

  if (errorFlag) {
    if (errorFlagGeneral) {
      Serial.print(errorMsgGeneral);
      errorFlagGeneral = 0;
      firstError = 0;
    } 
    for (int z=0; z<MAXNUMMOTORS; z++) {
      if (errorFlagBoard[z]) {
        if (!firstError) Serial.print("; ");
        Serial.print(errorMsgBoard[z]);
        errorFlagBoard[z] = 0;
        firstError = 0;
      } 
    }
    errorFlag = 0;
    return 1; // there was an error
  }
  return 0;
}
