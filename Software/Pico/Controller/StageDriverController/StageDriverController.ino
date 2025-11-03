#include <Arduino.h>
#include "Common.h"

#include "Parameters.h"
#include "Motors.h"
#include "RemoteComm.h"
#include "SerialComm.h"


//************************************************
// global variables
//************************************************
Motors g_motors = Motors();
Parameters g_params = Parameters();
RemoteComm g_remComm = RemoteComm();
SerialComm g_serComm = SerialComm();


//************************************************
// functions for debugging
//************************************************


//************************************************
// setup
//************************************************
void setup()
{
  // prep pin to read later
  pinMode(DEFAULT_STARTUP_PIN, INPUT_PULLUP);

  Serial.begin(115200);

  // initialize all the modules
  g_params.Init(&g_motors, &g_remComm);
  g_motors.Init(&g_params);
  delay(500); // wait for a bit for remote to boot up
  g_remComm.Init(&g_params, &g_motors, 100); // 100 ms timeout
  g_serComm.Init(&g_params, &g_motors, &g_remComm, 1000); // 1 s timeout

  // configuration of the Parameter modules also configures the Motor and Remote modules
  // The Serial module doesn't require a config
  // If the default button is pressed during the boot, the config bypasses the "load from flash"
  // and uses default "safe" configurations with the given number of motors instead
  
  if (digitalRead(DEFAULT_STARTUP_PIN)==LOW) { // pin pressed during boot
    g_params.Config(CONFIG_DEFAULT); // set to default motors
  } else {
    g_params.Config(CONFIG_LOAD_FROM_FLASH); // load from flash
  }

}

//************************************************
// loop
//************************************************
void loop()
{

  g_serComm.CheckSerialCommand();
  g_motors.ProcessUpdateChanges();
#if REMOTE_ENABLED
  g_remComm.SendPositionUpdates();
  g_remComm.CheckRemoteCommands();
#endif // REMOTE_ENABLED
}
