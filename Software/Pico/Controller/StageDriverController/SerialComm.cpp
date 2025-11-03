#include <Arduino.h>
#include "Common.h"

#include "SerialComm.h"


#define SERIAL_DEBUG  0
#include "Serial_Debug.h"


// *************************************************************************************
// defines
// *************************************************************************************
#define MSG_MAXLENGTH  100



// *************************************************************************************
// SerialComm class
// *************************************************************************************

// ----------------------------
// Constructor
// ----------------------------

SerialComm::SerialComm(){}


// ----------------------------
// Initialize stuff
// ----------------------------

void SerialComm::Init(Parameters *paramPtr, Motors *motorPtr, RemoteComm *remotePtr, long timeout_ms)
{
  D_println("SerialComm::Init");
  params = paramPtr;
  motors = motorPtr;
  remote = remotePtr;
  Serial.begin(SERIAL_BAUDRATE);
  // note: default time out for readBytesUntil is 1000 ms, set to desired value
  Serial.setTimeout(timeout_ms);
}


// ----------------------------
// Check for serial requests
// ----------------------------

void SerialComm::CheckSerialCommand(void)
{
  int8_t board;
  int8_t status;
  int8_t err;
  uint8_t registerAddr;
  int32_t idx;
  int32_t intVal;
  char tmpStr[MSG_MAXLENGTH];
  unsigned long currentTime = millis();
  static unsigned long lastCheckTime = 0;


  static char serialData[MSG_MAXLENGTH+1]; // one extra for the null char

  if (currentTime - lastCheckTime > SERIAL_CHECK_INTERVAL_MS) {

    if (Serial.available() > 0){
      // max allowed command size is MSG_MAXLENGTH char, ends with a term char
      int bytesRead = Serial.readBytesUntil(SERIAL_TERMCHAR, serialData, MSG_MAXLENGTH); // LF or CR
      // terminate with an end character
      serialData[bytesRead]='\0';
      // check for at least some bytes
      if (bytesRead<5) {
        snprintf(tmpStr, MSG_MAXLENGTH, "Command <5 chars. Recvd: %s", serialData);
        SetErrorMsg(tmpStr);
        ReportErrorCode(ERR_Serial);
        return;
      }

      /////////////////////
      // check for SetMotorPosition command
      if (strncmp(serialData, "SMC_MPOS", 8)  == 0){
        if (sscanf(serialData, "SMC_MPOS%hhi,%d", &board, &intVal)!=2) {
          SetErrorMsg("Invalid SMC_MPOS command format");
          ReportErrorCode(ERR_Serial); return;
        }
        if (err=CheckRemoteControl(board)) {ReportErrorCode(err); return;}
        err=motors->MoveToPos(board, intVal, 1); // set the velocity
        ReportErrorCode(err);
        return; 
      }  

      /////////////////////
      // check for SetMotorVelocity command
      if (strncmp(serialData, "SMC_MVEL", 8)  == 0){
        if (sscanf(serialData, "SMC_MVEL%hhi,%d", &board, &intVal)!=2) {
          SetErrorMsg("Invalid SMC_MVEL command format");
          ReportErrorCode(ERR_Serial); return;
        }
        if (err=CheckRemoteControl(board)) {ReportErrorCode(err); return;}
        err=motors->MoveAtVel(board, intVal);
        ReportErrorCode(err);
        return;
      }  

      /////////////////////
      // check for GetStatus command
      if (strncmp(serialData, "GMC_STAT", 8)  == 0){
        if (sscanf(serialData, "GMC_STAT%hhi", &board)!=1) {
          SetErrorMsg("Invalid GMC_STAT command format");
          ReportErrorCode(ERR_Serial); return;
        }
        if (err=motors->GetStatusFlags(board, intVal)) {ReportErrorCode(err); return;} 
        Serial.print("MC_STAT");Serial.print(board);Serial.print("=");
        Serial.println(intVal);
        return;
      }  

      /////////////////////
      // check for GetPositionReached command
      if (strncmp(serialData, "GMC_POSR", 8)  == 0){
        if (sscanf(serialData, "GMC_POSR%hhi", &board)!=1) {
          SetErrorMsg("Invalid GMC_POSR command format");
          ReportErrorCode(ERR_Serial); return;
        }
        if (err=motors->IsMotionDone(board, intVal)) {ReportErrorCode(err); return;} 
        Serial.print("MC_POSR");Serial.print(board);Serial.print("=");
        Serial.println(intVal);
        return;
      }  

      /////////////////////
      // check for Set Motor Status command (starts with an 'SMS_')
      if (strncmp(serialData, "SMS_", 4)  == 0) {
        for (idx=0; idx<MOTORS_NUM_STATUS; idx++) {
          if (strncmp(serialData+4, motors->motStatIDList[idx], 4)  == 0){ // loop through possible IDs
            sprintf(tmpStr, "SMS_%s%%hhd,%%d", motors->motStatIDList[idx]);
            if (sscanf(serialData, tmpStr, &board, &intVal)!=2) {
              snprintf(tmpStr, MSG_MAXLENGTH, "Invalid command format: %s", serialData);
              SetErrorMsg(tmpStr); ReportErrorCode(ERR_Serial); return;
            }
            err=motors->SetStatusValue(board, idx, intVal);
            ReportErrorCode(err);
            return;
          }
        }
        SetErrorMsg("Unrecognized SMS_ parameter"); ReportErrorCode(ERR_Serial); return;
      } // if (strncmp(serialData, "SMS_")

      /////////////////////
      // check for Get Motor Status command (starts with an 'GMS_')
      if (strncmp(serialData, "GMS_", 4)  == 0) {
        for (idx=0; idx<MOTORS_NUM_PARAMS; idx++) {
          if (strncmp(serialData+4, motors->motStatIDList[idx], 4)  == 0){ // loop through possible IDs
            sprintf(tmpStr, "GMS_%s%%hhd", motors->motStatIDList[idx]);
            if (sscanf(serialData, tmpStr, &board)!=1) {
              snprintf(tmpStr, MSG_MAXLENGTH, "Invalid command format: %s", serialData);
              SetErrorMsg(tmpStr); ReportErrorCode(ERR_Serial); return;
            }
            if (err=motors->GetStatusValue(board, idx, intVal)) {ReportErrorCode(err); return;}
            Serial.print("MS_"); Serial.print(motors->motStatIDList[idx]); Serial.print(board); Serial.print("=");
            Serial.println(intVal);
            return;
          }
        }
        SetErrorMsg("Unrecognized GMS_ parameter"); ReportErrorCode(ERR_Serial); return;
      } // if (strncmp(serialData, "GMS_")

      /////////////////////
      // check for SetDeviceType command ()
      if (strncmp(serialData, "SMP_TDEV", 8)  == 0){
        if (sscanf(serialData, "SMP_TDEV%hhi,%d", &board, &intVal)!=2) {
          SetErrorMsg("Invalid SMP_TDEV command format");
          ReportErrorCode(ERR_Serial); return;
        }
        err=params->SetDeviceType(board, intVal);
        ReportErrorCode(err);
        return;
      }

      /////////////////////
      // check for GetDeviceType command ()
      if (strncmp(serialData, "GMP_TDEV", 8)  == 0){
        if (sscanf(serialData, "GMP_TDEV%hhi", &board)!=1) {
          SetErrorMsg("Invalid GMP_TDEV command format");
          ReportErrorCode(ERR_Serial); return;
        }
        if (err=params->GetDeviceType(board, intVal)) {ReportErrorCode(err); return;}
        Serial.print("MP_TDEV"); Serial.print(board); Serial.print("=");
        Serial.println(intVal);
        return;
      }

      /////////////////////
      // check for SetAxisType command ()
      if (strncmp(serialData, "SMP_TAXI", 8)  == 0){
        if (sscanf(serialData, "SMP_TAXI%hhi,%d", &board, &intVal)!=2) {
          SetErrorMsg("Invalid SMP_TAXI command format");
          ReportErrorCode(ERR_Serial); return;
        }
        err=params->SetAxisType(board, intVal);
        ReportErrorCode(err);
        return;
      }

      /////////////////////
      // check for GetAxisType command ()
      if (strncmp(serialData, "GMP_TAXI", 8)  == 0){
        if (sscanf(serialData, "GMP_TAXI%hhi", &board)!=1) {
          SetErrorMsg("Invalid GMP_TAXI command format");
          ReportErrorCode(ERR_Serial); return;
        }
        if (err=params->GetAxisType(board, intVal)) {ReportErrorCode(err); return;}
        Serial.print("MP_TAXI"); Serial.print(board); Serial.print("=");
        Serial.println(intVal);
        return;
      }

      /////////////////////
      // check for Set Motor Parameter command (starts with an 'SMP_')
      if (strncmp(serialData, "SMP_", 4)  == 0) {
        for (idx=0; idx<MOTORS_NUM_PARAMS; idx++) {
          if (strncmp(serialData+4, params->motParamsIDList[idx], 4)  == 0){ // loop through possible IDs
            sprintf(tmpStr, "SMP_%s%%hhd,%%d", params->motParamsIDList[idx]);
            if (sscanf(serialData, tmpStr, &board, &intVal)!=2) {
              snprintf(tmpStr, MSG_MAXLENGTH, "Invalid command format: %s", serialData);
              SetErrorMsg(tmpStr); ReportErrorCode(ERR_Serial); return;
            }
            err=params->SetMotorParams(board, idx, intVal);
            ReportErrorCode(err);
            return;
          }
        }
        sprintf(tmpStr, "Unrecognized parameter  %s", serialData);
        SetErrorMsg(tmpStr); ReportErrorCode(ERR_Serial); return;
      } // if (strncmp(serialData, "SMP_")

      /////////////////////
      // check for Get Motor Parameter command (starts with an 'GMP_')
      if (strncmp(serialData, "GMP_", 4)  == 0) {
        for (idx=0; idx<MOTORS_NUM_PARAMS; idx++) {
          if (strncmp(serialData+4, params->motParamsIDList[idx], 4)  == 0){ // loop through possible IDs
            sprintf(tmpStr, "GMP_%s%%hhd", params->motParamsIDList[idx]);
            if (sscanf(serialData, tmpStr, &board)!=1) {
              snprintf(tmpStr, MSG_MAXLENGTH, "Invalid command format: %s", serialData);
              SetErrorMsg(tmpStr); ReportErrorCode(ERR_Serial); return;
            }
            if (err=params->GetMotorParams(board, idx, intVal)) {ReportErrorCode(err); return;}
            Serial.print("MP_"); Serial.print(params->motParamsIDList[idx]); Serial.print(board); Serial.print("=");
            Serial.println(intVal);
            return;
          }
        }
        sprintf(tmpStr, "Unrecognized parameter  %s", serialData);
        SetErrorMsg(tmpStr); ReportErrorCode(ERR_Serial); return;
      } // if (strncmp(serialData, "GMP_")

      /////////////////////
      // check for Set Remote Parameter command (starts with an 'SRP_')
      if (strncmp(serialData, "SRP_", 4)  == 0) {
        for (idx=0; idx<REMOTE_NUM_PARAMS; idx++) {
          if (strncmp(serialData+4, params->remoteIDList[idx], 4)  == 0){ // loop through possible IDs
            sprintf(tmpStr, "SRP_%s%%hhd,%%d", params->remoteIDList[idx]);
            if (sscanf(serialData, tmpStr, &board, &intVal)!=2) {
              snprintf(tmpStr, MSG_MAXLENGTH, "Invalid command format: %s", serialData);
              SetErrorMsg(tmpStr); ReportErrorCode(ERR_Serial); return;
            }
#if REMOTE_ENABLED
            if (err=remote->SendRemoteCommand(params->remoteIDList[idx], board, intVal)) {ReportErrorCode(err); return;}
#endif
            err=params->SetRemoteParams(board, idx, intVal);
            ReportErrorCode(err);
            return;
          }
        }
        sprintf(tmpStr, "Unrecognized parameter  %s", serialData);
        SetErrorMsg(tmpStr); ReportErrorCode(ERR_Serial); return;
      } // if (strncmp(serialData, "SRP_")

      /////////////////////
      // check for Get Remote Parameter command (starts with an 'GRP_')
      if (strncmp(serialData, "GRP_", 4)  == 0) {
        for (idx=0; idx<MOTORS_NUM_PARAMS; idx++) {
          if (strncmp(serialData+4, params->remoteIDList[idx], 4)  == 0){ // loop through possible IDs
            sprintf(tmpStr, "GRP_%s%%hhd", params->remoteIDList[idx]);
            if (sscanf(serialData, tmpStr, &board)!=1) {
              snprintf(tmpStr, MSG_MAXLENGTH, "Invalid command format: %s", serialData);
              SetErrorMsg(tmpStr); ReportErrorCode(ERR_Serial); return;
            }
            if (err=params->GetRemoteParams(board, idx, intVal)) {ReportErrorCode(err); return;}
            Serial.print("RP_"); Serial.print(params->remoteIDList[idx]); Serial.print(board); Serial.print("=");
            Serial.println(intVal);
            return;
          }
        }
        sprintf(tmpStr, "Unrecognized parameter  %s", serialData);
        SetErrorMsg(tmpStr); ReportErrorCode(ERR_Serial); return;
      } // if (strncmp(serialData, "GRP_")


      /////////////////////
      // check for set board config command
      if (strncmp(serialData, "SMC_CONF", 8)  == 0){
        if (sscanf(serialData, "SMC_CONF%hhi", &board)!=1) {
          SetErrorMsg("Invalid SMC_CONF command format");
          ReportErrorCode(ERR_Serial); return;
        }
        err=motors->ConfigBoard(board);
        if (err) {ReportErrorCode(err); return;}
        err=remote->Config(board);
        ReportErrorCode(err);
        return;
      }

      /////////////////////
      // check for ClearStatusRegs command
      if (strncmp(serialData, "SMC_SCLR", 8)  == 0){
        if (sscanf(serialData, "SMC_SCLR%hhi", &board)!=1) {
          SetErrorMsg("Invalid SMC_SCLR command format");
          ReportErrorCode(ERR_Serial); return;
        }
        err=motors->ClearStatusRegs(board);
        ReportErrorCode(err);
        return;
      }  

      /////////////////////
      // check for SetHomingMode command
      if (strncmp(serialData, "SMC_HOME", 8)  == 0){
        if (sscanf(serialData, "SMC_HOME%hhi", &board)!=1) {
          SetErrorMsg("Invalid SMC_HOME command format");
          ReportErrorCode(ERR_Serial); return;
        }
        if (err=CheckRemoteControl(board)) {ReportErrorCode(err); return;}
        err=motors->StartHoming(board);
        ReportErrorCode(err);
        return;
      }  


      /////////////////////
      // check for SetDriverRegisterValue command
      if (strncmp(serialData, "SMC_DREG", 8)  == 0){
        if (sscanf(serialData, "SMC_DREG%hhi,%hhu,%i", &board, &registerAddr, &intVal)!=3) {
          SetErrorMsg("Invalid SMC_DREG command format");
          ReportErrorCode(ERR_Serial); return;
        }
        err=motors->SetRegisterValue(board, registerAddr, intVal);
        ReportErrorCode(err);
        return;
      }  

      /////////////////////
      // check for GetDriverRegisterValue command
      if (strncmp(serialData, "GMC_DREG", 8)  == 0){
        if (sscanf(serialData, "GMC_DREG%hhi,%hhu", &board, &registerAddr)!=2) {
          SetErrorMsg("Invalid GMC_DREG command format");
          ReportErrorCode(ERR_Serial); return;
        }
        if (err=motors->GetRegisterValue(board, registerAddr, intVal)) {ReportErrorCode(err); return;} 
        Serial.print("MC_DREG");Serial.print(board);Serial.print("=");
        Serial.println(intVal);
        return;
      }  

      /////////////////////
      // check for ID query
      if (strncmp(serialData, "*IDN?", 5)  == 0){
        Serial.println(SERIAL_ID_STRING);
        return;
      }

      /////////////////////
      // check for GetNumberDevices command
      if (strncmp(serialData, "GPC_NDEV", 8)  == 0){
        Serial.print("PC_NDEV=");
        Serial.println(MAXNUMMOTORS);
        return;
      }

      /////////////////////
      // check for version query
      if (strncmp(serialData, "GPC_VERS", 8)  == 0){
        Serial.print("PC_VERS=");
        Serial.println(VERSION);
        return;
      }

      /////////////////////
      // check for error query
      if (strncmp(serialData, "GPC_EMSG", 8)  == 0){
        ReportErrorMsg();
        return;
      }

      /////////////////////
      // check for EEPROM save
      if (strncmp(serialData, "SPC_SAFL", 8)  == 0){
        err=params->SaveConfigToFlash();
        ReportErrorCode(err);
        return;
      }

      /////////////////////
      // if we ever get to here, it was an unrecognized command
      SetErrorMsg("Unrecognized command");
      ReportErrorCode(ERR_Serial);

    } // if (Serial.available() > 0) 

    // reset the check time
    lastCheckTime = currentTime;

  } // if (currentTime - lastCheckTime > SERIAL_CHECK_INTERVAL_MS)
}


// ----------------------------
// Check if the motor is under remote control
// ----------------------------

int8_t SerialComm::CheckRemoteControl(int8_t board)
{
  if (motors->isRemoteControlled[board]) {
    SetErrorMsg("Motor is under remote control");
    return ERR_Serial;
  } else {
    return ERR_None;
  }
}


// ----------------------------
// Set the error message and throw the error flag
// ----------------------------

void SerialComm::SetErrorMsg(const char *msg)
{
  if (errorFlag) return; // do not overwrite the previous entry
  errorFlag = 1;
  snprintf(errorMsg, MAXERRORSTRINGSIZE, "%s", msg);
}


// ----------------------------
// Print the error to serial and clear the error flag
// ----------------------------

int8_t SerialComm::PrintErrorMsg(void)
{
  if (errorFlag) {
    Serial.print(errorMsg);
    errorFlag = 0;
    return 1; // there was an error
  }
  return 0;
}


// ----------------------------
// Print an error code
// ----------------------------

void SerialComm::ReportErrorCode(int8_t error)
{
  Serial.print("ERROR=");
  Serial.println(error);
}


// ----------------------------
// Print all errors to serial
// ----------------------------

void SerialComm::ReportErrorMsg(void)
{
  int8_t firstError = 1;

  Serial.print("PC_EMSG=");
  if(errorFlag || params->errorFlag || motors->errorFlag || remote->errorFlag) {
    // print all the error messages
    if (errorFlag) {
      Serial.print("Serial: ");
      PrintErrorMsg();
      firstError = 0;
    }
    if (params->errorFlag) {
      if (!firstError) Serial.print("; "); 
      Serial.print("Params: ");
      params->PrintErrorMsg();
      firstError = 0;
    }
    if (motors->errorFlag) {
      if (!firstError) Serial.print("; "); 
      Serial.print("Motors: ");
      motors->PrintErrorMsg();
      firstError = 0;
    }
    if (remote->errorFlag) {
      if (!firstError) Serial.print("; "); 
      Serial.print("Remote: ");
      remote->PrintErrorMsg();
    }
    Serial.println("");
  } else {
    Serial.println("No error");
  }
}
