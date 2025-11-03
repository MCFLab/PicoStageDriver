

/**
 * @file main.cpp
 * @brief Main entry point for the StageDriverRemote controller firmware.
 *
 * This file initializes and manages the main components of the remote controller,
 * including encoders, sensitivity adjustment, joystick, display, and controller communication.
 * It sets up hardware interfaces, calibrates the joystick, and manages the main control loop
 * for sending and receiving updates to and from the stage controller.
 *
 * Components initialized:
 * - Encoders: Handles rotary encoder input.
 * - SensAdjust: Manages sensitivity adjustment.
 * - Joystick: Reads and calibrates joystick input.
 * - Display: Manages the user interface display.
 * - ControllerComm: Handles communication with the stage controller.
 *
 * The main loop is responsible for:
 * - Sending movement commands to the controller.
 * - Receiving updates from the controller (such as position or mode changes).
 * - Checking for input mode switches.
 *
 * Debug output is available via serial if SERIAL_DEBUG is enabled.
 */

#include "Encoders.h"
#include "SensAdjust.h"
#include "Joystick.h"
#include "Display.h"
#include "ControllerComm.h"

#define SERIAL_DEBUG  0
#include "Serial_Debug.h"




// ----------------------------
// Initializes hardware and software components for the remote controller.
// ----------------------------

void setup() {

  D_SerialBegin(115200);
  delay(100); // delay for a bit to establish connection
  
  D_println("\nHello from remote controller");
  
  Encoders::getInstance().init();
  SensAdjust::getInstance().init();
  
  Joystick& joystick = Joystick::getInstance();
  joystick.init();
  
  joystick.setCenterMargin(0, 2);
  joystick.setMaxValue(0, 10000);
  joystick.setDirection(0, -1);
  
  joystick.setCenterMargin(1, 2);
  joystick.setMaxValue(1, 10000);
  joystick.setDirection(1, -1);
  
  // do some updates to get a baseline for the center position
  delay(2*ADC_UPDATE_INTERVAL_MS*(1<<ADC_AVERAGING_BASE));
  joystick.updateCalibration(0);
  joystick.updateCalibration(1);
  D_println("Joystick calibration complete.");

  Display::getInstance().init();
  ControllerComm::getInstance().init(100); // 100 ms timeout

}


// ----------------------------
// Main loop function for handling controller communication and input mode checks.
// ----------------------------

void loop()
{
  ControllerComm& controller = ControllerComm::getInstance();

  // send move commands to the controller (if channel is in remote control mode)
  controller.sendUpdatesToController();

  // check for updates from the controller (e.g. position or mode updates)
  controller.receiveUpdatesFromController();

  // check for a switch in input mode
  controller.inputModeCheck();

}

