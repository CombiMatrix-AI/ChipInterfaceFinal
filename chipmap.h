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

#define  CHIPMAP                         1       /* callback function: ChipMapPanelCB */
#define  CHIPMAP_NUMBERSAMPLES           2
#define  CHIPMAP_FLOAT                   3
#define  CHIPMAP_NEIGHBOR                4
#define  CHIPMAP_RELAXDURATION           5
#define  CHIPMAP_SAMPLEDURATION          6
#define  CHIPMAP_VOLTAGE_2               7
#define  CHIPMAP_VOLTAGE_1               8
#define  CHIPMAP_WORKING                 9
#define  CHIPMAP_ENDCOLUMN               10
#define  CHIPMAP_ENDROW                  11
#define  CHIPMAP_STARTCOLUMN             12
#define  CHIPMAP_CHANNEL                 13
#define  CHIPMAP_STARTROW                14
#define  CHIPMAP_TABLE                   15
#define  CHIPMAP_ABORT                   16      /* callback function: AbortMappingCB */
#define  CHIPMAP_START                   17      /* callback function: StartMappingCB */
#define  CHIPMAP_CANVAS                  18
#define  CHIPMAP_DISABLEKEITHLEY         19
#define  CHIPMAP_GRAPH                   20
#define  CHIPMAP_DATAFOLDER              21
#define  CHIPMAP_SELECTFOLDER            22      /* callback function: SelectFolderCB */
#define  CHIPMAP_ELAPSEDSAMPLETIME       23


     /* Menu Bars, Menus, and Menu Items: */

#define  CTMENU                          1
#define  CTMENU_FILE                     2
#define  CTMENU_FILE_NEW                 3       /* callback function: ChipTestMenuBarCB */
#define  CTMENU_FILE_OPEN                4       /* callback function: ChipTestMenuBarCB */
#define  CTMENU_FILE_SEPARATOR           5
#define  CTMENU_FILE_SAVE                6       /* callback function: ChipTestMenuBarCB */
#define  CTMENU_FILE_SEPARATOR_2         7
#define  CTMENU_FILE_CLOSE               8       /* callback function: ChipTestMenuBarCB */


     /* Callback Prototypes: */ 

int  CVICALLBACK AbortMappingCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChipMapPanelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK ChipTestMenuBarCB(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK SelectFolderCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK StartMappingCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
