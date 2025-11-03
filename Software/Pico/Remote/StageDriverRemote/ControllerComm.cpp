#include <Arduino.h>
#include "Common.h"

#include "ControllerComm.h"
#include "Display.h"
#include "Encoders.h"
#include "Joystick.h"


#define SERIAL_DEBUG  1
#include "Serial_Debug.h"


// *************************************************************************************
// ControllerComm class
// *************************************************************************************

#define MSG_MAXLENGTH  1024

// ----------------------------
// Private constructor to enforce the singleton pattern.
// ----------------------------

ControllerComm::ControllerComm(){}


// ----------------------------
// Gets the singleton instance of the ControllerComm class
// ----------------------------

ControllerComm& ControllerComm::getInstance() {
  static ControllerComm instance;  // Only created once
  return instance;
}


// ----------------------------
// Initializes the UART communication and sets up the remote access button
// ----------------------------

void ControllerComm::init(long timeout_ms)
{
  // UART is Serial1
  Serial1.setRX(UART_PIN_RX);
  Serial1.setTX(UART_PIN_TX);
  Serial1.setFIFOSize(UART_BUFFER_SIZE);
  Serial1.begin(UART_BAUDRATE);
  D_println("\nControllerComm started.");
  Serial1.setTimeout(timeout_ms);
}


// ----------------------------
// Receives updates from the controller
// ----------------------------

void ControllerComm::receiveUpdatesFromController(void)
{
  Joystick& joystick = Joystick::getInstance();
  Encoders& encoders = Encoders::getInstance();
  static unsigned long lastUARTReceiveTime = 0;
  char uartData[MSG_MAXLENGTH+1]; // one extra for the null char
  size_t len = 0;
  uint8_t checksum = 0;
  int32_t intVal;
  int8_t valChanged = 0;

  unsigned long currentTime = millis();

  if (currentTime - lastUARTReceiveTime > UART_RECEIVE_INTERVAL_MS) {

    if (Serial1.available() > 0){
      // max allowed command size is MSG_MAXLENGTH char, ends with a term char
      int bytesRead = Serial1.readBytesUntil('>', uartData, MSG_MAXLENGTH);
      // terminate with an end character
      uartData[bytesRead]='\0';

      if (validateChecksum(uartData)) {
        D_print("recd: ");
        D_println(uartData+1); // skip the first char, which is '<'
        return;
      }

      if (strchr(uartData+1, ';') == NULL) { // single command
        processCommand(uartData+1);
        return;
      } else { // multiple commands
        char* savePtr;
        char* token = strtok_r(uartData+1, ";", &savePtr);
        while (token != NULL) {
          processCommand(token);
          // Get the next token
          token = strtok_r(NULL, ";", &savePtr);
        }
      }
    } // if (Serial1.available() > 0) 
    lastUARTReceiveTime = currentTime;
  } // if (currentTime - lastUARTReceiveTime > UART_CONTROLLER_RECEIVE_INTERVAL_MS)
}


// ----------------------------
// Processes commands received from the controller
// ----------------------------

int8_t ControllerComm::processCommand(const char *cmd)
{
  Joystick& joystick = Joystick::getInstance();
  Encoders& encoders = Encoders::getInstance();
  int8_t channel;
  int32_t intVal;

  // check for position update
  if (strncmp(cmd, "POS", 3)  == 0){
    if (sscanf(cmd, "POS%hhi=%i", &channel, &intVal) != 2) {
      D_println("Invalid position command."); return -1;
    }
    Display::getInstance().setPosition(channel, intVal);
    return 0;
  }

  // check for set remote enable command
  if (strncmp(cmd, "ENAB", 3)  == 0){
    if (sscanf(cmd, "ENAB%hhi=%i", &channel, &intVal) != 2) {
      D_println("Invalid enable command."); return -1;
    }
    isRemoteControlled[channel] = (intVal==0 ? 0 : 1);
    if (intVal) encoders.resetEncoderReference(channel);
    return 0;
  }

  // check for set remote joystick max value command
  if (strncmp(cmd, "JMAX", 3)  == 0){
    if (sscanf(cmd, "JMAX%hhi=%i", &channel, &intVal) != 2) {
      D_println("Invalid joystick max value command."); return -1;
    }
    joystick.setMaxValue(channel, intVal);
    joystick.updateCalibration(channel);
    return 0;
  }

  // check for set remote joystick direction command
  if (strncmp(cmd, "JDIR", 3)  == 0){
    if (sscanf(cmd, "JDIR%hhi=%i", &channel, &intVal) != 2) {
      D_println("Invalid joystick direction command."); return -1;
    }
    joystick.setDirection(channel, intVal);
    joystick.updateCalibration(channel);
    return 0;
  }

  // check for set encoder step size command
  if (strncmp(cmd, "ESTP", 3)  == 0){
    if (sscanf(cmd, "ESTP%hhi=%i", &channel, &intVal) != 2) {
      D_println("Invalid encoder step size command."); return -1;
    }
    encoders.setStepSize(channel, intVal);
    encoders.resetEncoderReference(channel);
    return 0;
  }

  // check for set encoder direction command
  if (strncmp(cmd, "EDIR", 3)  == 0){
    if (sscanf(cmd, "EDIR%hhi=%i", &channel, &intVal) != 2) {
      D_println("Invalid encoder direction command."); return -1;
    }
    encoders.setDirection(channel, intVal);
    encoders.resetEncoderReference(channel);
    return 0;
  }

  return 0;
}


// ----------------------------
// Periodically sends updates to the controller
// ----------------------------

void ControllerComm::sendUpdatesToController(void)
{
  Joystick& joystick = Joystick::getInstance();
  Encoders& encoders = Encoders::getInstance();
  static unsigned long lastUARTSendTime = 0;
  char uartData[MSG_MAXLENGTH+1]; // one extra for the null char
  size_t len = 0;
  int32_t intVal;
  int8_t valChanged = 0;

  unsigned long currentTime = millis();

  if (currentTime - lastUARTSendTime > UART_SEND_INTERVAL_MS) {
    
    uartData[0]='\0';
    for (int8_t idx=0; idx<MAX_NUM_MOTORS; idx++) {
      if (!isRemoteControlled[idx]) continue; // skip if not remote controlled
      if (isJoystickControlled[idx] && joystick.isChannelPresent(idx)) {
        valChanged = joystick.getUpdatedValue(idx, intVal);
        if ( valChanged)  
          len += snprintf(uartData+len, sizeof(uartData)-len, "VEL%hhi=%i;", idx, intVal);
      } else if ( encoders.isChannelPresent(idx) ) { // encoder controlled
        if (encoders.getChangedPosition(idx, intVal))
          len += snprintf(uartData+len, sizeof(uartData)-len, "POS%hhi=%i;", idx, intVal);
      }
    }
    if (len>0)
      *(uartData+len-1) = '\0'; // overwrite the last ';'
    else
      return; // nothing in the string
    sendCommand(uartData);
    lastUARTSendTime = currentTime;
  }
}


// ----------------------------
// Sends a command to the controller
// ----------------------------

void ControllerComm::sendCommand(const char *cmd)
{
  uint8_t checksum = calculateChecksum(cmd);
  D_print("send: ");
  D_println(cmd);
  Serial1.printf("<%s|%hhu>", cmd, checksum);
}


// ----------------------------
// Check if any change of input mode is needed
// ----------------------------

void ControllerComm::inputModeCheck(void)
{
  Joystick& joystick = Joystick::getInstance();
  Encoders& encoders = Encoders::getInstance();
  static unsigned long lastModeUpdateTime = 0;
  static unsigned long lastButtonActionTime[MAX_NUM_MOTORS] = {0};      
  char uartData[MSG_MAXLENGTH+1]; // one extra for the null char


  unsigned long currentTime = millis();

  if (currentTime - lastModeUpdateTime > INPUT_MODE_CHECK_INTERVAL_MS) {
    // go through all the channels
    for (int8_t idx=0; idx<MAX_NUM_MOTORS; idx++) {

      if (!encoders.getButtonPressed(idx) && !joystick.getButtonPressed(idx)) {
        continue; // no button pressed, skip this channel
      }

      // some button was pressed, but only do something if debounce time has elapsed
      if (currentTime - lastButtonActionTime[idx] > INPUT_MODE_DEBOUNCE_TIMEOUT_MS)
      {
        // request access if required
        if (!isRemoteControlled[idx]) { // request remote control
          snprintf(uartData, sizeof(uartData), "ACCREQ%hhi", idx);
          sendCommand(uartData);
        }

        if (isJoystickControlled[idx]) {

          if (encoders.getButtonPressed(idx)) {
            D_print("Switching to encoder input mode on channel "); D_println(idx);
            isJoystickControlled[idx] = 0;
            // reset the encoder reference position
            encoders.resetEncoderReference(idx);
          }

        } else { // encoder controlled

          if (joystick.getButtonPressed(idx)) {
            D_print("Switching to joystick input mode on channel "); D_println(idx);    
            isJoystickControlled[idx] = 1;
          }
        }

        encoders.resetButtonPressed(idx); // reset the button
        joystick.resetButtonPressed(idx); // make sure the button is cleared
        lastButtonActionTime[idx] = currentTime; // update the last action time
      }
    }
    lastModeUpdateTime = currentTime;
  } // if (currentTime - lastModeUpdateTime > INPUT_MODE_CHECK_INTERVAL_MS)
}


// ----------------------------
// Validates the checksum of the input string
// ----------------------------

int8_t ControllerComm::validateChecksum(char *data)
{
  uint8_t checksum;

  char *sepChar = strchr(data, '|');
  if (sepChar == NULL)
  {
    D_println("Could not find checksum.");
    return -1;
  }
  sscanf(sepChar + 1, "%hhu", &checksum);
  // overwrite the sepChar with the EOS
  *sepChar = '\0';
  if (calculateChecksum(data + 1) != checksum) // ignore the beginning '<' char
  {
    D_println("Checksum mismatch.");
    return -1;
  }
  return 0;
}


// ----------------------------
// Calculates the checksum for a given data string
// ----------------------------

uint8_t ControllerComm::calculateChecksum(const char *data)
{
  uint8_t sum = 0;

  while (*data) {
      sum += *data++;
  }
  return sum;
}

