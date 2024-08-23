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

#define  KETEST                          1       /* callback function: KeithleyPanelCB */
#define  KETEST_INIT                     2       /* callback function: InitKeithleyCB */
#define  KETEST_LASTRESULT               3
#define  KETEST_PK2PK                    4
#define  KETEST_MAX                      5
#define  KETEST_STDEV                    6
#define  KETEST_MIN                      7
#define  KETEST_SAMPLE                   8
#define  KETEST_MEAN                     9
#define  KETEST_AQUIRESAMPLES            10      /* callback function: AquireSamplesCB */
#define  KETEST_AQUIRESAMPLE             11      /* callback function: AquireSampleCB */
#define  KETEST_AQUIRESTATS              12      /* callback function: AquireStatisticsCB */
#define  KETEST_MAXTIMESTATS             13
#define  KETEST_TOTALDURATION            14
#define  KETEST_NUMSAMPLESSAMP           15
#define  KETEST_RANGE                    16
#define  KETEST_RESOLUTION               17
#define  KETEST_NPLC                     18
#define  KETEST_NUMSAMPLESSTATS          19
#define  KETEST_SAMPLETABLE              20
#define  KETEST_INFINATEACQUIRE          21      /* callback function: InfinateAcquireCB */
#define  KETEST_CLEARTABLE               22      /* callback function: ClearTableCB */
#define  KETEST_SIMULATE                 23
#define  KETEST_AUTORANGE                24


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK AquireSampleCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK AquireSamplesCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK AquireStatisticsCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ClearTableCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK InfinateAcquireCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK InitKeithleyCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK KeithleyPanelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
