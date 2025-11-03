#include <cvirte.h>		
#include <utility.h>
#include <formatio.h>
#include <ansi_c.h>
#include <userint.h>

#include "StageDriver.h"
#include "StageDriverGUI.h"


/*
 * StageDriverGUI.c
 * ----------------
 * LabWindows/CVI GUI for controlling the StageDriver device.
 * This program opens a serial (ASRL) connection to the stage controller,
 * populates parameter selection controls, and provides callbacks for
 * motor commands, parameter get/set, direct commands, and periodic status
 * polling.
 *
 * Typical flow:
 *  - init CVI runtime and load the panel
 *  - prompt user for COM port and open the device
 *  - fill parameter selection rings
 *  - run the UI (callbacks handle rest)
 *
 * Note: This file only contains UI glue code; device logic lives in
 *       the StageDriver library.
 */


// *****************************************************************************************
// Defines
// *****************************************************************************************
#define SD_DEV_ADDR	"ASRL9::INSTR"
#define MAX_FILENAME_SIZE	256


// *****************************************************************************************
// Internal function prototypes
// *****************************************************************************************
static void newTextLine (int panelHandle, int controlID, const char* text);
static void appendToTextLine (int panelHandle, int controlID, const char* text);
static void setUpParamRings(void);

// *****************************************************************************************
// Global variables
// *****************************************************************************************
static int g_panMain;
static int g_devHandle = 0;

// *****************************************************************************************
// main
// *****************************************************************************************
int main (int argc, char *argv[])
{
	/*
	 * Startup and initialization
	 * - Initialize the CVI runtime
	 * - Load and display the main panel
	 * - Ask user for COM port and open the StageDriver device
	 * - Populate the parameter selection rings
	 * - Enter the CVI event loop
	 */
	char buffer[100];
	char adrStr[100]; 
	int status;
	
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((g_panMain = LoadPanel (0, "StageDriverGUI.uir", PAN_MAIN)) < 0)
		return -1;
	DisplayPanel (g_panMain);

	// Ask user for instrument address (COM port). Example: "9" -> ASRL9::INSTR
	PromptPopup ("Stage Driver Interface", "COM address", buffer, 99);
	sprintf(adrStr, "ASRL%s::INSTR",buffer);
	
	// Opening connection to device and report status in the UI status box
	newTextLine (g_panMain, PAN_MAIN_BOX_STATUS, "Opening connection to device ... ");
//	status = SD_Init(&g_devHandle, SD_DEV_ADDR);
	status = SD_Init(&g_devHandle, adrStr);
	if (status)
		appendToTextLine (g_panMain, PAN_MAIN_BOX_STATUS, "failed.");
	else
		appendToTextLine (g_panMain, PAN_MAIN_BOX_STATUS, "done.");
	setUpParamRings();
	
	RunUserInterface ();

	DiscardPanel (g_panMain);
	return 0;
}


// *****************************************************************************************
// Callback functions
// *****************************************************************************************

////////////////////////////////////////////////////////
// Close the panel
////////////////////////////////////////////////////////

int CVICALLBACK cb_Close (int panel, int control, int event,
													void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			newTextLine (g_panMain, PAN_MAIN_BOX_STATUS, "Closing device ... ");
			SD_Close(&g_devHandle);
			QuitUserInterface (0);
			break;
	}
	return 0;
}


////////////////////////////////////////////////////////
// gets/sets the parameters
////////////////////////////////////////////////////////

int CVICALLBACK cb_motorParam (int panel, int control, int event,
													void *callbackData, int eventData1, int eventData2)
{
	char paramName[SD_MAX_COMMAND_LENGTH];
	int motor;
	int value;
	int status;
	
	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlVal (g_panMain, PAN_MAIN_NUM_MOTOR, &motor);
			GetCtrlVal (g_panMain, PAN_MAIN_RNG_MOTOR_PARAM, paramName);

			switch (control)
			{
				case PAN_MAIN_BUT_MOTOR_GETPARAM:
					status = SD_GetMotorParameter(g_devHandle, motor, paramName, &value);
					if (status) newTextLine (g_panMain, PAN_MAIN_BOX_STATUS, "Could not get parameter");
					SetCtrlVal (g_panMain, PAN_MAIN_NUM_MOTOR_PARAM_VAL, value);
					break;
				case PAN_MAIN_BUT_MOTOR_SETPARAM:
					GetCtrlVal (g_panMain, PAN_MAIN_NUM_MOTOR_PARAM_VAL, &value);
					status = SD_SetMotorParameter(g_devHandle, motor, paramName, value);
					if (status) newTextLine (g_panMain, PAN_MAIN_BOX_STATUS, "Could not set parameter");
					break;
			}
			break;
	}
	return 0;
}

int CVICALLBACK cb_remoteParam (int panel, int control, int event,
													void *callbackData, int eventData1, int eventData2)
{
	char paramName[SD_MAX_COMMAND_LENGTH];
	int motor;
	int value;
	int status;
	
	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlVal (g_panMain, PAN_MAIN_NUM_MOTOR, &motor);
			GetCtrlVal (g_panMain, PAN_MAIN_RNG_REMOTE_PARAM, paramName);

			switch (control)
			{
				case PAN_MAIN_BUT_REMOTE_GETPARAM:
					status = SD_GetRemoteParameter(g_devHandle, motor, paramName, &value);
					if (status) newTextLine (g_panMain, PAN_MAIN_BOX_STATUS, "Could not get parameter");
					SetCtrlVal (g_panMain, PAN_MAIN_NUM_REMOTE_PARAM_VAL, value);
					break;
				case PAN_MAIN_BUT_REMOTE_SETPARAM:
					GetCtrlVal (g_panMain, PAN_MAIN_NUM_REMOTE_PARAM_VAL, &value);
					status = SD_SetRemoteParameter(g_devHandle, motor, paramName, value);
					if (status) newTextLine (g_panMain, PAN_MAIN_BOX_STATUS, "Could not set parameter");
					break;
			}
			break;
	}
	return 0;
}


////////////////////////////////////////////////////////
// Motor buttons
////////////////////////////////////////////////////////

int CVICALLBACK cb_Config (int panel, int control, int event,
													 void *callbackData, int eventData1, int eventData2)
{
	int motor;

	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlVal (g_panMain, PAN_MAIN_NUM_MOTOR, &motor);
			int status = SD_SetMotorCommand(g_devHandle, motor, "Config", 0); // value is ignored
			if (status) newTextLine (g_panMain, PAN_MAIN_BOX_STATUS, "Could not configure motor.");

			break;
	}
	return 0;
}

int CVICALLBACK cb_Clear (int panel, int control, int event,
													void *callbackData, int eventData1, int eventData2)
{
	int motor;

	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlVal (g_panMain, PAN_MAIN_NUM_MOTOR, &motor);
			int status = SD_SetMotorCommand(g_devHandle, motor, "StatusClear", 0); // value is ignored
			if (status) newTextLine (g_panMain, PAN_MAIN_BOX_STATUS, "Could not clear motor.");

			break;
	}
	return 0;
}

int CVICALLBACK cb_Enable (int panel, int control, int event,
													 void *callbackData, int eventData1, int eventData2)
{
	int motor;

	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlVal (g_panMain, PAN_MAIN_NUM_MOTOR, &motor);
			int status = SD_SetMotorStatus(g_devHandle, motor, "Enabled", 1);
			if (status) newTextLine (g_panMain, PAN_MAIN_BOX_STATUS, "Could not enable motor.");

			break;
	}
	return 0;
}

int CVICALLBACK cb_Disable (int panel, int control, int event,
														void *callbackData, int eventData1, int eventData2)
{
	int motor;

	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlVal (g_panMain, PAN_MAIN_NUM_MOTOR, &motor);
			int status = SD_SetMotorStatus(g_devHandle, motor, "Enabled", 0);
			if (status) newTextLine (g_panMain, PAN_MAIN_BOX_STATUS, "Could not disable motor.");

			break;
	}
	return 0;
}


////////////////////////////////////////////////////////
// Position and velocity buttons
////////////////////////////////////////////////////////

int CVICALLBACK cb_Velocity (int panel, int control, int event,
														 void *callbackData, int eventData1, int eventData2)
{
	int motor;
	int vel, velMax;

	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlVal (g_panMain, PAN_MAIN_NUM_MOTOR, &motor);
			GetCtrlVal(panel, control, &vel);
			SD_GetMotorParameter(g_devHandle, motor, "RateMaxVelocity", &velMax);
			vel *= velMax/100;
			int status = SD_SetMotorCommand(g_devHandle, motor, "MoveAtVelocity", vel);
			if (status) newTextLine (g_panMain, PAN_MAIN_BOX_STATUS, "Could not set motor velocity.");

			break;
	}
	return 0;
}

int CVICALLBACK cb_Stop (int panel, int control, int event,
												 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			SetCtrlVal(panel, PAN_MAIN_NUM_VELOCITY, 0);
			cb_Velocity(panel, PAN_MAIN_NUM_VELOCITY, EVENT_COMMIT, NULL, 0, 0);
			
			break;
	}
	return 0;
}

int CVICALLBACK cb_Position (int panel, int control, int event,
														 void *callbackData, int eventData1, int eventData2)
{
	int motor;
	int pos;

	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlVal (g_panMain, PAN_MAIN_NUM_MOTOR, &motor);
			GetCtrlVal(panel, control, &pos);
			int status = SD_SetMotorCommand(g_devHandle, motor, "MoveToPosition", pos);
			if (status) newTextLine (g_panMain, PAN_MAIN_BOX_STATUS, "Could not position motor.");

			break;
	}
	return 0;
}

int CVICALLBACK cb_SetPosition (int panel, int control, int event,
																void *callbackData, int eventData1, int eventData2)
{
	int motor, pos, setTo;
	int status;
	int isEnabled;
	
	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlVal (panel, PAN_MAIN_NUM_MOTOR, &motor);
			GetCtrlVal (panel, PAN_MAIN_RNG_SETTO, &setTo); // 0->encoder, 1->all
			GetCtrlVal(panel, PAN_MAIN_NUM_SETPOSITION, &pos);
			switch (setTo) {
				case 0: // encoder
					status = SD_SetMotorStatus(g_devHandle, motor, "EncoderPosition", pos);
					break;
				case 1: // all
					status = SD_GetMotorStatus(g_devHandle, motor, "Enabled", &isEnabled); // store previous state
					status |= SD_SetMotorStatus(g_devHandle, motor, "Enabled", 0); // turn off to be able to set positions without motion
					if (status) newTextLine (g_panMain, PAN_MAIN_BOX_STATUS, "Could not disable motor.");
					status = SD_SetMotorStatus(g_devHandle, motor, "ActualPosition", pos);
					status |= SD_SetMotorStatus(g_devHandle, motor, "TargetPosition", pos);
					status |= SD_SetMotorStatus(g_devHandle, motor, "Enabled", isEnabled); // turn off to be able to set positions without motion
					if (status) newTextLine (g_panMain, PAN_MAIN_BOX_STATUS, "Could not enable motor.");
					Delay(0.05); // give motor time to engage	
					status = SD_SetMotorStatus(g_devHandle, motor, "EncoderPosition", pos);
					if (status) newTextLine (g_panMain, PAN_MAIN_BOX_STATUS, "Could not set encoder position.");
			}
			
			break;
	}
	return 0;
}


////////////////////////////////////////////////////////
// Error check
////////////////////////////////////////////////////////

int CVICALLBACK cb_CheckError (int panel, int control, int event,
															 void *callbackData, int eventData1, int eventData2)
{
	char instrResp[SD_MAX_INSTR_RESP_LENGTH];

	switch (event)
	{
		case EVENT_COMMIT:

			SD_GetErrorMessage(g_devHandle, instrResp, SD_MAX_INSTR_RESP_LENGTH);
			newTextLine (g_panMain, PAN_MAIN_BOX_STATUS, instrResp);
			
			break;
	}
	return 0;
}


////////////////////////////////////////////////////////
// Direct command
////////////////////////////////////////////////////////

int CVICALLBACK cbDirectCommand (int panel, int control, int event,
																 void *callbackData, int eventData1, int eventData2)
{
	char command[SD_MAX_COMMAND_LENGTH];
	char instrResp[SD_MAX_INSTR_RESP_LENGTH];
	
	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlVal(g_panMain, PAN_MAIN_STR_DIRECTCOM, command);
			SetCtrlVal(g_panMain, PAN_MAIN_STR_DIRECTCOM, "");
			SD_SendDirectCommand(g_devHandle, command, instrResp, SD_MAX_INSTR_RESP_LENGTH);
			newTextLine (g_panMain, PAN_MAIN_BOX_STATUS, instrResp);
			
			break;
	}
	return 0;
}


////////////////////////////////////////////////////////
// timer callback
////////////////////////////////////////////////////////

int CVICALLBACK cb_StatusTimer (int panel, int control, int event,
																void *callbackData, int eventData1, int eventData2)
{
	int motor;
	int pos, enc, temp;
	int devType;
	int devStatus;
	
	switch (event)
	{
		case EVENT_TIMER_TICK:

			GetCtrlVal (g_panMain, PAN_MAIN_NUM_MOTOR, &motor);
			SD_GetMotorParameter(g_devHandle, motor, "TypeDevice", &devType);
			if (devType) {
				SD_GetMotorCommand(g_devHandle, motor, "GetStatus", &devStatus);
				int ctrlOffset = PAN_MAIN_LED_0; 
				for (int idx=0; idx<12; idx++) {
					SetCtrlVal(panel, ctrlOffset+idx, devStatus & (0x1 << idx)); 
				}
				SD_GetMotorStatus(g_devHandle, motor, "TargetPosition", &pos);
				SetCtrlVal(panel, PAN_MAIN_NUM_XTARGET, pos);
				SD_GetMotorStatus(g_devHandle, motor, "ActualPosition", &pos);
				SetCtrlVal(panel, PAN_MAIN_NUM_XACT, pos);
				SD_GetMotorStatus(g_devHandle, motor, "EncoderPosition", &enc);
				SetCtrlVal(panel, PAN_MAIN_NUM_ENC, enc);
				SetCtrlVal(panel, PAN_MAIN_NUM_DEVIATION, enc-pos);
				SD_GetMotorStatus(g_devHandle, motor, "Temperature", &temp);
				SetCtrlVal(panel, PAN_MAIN_NUM_TEMP, temp);
			}
			break;
	}
	return 0;
}


////////////////////////////////////////////////////////
// Parameter load/save
////////////////////////////////////////////////////////

int CVICALLBACK cb_FileLoad (int panel, int control, int event,
														 void *callbackData, int eventData1, int eventData2)
{
	char fileName[MAX_FILENAME_SIZE];
	
	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlVal(panel, PAN_MAIN_STR_FILENAME, fileName);
			int status = SD_LoadConfigFromFile(g_devHandle, fileName);
			if (status) newTextLine (g_panMain, PAN_MAIN_BOX_STATUS, "Could not load parameters from file.");
			
			break;
	}
	return 0;
}

int CVICALLBACK cb_FileSave (int panel, int control, int event,
														 void *callbackData, int eventData1, int eventData2)
{
	char fileName[MAX_FILENAME_SIZE];
	
	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlVal(panel, PAN_MAIN_STR_FILENAME, fileName);
			int status = SD_SaveConfigToFile(g_devHandle, fileName);
			if (status) newTextLine (g_panMain, PAN_MAIN_BOX_STATUS, "Could not save parameters to file.");
			
			break;
	}
	return 0;
}



// *****************************************************************************************
// Internal functions
// *****************************************************************************************

////////////////////////////////////////////////////////
// adds the allowed items in the parameter ring
////////////////////////////////////////////////////////
static void setUpParamRings(void)
{
	size_t numParams;
	const char **paramNames=NULL;			
		
	SD_GetMotorParameterNames(&paramNames, &numParams);
	for (size_t idx=0; idx<numParams; idx++) {
		InsertListItem (g_panMain, PAN_MAIN_RNG_MOTOR_PARAM, -1, paramNames[idx], paramNames[idx]);
	}
	SD_GetRemoteParameterNames(&paramNames, &numParams);
	for (size_t idx=0; idx<numParams; idx++) {
		InsertListItem (g_panMain, PAN_MAIN_RNG_REMOTE_PARAM, -1, paramNames[idx], paramNames[idx]);
	}
}


////////////////////////////////////////////////////////
// adds a new text line to a TextBox and scolls if appropriate
////////////////////////////////////////////////////////
static void newTextLine (int panelHandle, int controlID, const char* text)
{
	int numTextLines;
	int visibleLines;

	InsertTextBoxLine (panelHandle, controlID, -1, text);
	ProcessDrawEvents ();
	GetNumTextBoxLines (panelHandle, controlID, &numTextLines);
	GetCtrlAttribute (panelHandle, controlID, ATTR_VISIBLE_LINES, &visibleLines);
	if (numTextLines-1>visibleLines)
		SetCtrlAttribute (panelHandle, controlID, ATTR_FIRST_VISIBLE_LINE,
								numTextLines-1-visibleLines);
	ProcessDrawEvents ();
}


////////////////////////////////////////////////////////
// appends text to the last line in a TextBox
////////////////////////////////////////////////////////
static void appendToTextLine (int panelHandle, int controlID, const char* text)
{
	int numTextLines;
	int length;
	char line[200];

	GetNumTextBoxLines (panelHandle, controlID, &numTextLines);
	GetTextBoxLine (panelHandle, controlID, numTextLines-2, line);
	GetTextBoxLineLength (panelHandle, controlID, numTextLines-2, &length);
	CopyString (line, length, text, 0, (ssize_t) strlen(text)+1);
	ReplaceTextBoxLine (panelHandle, controlID, numTextLines-2, line);
	ProcessDrawEvents ();
}
