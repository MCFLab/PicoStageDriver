#include <Arduino.h>
#include "Common.h"

#include <EEPROM.h>

#include "Parameters.h"
#include "DefaultParams.h"

#define SERIAL_DEBUG  0
#include "Serial_Debug.h"


// *************************************************************************************
// Parameters class
// *************************************************************************************

// ----------------------------
// Constructor
// ----------------------------

Parameters::Parameters(void) {}


// ----------------------------
// Destructor
// ----------------------------

Parameters::~Parameters(void) {}


// ----------------------------
// Init
// ----------------------------

int8_t Parameters::Init(Motors *motorPtr, RemoteComm *remotePtr)
{
  D_println("Parameters::Init");
  motors = motorPtr;
  remote = remotePtr;
  EEPROM.begin(PRARAMETERS_FLASH_SIZE);
  return 0;
}


// ----------------------------
// Init and set default/saved parameters
// ----------------------------

int8_t Parameters::Config(ConfigType confType)
{
  int8_t err;
  int address = 0;
  int version;

  if (confType==CONFIG_LOAD_FROM_FLASH) {
    // load the version number first and make sure it matches
    EEPROM.get(address, version);
    if (version==VERSION) {
      // load the rest of the parameters from flash
      address += sizeof(version);
      EEPROM.get(address, hwParameters);
      address += sizeof(hwParameters);
      EEPROM.get(address, motorParamArr);
      address += sizeof(motorParamArr);
      EEPROM.get(address, remoteParamArr);
    } else {
      // version doesn't match
      for (int mot=0; mot<MAXNUMMOTORS; mot++) 
        hwParameters.motorType[mot] = MOTOR_NONE;
      SetErrorMsg("Version mismatch in flash");
      return ERR_Parameter;
    }
  } else if (confType==CONFIG_DEFAULT) {
    for (int mot=0; mot<MAXNUMMOTORS; mot++) {
      for (int z=0; z<MOTORS_NUM_PARAMS; z++) 
        motorParamArr[mot][z] = defaultSafeMotorParams[z];
        // motorParamArr[mot][z] = defaultStepperMotorParams[z]; // for testing
      for (int z=0; z<REMOTE_NUM_PARAMS; z++) 
        remoteParamArr[mot][z] = defaultRemoteParams[z];
    }
  }

  if (motors->ConfigBoard(-1)) { // configure all boards
    SetErrorMsg("Could not configure motors");
    return ERR_Parameter;
  }
#if REMOTE_ENABLED
  if (remote->Config(-1)) { // configure all remotes
    SetErrorMsg("Could not configure remote");
    return ERR_Parameter;
  }
#endif // REMOTE_ENABLED
  return ERR_None; 
}


// ----------------------------
// Save the current configuration to flash memory
// ----------------------------

int8_t Parameters::SaveConfigToFlash(void)
{
  int address = 0;
  int version = VERSION;

  EEPROM.put(address, version);
  address += sizeof(version);
  EEPROM.put(address, hwParameters);
  address += sizeof(hwParameters);
  EEPROM.put(address, motorParamArr);
  address += sizeof(motorParamArr);
  EEPROM.put(address, remoteParamArr);
  if (!EEPROM.commit()) {
    SetErrorMsg("Could not save config to flash");
    return ERR_Parameter;
  }
  return ERR_None;
}


// ----------------------------
// Set the type of device (TMC, SIM, or None)
// ----------------------------

int8_t Parameters::SetDeviceType(int8_t board, int32_t value)
{
  if(!IsValidMotor(board)) return ERR_Parameter;
  switch (value) {
    case 0: hwParameters.motorType[board] = MOTOR_NONE; break;
    case 1: hwParameters.motorType[board] = MOTOR_SIM; break;
    case 2: hwParameters.motorType[board] = MOTOR_TMC; break;
    default: SetErrorMsg("Invalid device type (0..2)"); return ERR_Parameter;
  }
  return ERR_None;
}


// ----------------------------
// Get the type of device (TMC, SIM, or None)
// ----------------------------

int8_t Parameters::GetDeviceType(int8_t board, int32_t &value)
{
  if(!IsValidMotor(board)) return ERR_Parameter;
  value = hwParameters.motorType[board];
  return ERR_None;
}


// ----------------------------
// Set the type of axis (UNDEF, X, Y, Z, or Aux)
// ----------------------------

int8_t Parameters::SetAxisType(int8_t board, int32_t value)
{
  if(!IsValidMotor(board)) return ERR_Parameter;
  switch (value) {
    case 0: hwParameters.axisType[board] = AXIS_UNDEF; break;
    case 1: hwParameters.axisType[board] = AXIS_X; break;
    case 2: hwParameters.axisType[board] = AXIS_Y; break;
    case 3: hwParameters.axisType[board] = AXIS_Z; break;
    case 4: hwParameters.axisType[board] = AXIS_AUX; break;
    default: SetErrorMsg("Invalid axis type (0..4)"); return ERR_Parameter;
  }
  return ERR_None;
}


// ----------------------------
// Get the type of axis (UNDEF, X, Y, Z, or Aux)
// ----------------------------

int8_t Parameters::GetAxisType(int8_t board, int32_t &value)
{
  if(!IsValidMotor(board)) return ERR_Parameter;
  value = hwParameters.axisType[board];
  return ERR_None;
}


// ----------------------------
// Check if the board number is valid
// ----------------------------

int8_t Parameters::IsValidMotor(int8_t board)
{
  if (board < 0 || board>=MAXNUMMOTORS) {
    SetErrorMsg("Invalid board number");
    D_println("Error: Invalid board number");
    return 0;
  } else {
    return 1;
  }
}


// ----------------------------
// Check if the board number is active (valid and not MOTOR_NONE)
// ----------------------------

int8_t Parameters::IsActiveMotor(int8_t board, int8_t raiseError)
{
  if (board < 0 || board>=MAXNUMMOTORS || hwParameters.motorType[board]==MOTOR_NONE) {
    if (raiseError) {
      SetErrorMsg("Inactive board number");
      D_println("Error: Inactive board number");
    }
    return 0;
  } else {
    return 1;
  }
}


// ----------------------------
// Set the motor parameter for the board
//  Note: no parameter error checking here
// ----------------------------

int8_t Parameters::SetMotorParams(int8_t board, int8_t index, int32_t value)
{
  if(!IsValidMotor(board)) return ERR_Parameter;
  motorParamArr[board][index] = value;
  return ERR_None;
}


// ----------------------------
// Get the motor parameter for the board
// ----------------------------

int8_t Parameters::GetMotorParams(int8_t board, int8_t index, int32_t &value)
{
  if(!IsValidMotor(board)) return ERR_Parameter;
  value = motorParamArr[board][index];
  return ERR_None;
}


// ----------------------------
// Set the remote parameter for the board
//  Note: no parameter error checking here
// ----------------------------

int8_t Parameters::SetRemoteParams(int8_t board, int8_t index, int32_t value)
{
  // enable is the only command that allows for a -1 for the board
  if (strncmp(remoteIDList[index], "ENAB", 4)==0 && board==-1) {
    for (int z=0; z<MAXNUMMOTORS; z++)
      remoteParamArr[z][index] = value;
  } else {
    if(!IsValidMotor(board)) return ERR_Parameter;
    remoteParamArr[board][index] = value;
  }
  return ERR_None;
}


// ----------------------------
// Get the remote parameter for the board
// ----------------------------

int8_t Parameters::GetRemoteParams(int8_t board, int8_t index, int32_t &value)
{
  if(!IsValidMotor(board)) return ERR_Parameter;
  value = remoteParamArr[board][index];
  return ERR_None;
}


// ----------------------------
// Get the pointer to the motor parameters array for a specific board
// ----------------------------

int32_t* Parameters::GetMotorParamPtr(int8_t board)
{
  if(!IsValidMotor(board)) return nullptr;
  return motorParamArr[board];
}


// ----------------------------
// Get the pointer to the parameters
// ----------------------------

HWParamStruct* Parameters::GetHWParamPtr(void)
{
  return &hwParameters;
}


// ----------------------------
// Set the error message and throw the error flag
// ----------------------------

void Parameters::SetErrorMsg(const char *msg)
{
  if (errorFlag) return; // do not overwrite the previous entry
  errorFlag = 1;
  snprintf(errorMsg, MAXERRORSTRINGSIZE, "%s", msg);
}


// ----------------------------
// Print the error to serial and clear the error flag
// ----------------------------

int8_t Parameters::PrintErrorMsg(void)
{
  if (errorFlag) {
    Serial.print(errorMsg);
    errorFlag = 0;
    return 1; // there was an error
  }
  return 0;
}