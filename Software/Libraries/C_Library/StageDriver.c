

// *****************************************************************************************
//
// Interface functions for the Pico Stage Driver - provides high-level control over
// Pico motor stages via VISA/serial communication. Handles parameter configuration,
// motor commands, and configuration file I/O with JSON format.
//
// *****************************************************************************************

#include <ansi_c.h>
#include <visa.h>
#include <userint.h>

#include "StageDriver.h"
#include "cJSON.h"


// *****************************************************************************************
// Configuration Defines
// *****************************************************************************************

// Expected response prefix when identifying the device
#define SD_ID_RESPONSE	"Stage Driver Pico"

// Serial communication settings
#define SERIAL_BAUDRATE	115200         // Baud rate for serial communication
#define SERIAL_TERMCHAR	0xA            // Line terminator character (newline)

// Buffer size limits
#define MAX_ERROR_STRING_LENGTH 1024    // Maximum length for error message strings
#define MAX_FORMAT_STRING_LENGTH 100    // Maximum length for command format strings


// *****************************************************************************************
// Command Lists and Parameter Mappings
// *****************************************************************************************
// These arrays maintain parallel lists of user-friendly parameter names and their
// corresponding command codes used in VISA communication with the device

// Motor Parameter Commands: Control motor behavior and configuration
// Includes current scaling, operating modes, homing settings, encoder tuning,
// limit switches, and device identification
static const char *motorParameterCommands[] = 
	{
		"MP_CSCA", "MP_CRAN", "MP_CRUN", "MP_CHOL",
		"MP_MMIC", "MP_MINV", "MP_MTOF", "MP_MSGE", "MP_MSGT", "MP_MTCT",
		"MP_HMOD", "MP_HDIR", "MP_HVEL", "MP_HSST", "MP_HNEV",
		"MP_RSEV", "MP_RMXV", "MP_RSEA", "MP_RMXA",
		"MP_ECON", "MP_EDEV", "MP_ETOL", "MP_EMAX", "MP_ERST",
		"MP_SLEN", "MP_SREN", "MP_SLPO", "MP_SRPO", "MP_SSWP",
		"MP_LENC", "MP_LLEN", "MP_LREN", "MP_LLPS", "MP_LRPS",
		"MP_TDEV", "MP_TAXI"
	};

// User-friendly names for motor parameters (parallel to motorParameterCommands)
static const char *motorParameterNames[] = 
	{
		"CurrScaler", "CurrRange", "CurrRun", "CurrHold",
		"ModeMicroStep", "ModeInvDir", "ModeTOff", "ModeSGEnable", "ModeSGT", "ModeTCT",
		"HomingMode", "HomingDirection", "HomingVelocity", "HomingSoftStop", "HomingIndexEvent",
		"RateSetVelocity", "RateMaxVelocity", "RateSetAcc", "RateMaxAcc",
		"EncConstant", "EncDeviation", "EncLoopTolerance", "EncLoopMax", "EncResetXafterCL",
		"SwitchLeftEnable", "SwitchRightEnable", "SwitchLeftPolarity", "SwitchRightPolarity", "SwitchSwap",
		"LimEncoder", "LimLeftEnable", "LimRightEnable", "LimLeftPosition", "LimRightPosition",
		"TypeDevice", "TypeAxis"
	};

// Remote Control Parameters: Joystick and encoder settings for manual control
static const char *remoteParameterNames[] = 
	{
		"RemoteEnabled", "JoystickDirection", "JoystickMax", "EncoderDirection", "EncoderStepSize"
	};

// Command codes for remote parameters (parallel to remoteParameterNames)
static const char *remoteParameterCommands[] = 
	{
		"RP_ENAB", "RP_JDIR", "RP_JMAX", "RP_EDIR", "RP_ESTP"
	};

// Motor Status Values: Real-time information about motor state (read-only)
static const char *motorStatusNames[] = 
	{
		"ActualPosition", "TargetPosition", "EncoderPosition",
		"TargetVelocity", "TargetAcc", "Enabled", "Temperature", "LastPullInTries"
	};

// Command codes for reading motor status (parallel to motorStatusNames)
static const char *motorStatusCommands[] = 
	{
		"MS_XACT", "MS_XTAR", "MS_XENC",
		"MS_VELO", "MS_ACCE", "MS_ENAB", "MS_TEMP", "MS_PULL"
	};

// Motor Commands: Actions to execute on the motor (home, move, configure, etc.)
static const char *motorCommandNames[] =
	{
		"FindHome", "Config", "StatusClear",
		"MoveToPosition", "MoveAtVelocity", "HasPositionReached", "GetStatus"
	};

// Command codes for motor actions (parallel to motorCommandNames)
static const char *motorCommandCommands[] =
	{
		"MC_HOME", "MC_CONF", "MC_SCLR",
		"MC_MPOS", "MC_MVEL", "MC_POSR", "MC_STAT"
	};


// *****************************************************************************************
// Global Variables
// *****************************************************************************************

// VISA resource manager handle - manages all serial connections
static ViSession g_resManager = 0;


// *****************************************************************************************
// Internal Function Prototypes
// *****************************************************************************************
// These helper functions implement the core functionality and are not exposed to users

// Generic getter for parameters/status values - searches name lists for matching parameter
static int SD_GetRequest(int handle, const char* nameListPtr[], const char* commandListPtr[], int numCommands,
												 int motor, const char *paramName, int *value);

// Generic setter for parameters/status values - searches name lists for matching parameter
static int SD_SetRequest(int handle, const char* nameListPtr[], const char* commandListPtr[], int numCommands,
												 int motor, const char *paramName, int value);

// Gets a single value from a motor using its command code
static int SD_GetMotorValue(int handle, int motor, const char *command, int *value);

// Sets a single value on a motor using its command code
static int SD_SetMotorValue(int handle, int motor, const char *command, int value);

// Low-level VISA communication - sends command and receives response
static int SD_SendCommandGetResponse(int handle, const char *command, char *responseStr);

// Checks if a response indicates an error and retrieves the error message
static int checkErrorResponse(int handle, char* response);

// JSON conversion helper - extracts parameter from device and adds to JSON object
static int SD_ToJSONFromParam(int handle, int motor, cJSON *object, char *paramName, char *paramCommand);

// JSON conversion helper - reads parameter from JSON object and sets it on device
static int SD_SetParamFromJSON(int handle, int motor, cJSON *object, char *paramName, char *paramCommand);

// Error reporting functions - log errors with location information
static void reportError(int line, const char* function, char* description );
static void reportVisaError(int line, const char* function, ViSession instr, ViStatus errStatus );
static void reportSDError(int line, const char* function, char* description );

// Utility function - removes trailing carriage return and newline characters
void stripEndChars(char *s);

// *****************************************************************************************
// Exported Functions - Implementation
// *****************************************************************************************

////////////////////////////////////////////////////////
// Initialize Connection to Stage Driver Device
////////////////////////////////////////////////////////
// Opens a VISA connection to the stage driver at the specified address,
// configures serial port parameters, and verifies device identity.
// 
// Parameters:
//   handle     - Pointer to int to receive the connection handle
//   address    - Device address string (e.g., "COM1" or VISA address)
//
// Returns: 0 on success, -1 on failure
int SD_Init(int *handle, const char *address)
{
	ViSession io = 0;
	ViStatus status;
	unsigned char instrResp[SD_MAX_INSTR_RESP_LENGTH];
	ViUInt32 charsRead;
	int isLocked=0;

	if (*handle != 0) {
		reportError (__LINE__-2, __func__, "Stage driver already open.");
		goto fail;
	}
	status = viOpenDefaultRM(&g_resManager);
	if(status) {
		reportError (__LINE__-2, __func__, "Could not get access to the VISA resource manager.");
		goto fail;
	}
	status = viOpen (g_resManager, address, VI_NULL, 1000, &io);
	if(status) {
		reportVisaError (__LINE__-2, __func__, io, status);
		goto fail;
	}
	status = viLock (io, VI_EXCLUSIVE_LOCK, 5000, VI_NULL, VI_NULL);
	if(status) {
		reportVisaError (__LINE__-2, __func__, io, status);
		goto fail;
	}
	isLocked=1;
	
	viSetAttribute(io, VI_ATTR_ASRL_BAUD, SERIAL_BAUDRATE);
	viSetAttribute(io, VI_ATTR_ASRL_DATA_BITS, 8);
	viSetAttribute(io, VI_ATTR_ASRL_STOP_BITS, VI_ASRL_STOP_ONE);
	viSetAttribute(io, VI_ATTR_ASRL_PARITY, VI_ASRL_PAR_NONE);
	viSetAttribute(io, VI_ATTR_ASRL_FLOW_CNTRL, VI_ASRL_FLOW_NONE);
	viSetAttribute(io, VI_ATTR_TERMCHAR, SERIAL_TERMCHAR);
	viSetAttribute(io, VI_ATTR_ASRL_END_IN, VI_ASRL_END_TERMCHAR);

	// ask for identification; here I use printf/read because of the spaces in the return string
	status = viPrintf(io, "*IDN?\n");
	if(status) {
		reportVisaError (__LINE__-2, __func__, io, status);
		goto fail;
	}
	status = viRead (io, instrResp, SD_MAX_INSTR_RESP_LENGTH, &charsRead);
	if(status) {
		reportVisaError (__LINE__-2, __func__, io, status);
		goto fail;
	}
	if (charsRead<2) {
		reportError (__LINE__-6, __func__, "No ID response received.");
		goto fail;
	}
	instrResp[charsRead]='\0';
	stripEndChars((char *)instrResp);
	if ( strncmp ((char *)instrResp, SD_ID_RESPONSE, strlen(SD_ID_RESPONSE)) != 0) {
		reportError (__LINE__-1, __func__, "Device is not a stage driver.");
		goto fail;
	}
	status = viUnlock (io);
	if(status) {
		reportVisaError (__LINE__-2, __func__, io, status);
		goto fail;
	}

	*handle = (int) io;
	return 0;
	
fail:
	if (io) {
		if (isLocked) viUnlock(io);
		viClose(io);
		io=0;
	}
	if (g_resManager) {
		viClose(g_resManager);
		g_resManager = 0;
	}
	return -1;
}


////////////////////////////////////////////////////////
// Close Connection to Stage Driver Device
////////////////////////////////////////////////////////
// Closes the VISA connection to the stage driver and releases all resources
// including the connection handle and resource manager.
//
// Parameters:
//   handle - Pointer to the connection handle (will be set to 0 on success)
//
// Returns: 0 on success, -1 on failure
int SD_Close(int *handle)
{
	ViStatus status;
	if (*handle) {
		status = viClose((ViSession) *handle);
		if(status) {
			reportVisaError (__LINE__-2, __func__,(ViSession) *handle, status);
			return -1;
		}
		*handle = 0; // set handle to zero
	}
	if (g_resManager) {
		status = viClose(g_resManager);
		if(status) {
			reportError (__LINE__-2, __func__, "Unable to close resource manager.");
			return -1;
		}
		g_resManager = 0; // set handle to zero
	}
	return 0;
}



////////////////////////////////////////////////////////
// Wrapper Functions - Motor Parameters
////////////////////////////////////////////////////////
// These functions provide a high-level interface for getting/setting motor parameters
// by handling the lookup and command generation internally.

int SD_GetMotorParameter(int handle, int motor, const char *paramName, int *value)
{
	int numCommands = sizeof(motorParameterCommands) / sizeof(motorParameterCommands[0]);
	int status = SD_GetRequest(handle, motorParameterNames, motorParameterCommands, numCommands,
							  motor, paramName, value);
	return status;
}

// Sets a motor parameter value by name
// Returns: 0 on success, -1 on failure or unknown parameter name
int SD_SetMotorParameter(int handle, int motor, const char *paramName, int value)
{
	int numCommands = sizeof(motorParameterCommands) / sizeof(motorParameterCommands[0]);
	int status = SD_SetRequest(handle, motorParameterNames, motorParameterCommands, numCommands,
							  motor, paramName, value);
	return status;
}

// Gets a remote control parameter value by name (e.g., "JoystickDirection", "EncoderStepSize")
// Returns: 0 on success, -1 on failure or unknown parameter name
int SD_GetRemoteParameter(int handle, int motor, const char *paramName, int *value)
{
	int numCommands = sizeof(remoteParameterCommands) / sizeof(remoteParameterCommands[0]);
	int status = SD_GetRequest(handle, remoteParameterNames, remoteParameterCommands, numCommands,
							  motor, paramName, value);
	return status;
}

// Sets a remote control parameter value by name
// Returns: 0 on success, -1 on failure or unknown parameter name
int SD_SetRemoteParameter(int handle, int motor, const char *paramName, int value)
{
	int numCommands = sizeof(remoteParameterCommands) / sizeof(remoteParameterCommands[0]);
	int status = SD_SetRequest(handle, remoteParameterNames, remoteParameterCommands, numCommands,
							  motor, paramName, value);
	return status;
}

// Gets current motor status information by name (e.g., "ActualPosition", "Temperature")
// Returns: 0 on success, -1 on failure or unknown parameter name
int SD_GetMotorStatus(int handle, int motor, const char *paramName, int *value)
{
	int numCommands = sizeof(motorStatusCommands) / sizeof(motorStatusCommands[0]);
	int status = SD_GetRequest(handle, motorStatusNames, motorStatusCommands, numCommands,
							  motor, paramName, value);
	return status;
}

// Sets motor status values (for settable status parameters only)
// Returns: 0 on success, -1 on failure, or if parameter is not settable
int SD_SetMotorStatus(int handle, int motor, const char *paramName, int value)
{
	char errStr[MAX_ERROR_STRING_LENGTH];

	// some values are not settable
	if (strcmp(paramName, "MS_TEMP")==0 || strcmp(paramName, "MS_PULL")==0) {
		snprintf(errStr, MAX_ERROR_STRING_LENGTH, "Not a settable motor status command: %s.", paramName);
		reportError (__LINE__-2, __func__, errStr);
		return -1;
	}
	int numCommands = sizeof(motorStatusCommands) / sizeof(motorStatusCommands[0]);
	int status = SD_SetRequest(handle, motorStatusNames, motorStatusCommands, numCommands,
							  motor, paramName, value);
	return status;
}

// Gets the status of a motor command (e.g., whether position reached)
// Note: Some commands cannot be queried and will return an error
// Returns: 0 on success, -1 on failure or if parameter is not gettable
int SD_GetMotorCommand(int handle, int motor, const char *paramName, int *value)
{
	char errStr[MAX_ERROR_STRING_LENGTH];

	// some values are not gettable
	if (strcmp(paramName, "MC_HOME")==0 || strcmp(paramName, "MC_CONF")==0 || strcmp(paramName, "MC_SCLR")==0
		  || strcmp(paramName, "MC_MPOS")==0 || strcmp(paramName, "MC_MVEL")==0) {
		snprintf(errStr, MAX_ERROR_STRING_LENGTH, "Not a gettable motor command: %s.", paramName);
		reportError (__LINE__-2, __func__, errStr);
		return -1;
	}
	int numCommands = sizeof(motorCommandCommands) / sizeof(motorCommandCommands[0]);
	int status = SD_GetRequest(handle, motorCommandNames, motorCommandCommands, numCommands,
							  motor, paramName, value);
	return status;
}

// Executes a motor command (e.g., "FindHome", "MoveToPosition", "Config")
// Note: Some commands cannot be set directly
// Returns: 0 on success, -1 on failure or if parameter is not settable
int SD_SetMotorCommand(int handle, int motor, const char *paramName, int value)
{
	char errStr[MAX_ERROR_STRING_LENGTH];

	// some values are not settable
	if (strcmp(paramName, "MC_POSR")==0 || strcmp(paramName, "MC_STAT")==0) {
		snprintf(errStr, MAX_ERROR_STRING_LENGTH, "Not a settable motor status command: %s.", paramName);
		reportError (__LINE__-2, __func__, errStr);
		return -1;
	}
	int numCommands = sizeof(motorCommandCommands) / sizeof(motorCommandCommands[0]);
	int status = SD_SetRequest(handle, motorCommandNames, motorCommandCommands, numCommands,
							  motor, paramName, value);
	return status;
}


////////////////////////////////////////////////////////
// Pico Controller Commands - Get Information
////////////////////////////////////////////////////////
// Queries the Pico controller for device information.
// Supported commands: "PC_NDEV" (number of devices), "PC_VERS" (firmware version)
//
// Returns: 0 on success, -1 on failure or unsupported command
int SD_GetPicoCommand(int handle, const char *command, int *value)
{
	int err = 0;
	char instrResp[SD_MAX_INSTR_RESP_LENGTH];
	char commandStr[MAX_FORMAT_STRING_LENGTH];
	char errStr[MAX_ERROR_STRING_LENGTH];
	char formatStr[MAX_FORMAT_STRING_LENGTH];
	
	if (strcmp(command, "PC_NDEV")==0 || strcmp(command, "PC_VERS")==0)	
		snprintf(commandStr, MAX_FORMAT_STRING_LENGTH, "G%s", command);
	else {
		snprintf(errStr, MAX_ERROR_STRING_LENGTH, "Not a gettable Pico command: %s.", command);
		reportError (__LINE__-1, __func__, errStr);
		return -1;
	}		
	err = SD_SendCommandGetResponse(handle,commandStr, instrResp);
	if(err) {
		snprintf(errStr, MAX_ERROR_STRING_LENGTH, "Could not get value from Pico in reponse to %s.", commandStr);
		reportError (__LINE__-2, __func__, errStr);
		return -1;
	}
	snprintf(formatStr, MAX_FORMAT_STRING_LENGTH, "%s=%%d", command);
	if (sscanf(instrResp, formatStr, value) != 1) {
		reportError (__LINE__-1, __func__, "Invalid number of parameters received (want 1).");
		return -1;
	}
	
	return 0;
}


////////////////////////////////////////////////////////
// Pico Controller Commands - Set/Execute Operations
////////////////////////////////////////////////////////
// Executes commands on the Pico controller.
// Supported commands: "PC_SAFL" (save configuration to flash memory)
//
// Returns: 0 on success, -1 on failure or unsupported command
int SD_SetPicoCommand(int handle, const char *command)
{
	int err = 0;
	char instrResp[SD_MAX_INSTR_RESP_LENGTH];
	char commandStr[MAX_FORMAT_STRING_LENGTH];
	char errStr[MAX_ERROR_STRING_LENGTH];
	
	if (strcmp(command, "PC_SAFL")==0)	
		snprintf(commandStr, MAX_FORMAT_STRING_LENGTH, "S%s", command);
	else {
		snprintf(errStr, MAX_ERROR_STRING_LENGTH, "Not a settable Pico command: %s.", command);
		reportError (__LINE__-1, __func__, errStr);
		return -1;
	}		
	err = SD_SendCommandGetResponse(handle, commandStr, instrResp);
	if(err) {
		snprintf(errStr, MAX_ERROR_STRING_LENGTH, "Error setting Pico command with %s.", commandStr);
		reportError (__LINE__-2, __func__, errStr);
		return -1;
	}
	return 0;
}


////////////////////////////////////////////////////////
// Parameter Name Lookup - Get Available Parameter Names
////////////////////////////////////////////////////////
// These functions return pointers to the arrays of available parameter names.
// Useful for enumerating all supported parameters for user interfaces.

// Returns the list of available motor parameter names and the count
// Returns: 0 on success
int SD_GetMotorParameterNames(const char ***array, size_t *count)
{
  if (array) *array = motorParameterNames;
  if (count) *count = sizeof(motorParameterNames) / sizeof(motorParameterNames[0]);
	return 0;
}

// Returns the list of available remote control parameter names and the count
// Returns: 0 on success
int SD_GetRemoteParameterNames(const char ***array, size_t *count)
{
  if (array) *array = remoteParameterNames;
  if (count) *count = sizeof(remoteParameterNames) / sizeof(remoteParameterNames[0]);
	return 0;
}


////////////////////////////////////////////////////////
// Direct Communication - Low-Level Command Interface
////////////////////////////////////////////////////////
// Send a raw command directly to the device for advanced/custom operations.
// Response will be the raw string returned by the device.
//
// Parameters:
//   handle    - Device connection handle
//   command   - Raw command string (without newline)
//   response  - Buffer to receive the device response
//   bufSize   - Size of response buffer
//
// Returns: 0 on success, -1 on failure
int SD_SendDirectCommand(int handle, const char *command, char* response, int bufSize)
{
	int err = 0;
	char instrResp[SD_MAX_INSTR_RESP_LENGTH];
	err = SD_SendCommandGetResponse(handle, command, instrResp);
	if (err) return err;
	strncpy(response, instrResp, (bufSize<SD_MAX_INSTR_RESP_LENGTH ? bufSize : SD_MAX_INSTR_RESP_LENGTH));
	return 0;
}


////////////////////////////////////////////////////////
// Get Device Error Message
////////////////////////////////////////////////////////
// Retrieves the most recent error message that occurred on the device.
// Error messages are generated by the device and stored internally.
//
// Parameters:
//   handle    - Device connection handle
//   response  - Buffer to receive the error message
//   bufSize   - Size of response buffer
//
// Returns: 0 on success, -1 on failure
int SD_GetErrorMessage(int handle, char* response, int bufSize)
{
	return SD_SendDirectCommand(handle, "GPC_EMSG", response, bufSize);
}


////////////////////////////////////////////////////////
// Direct Register Access - Low-Level Hardware Control
////////////////////////////////////////////////////////
// These functions provide direct access to device registers for advanced users.
// Register access allows direct hardware manipulation if needed.

// Gets the raw value from a device register
// Parameters:
//   handle - Device connection handle
//   motor  - Motor/device number
//   reg    - Register number
//   value  - Pointer to receive the register value
//
// Returns: 0 on success, -1 on failure
int SD_GetRegisterValue(int handle, int motor, int reg, int *value)
{
	int err = 0;
	char instrResp[SD_MAX_INSTR_RESP_LENGTH];
	char commandStr[MAX_FORMAT_STRING_LENGTH];
	char errStr[MAX_ERROR_STRING_LENGTH];
	int respDev;
	
	snprintf(commandStr, MAX_FORMAT_STRING_LENGTH, "GMC_DREG%d,%d", motor, reg);
	err = SD_SendCommandGetResponse(handle, commandStr, instrResp);
	if(err) {
		snprintf(errStr, MAX_ERROR_STRING_LENGTH, "Error getting register %d in motor %d.", reg, motor);
		reportError (__LINE__-2, __func__, errStr);
		return -1;
	}
	if (sscanf(instrResp, "MC_DREG%d=%d", &respDev, value) != 2) {
		reportError (__LINE__-1, __func__, "Invalid number of parameters received (want 2).");
		return -1;
	}
	if (respDev!=motor) {
		reportError (__LINE__-1, __func__, "Responded with wrong motor number.");
		return -1;
	}
	return 0;
}


// Sets the raw value in a device register
// Parameters:
//   handle - Device connection handle
//   motor  - Motor/device number
//   reg    - Register number
//   value  - Value to write to the register
//
// Returns: 0 on success, -1 on failure
int SD_SetRegisterValue(int handle, int motor, int reg, int value)
{
	int err = 0;
	char instrResp[SD_MAX_INSTR_RESP_LENGTH];
	char commandStr[MAX_FORMAT_STRING_LENGTH];
	char errStr[MAX_ERROR_STRING_LENGTH];
	
	snprintf(commandStr, MAX_FORMAT_STRING_LENGTH, "SMC_DREG%d,%d,%d", motor, reg, value);
	err = SD_SendCommandGetResponse(handle, commandStr, instrResp);
	if(err) {
		snprintf(errStr, MAX_ERROR_STRING_LENGTH, "Error setting register %d in motor %d.", reg, motor);
		reportError (__LINE__-2, __func__, errStr);
		return -1;
	}
	return 0;
}


////////////////////////////////////////////////////////
// Configuration File I/O - Load Configuration from JSON File
////////////////////////////////////////////////////////
// Loads motor and remote parameters from a JSON configuration file and 
// applies them to all motors on the device.
//
// JSON Format Expected:
// {
//   "motor0": { "CurrRun": 100, "HomingVelocity": 50, ... },
//   "motor1": { ... },
//   ...
// }
//
// Parameters:
//   handle    - Device connection handle
//   fileName  - Path to JSON configuration file
//
// Returns: 0 on success, -1 on failure
int SD_LoadConfigFromFile(int handle, char *fileName)
{
	int maxNumDevs;
	char *data;
	char errStr[MAX_ERROR_STRING_LENGTH];
	int status;
	int numMotorParams = sizeof(motorParameterNames) / sizeof(motorParameterNames[0]);
	int numRemoteParams = sizeof(remoteParameterNames) / sizeof(remoteParameterNames[0]);

	// get the maximum number of motors
	status = SD_GetPicoCommand(handle, "PC_NDEV", &maxNumDevs);	
  if (status) {
		reportError (__LINE__-2, __func__, "Could not get max number of motors");
    return -1;
  }
	FILE *fp = fopen(fileName, "r");
  if (fp == NULL) {
		snprintf(errStr, MAX_ERROR_STRING_LENGTH, "Unable to open config file %s.", fileName);
		reportError (__LINE__-2, __func__, errStr);
    return -1;
  }
	// figure out how big of a buffer I need and allocate the buffer
  fseek(fp, 0, SEEK_END);
  long len = ftell(fp);
  rewind(fp);
	data = malloc(len + 1);
  if (!data) { fclose(fp); reportError (__LINE__-2, __func__, "Could not allocate memory."); return -1; }

	// read to contents
	fread(data, 1, len, fp);
  data[len] = '\0';  // null-terminate
  fclose(fp);
	
	// start parsing
	cJSON *root = cJSON_Parse(data);
  if (!root) {
		snprintf(errStr, MAX_ERROR_STRING_LENGTH, "JSON parse error:  %s.", cJSON_GetErrorPtr());
		reportError (__LINE__-3, __func__, errStr);
    return -1;
  }

  // Iterate over motors
  for (int idx = 0; idx < maxNumDevs; idx++) {
    char key[16];
    snprintf(key, sizeof(key), "motor%d", idx);

    cJSON *motorObj = cJSON_GetObjectItemCaseSensitive(root, key);
    if (!motorObj) continue;

		// motor parameters
		for (int iParam=0; iParam<numMotorParams; iParam++) {
			status = SD_SetParamFromJSON(handle, idx, motorObj, motorParameterNames[iParam], motorParameterCommands[iParam]);
			if (status) reportError (__LINE__-1, __func__, "Could not set parameter value");
		}
		// remote parameters
		for (int iParam=0; iParam<numRemoteParams; iParam++) {
			status = SD_SetParamFromJSON(handle, idx, motorObj, remoteParameterNames[iParam], remoteParameterCommands[iParam]);
			if (status) reportError (__LINE__-1, __func__, "Could not set parameter value");
		}
  }	
	cJSON_Delete(root);  // free memory
  return 0;
}


////////////////////////////////////////////////////////
// Configuration File I/O - Save Configuration to JSON File
////////////////////////////////////////////////////////
// Reads the current motor and remote parameters from the device and saves
// them to a JSON configuration file. The resulting file can be loaded later
// using SD_LoadConfigFromFile() to restore the configuration.
//
// Parameters:
//   handle    - Device connection handle
//   fileName  - Path to JSON configuration file (will be created/overwritten)
//
// Returns: 0 on success, -1 on failure
int SD_SaveConfigToFile(int handle, char *fileName)
{
	int maxNumDevs;
	char *data;
	char labelStr[20];
	int status;
	int numMotorParams = sizeof(motorParameterNames) / sizeof(motorParameterNames[0]);
	int numRemoteParams = sizeof(remoteParameterNames) / sizeof(remoteParameterNames[0]);

	// get the maximum number of motors
	status = SD_GetPicoCommand(handle, "PC_NDEV", &maxNumDevs);	
  if (status) {
		reportError (__LINE__-2, __func__, "Could not get max number of motors");
    return -1;
  }
  cJSON *root = cJSON_CreateObject();
  // Iterate over motors
  for (int idx = 0; idx < maxNumDevs; idx++) {
    cJSON *motorObj = cJSON_CreateObject();
		// motor parameters
		for (int iParam=0; iParam<numMotorParams; iParam++) {
			SD_ToJSONFromParam(handle, idx, motorObj, motorParameterNames[iParam], motorParameterCommands[iParam]);					
		}
		// remote parameters
		for (int iParam=0; iParam<numRemoteParams; iParam++) {
			SD_ToJSONFromParam(handle, idx, motorObj, remoteParameterNames[iParam], remoteParameterCommands[iParam]);					
		}
		snprintf(labelStr, 20, "motor%d", idx);
		cJSON_AddItemToObject(root, labelStr, motorObj);
  }

	data = cJSON_Print(root);  // pretty-printed JSON
  if (!data) return -1;
  FILE *fp = fopen(fileName, "w");
  if (fp) {
      fputs(data, fp);
      fclose(fp);
  }
  free(data);	
	
	cJSON_Delete(root);  // free memory
  return 0;
}



// *****************************************************************************************
// *****************************************************************************************
// Internal Helper Functions - Not Exposed to Users
// *****************************************************************************************
// *****************************************************************************************

////////////////////////////////////////////////////////
// Generic Parameter/Status/Command Getter
////////////////////////////////////////////////////////
// Searches the provided lists to find a matching parameter name, then retrieves its value.
// This is a helper function used by all the SD_Get* wrapper functions.
//
// Parameters:
//   nameListPtr     - Array of parameter names
//   commandListPtr  - Parallel array of command codes
//   numCommands     - Number of items in the arrays
//   paramName       - Name of parameter to get
//
// Returns: 0 on success, -1 if parameter name not found or communication error
static int SD_GetRequest(int handle, const char* nameListPtr[], const char* commandListPtr[], int numCommands,
												 int motor, const char *paramName, int *value)
{
	char errStr[MAX_ERROR_STRING_LENGTH];
	for (int idx = 0; idx < numCommands; ++idx) {
			if (strcmp(paramName, nameListPtr[idx]) == 0) {
					return SD_GetMotorValue(handle, motor, commandListPtr[idx], value);
			}
	}	
	snprintf(errStr, MAX_ERROR_STRING_LENGTH, "Unknown parameter: %s.", paramName);
	reportError (__LINE__-1, __func__, errStr);
	return -1;
}

// Generic Parameter/Status/Command Setter
// Searches the provided lists to find a matching parameter name, then sets its value.
// This is a helper function used by all the SD_Set* wrapper functions.
//
// Parameters:
//   nameListPtr     - Array of parameter names
//   commandListPtr  - Parallel array of command codes
//   numCommands     - Number of items in the arrays
//   paramName       - Name of parameter to set
//   value           - Value to set
//
// Returns: 0 on success, -1 if parameter name not found or communication error
static int SD_SetRequest(int handle, const char* nameListPtr[], const char* commandListPtr[], int numCommands,
												 int motor, const char *paramName, int value)
{
	char errStr[MAX_ERROR_STRING_LENGTH];
	for (int idx = 0; idx < numCommands; ++idx) {
			if (strcmp(paramName, nameListPtr[idx]) == 0) {
					return SD_SetMotorValue(handle, motor, commandListPtr[idx], value);
			}
	}	
	snprintf(errStr, MAX_ERROR_STRING_LENGTH, "Unknown parameter: %s.", paramName);
	reportError (__LINE__-1, __func__, errStr);
	return -1;
}


////////////////////////////////////////////////////////
// Low-Level Get Motor Value
////////////////////////////////////////////////////////
// Gets a single integer value from a motor using its command code (e.g., "MP_CRUN").
// This function handles the VISA communication and response parsing.
//
// Parameters:
//   command - Command code for the desired value (e.g., "MP_CRUN")
//   value   - Pointer to receive the value from the device
//
// Returns: 0 on success, -1 on communication error or invalid response
static int SD_GetMotorValue(int handle, int motor, const char *command, int *value)
{
	int err = 0;
	char errStr[MAX_ERROR_STRING_LENGTH];
	char instrResp[SD_MAX_INSTR_RESP_LENGTH];
	char commandStr[MAX_FORMAT_STRING_LENGTH];
	char formatStr[MAX_FORMAT_STRING_LENGTH];
	int respDev;

	snprintf(commandStr, MAX_FORMAT_STRING_LENGTH, "G%s%d,%d", command, motor, value);
	err = SD_SendCommandGetResponse(handle, commandStr, instrResp);
	if(err) {
		snprintf(errStr, MAX_ERROR_STRING_LENGTH, "Could not get value from motor in reponse to %s.", commandStr);
		reportError (__LINE__-2, __func__, errStr);
		return -1;
	}
		snprintf(formatStr, MAX_FORMAT_STRING_LENGTH, "%s%%d=%%d", command);
	if (sscanf(instrResp, formatStr, &respDev, value) != 2) {
		reportError (__LINE__-1, __func__, "Invalid number of parameters received (want number and value).");
		return -1;
	}
	if (respDev!=motor) {
		reportError (__LINE__-1, __func__, "Responded with wrong motor number.");
		return -1;
	}
	return 0;	
}
	

////////////////////////////////////////////////////////
// Low-Level Set Motor Value
////////////////////////////////////////////////////////
// Sets a single integer value on a motor using its command code (e.g., "MP_CRUN").
// This function handles the VISA communication and device response validation.
//
// Parameters:
//   command - Command code for the value to set (e.g., "MP_CRUN")
//   value   - Value to write to the device
//
// Returns: 0 on success, -1 on communication error or device rejection
static int SD_SetMotorValue(int handle, int motor, const char *command, int value)
{
	int err = 0;
	char errStr[MAX_ERROR_STRING_LENGTH];
	char instrResp[SD_MAX_INSTR_RESP_LENGTH];
	char commandStr[MAX_FORMAT_STRING_LENGTH];

	snprintf(commandStr, MAX_FORMAT_STRING_LENGTH, "S%s%d,%d", command, motor, value);
	err = SD_SendCommandGetResponse(handle, commandStr, instrResp);
	if(err) {
		snprintf(errStr, MAX_ERROR_STRING_LENGTH, "Could not set value for motor with %s.", commandStr);
		reportError (__LINE__-2, __func__, errStr);
		return -1;
	}
	return 0;	
}


////////////////////////////////////////////////////////
// VISA Communication - Send Command and Get Response
////////////////////////////////////////////////////////
// Low-level VISA communication function. Sends a command string to the device
// via serial/VISA and waits for the response. Handles device locking, error checking,
// and response validation.
//
// Protocol: Send "command\n", receive "RESPONSE=value\n" or "ERROR=code\n"
//
// Parameters:
//   command     - Command string to send (without newline)
//   responseStr - Buffer to receive device response
//
// Returns: 0 on success, -1 on communication error or device error response
static int SD_SendCommandGetResponse(int handle, const char *command, char *responseStr)
{
	ViStatus status;
	char errorResp[SD_MAX_INSTR_RESP_LENGTH];
	ViUInt32 charsRead;
	int isLocked=0;

	if (!handle) {
		reportError (__LINE__-1, __func__, "Device not open.");
		goto fail;
	}
	status = viLock ((ViSession) handle, VI_EXCLUSIVE_LOCK, 100, VI_NULL, VI_NULL);
	if(status) {
		reportVisaError (__LINE__-2, __func__, (ViSession) handle, status);
		goto fail;
	}
	isLocked=1;

	status = viPrintf((ViSession) handle, "%s\n", command);
	if(status) {
		reportVisaError (__LINE__-2, __func__, (ViSession) handle, status);
		goto fail;
	}
	status = viRead ((ViSession) handle, (unsigned char*)responseStr, SD_MAX_INSTR_RESP_LENGTH, &charsRead);
	if(status) {
		reportVisaError (__LINE__-2, __func__, (ViSession) handle, status);
		goto fail;
	}
	if (charsRead<7) { // at least it should return "XX_XXXX=" or "ERROR=..."
		reportError (__LINE__-1, __func__, "No command response received.");
		goto fail;
	}
	// make sure the response is null-terminated
	responseStr[charsRead]='\0';
	stripEndChars(responseStr);

	// check for error response
	if (strnicmp(responseStr, "ERROR=", 6)==0 && responseStr[6]!='0'){
		responseStr[charsRead-2]='\0';	
		reportSDError (__LINE__-2, __func__, responseStr);
		checkErrorResponse(handle, errorResp);
		reportSDError (__LINE__-4, __func__, errorResp);
		goto fail;
	}

	status = viUnlock ((ViSession) handle);
	if(status) {
		reportVisaError (__LINE__-2, __func__, (ViSession) handle, status);
		goto fail;
	}
	return 0;

fail:
	if (isLocked) viUnlock((ViSession) handle);
	return -1;
}


////////////////////////////////////////////////////////
// JSON Helper - Get Parameter and Add to JSON Object
////////////////////////////////////////////////////////
// Retrieves a parameter value from the device and adds it to a JSON object.
// Used by SD_SaveConfigToFile to build the JSON configuration.
//
// Parameters:
//   object      - JSON object to add the parameter to
//   paramName   - User-friendly parameter name (key for JSON)
//   paramCommand - Command code to use for retrieving the value
//
// Returns: 0 on success, -1 on failure (but continues processing)
static int SD_ToJSONFromParam(int handle, int motor, cJSON *object, char *paramName, char *paramCommand)
{
	int paramVal;
	char errStr[MAX_ERROR_STRING_LENGTH];

	int status = SD_GetMotorValue(handle, motor, paramCommand, &paramVal);
	if (status) {
		snprintf(errStr, MAX_ERROR_STRING_LENGTH, "Unable to get parameter %s for motor %d.", paramName, motor);
		reportError (__LINE__-3, __func__, errStr);
	}
	cJSON_AddNumberToObject(object, paramName, paramVal);
	return 0;
}

// JSON Helper - Set Parameter from JSON Object
// Retrieves a parameter value from a JSON object and sets it on the device.
// Used by SD_LoadConfigFromFile to apply configuration from JSON.
//
// Parameters:
//   object      - JSON object containing the parameter
//   paramName   - User-friendly parameter name (key to lookup in JSON)
//   paramCommand - Command code to use for setting the value
//
// Returns: 0 on success or if parameter not found in JSON, -1 on error
static int SD_SetParamFromJSON(int handle, int motor, cJSON *object, char *paramName, char *paramCommand)
{
	char errStr[MAX_ERROR_STRING_LENGTH];

	cJSON *name = cJSON_GetObjectItemCaseSensitive(object, paramName); 
	if ( cJSON_IsNumber(name) ) {
		int paramVal = name->valueint;	
		int status = SD_SetMotorValue(handle, motor, paramCommand, paramVal);
		if (status) {
			reportError (__LINE__-1, __func__, "Could not set parameter value");
			snprintf(errStr, MAX_ERROR_STRING_LENGTH, "Unable to set parameter %s for motor %d.", paramName, motor);
			reportError (__LINE__-1, __func__, errStr);
		}
  }
	return 0;
}
	

////////////////////////////////////////////////////////
// Check for Error Response from Device
////////////////////////////////////////////////////////
// When the device returns an error, this function retrieves the error message
// from the device's internal error register and stores it in the response buffer.
// The device must be locked before calling this function.
//
// Parameters:
//   response - Buffer to receive the error message from the device
//
// Returns: 0 on success, -1 on communication error
// Note: Device should already be locked when this is called
static int checkErrorResponse(int handle, char *response)
{
	ViStatus status;
	ViUInt32 charsRead;	

	status = viPrintf((ViSession) handle, "GPC_EMSG\n");
	if(status) {
		reportVisaError (__LINE__-2, __func__, (ViSession) handle, status);
		return -1;
	}
	status = viRead ((ViSession) handle, (unsigned char*)response, SD_MAX_INSTR_RESP_LENGTH, &charsRead);
	if(status) {
		reportVisaError (__LINE__-2, __func__, (ViSession) handle, status);
		return -1;
	}
	// make sure the response is null-terminated
	response[charsRead]='\0';
	stripEndChars(response);

	return 0;
}


////////////////////////////////////////////////////////
// Error Reporting - Generic Errors
////////////////////////////////////////////////////////
// Prints a generic error message with location information (function and line number).
// If line or function is not provided, prints just the description.
//
// Parameters:
//   line         - Line number where error occurred (use __LINE__)
//   function     - Function name where error occurred (use __func__)
//   description  - Human-readable error description
static void reportError(int line, const char* function, char* description )
{
	if ((line!=0) && (function!=NULL))
		printf("\nError in function %s (line %i of file %s): %s\n", function, line, __FILE__, description );
	else 
		printf("%s\n", description );
}

////////////////////////////////////////////////////////
// Error Reporting - VISA Library Errors
////////////////////////////////////////////////////////
// Prints error messages generated by the VISA library.
// Uses viStatusDesc to convert VISA status codes to human-readable descriptions.
//
// Parameters:
//   line       - Line number where error occurred (use __LINE__)
//   function   - Function name where error occurred (use __func__)
//   instr      - VISA session handle (used to get error description)
//   errStatus  - VISA status code from failed operation
static void reportVisaError(int line, const char* function, ViSession instr, ViStatus errStatus )
{
	ViStatus status;
	char desc[SD_MAX_INSTR_RESP_LENGTH];

	status = viStatusDesc (instr, errStatus, desc);
	if (!status)
		printf("\nVisa error in function %s (line %i of file %s): %s\n", function, line, __FILE__, desc );
	else
		printf("\nVisa error in function %s (line %i of file %s), but could not get function description.\n",
					 function, line, __FILE__);
}

////////////////////////////////////////////////////////
// Error Reporting - Stage Driver Device Errors
////////////////////////////////////////////////////////
// Prints error messages returned by the stage driver device.
// These are protocol-level errors from the device itself.
//
// Parameters:
//   line         - Line number where error occurred (use __LINE__)
//   function     - Function name where error occurred (use __func__)
//   description  - Error description from the device
static void reportSDError(int line, const char* function, char* description )
{
	printf("\nSD returned error in function %s (line %i of file %s): %s\n", function, line, __FILE__, description );
}


////////////////////////////////////////////////////////
// Utility - Strip Trailing Whitespace
////////////////////////////////////////////////////////
// Removes trailing carriage return (\r) and newline (\n) characters from a string.
// Useful for cleaning up device responses that include line terminators.
//
// Parameters:
//   s - String to process (modified in-place)
void stripEndChars(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len-1] == '\r' || s[len-1] == '\n')) {
        s[--len] = '\0';  // replace with null terminator
    }
}

