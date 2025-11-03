#include <Arduino.h>
#include "Common.h"

#include "RemoteComm.h"

#define SERIAL_DEBUG  0
#include "Serial_Debug.h"


// *************************************************************************************
// defines
// *************************************************************************************
#define MSG_MAXLENGTH  1024


// *************************************************************************************
// RemoteComm class
// *************************************************************************************

// ----------------------------
// Constructor
// ----------------------------

RemoteComm::RemoteComm(){}


// ----------------------------
// Initialize stuff
// ----------------------------

void RemoteComm::Init(Parameters *paramPtr, Motors *motorPtr, long timeout_ms)
{
  D_println("RemoteComm::Init");
  params = paramPtr;
  motors = motorPtr;
#if REMOTE_ENABLED
  // UART is Serial1
  Serial1.setRX(REMOTE_PIN_RX);
  Serial1.setTX(REMOTE_PIN_TX);
  Serial1.setFIFOSize(REMOTE_UART_BUFFER_SIZE); // increade buffer size a bit
  Serial1.begin(REMOTE_BAUDRATE);
  // note: default time out for readBytesUntil is 1000 ms, set to desired value
  Serial1.setTimeout(timeout_ms);
#else 
  for (int mot=0; mot<MAXNUMMOTORS; mot++) {
    motors->SetRemoteEnabled(mot, 0); // disable remote for all motors
  }
#endif // REMOTE_ENABLED
}


// ----------------------------
// Configure, i.e., send the parameters to the remote
// ----------------------------

int8_t RemoteComm::Config(int8_t mot)
{
  int32_t value;

  // initialize the remote parameters
  if (mot == -1) { // all motors
    for (int8_t z=0; z<MAXNUMMOTORS; z++) {
      if (!params->IsActiveMotor(z)) continue; // silently skip if not defined
      for (int idx=0; idx<REMOTE_NUM_PARAMS; idx++) {
        params->GetRemoteParams(z, idx, value);
        if (SendRemoteCommand(params->remoteIDList[idx], z, value)) {
          SetErrorMsg("Unable to set remote parameter");
          return ERR_Remote;
        }
      }
    }
  } else if (params->IsActiveMotor(mot, 1)) { 
    for (int idx=0; idx<REMOTE_NUM_PARAMS; idx++) {
      params->GetRemoteParams(mot, idx, value);
      if (SendRemoteCommand(params->remoteIDList[idx], mot, value)) {
        SetErrorMsg("Unable to set remote parameter");
        return ERR_Remote;
      }
    }
  } else {
    SetErrorMsg("Invalid motor number");
    return ERR_Motor;
  }
  return ERR_None;
}


// ----------------------------
// Send the current positions to the remote
// ----------------------------

void RemoteComm::SendPositionUpdates(void)
{
  int32_t pos;
  char cmdData[MSG_MAXLENGTH+1]; // one extra for the null char
  char uartData[MSG_MAXLENGTH+1];
  size_t len = 0;
  uint8_t checksum;
  unsigned long currentTime = millis();
  static unsigned long lastSendTime = 0;


  if (currentTime - lastSendTime > REMOTE_SEND_INTERVAL_MS) {
    D_println("Updating positions");

    cmdData[0]='\0';
    for (int8_t idx=0; idx<MAXNUMMOTORS; idx++) {
      if (!params->IsActiveMotor(idx)) continue; // silently skip if not defined
      motors->GetPos(idx, pos);
      len += snprintf(cmdData + len, sizeof(cmdData) - len, "POS%hhi=%i;", idx, pos);
    }
    if (len>0)
      *(cmdData+len-1) = '\0'; // overwrite the last ';'
    else
      return; // nothing in the string
    checksum = calculateChecksum(cmdData);
    snprintf (uartData, sizeof(uartData), "<%s|%hhu>", cmdData, checksum);
    D_println(uartData);
    safeSerial1Write(uartData, strlen(uartData));

    lastSendTime = currentTime;
  }
}


// ----------------------------
// Send a command to the remote
// ----------------------------

int8_t RemoteComm::SendRemoteCommand(const char *cmd, int8_t channel, int32_t value)
{
  char cmdData[MSG_MAXLENGTH+1]; // one extra for the null char
  int32_t intValue;

  // Check for validity of parameters
  if (strncmp(cmd, "ENAB", 4)  == 0) { // allow for axis=-1 for the ENAB command
    if (!IsValueInRange(channel, "channel", -1, MAXNUMMOTORS)) return ERR_Remote;
  } else {
    if (!IsValueInRange(channel, "channel", 0, MAXNUMMOTORS-1)) return ERR_Remote;
  }
  if (strncmp(cmd, "ENAB", 4)  == 0) {
     if (!IsValueInRange(value, cmd, 0, 1)) return ERR_Remote;
  } else if (strncmp(cmd, "JDIR", 4)  == 0) { 
     if (!IsValueInRange(abs(value), cmd, 1, 1)) return ERR_Remote;
  } else if (strncmp(cmd, "JMAX", 4)  == 0) {
    FindParamVal(channel, "RMXV", intValue);
    if (!IsValueInRange(value, cmd, 0, intValue)) return ERR_Remote;
  } else if (strncmp(cmd, "EDIR", 4)  == 0) {
    if (!IsValueInRange(abs(value), cmd, 1, 1))return ERR_Remote;
  }  
    
   // ENAB is the only command that needs to send info to the motor section and allows -1 for the channel
  if (strncmp(cmd, "ENAB", 4)  == 0) {
    if (motors->SetRemoteEnabled(channel, (int8_t)value)) { // channel -1 allowed here
      SetErrorMsg("Could not set remote enable in motor section");
      return ERR_Remote;
    }
    if (channel == -1) {  
      for (int8_t z=0; z<MAXNUMMOTORS; z++) {
        if (!params->IsActiveMotor(z, 0)) continue;
        snprintf(cmdData, sizeof(cmdData), "%s%hhi=%i", cmd, z, value);
        transmitRemoteCommand(cmdData);
      }
    } else {
      snprintf(cmdData, sizeof(cmdData), "%s%hhi=%i", cmd, channel, value);
      transmitRemoteCommand(cmdData);
    }
  } else { // any other command
    snprintf(cmdData, sizeof(cmdData), "%s%hhi=%i", cmd, channel, value);
    transmitRemoteCommand(cmdData);
  }
  return ERR_None;
}


// ----------------------------
// Send the formatted command
// ----------------------------

int8_t RemoteComm::transmitRemoteCommand(const char *cmd)
{
  uint8_t checksum;
  char uartData[MSG_MAXLENGTH+1]; // one extra for the null char

  D_print("Sending remote command ");
  checksum = calculateChecksum(cmd);
  D_println(cmd);
  snprintf (uartData, sizeof(uartData), "<%s|%hhu>", cmd, checksum);
  safeSerial1Write(uartData, strlen(uartData));
  return ERR_None;
}



// ----------------------------
// Check for commands send by the remote
// ----------------------------

void RemoteComm::CheckRemoteCommands(void)
{
  char uartData[MSG_MAXLENGTH+1]; // one extra for the null char
  unsigned long currentTime = millis();
  static unsigned long lastReceiveTime = 0;

  if (currentTime - lastReceiveTime > REMOTE_RECEIVE_INTERVAL_MS) {

    if (Serial1.available() > 0){
      // max allowed command size is MSG_MAXLENGTH char, ends with a term char
      int bytesRead = Serial1.readBytesUntil('>', uartData, MSG_MAXLENGTH);
      // check for at least some bytes
      if (bytesRead<3) {
        D_println("Invalid UART command string");
        SetErrorMsg("Invalid UART command string");
        return;
      }
      // terminate with an end character
      uartData[bytesRead]='\0';
      if (validateChecksum(uartData)) {
        D_println("Checksum error");
        return;
      }

      if (strchr(uartData+1, ';') == NULL) { // single command
        processRemoteCommand(uartData+1);
      } else { // multiple commands
        char* savePtr;
        char* token = strtok_r(uartData+1, ";", &savePtr);
        while (token != NULL) {
          processRemoteCommand(token);
          // Get the next token
          token = strtok_r(NULL, ";", &savePtr);
        }
      }
    } // if (Serial1.available() > 0) 
    lastReceiveTime = currentTime;
  }
}


// ----------------------------
// Process the remote command
// ----------------------------

void RemoteComm::processRemoteCommand(const char *cmd)
{
  int8_t status;
  int8_t board;
  int32_t intVal;
  static uint8_t repeatPosSet[MAXNUMMOTORS]={0};

  D_println(cmd);
  /////////////////////
  // check for SetMotorPosition command
  if (strncmp(cmd, "POS", 3)  == 0){
    if (sscanf(cmd, "POS%hhi=%d", &board, &intVal)!=2) {
      SetErrorMsg("Invalid remote POS command format");
      return;
    }
    if (!params->IsActiveMotor(board, 1) || !motors->isRemoteControlled[board]) return;
    if (repeatPosSet[board]) {
      status = motors->MoveToPos(board, intVal, 0); // do not set the velocity
    } else {
      status = motors->MoveToPos(board, intVal, 1); // set the velocity
      repeatPosSet[board]=1;
    }
    if (status) { 
      SetErrorMsg("Could not set motor pos with remote");
      return;
    }
    return;
  }  

  /////////////////////
  // check for SetMotorVelocity command
  if (strncmp(cmd, "VEL", 3)  == 0){
    if (sscanf(cmd, "VEL%hhi=%d", &board, &intVal)!=2) {
      SetErrorMsg("Invalid remote VEL command format");
      return;
    }
    if (!params->IsActiveMotor(board, 1) || !motors->isRemoteControlled[board]) return;
    repeatPosSet[board]=0;
    if (motors->MoveAtVel(board, intVal)) { 
      SetErrorMsg("Could not set motor vel with remote");
      return;
    }
    return;
  }

  /////////////////////
  // check for AccessRequest command
  if (strncmp(cmd, "ACCREQ", 6)  == 0){
    if (sscanf(cmd, "ACCREQ%hhi", &board)!=1) {
      SetErrorMsg("Invalid remote ACCREQ command format");
      return;
    }
    if (!params->IsActiveMotor(board, 1)) return;
    repeatPosSet[board]=0;
    SendRemoteCommand("ENAB", board, 1); // enable the remote control (also notifies the motor module)
    // also need to set the parameter
    FindRemoteParamIndex(board, "ENAB", intVal);
    params->SetRemoteParams(board, intVal, 1);
    return;
  }

}


// ----------------------------
// Safe write to Serial1
// ----------------------------

void RemoteComm::safeSerial1Write(const char* data, size_t len)
{
    size_t sent = 0;

    while (serial1Busy);  // wait here if another write is happening
    serial1Busy = true;

    while (sent < len) {
        int space = Serial1.availableForWrite();

        if (space > 0) {
            size_t chunk = (len - sent < (size_t)space) ? (len - sent) : (size_t)space;
            Serial1.write((const uint8_t*)(data + sent), chunk);
            sent += chunk;
        } else {
            delay(1);  // short wait to avoid busy loop
        }
    }

    serial1Busy = false;
}


// ----------------------------
// Set the error message and throw the error flag
// ----------------------------

void RemoteComm::SetErrorMsg(const char *msg)
{
  if (errorFlag) return; // do not overwrite the previous entry
  errorFlag = 1;
  snprintf(errorMsg, MAXERRORSTRINGSIZE, "%s", msg);
}


// ----------------------------
// Print the error to serial and clear the error flag
// ----------------------------

int8_t RemoteComm::PrintErrorMsg(void)
{
  if (errorFlag) {
    Serial.print(errorMsg);
    errorFlag = 0;
    return 1; // there was an error
  }
  return 0;
}


// ----------------------------
// Make sure the checksum is valid
// ----------------------------

int8_t RemoteComm::validateChecksum(char *data)
{
  uint8_t checksum;

  char *sepChar = strchr(data, '|');
  if (sepChar == NULL)
  {
    D_println("Could not find checksum");
    return -1;
  }
  sscanf(sepChar + 1, "%hhu", &checksum);
  // overwrite the sepChar with the EOS
  *sepChar = '\0';
  if (calculateChecksum(data + 1) != checksum)
  {
    D_println("Checksum mismatch");
    return -1;
  }
  return 0;
}


// ----------------------------
// Calculate the checksum for the data
// ----------------------------

uint8_t RemoteComm::calculateChecksum(const char *data)
{
  uint8_t sum = 0;

  while (*data) {
      sum += *data++;
  }
  return sum;
}


// ----------------------------
// Find the index of the parameter value
// ----------------------------

int8_t RemoteComm::FindParamVal(int8_t board, const char *name, int32_t &value)
{
  char errMsg[MSG_MAXLENGTH];

  for (int32_t idx=0; idx<MOTORS_NUM_PARAMS; idx++) {
    if (strncmp(Parameters::motParamsIDList[idx], name, 4)  == 0) {
      value = params->motorParamArr[board][idx];
      return ERR_None;
    }
  }
  // error if not found
  snprintf(errMsg, MSG_MAXLENGTH, "Parameter %s not found", name);
  SetErrorMsg(errMsg);
  return ERR_Remote;
}


// ----------------------------
// Find the index and the value of the parameter
// ----------------------------

int8_t RemoteComm::FindRemoteParamIndex(int8_t board, const char *name, int32_t &index)
{
  char errMsg[MSG_MAXLENGTH];

  for (int32_t idx=0; idx<REMOTE_NUM_PARAMS; idx++) {
    if (strncmp(Parameters::remoteIDList[idx], name, 4)  == 0) {
      index=idx;
      return ERR_None;
    }
  }
  // error if not found
  snprintf(errMsg, MSG_MAXLENGTH, "Remote parameter %s not found", name);
  SetErrorMsg(errMsg);
  return ERR_TMC;
}




// ----------------------------
// Check value valididty
// ----------------------------

int8_t RemoteComm::IsValueInRange(int32_t value, const char* name, int32_t min, int32_t max)
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


