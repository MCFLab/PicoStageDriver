// *****************************************************************************************
//
// Header file for the Stage Driver - provides interface functions for controlling
// Pico motor stages via VISA serial communication
//
// *****************************************************************************************

#ifndef STAGE_DRIVER_H
#define STAGE_DRIVER_H

// Maximum length for instrument response strings from the stage driver
#define SD_MAX_INSTR_RESP_LENGTH 1024
// Maximum length for commands sent to the stage driver
#define SD_MAX_COMMAND_LENGTH	100




// *****************************************************************************************
// Exported function prototypes
// *****************************************************************************************

// ============ Initialization and Cleanup ============

// Initializes the stage driver connection to the device at the specified address
// Returns: 0 on success, -1 on failure
int SD_Init(int *handle, const char *address);

// Closes the stage driver connection and releases resources
// Returns: 0 on success, -1 on failure
int SD_Close(int *handle);

// ============ Motor Parameters (configuration settings) ============

// Gets a motor parameter value (e.g., current scaling, speed, acceleration)
// Returns: 0 on success, -1 on failure
int SD_GetMotorParameter(int handle, int motor, const char *param, int *value);

// Sets a motor parameter value
// Returns: 0 on success, -1 on failure
int SD_SetMotorParameter(int handle, int motor, const char *param, int value);

// ============ Remote Control Parameters ============

// Gets a remote control parameter (e.g., joystick direction, encoder step size)
// Returns: 0 on success, -1 on failure
int SD_GetRemoteParameter(int handle, int motor, const char *param, int *value);

// Sets a remote control parameter
// Returns: 0 on success, -1 on failure
int SD_SetRemoteParameter(int handle, int motor, const char *param, int value);

// ============ Motor Commands (actions like move, home, etc.) ============

// Gets the status of a motor command (e.g., whether position has been reached)
// Returns: 0 on success, -1 on failure
int SD_GetMotorCommand(int handle, int motor, const char *param, int *value);

// Executes a motor command (e.g., find home, move to position)
// Returns: 0 on success, -1 on failure
int SD_SetMotorCommand(int handle, int motor, const char *param, int value);

// ============ Motor Status Information (read-only state) ============

// Gets motor status information (e.g., actual position, temperature)
// Returns: 0 on success, -1 on failure
int SD_GetMotorStatus(int handle, int motor, const char *param, int *value);

// Sets motor status values (only for settable status parameters)
// Returns: 0 on success, -1 on failure
int SD_SetMotorStatus(int handle, int motor, const char *param, int value);

// ============ Pico Controller Commands ============

// Gets a Pico controller command value (e.g., number of devices, firmware version)
// Returns: 0 on success, -1 on failure
int SD_GetPicoCommand(int handle, const char *param, int *value);

// Executes a Pico controller command (e.g., save configuration)
// Returns: 0 on success, -1 on failure
int SD_SetPicoCommand(int handle, const char *param);

// ============ Direct Register Access ============

// Gets the raw value from a device register for direct hardware control
// Returns: 0 on success, -1 on failure
int SD_GetRegisterValue(int handle, int motor, int reg, int *value);

// Sets a raw value in a device register for direct hardware control
// Returns: 0 on success, -1 on failure
int SD_SetRegisterValue(int handle, int motor, int reg, int value);

// ============ Parameter Name Lookup ============

// Returns a pointer to the array of motor parameter names and the count
// Returns: 0 on success, -1 on failure
int SD_GetMotorParameterNames(const char ***array, size_t *count);

// Returns a pointer to the array of remote parameter names and the count
// Returns: 0 on success, -1 on failure
int SD_GetRemoteParameterNames(const char ***array, size_t *count);

// ============ Direct Communication ============

// Sends a raw command string directly to the device and gets the response
// Returns: 0 on success, -1 on failure
int SD_SendDirectCommand(int handle, const char *command, char* response, int bufSize);

// Retrieves the most recent error message from the device
// Returns: 0 on success, -1 on failure
int SD_GetErrorMessage(int handle, char* response, int bufSize);

// ============ Configuration File I/O ============

// Loads motor and remote parameters from a JSON configuration file and applies them to the device
// Returns: 0 on success, -1 on failure
int SD_LoadConfigFromFile(int handle, char *fileName);

// Reads the current motor and remote parameters from the device and saves them to a JSON file
// Returns: 0 on success, -1 on failure
int SD_SaveConfigToFile(int handle, char *fileName);



#endif // STAGE_DRIVER_H
