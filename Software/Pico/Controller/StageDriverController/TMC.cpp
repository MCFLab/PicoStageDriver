#include <Arduino.h>
#include "Common.h"

#include "SPI.h"
#include "TMC.h"
#include "Motors.h"

#define SERIAL_DEBUG  0
#include "Serial_Debug.h"

// *************************************************************************************
// TMC includes
// *************************************************************************************
extern "C"
{
#include "TMC5240_HW_Abstraction.h"
#include "TMC5240.h" 
}


// *************************************************************************************
// defines
// *************************************************************************************
#define MSG_MAXLENGTH  50


// *************************************************************************************
// global variables used for the TMC functions
// *************************************************************************************
static TMC5240BusType activeBus = IC_BUS_SPI; // currently only SPI is supported, but UART is possible
static uint8_t nodeAddress = 0; 
static pin_size_t g_csPin[MAXNUMMOTORS] = {0};


// *************************************************************************************
// TMC helper functions
// *************************************************************************************

uint8_t tmc5240_getNodeAddress(uint16_t icID) {
    return nodeAddress;
}

TMC5240BusType tmc5240_getBusType(uint16_t icID) {
    return activeBus;
}

void tmc5240_readWriteSPI(uint16_t icID, uint8_t *data, size_t dataLength) {

    digitalWrite(g_csPin[icID], LOW);
    delayMicroseconds(10);
    for (uint32_t i = 0; i < dataLength; i++) {
        data[i] = SPI.transfer(data[i]);
    }
    delayMicroseconds(10);
    digitalWrite(g_csPin[icID], HIGH);
}

bool tmc5240_readWriteUART(uint16_t icID, uint8_t *data, size_t writeLength, size_t readLength)
{
  // UART here not implemented
   return false;
}



// *************************************************************************************
// TMC class
// *************************************************************************************
// ----------------------------
// Default constructor
// ----------------------------

TMC::TMC()
{
  D_println("TMC::TMC");
}


// ----------------------------
// Destructor
// ----------------------------

TMC::~TMC()
{
  if (board) SetEnable(0);
}


// ----------------------------
// Initialize the TMC controller
// ----------------------------

int8_t TMC::Init(int8_t tNum, Motors *motorsPtr, int32_t *mParamPtr, HWParamStruct *hwParamPtr)
{
  D_print("TMC::Init board ");
  D_println(tNum);
  board = tNum;
  motors = motorsPtr;
  motorParam = mParamPtr;
  hwParam = hwParamPtr;
  // transfer the pin information into the global variable so the SPIwrite function can access
  int8_t defaultDRIVER_CS[MAXNUMMOTORS] = MOTORS_DEFAULT_DRIVER_CS;
  g_csPin[board] = defaultDRIVER_CS[board];

  return ERR_None;
}


// ----------------------------
// Configure the TMC controller
// ----------------------------

int8_t TMC::Config(void)
{
  int8_t err;
  int32_t index, value;

  D_print("TMC::Config board ");
  D_println(board);


  if (hwParam->motorType[board]==MOTOR_TMC) {

    if (g_csPin[board]<0) {
      SetErrorMsg("Invalid driver CS pin number");
      return ERR_TMC;
    }

    pinMode(g_csPin[board], OUTPUT);
    digitalWrite(g_csPin[board], HIGH);

    // clear the error flags (reset and such)
//    tmc5240_writeRegister(board, TMC5240_GSTAT, ~0);

    if (err=CheckError()) return err;

    // config the driver
    for (int8_t idx=0; idx<MOTORS_NUM_PARAMS; idx++) {

      // CurrentParams
      if (strncmp(Parameters::motParamsIDList[idx], "CSCA", 4)  == 0) {
        if (!IsParamInRange(idx, 32, 255)) return ERR_TMC;
        tmc5240_fieldWrite(board, TMC5240_GLOBAL_SCALER_FIELD, motorParam[idx]); 
      } else if (strncmp(Parameters::motParamsIDList[idx], "CRAN", 4)  == 0) {
        if (!IsParamInRange(idx, 0, 3)) return ERR_TMC;
        tmc5240_fieldWrite(board, TMC5240_CURRENT_RANGE_FIELD, motorParam[idx]); 
      }  else if (strncmp(Parameters::motParamsIDList[idx], "CRUN", 4)  == 0) {
        if (!IsParamInRange(idx, 0, 31)) return ERR_TMC;
        tmc5240_fieldWrite(board, TMC5240_IRUN_FIELD, motorParam[idx]); 
      } else if (strncmp(Parameters::motParamsIDList[idx], "CHOL", 4)  == 0) {
        if (!IsParamInRange(idx, 0, 31)) return ERR_TMC;
        tmc5240_fieldWrite(board, TMC5240_IHOLD_FIELD, motorParam[idx]); 

      // ModeParams
      }  else if (strncmp(Parameters::motParamsIDList[idx], "MMIC", 4)  == 0) {
        if (!IsParamInRange(idx, 0, 8)) return ERR_TMC;
        tmc5240_fieldWrite(board, TMC5240_MRES_FIELD, motorParam[idx]); 
      }  else if (strncmp(Parameters::motParamsIDList[idx], "MINV", 4)  == 0) {
        if (!IsParamInRange(idx, 0, 1)) return ERR_TMC;
        tmc5240_fieldWrite(board, TMC5240_SHAFT_FIELD, motorParam[idx]); 
      } else if (strncmp(Parameters::motParamsIDList[idx], "MTOF", 4)  == 0) {
        if (!IsParamInRange(idx, 0, 10)) return ERR_TMC;
        tmc5240_fieldWrite(board, TMC5240_TOFF_FIELD, 0); // TOFF=0 so the motor won't start
      }  else if (strncmp(Parameters::motParamsIDList[idx], "MSGE", 4)  == 0) {
        if (!IsParamInRange(idx, 0, 1)) return ERR_TMC;
        tmc5240_fieldWrite(board, TMC5240_SG_STOP_FIELD, motorParam[idx]); 
      }  else if (strncmp(Parameters::motParamsIDList[idx], "MSGT", 4)  == 0) {
        if (!IsParamInRange(idx, -64, 63)) return ERR_TMC;
        tmc5240_fieldWrite(board, TMC5240_SGT_FIELD, motorParam[idx]); 
      }  else if (strncmp(Parameters::motParamsIDList[idx], "MCTC", 4)  == 0) {
        if (!IsParamInRange(idx, 0, 100000000)) return ERR_TMC;
        tmc5240_fieldWrite(board, TMC5240_TCOOLTHRS_FIELD, motorParam[idx]); 

      // HomingParams (not set here)

      // RateParams (RMXV, RMXA, RSEV, HVEL are not set here)
      }  else if (strncmp(Parameters::motParamsIDList[idx], "RSEA", 4)  == 0) {
        FindParamIndexVal("RMXA", index, value);
        if (!IsParamInRange(idx, 0, value)) return ERR_TMC;
        tmc5240_fieldWrite(board, TMC5240_AMAX_FIELD, motorParam[idx]); 
        tmc5240_fieldWrite(board, TMC5240_DMAX_FIELD, motorParam[idx]); 

      // EncoderParams
      } else if (strncmp(Parameters::motParamsIDList[idx], "ECON", 4)  == 0) {
        encConst = motorParam[idx]; // transfer to local copy
        tmc5240_fieldWrite(board, TMC5240_ENC_SEL_DECIMAL_FIELD, 1);
        tmc5240_fieldWrite(board, TMC5240_ENC_CONST_FIELD, motorParam[idx]); 
      }  else if (strncmp(Parameters::motParamsIDList[idx], "EDEV", 4)  == 0) {
        if (!IsParamInRange(idx, 0, 1000000000)) return ERR_TMC;
        tmc5240_fieldWrite(board, TMC5240_ENC_DEVIATION_FIELD, motorParam[idx]); 
      // these just get transferred to local copies (ECON already done above)
      }  else if (strncmp(Parameters::motParamsIDList[idx], "EMAX", 4)  == 0) {
        maxIterations = motorParam[idx];
      }  else if (strncmp(Parameters::motParamsIDList[idx], "ETOL", 4)  == 0) {
        tolerance = motorParam[idx];
      }  else if (strncmp(Parameters::motParamsIDList[idx], "ERST", 4)  == 0) {
        resetXafterCL = motorParam[idx];

      // SwitchParams
      }  else if (strncmp(Parameters::motParamsIDList[idx], "SLEN", 4)  == 0) {
        if (!IsParamInRange(idx, 0, 1)) return ERR_TMC;
        tmc5240_fieldWrite(board, TMC5240_STOP_L_ENABLE_FIELD, motorParam[idx]); 
      }  else if (strncmp(Parameters::motParamsIDList[idx], "SREN", 4)  == 0) {
        if (!IsParamInRange(idx, 0, 1)) return ERR_TMC;
        tmc5240_fieldWrite(board, TMC5240_STOP_R_ENABLE_FIELD, motorParam[idx]); 
      }  else if (strncmp(Parameters::motParamsIDList[idx], "SLPO", 4)  == 0) {
        if (!IsParamInRange(idx, 0, 1)) return ERR_TMC;
        tmc5240_fieldWrite(board, TMC5240_POL_STOP_L_FIELD, motorParam[idx]); 
      }  else if (strncmp(Parameters::motParamsIDList[idx], "SRPO", 4)  == 0) {
        if (!IsParamInRange(idx, 0, 1)) return ERR_TMC;
        tmc5240_fieldWrite(board, TMC5240_POL_STOP_R_FIELD, motorParam[idx]); 
      }  else if (strncmp(Parameters::motParamsIDList[idx], "SSWP", 4)  == 0) {
        if (!IsParamInRange(idx, 0, 1)) return ERR_TMC;
        tmc5240_fieldWrite(board, TMC5240_SWAP_LR_FIELD, motorParam[idx]); 

      // LimitsParam

      }  else if (strncmp(Parameters::motParamsIDList[idx], "LENC", 4)  == 0) {
        if (!IsParamInRange(idx, 0, 1)) return ERR_TMC;
        tmc5240_fieldWrite(board, TMC5240_VIRTUAL_STOP_ENC_FIELD, motorParam[idx]); 
      }  else if (strncmp(Parameters::motParamsIDList[idx], "LLEN", 4)  == 0) {
        if (!IsParamInRange(idx, 0, 1)) return ERR_TMC;
        tmc5240_fieldWrite(board, TMC5240_EN_VIRTUAL_STOP_L_FIELD, motorParam[idx]); 
      }  else if (strncmp(Parameters::motParamsIDList[idx], "LREN", 4)  == 0) {
        if (!IsParamInRange(idx, 0, 1)) return ERR_TMC;
        tmc5240_fieldWrite(board, TMC5240_EN_VIRTUAL_STOP_R_FIELD, motorParam[idx]); 
      }  else if (strncmp(Parameters::motParamsIDList[idx], "LLPS", 4)  == 0) {
        tmc5240_fieldWrite(board, TMC5240_VIRTUAL_STOP_L_FIELD, motorParam[idx]); 
      }  else if (strncmp(Parameters::motParamsIDList[idx], "LRPS", 4)  == 0) {
        tmc5240_fieldWrite(board, TMC5240_VIRTUAL_STOP_R_FIELD, motorParam[idx]); 
      }
    }

    // set overtemp pre-warn threshold
    tmc5240_fieldWrite(board, TMC5240_OVERTEMPPREWARNING_VTH_FIELD, TMC_OVERTEMP_PREWARN);

    // by default, set to position mode, but reset the target position to the current position to avoid initial moves
    // if encoder is present, set both to the encoder position
    D_println("Resetting the target position to the actual position");
    FindParamIndexVal("ECON", index, value);
    if (value != 0) {
      value = tmc5240_readRegister(board, TMC5240_XENC);
      tmc5240_writeRegister(board, TMC5240_XACTUAL,  value);
    } else {
      value = tmc5240_readRegister(board, TMC5240_XACTUAL);
    }
    tmc5240_writeRegister(board, TMC5240_XTARGET,  value);

    D_println("Setting the rampmode to position");
    tmc5240_fieldWrite(board, TMC5240_RAMPMODE_FIELD, TMC5240_MODE_POSITION);

    // clear all the registers
//    ClearStatusRegs();
    tmc5240_writeRegister(board, TMC5240_ENC_STATUS, ~0); // clear the enc following error flag
    tmc5240_writeRegister(board, TMC5240_RAMPSTAT, ~0);

  } else if (hwParam->motorType[board]==MOTOR_SIM) {

    // just in case this has been reconfigured
    if (g_csPin[board]>=0) {
      pinMode(g_csPin[board], INPUT_PULLUP);
    }

    encConst = 0;
    maxIterations = 1;
    
    // for (int8_t idx=0; idx<MOTORS_NUM_PARAMS; idx++) {
    //   // just copy the value over, no error checking
    //   simValues.params[idx] = motorParam[idx];
    // }

    // set the limits
    FindParamIndexVal("LLEN", index, value);
    if (value != 0) {
        FindParamIndexVal("LLPS", index, value);
        simValues.xmin = value;
    } else {
        simValues.xmin = INT32_MIN;
    }
    FindParamIndexVal("LREN", index, value);
    if (value != 0) {
        FindParamIndexVal("LRPS", index, value);
        simValues.xmax = value;
    } else {
        simValues.xmax = INT32_MAX;
    }

    simValues.xact = simValues.xenc = 0;
    simValues.vel = 0;

  } else { // Motor not defined
    SetErrorMsg("Motor is defined as MOTOR_NONE");
    return ERR_TMC;
  }

  motors->isMotorEnabled[board]=0;
  motors->isMotorMoving[board]=0;
  motors->isMotorHoming[board]=0;

  return ERR_None;
}


// ----------------------------
// Clear the registers
// ----------------------------

int8_t TMC::ClearStatusRegs(void)
{
  D_println("Clearing the GSTAT, ENC_STATUS, and RAMPSTAT registers");
  tmc5240_writeRegister(board, TMC5240_GSTAT, ~0); // set all bits, only the lowest few are relevant
  tmc5240_writeRegister(board, TMC5240_ENC_STATUS, ~0); // clear the enc following error flag
  tmc5240_writeRegister(board, TMC5240_RAMPSTAT, ~0);
  return ERR_None;
}


// ----------------------------
// Move at a given velocity
// ----------------------------

int8_t TMC::MoveAtVel(int32_t velocity)
{
  int32_t index, value;

  D_println("Setting the motor velocity/direction");
  FindParamIndexVal("RMXV", index, value);
  if (!IsValueInRange(velocity, "VEL", -value, value)) return ERR_TMC;
  
  if (hwParam->motorType[board]==MOTOR_TMC) {
    tmc5240_fieldWrite(board, TMC5240_RAMPMODE_FIELD, velocity>0 ? TMC5240_MODE_VELPOS : TMC5240_MODE_VELNEG);
    tmc5240_writeRegister(board, TMC5240_VMAX, abs(velocity));
    if (int8_t err=CheckError()) return err;
  } else if (hwParam->motorType[board]==MOTOR_SIM) {
    simValues.vel = velocity;
    simValues.lastVelCalcTime = millis();
  } else {
    SetErrorMsg("Motor is defined as MOTOR_NONE");
    return ERR_TMC;
  }
  return ERR_None;
}


// ----------------------------
// Move to position
// ----------------------------

int8_t TMC::MoveToPos(int32_t pos, int8_t setVel)
{
  int32_t index, value;

  D_println("Setting the rampmode to position");
  if (hwParam->motorType[board]==MOTOR_TMC) {
    tmc5240_fieldWrite(board, TMC5240_EVENT_POS_REACHED_FIELD, 1); // clear the flag
    tmc5240_fieldWrite(board, TMC5240_RAMPMODE_FIELD, TMC5240_MODE_POSITION);
    if (setVel) {
      FindParamIndexVal("RSEV", index, value);
      D_print("Speed="); D_println(value);
      tmc5240_writeRegister(board, TMC5240_VMAX, value);
    }
    D_print("Pos="); D_println(pos);
    tmc5240_writeRegister(board, TMC5240_XTARGET, pos);
    if (int8_t err=CheckError()) return err;

  } else if (hwParam->motorType[board]==MOTOR_SIM) {
    simValues.vel = 0;
    if (pos < simValues.xmin) pos = simValues.xmin;
    if (pos > simValues.xmax) pos = simValues.xmax;
    simValues.xact = simValues.xenc = pos;
  } else {
    SetErrorMsg("Motor is defined as MOTOR_NONE");
    return ERR_TMC;
  }
  return ERR_None;
}


// ----------------------------
// Sets the X position
// ----------------------------

int8_t TMC::SetXPos(int32_t pos)
{
  int32_t vel;

  if (hwParam->motorType[board]==MOTOR_TMC) {
    vel = tmc5240_readRegister(board, TMC5240_VMAX); 
    tmc5240_writeRegister(board, TMC5240_VMAX, 0);
    tmc5240_writeRegister(board, TMC5240_XTARGET, pos);
    tmc5240_writeRegister(board, TMC5240_XACTUAL, pos);
    tmc5240_writeRegister(board, TMC5240_VMAX, vel);
  } else if (hwParam->motorType[board]==MOTOR_SIM) {
    simValues.vel = 0;
    simValues.xact = simValues.xtar = pos;
  }
  return ERR_None;
}


// ----------------------------
// Get current motor position (X_ACT)
// ----------------------------

int8_t TMC::GetPos(int32_t &pos)
{
  if (hwParam->motorType[board]==MOTOR_TMC) {
    pos = tmc5240_readRegister(board, TMC5240_XACTUAL); 
  } else if (hwParam->motorType[board]==MOTOR_SIM) {
    pos = simValues.xact; 
  } else {
    SetErrorMsg("Motor is defined as MOTOR_NONE");
    return ERR_TMC;
  }
  return ERR_None;
}


// ----------------------------
// Get current encoder position (X_ENC)
// ----------------------------

int8_t TMC::GetEnc(int32_t &pos)
{
  if (hwParam->motorType[board]==MOTOR_TMC) {
    pos = tmc5240_readRegister(board, TMC5240_XENC); 
  } else if (hwParam->motorType[board]==MOTOR_SIM) {
    pos = simValues.xact; 
  } else {
    SetErrorMsg("Motor is defined as MOTOR_NONE");
    return ERR_TMC;
  }
  return ERR_None;
}


// ----------------------------
// Set the enable state of the motor driver
// ----------------------------

int8_t TMC::SetEnable(int32_t mode)
{
  int32_t index, value;

  if (hwParam->motorType[board]==MOTOR_TMC) {

    if (mode!=0) {
      D_println("Enabling the motor");
      FindParamIndexVal("MTOF", index, value);
      tmc5240_fieldWrite(board, TMC5240_TOFF_FIELD, value);
      motors->isMotorEnabled[board]=1;
    } else {
      D_println("Disabling the motor");
      // need to set the velocity to zero, otherwise the XACT keeps going  
      tmc5240_fieldWrite(board, TMC5240_RAMPMODE_FIELD, TMC5240_MODE_VELPOS);
      tmc5240_writeRegister(board, TMC5240_VMAX, 0);
      // now turn the motor off
      tmc5240_fieldWrite(board, TMC5240_TOFF_FIELD, 0);
      motors->isMotorEnabled[board]=0;
      if (motors->isMotorHoming[board]) {
        // if the motor is homing, cancel it
        return CancelHoming();
      }
    }
  
  } else if (hwParam->motorType[board]==MOTOR_SIM) {

    motors->isMotorEnabled[board]= (mode!=0);
    if (mode==0) simValues.vel = 0;

  } else {
    SetErrorMsg("Motor is defined as MOTOR_NONE");
    return ERR_TMC;
  }

  return ERR_None;
}


// ----------------------------
// Start homing the motor
// ----------------------------

int8_t TMC::StartHoming(void)
{
  int32_t index, value;
  int32_t searchMode, dir, indexMode, homeVel, softStop, velMax;

  if (hwParam->motorType[board]==MOTOR_TMC) {
 
    FindParamIndexVal("HSST", index, softStop);
    if (!IsParamInRange(index, 0, 1)) return ERR_TMC;

    FindParamIndexVal("HMOD", index, searchMode);
    if (!IsParamInRange(index, 0, 2)) return ERR_TMC;
    if ( searchMode==0) { // disabled homing 
        SetErrorMsg("Homing disabled by config setting");
        return ERR_TMC;
    }

    FindParamIndexVal("HDIR", index, dir);
    if ( abs(dir)!=1) { // limit switch homing 
        SetErrorMsg("Homing direction undefined (needs -1 or 1)");
        return ERR_TMC;
    }

    if (searchMode==1) { // limit switches

      // only enable if the appropriate switch is enabled
      FindParamIndexVal( (dir==1 ? "SREN" : "SLEN"), index, value);
      if (value!=1) {
        SetErrorMsg("Homing only allowed if switch is enabled");
        return ERR_TMC;
      }
      // clear the status flags (clearing both is fine)
      tmc5240_fieldWrite(board, TMC5240_STATUS_LATCH_L_FIELD, 1);
      tmc5240_fieldWrite(board, TMC5240_STATUS_LATCH_R_FIELD, 1);

    } else if (searchMode==2) { // index

      FindParamIndexVal("HNEV", index, indexMode);
      if ( indexMode<0 || indexMode>3) { 
          SetErrorMsg("Invalid index homeing mode (needs 0..3)");
          return ERR_TMC;
      }
      // set up modes and clear the status flag
      tmc5240_fieldWrite(board, TMC5240_IGNORE_AB_FIELD, 1);
      tmc5240_fieldWrite(board, TMC5240_CLR_CONT_FIELD, 1);
      tmc5240_fieldWrite(board, TMC5240_POS_NEG_EDGE_FIELD, indexMode);
      tmc5240_fieldWrite(board, TMC5240_N_EVENT_FIELD, 1);

    } else { // disabled or unknown
      SetErrorMsg("Homing disabled or mode not defined");
      return ERR_TMC;
    }

    motors->isMotorHoming[board]=1;

    tmc5240_fieldWrite(board, TMC5240_EN_SOFTSTOP_FIELD, softStop);

    if ( dir==-1 ) { // -> left (or more negative) direction
      // disable the virtual limit
      tmc5240_fieldWrite(board, TMC5240_EN_VIRTUAL_STOP_L_FIELD, 0);
      // prepare the latch
      if (searchMode==1) { // limit switches
        tmc5240_fieldWrite(board, TMC5240_LATCH_L_ACTIVE_FIELD, 1);
      } else { // index mode
        tmc5240_fieldWrite(board, TMC5240_LATCH_X_ACT_FIELD, 1);
      }
      // set the move direction
      tmc5240_fieldWrite(board, TMC5240_RAMPMODE_FIELD, TMC5240_MODE_VELNEG);
    } else if ( dir==1 ) { // -> right (or more positive) direction
      // disable the virtual limit
      tmc5240_fieldWrite(board, TMC5240_EN_VIRTUAL_STOP_R_FIELD, 0);
      // prepare the latch
      if (searchMode==1) { // limit switches
        tmc5240_fieldWrite(board, TMC5240_LATCH_R_ACTIVE_FIELD, 1);
      } else { // index mode
        tmc5240_fieldWrite(board, TMC5240_LATCH_X_ACT_FIELD, 1);
      }
      // set the move direction
      tmc5240_fieldWrite(board, TMC5240_RAMPMODE_FIELD, TMC5240_MODE_VELPOS);
    }
    FindParamIndexVal("RMXV", index, velMax);
    FindParamIndexVal("HVEL", index, homeVel);
    if (!IsValueInRange(homeVel, "HVEL", 0, velMax)) return ERR_TMC;

    tmc5240_writeRegister(board, TMC5240_VMAX, homeVel);
    if (int8_t err=CheckError()) return err;

  } else if (hwParam->motorType[board]==MOTOR_SIM) {

    simValues.xact = simValues.xenc = 0; 
    simValues.vel = 0;
    motors->isMotorHoming[board]=0;

  } else {
    SetErrorMsg("Motor is defined as MOTOR_NONE");
    return ERR_TMC;
  }

  return ERR_None;
}


// ----------------------------
// Cancel homing
// ----------------------------

int8_t TMC::CancelHoming(void)
{
  int32_t index, value;
  int32_t xact, xlatch;

  if (motors->isMotorHoming[board]==0) return ERR_None;

  // reset the virtual limits
  FindParamIndexVal( "LLEN", index, value);
  tmc5240_fieldWrite(board, TMC5240_EN_VIRTUAL_STOP_L_FIELD, value);
  FindParamIndexVal( "LREN", index, value);
  tmc5240_fieldWrite(board, TMC5240_EN_VIRTUAL_STOP_R_FIELD, value);

  // clear the status flags and enable bits (clearing all is fine)
  tmc5240_fieldWrite(board, TMC5240_LATCH_L_ACTIVE_FIELD, 0);
  tmc5240_fieldWrite(board, TMC5240_LATCH_R_ACTIVE_FIELD, 0);
  tmc5240_fieldWrite(board, TMC5240_LATCH_X_ACT_FIELD, 0);
  tmc5240_fieldWrite(board, TMC5240_STATUS_LATCH_L_FIELD, 1);
  tmc5240_fieldWrite(board, TMC5240_STATUS_LATCH_R_FIELD, 1);
  tmc5240_fieldWrite(board, TMC5240_N_EVENT_FIELD, 1);

  motors->isMotorHoming[board]=0;
  return ERR_None;
}


// ----------------------------
// Clean up after homing is done
// ----------------------------

int8_t TMC::EndHoming(void)
{
  int32_t index, value;
  int32_t xact, xlatch;

  motors->isMotorHoming[board]=0;

  // wait for the motor to stop, raise error if not
  for (int idx=0; idx<TMC_HOMING_STANDSTILL_TIMEOUT_MS/50; idx++) {
    if (tmc5240_fieldRead(board, TMC5240_STST_FIELD)!=1) { // motor is at standstill
      break;
    }
    delay(50);
  }
  delay(500); // wait a bit more to be sure

  if (tmc5240_fieldRead(board, TMC5240_STST_FIELD)!=1) {
    SetErrorMsg("Motor hasn't stopped after homing position reached");
    SetEnable(0);
    return ERR_TMC;
  }
  
  // disable the motor so I can change the register value safely
  SetEnable(0);

  // figure out the current positions and write the difference into all the registers (XACT, XTarget, and Enc)
  xact = tmc5240_readRegister(board, TMC5240_XACTUAL); 
  xlatch = tmc5240_readRegister(board, TMC5240_XLATCH); 
  tmc5240_writeRegister(board, TMC5240_XACTUAL, xact - xlatch); 
  SetEnable(1);
  if (tmc5240_fieldRead(board, TMC5240_ENC_CONST_FIELD)!=0) { // only update if encoder is present
    tmc5240_writeRegister(board, TMC5240_XENC, xact - xlatch); 
    tmc5240_writeRegister(board, TMC5240_ENC_STATUS, ~0); // clear the enc following error flag
  }

  // reset the virtual limits
  FindParamIndexVal( "LLEN", index, value);
  tmc5240_fieldWrite(board, TMC5240_EN_VIRTUAL_STOP_L_FIELD, value);
  FindParamIndexVal( "LREN", index, value);
  tmc5240_fieldWrite(board, TMC5240_EN_VIRTUAL_STOP_R_FIELD, value);

  // clear the status flags and enable bits (clearing all is fine)
  tmc5240_fieldWrite(board, TMC5240_LATCH_L_ACTIVE_FIELD, 0);
  tmc5240_fieldWrite(board, TMC5240_LATCH_R_ACTIVE_FIELD, 0);
  tmc5240_fieldWrite(board, TMC5240_LATCH_X_ACT_FIELD, 0);
  tmc5240_fieldWrite(board, TMC5240_STATUS_LATCH_L_FIELD, 1);
  tmc5240_fieldWrite(board, TMC5240_STATUS_LATCH_R_FIELD, 1);
  tmc5240_fieldWrite(board, TMC5240_N_EVENT_FIELD, 1);
  
  MoveToPos(0, 1); // move to the new zero at the regular set speed
  for (int idx=0; idx<TMC_HOMING_STANDSTILL_TIMEOUT_MS/50; idx++) {
    if (tmc5240_fieldRead(board, TMC5240_POSITION_REACHED_FIELD)!=1) { // motor is at position
      break;
    }
    delay(50);
  }
  delay(200); // give motor a bit of time to settle
  // reset the encoder again. This shouldn't be necessary, but just in case
  if (tmc5240_fieldRead(board, TMC5240_ENC_CONST_FIELD)!=0) { // only update if encoder is present
    tmc5240_writeRegister(board, TMC5240_XENC, 0); 
  }

  return ERR_None;
}



// ----------------------------
// Set the status value
// ----------------------------

int8_t TMC::SetStatusValue(int32_t index, int32_t value)
{
  char errMsg[MSG_MAXLENGTH];
  int32_t limIndex, limValue;

  if (hwParam->motorType[board]==MOTOR_TMC) {

    if (strncmp(motors->motStatIDList[index], "ENAB", 4)  == 0) {
      SetEnable(value); 
    } else { // the rest of the settings are only allowed if not remote controlled
      if (motors->isRemoteControlled[board]) {
        SetErrorMsg("Motor is under remote control");
        return ERR_Motor;
      } else {
        // set stuff (if allowed) 
        if (strncmp(motors->motStatIDList[index], "XACT", 4)  == 0) {
          tmc5240_writeRegister(board, TMC5240_XACTUAL, value); 
        } else if (strncmp(motors->motStatIDList[index], "XTAR", 4)  == 0) {
            tmc5240_writeRegister(board, TMC5240_XTARGET, value); 
        } else if (strncmp(motors->motStatIDList[index], "XENC", 4)  == 0) {
          tmc5240_writeRegister(board, TMC5240_XENC, value);
          tmc5240_writeRegister(board, TMC5240_ENC_STATUS, ~0); // clear the enc following error flag
        } else if (strncmp(motors->motStatIDList[index], "VELO", 4)  == 0) {
          FindParamIndexVal("RMXV", limIndex, limValue);
          if (!IsValueInRange(value, motors->motStatIDList[index], -limValue, limValue)) return ERR_TMC;
          tmc5240_writeRegister(board, TMC5240_VMAX, value); 
        } else if (strncmp(motors->motStatIDList[index], "ACCE", 4)  == 0) {
          FindParamIndexVal("RMXA", limIndex, limValue);
          if (!IsValueInRange(value, motors->motStatIDList[index], 0, limValue)) return ERR_TMC;
          tmc5240_writeRegister(board, TMC5240_AMAX, value); 
          tmc5240_writeRegister(board, TMC5240_DMAX, value); 
        }
      }
    }

  } else if (hwParam->motorType[board]==MOTOR_SIM) {

    if (strncmp(motors->motStatIDList[index], "ENAB", 4)  == 0) {
      SetEnable(value); 
    } else { // the rest of the settings are only allowed if not remote controlled
      if (motors->isRemoteControlled[board]) {
        SetErrorMsg("Motor is under remote control");
        return ERR_Motor;
      } else {
        // set stuff (if allowed) 
        if (strncmp(motors->motStatIDList[index], "XACT", 4)  == 0) {
          simValues.xact = value;
        } else if (strncmp(motors->motStatIDList[index], "XTAR", 4)  == 0) {
          simValues.xact = value;
        } else if (strncmp(motors->motStatIDList[index], "XENC", 4)  == 0) {
          simValues.xenc = value;
        } else if (strncmp(motors->motStatIDList[index], "VELO", 4)  == 0) {
          simValues.vel = value;
        }
      }
    }

  } else {
    SetErrorMsg("Motor is defined as MOTOR_NONE");
    return ERR_TMC;
  }
  return ERR_None;
}


// ----------------------------
// Get the status value
// ----------------------------

int8_t TMC::GetStatusValue(int32_t index, int32_t &value)
{
  char errMsg[MSG_MAXLENGTH];


  if (hwParam->motorType[board]==MOTOR_TMC) {

    if (strncmp(motors->motStatIDList[index], "XACT", 4)  == 0) {
      value = tmc5240_readRegister(board, TMC5240_XACTUAL); 
    } else if (strncmp(motors->motStatIDList[index], "XTAR", 4)  == 0) {
        value = tmc5240_readRegister(board, TMC5240_XTARGET); 
    } else if (strncmp(motors->motStatIDList[index], "XENC", 4)  == 0) {
      value = tmc5240_readRegister(board, TMC5240_XENC);
    } else if (strncmp(motors->motStatIDList[index], "VELO", 4)  == 0) {
      value = tmc5240_readRegister(board, TMC5240_VMAX); 
    } else if (strncmp(motors->motStatIDList[index], "ACCE", 4)  == 0) {
      value = tmc5240_readRegister(board, TMC5240_AMAX); 
    } else if (strncmp(motors->motStatIDList[index], "ENAB", 4)  == 0) {
      value = (int32_t) motors->isMotorEnabled[board];
    } else if (strncmp(motors->motStatIDList[index], "TEMP", 4)  == 0) {
      value = (int32_t) ( (tmc5240_fieldRead(board, TMC5240_ADC_TEMP_FIELD) - 2038) / 7.7);
    }

  } else if (hwParam->motorType[board]==MOTOR_SIM) {

    if (strncmp(motors->motStatIDList[index], "XACT", 4)  == 0) {
      value = simValues.xact; 
    } else if (strncmp(motors->motStatIDList[index], "XTAR", 4)  == 0) {
      value = simValues.xact; 
    } else if (strncmp(motors->motStatIDList[index], "XENC", 4)  == 0) {
      value = simValues.xenc; 
    } else if (strncmp(motors->motStatIDList[index], "VELO", 4)  == 0) {
      value = simValues.vel; 
    } else if (strncmp(motors->motStatIDList[index], "ACCE", 4)  == 0) {
      value = 0; 
    } else if (strncmp(motors->motStatIDList[index], "ENAB", 4)  == 0) {
      value = (int32_t) motors->isMotorEnabled[board];
    } else if (strncmp(motors->motStatIDList[index], "TEMP", 4)  == 0) {
      value = 0;
    }

  } else {
    SetErrorMsg("Motor is defined as MOTOR_NONE");
    return ERR_TMC;
  }

  return ERR_None;
}

// ----------------------------
// Set the register value at address
// ----------------------------

int8_t TMC::SetRegisterValue(uint8_t address, int32_t value)
{
  if (hwParam->motorType[board]==MOTOR_TMC) {
    D_println("Setting register value");
    tmc5240_writeRegister(board, address, value);
  }
  return ERR_None;
}


// ----------------------------
// Get the register value at address
// ----------------------------

int8_t TMC::GetRegisterValue(uint8_t address, int32_t &value)
{
  if (hwParam->motorType[board]==MOTOR_TMC) {
    D_println("Reading register value");
    value = tmc5240_readRegister(board, address);
  } else {
    value = 0; // no register value for sim
  }
  return ERR_None;
}


// ----------------------------
// Check the board for errors
// ----------------------------

int8_t TMC::CheckError(void)
{
  int32_t value;
  static int32_t errorMask = TMC5240_S2VSA_MASK | TMC5240_S2VSB_MASK 
                            | TMC5240_S2GA_MASK | TMC5240_S2GB_MASK 
                            | TMC5240_OLA_MASK | TMC5240_OLB_MASK
                            | TMC5240_STALLGUARD_MASK                          
                            | TMC5240_OTPW_MASK | TMC5240_OT_MASK ;
  char msg[MSG_MAXLENGTH];

  if (hwParam->motorType[board]==MOTOR_TMC) {
    D_println("Reading GSTAT register value");
    value = tmc5240_readRegister(board, TMC5240_GSTAT);
    if (value) { // any error
      SetEnable(0);
      if (value & TMC5240_RESET_MASK) {
        sprintf(msg, "GSTAT: reset error bit set");
      } else if (value & TMC5240_UV_CP_MASK) {
        sprintf(msg, "GSTAT: undervoltage warning bit set");
      } else if (value & TMC5240_REGISTER_RESET_MASK) {
        sprintf(msg, "GSTAT: register reset error bit set");
      } else if (value & TMC5240_VM_UVLO_MASK) {
        sprintf(msg, "GSTAT: undervoltage since last reset bit set");
      } else if (value & TMC5240_DRV_ERR_MASK) {
        D_println("Reading DRVSTATUS register value");
        value = tmc5240_readRegister(board, TMC5240_DRVSTATUS);
        if (value & errorMask) {
          // overwrite with more specific msg
          if (value & TMC5240_S2VSA_MASK) {
            sprintf(msg, "DRVSTATUS: short to supply indicator phase A error bit set");
          } else if (value & TMC5240_S2VSB_MASK) {
            sprintf(msg, "DRVSTATUS: short to supply indicator phase B error bit set");
          } else if (value & TMC5240_S2GA_MASK) {
            sprintf(msg, "DRVSTATUS: short to ground indicator phase A error bit set");
          } else if (value & TMC5240_S2GB_MASK) {
            sprintf(msg, "DRVSTATUS: short to ground indicator phase B error bit set");
          } else if (value & TMC5240_OLA_MASK) {
            sprintf(msg, "DRVSTATUS: open load indicator phase A error bit set");
          } else if (value & TMC5240_OLB_MASK) {
            sprintf(msg, "DRVSTATUS: open load indicator phase B error bit set");
          } else if (value & TMC5240_STALLGUARD_MASK) {
            sprintf(msg, "DRVSTATUS: StallGuard error bit set");
          } else if (value & TMC5240_OT_MASK) {
            sprintf(msg, "DRVSTATUS: overtemperature flag set");
          } else if (value & TMC5240_OTPW_MASK) {
            sprintf(msg, "DRVSTATUS: overtemperature pre-warning flag set");
          } else {
            sprintf(msg, "DRVSTATUS error bits: %i", value & errorMask); // other bits
          }
        }
      }
      SetErrorMsg(msg);
      return ERR_TMC;
    }
  }
  return ERR_None;
}


// ----------------------------
// Check the board for its status
// ----------------------------

int8_t TMC::CheckStatus(int32_t &isMotionDone)
{
  int32_t flags;
  char msg[MSG_MAXLENGTH];

  if (hwParam->motorType[board]==MOTOR_TMC) {

    // check for following errors
    D_println("Reading ENC_STATUS register value");
    if( tmc5240_fieldRead(board, TMC5240_DEVIATION_WARN_FIELD) ) {
      SetEnable(0);
      SetErrorMsg("Following error");
      return ERR_TMC; 
    }
    D_println("Reading RAMPSTAT register value");
    flags = tmc5240_readRegister(board, TMC5240_RAMPSTAT);
    // set motion done flag
    isMotionDone = (   (flags & TMC5240_EVENT_POS_REACHED_MASK) 
                    || (flags & TMC5240_POSITION_REACHED_MASK)  ) ? 1 : 0;

    // check for errors
    if (flags & TMC5240_EVENT_STOP_SG_MASK) {
      SetErrorMsg("Stall guard2 tripped");
      SetEnable(0);
      return ERR_TMC;
    }
    // check if homing search has reached a switch
    if (motors->isMotorHoming[board]==1) {
      if ( (flags & TMC5240_STATUS_LATCH_L_MASK) || (flags & TMC5240_STATUS_LATCH_R_MASK) ) {
        int8_t err = EndHoming();
        return err;
      }
    }
    // check for stop flags
    if (flags & TMC5240_EVENT_STOP_L_MASK) {
      if (flags & TMC5240_STATUS_STOP_L_MASK) {
        SetErrorMsg("Left limit switch reached");
        SetEnable(0);
      } else if (flags & TMC5240_STATUS_VIRTUAL_STOP_L_MASK) {
        SetErrorMsg("Left virtual limit switch reached");
        // do not disable motor for virtual stop
      } else {
        SetErrorMsg("Unknown left stop condition");
        SetEnable(0);
      }
      return ERR_TMC;
    }
    if (flags & TMC5240_EVENT_STOP_R_MASK) {
      if (flags & TMC5240_STATUS_STOP_R_MASK) {
        SetErrorMsg("Right limit switch reached");
        SetEnable(0);
      } else if (flags & TMC5240_STATUS_VIRTUAL_STOP_R_MASK) {
        SetErrorMsg("Right virtual limit switch reached");
        // do not disable motor for virtual stop
      } else {
        SetErrorMsg("Unknown right stop condition");
        SetEnable(0);
      }
      return ERR_TMC;
    }
  } else if (hwParam->motorType[board]==MOTOR_SIM) {

    unsigned long currentTime = millis();
    if (simValues.vel==0) {
      isMotionDone = 1;
    } else {
      isMotionDone = 0;
      simValues.xact += ((int32_t)(currentTime - simValues.lastVelCalcTime) * simValues.vel)/1000;
    }
    simValues.lastVelCalcTime = currentTime;
    if (simValues.xact < simValues.xmin) {
      simValues.xact = simValues.xmin;
      simValues.vel = 0;
    }
    if (simValues.xact > simValues.xmax) {
      simValues.xact = simValues.xmax;
      simValues.vel = 0;
    }
    simValues.xenc = simValues.xact;

  } else {
    SetErrorMsg("Motor is defined as MOTOR_NONE");
    return ERR_TMC;
  }
  return ERR_None;
}


// ----------------------------
// Returns the status flags
//   Format:
//  bit:     11   |   10   |   9    |   8    |   7    |   6    |   5    |   4    |   3    |   2    |   1    |   0    |
// flag:  enabled |  atPos |  isMov |latch_R |latch_L | encDev |SG_evnt |SG_stat | virt_R | virt_L | stop_R | stop_L |
// ----------------------------

int8_t TMC::GetStatusFlags(int32_t &status)
{
  int32_t flags = 0;
  int32_t value;

  if (hwParam->motorType[board]==MOTOR_TMC) {

    // flags from the RAMP_STAT
    value = tmc5240_readRegister(board, TMC5240_RAMPSTAT);
    if ( motors->isMotorEnabled[board] )
      flags |= (0x1 << 11); // motor enabled
    if ( value & TMC5240_POSITION_REACHED_MASK )
      flags |= (0x1 << 10); // pos_reached status, NOT the event
    if ( !(value & TMC5240_VZERO_MASK) )
      flags |= (0x1 << 9); // velocity is not zero
    if ( value & TMC5240_STATUS_LATCH_R_MASK )
      flags |= (0x1 << 8); // status latch right available
    if ( value & TMC5240_STATUS_LATCH_L_MASK )
      flags |= (0x1 << 7); // status latch left available
    if ( value & TMC5240_EVENT_STOP_SG_MASK )
      flags |= (0x1 << 5); // SG event
    if ( value & TMC5240_STATUS_SG_MASK )
      flags |= (0x1 << 4); // SG status, NOT the event
    if ( value & TMC5240_STATUS_VIRTUAL_STOP_R_MASK )
      flags |= (0x1 << 3); // virt_R status, NOT the event
    if ( value & TMC5240_STATUS_VIRTUAL_STOP_L_MASK )
      flags |= (0x1 << 2); // virt_L status, NOT the event
    if ( value & TMC5240_STATUS_STOP_R_MASK )
      flags |= (0x1 << 1); // stop_R status, NOT the event
    if ( value & TMC5240_STATUS_STOP_L_MASK )
      flags |= 0x1; // stop_L status, NOT the event

    // flags from the RAMP_STAT
    value = tmc5240_readRegister(board, TMC5240_ENC_STATUS);
    if ( value & TMC5240_DEVIATION_WARN_MASK )
      flags |= (0x1 << 6); // stop_R status, NOT the event
    status = flags;

  } else if (hwParam->motorType[board]==MOTOR_SIM) {

    if ( motors->isMotorEnabled[board] )
      flags |= (0x1 << 11); // motor enabled
    if ( simValues.vel != 0 )
      flags |= (0x1 << 9); // velocity is not zero

    if ( simValues.xact >= simValues.xmax )
      flags |= (0x1 << 3); // virt_R status, NOT the event
    if ( simValues.xact <= simValues.xmin )
      flags |= (0x1 << 2); // virt_L status, NOT the event

    status = flags;
  } else {
    SetErrorMsg("Motor is defined as MOTOR_NONE");
    return ERR_TMC;
  }
  return ERR_None;  
}


// ----------------------------
// Find the index and the value of the parameter
// ----------------------------

int8_t TMC::FindParamIndexVal(const char *name, int32_t &index, int32_t &value)
{
  char errMsg[MSG_MAXLENGTH];

  for (int32_t idx=0; idx<MOTORS_NUM_PARAMS; idx++) {
    if (strncmp(Parameters::motParamsIDList[idx], name, 4)  == 0) {
      index=idx;
      value = motorParam[idx];
      return ERR_None;
    }
  }
  // error if not found
  snprintf(errMsg, MSG_MAXLENGTH, "Parameter %s not found", name);
  SetErrorMsg(errMsg);
  return ERR_TMC;
}


// ----------------------------
// Check parameter valididty
// ----------------------------

int8_t TMC::IsParamInRange(int32_t index, int32_t min, int32_t max)
{
  char errMsg[MSG_MAXLENGTH];

  int32_t val = motorParam[index];
  if (val >= min && val <= max)
    return 1;
  else { 
    snprintf(errMsg, MSG_MAXLENGTH, "Parameter %s out of range (%d)", Parameters::motParamsIDList[index], val);
    SetErrorMsg(errMsg);
    return 0;
  }
}


// ----------------------------
// Check value valididty
// ----------------------------

int8_t TMC::IsValueInRange(int32_t value, const char* name, int32_t min, int32_t max)
{
  char errMsg[MSG_MAXLENGTH];

  if (value >= min && value <= max)
    return 1;
  else { 
    snprintf(errMsg, MSG_MAXLENGTH, "Value %s out of range (%d)", name, value);
    SetErrorMsg(errMsg);
    return 0;
  }
}


// ----------------------------
// Set the error message and throw the error flag
// ----------------------------

void TMC::SetErrorMsg(const char *msg)
{
    motors->SetErrorMsg("TMC", board, msg);
}


