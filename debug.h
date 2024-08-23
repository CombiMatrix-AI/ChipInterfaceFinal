/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2004. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  DEBUG                           1       /* callback function: DebugPanelCB */
#define  DEBUG_OUTPUT                    2
#define  DEBUG_CLEAR                     3       /* callback function: ClearDebugOutputCB */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK ClearDebugOutputCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DebugPanelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
