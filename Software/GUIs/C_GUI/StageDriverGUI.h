/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PAN_MAIN                         1
#define  PAN_MAIN_BUT_CLOSE               2       /* control type: command, callback function: cb_Close */
#define  PAN_MAIN_BUT_LOAD                3       /* control type: command, callback function: cb_FileLoad */
#define  PAN_MAIN_BUT_SAVE                4       /* control type: command, callback function: cb_FileSave */
#define  PAN_MAIN_BUT_REMOTE_GETPARAM     5       /* control type: command, callback function: cb_remoteParam */
#define  PAN_MAIN_BUT_REMOTE_SETPARAM     6       /* control type: command, callback function: cb_remoteParam */
#define  PAN_MAIN_BUT_MOTOR_GETPARAM      7       /* control type: command, callback function: cb_motorParam */
#define  PAN_MAIN_NUM_REMOTE_PARAM_VAL    8       /* control type: numeric, callback function: (none) */
#define  PAN_MAIN_BUT_MOTOR_SETPARAM      9       /* control type: command, callback function: cb_motorParam */
#define  PAN_MAIN_RNG_REMOTE_PARAM        10      /* control type: ring, callback function: (none) */
#define  PAN_MAIN_BOX_STATUS              11      /* control type: textBox, callback function: (none) */
#define  PAN_MAIN_NUM_MOTOR_PARAM_VAL     12      /* control type: numeric, callback function: (none) */
#define  PAN_MAIN_NUM_SETPOSITION         13      /* control type: numeric, callback function: cb_SetPosition */
#define  PAN_MAIN_NUM_POSITION            14      /* control type: numeric, callback function: cb_Position */
#define  PAN_MAIN_NUM_MOTOR               15      /* control type: numeric, callback function: (none) */
#define  PAN_MAIN_RNG_MOTOR_PARAM         16      /* control type: ring, callback function: (none) */
#define  PAN_MAIN_TEXTMSG                 17      /* control type: textMsg, callback function: (none) */
#define  PAN_MAIN_BUT_CONFIG              18      /* control type: command, callback function: cb_Config */
#define  PAN_MAIN_BUT_CHECKERROR          19      /* control type: command, callback function: cb_CheckError */
#define  PAN_MAIN_BUT_STOP                20      /* control type: command, callback function: cb_Stop */
#define  PAN_MAIN_BUT_DISABLE             21      /* control type: command, callback function: cb_Disable */
#define  PAN_MAIN_BUT_ENABLE              22      /* control type: command, callback function: cb_Enable */
#define  PAN_MAIN_BUT_CLEAR               23      /* control type: command, callback function: cb_Clear */
#define  PAN_MAIN_STR_DIRECTCOM           24      /* control type: string, callback function: cbDirectCommand */
#define  PAN_MAIN_LED_0                   25      /* control type: LED, callback function: (none) */
#define  PAN_MAIN_LED_1                   26      /* control type: LED, callback function: (none) */
#define  PAN_MAIN_LED_2                   27      /* control type: LED, callback function: (none) */
#define  PAN_MAIN_LED_3                   28      /* control type: LED, callback function: (none) */
#define  PAN_MAIN_LED_4                   29      /* control type: LED, callback function: (none) */
#define  PAN_MAIN_LED_5                   30      /* control type: LED, callback function: (none) */
#define  PAN_MAIN_LED_6                   31      /* control type: LED, callback function: (none) */
#define  PAN_MAIN_LED_7                   32      /* control type: LED, callback function: (none) */
#define  PAN_MAIN_LED_8                   33      /* control type: LED, callback function: (none) */
#define  PAN_MAIN_LED_9                   34      /* control type: LED, callback function: (none) */
#define  PAN_MAIN_LED_10                  35      /* control type: LED, callback function: (none) */
#define  PAN_MAIN_LED_11                  36      /* control type: LED, callback function: (none) */
#define  PAN_MAIN_NUM_TEMP                37      /* control type: numeric, callback function: (none) */
#define  PAN_MAIN_NUM_DEVIATION           38      /* control type: numeric, callback function: (none) */
#define  PAN_MAIN_NUM_ENC                 39      /* control type: numeric, callback function: (none) */
#define  PAN_MAIN_NUM_XACT                40      /* control type: numeric, callback function: (none) */
#define  PAN_MAIN_NUM_XTARGET             41      /* control type: numeric, callback function: (none) */
#define  PAN_MAIN_NUM_VELOCITY            42      /* control type: scale, callback function: cb_Velocity */
#define  PAN_MAIN_TEXTMSG_2               43      /* control type: textMsg, callback function: (none) */
#define  PAN_MAIN_DECORATION_3            44      /* control type: deco, callback function: (none) */
#define  PAN_MAIN_DECORATION_4            45      /* control type: deco, callback function: (none) */
#define  PAN_MAIN_DECORATION_2            46      /* control type: deco, callback function: (none) */
#define  PAN_MAIN_DECORATION              47      /* control type: deco, callback function: (none) */
#define  PAN_MAIN_TEXTMSG_4               48      /* control type: textMsg, callback function: (none) */
#define  PAN_MAIN_TEXTMSG_7               49      /* control type: textMsg, callback function: (none) */
#define  PAN_MAIN_TEXTMSG_5               50      /* control type: textMsg, callback function: (none) */
#define  PAN_MAIN_TEXTMSG_6               51      /* control type: textMsg, callback function: (none) */
#define  PAN_MAIN_TEXTMSG_3               52      /* control type: textMsg, callback function: (none) */
#define  PAN_MAIN_RNG_SETTO               53      /* control type: ring, callback function: (none) */
#define  PAN_MAIN_STR_FILENAME            54      /* control type: string, callback function: (none) */
#define  PAN_MAIN_TIM_STATUS              55      /* control type: timer, callback function: cb_StatusTimer */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK cb_CheckError(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_Clear(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_Close(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_Config(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_Disable(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_Enable(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_FileLoad(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_FileSave(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_motorParam(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_Position(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_remoteParam(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_SetPosition(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_StatusTimer(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_Stop(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cb_Velocity(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbDirectCommand(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif