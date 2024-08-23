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

#define  BATCHVIEW                       1       /* callback function: BatchViewPanelCB */
#define  BATCHVIEW_GRAPH                 2
#define  BATCHVIEW_TEXTMSG_4             3
#define  BATCHVIEW_TEXTMSG_3             4
#define  BATCHVIEW_TEXTMSG_2             5
#define  BATCHVIEW_TEXTMSG_5             6
#define  BATCHVIEW_CURRENTCHANNEL        7
#define  BATCHVIEW_TEXTMSG               8
#define  BATCHVIEW_CURRENTBLOCK          9
#define  BATCHVIEW_TIMINGCFG             10
#define  BATCHVIEW_DURATION              11
#define  BATCHVIEW_STARTTIME             12

#define  BLOCK                           2
#define  BLOCK_STARTCOLUMN               2
#define  BLOCK_TABLE                     3
#define  BLOCK_STARTROW                  4
#define  BLOCK_DESCRIPTION               5
#define  BLOCK_NUMCOLUMNS                6
#define  BLOCK_ID                        7       /* callback function: SelectPatternCB */
#define  BLOCK_NUMROWS                   8
#define  BLOCK_UPDATE                    9       /* callback function: UpdateBlockCB */
#define  BLOCK_TILEBLOCK                 10      /* callback function: TileBlockCB */
#define  BLOCK_NEWBLOCK                  11      /* callback function: NewBlockCB */
#define  BLOCK_ADD                       12      /* callback function: AddBlockCB */
#define  BLOCK_COMMENTS                  13
#define  BLOCK_FILENAME                  14

#define  BLOCKDEF                        3       /* callback function: PatternPanelCB */
#define  BLOCKDEF_CHIPIMAGE              2       /* callback function: ChipImageCB */
#define  BLOCKDEF_TABS                   3
#define  BLOCKDEF_COLUMN                 4
#define  BLOCKDEF_ROW                    5
#define  BLOCKDEF_ELECTRODETIMER         6       /* callback function: ElectrodeTimer */
#define  BLOCKDEF_HARDWARESTATE          7
#define  BLOCKDEF_VERSION                8
#define  BLOCKDEF_TEXTMSG                9
#define  BLOCKDEF_TEXTMSG_2              10

#define  BLOCKLIST                       4       /* callback function: BlockListPanelCB */
#define  BLOCKLIST_CHANNEL               2
#define  BLOCKLIST_PATTERNS              3       /* callback function: PatternListCB */
#define  BLOCKLIST_DELETE                4       /* callback function: EditPatternList */
#define  BLOCKLIST_MOVEUP                5       /* callback function: EditPatternList */
#define  BLOCKLIST_ADDVOLTAGE            6       /* callback function: EditPatternList */
#define  BLOCKLIST_MOVEDOWN              7       /* callback function: EditPatternList */
#define  BLOCKLIST_READ                  8       /* callback function: ActivatePatternCB */
#define  BLOCKLIST_ON                    9       /* callback function: ActivatePatternCB */
#define  BLOCKLIST_OFF                   10      /* callback function: ActivatePatternCB */
#define  BLOCKLIST_ALLOFF                11      /* callback function: ActivatePatternCB */
#define  BLOCKLIST_ALLON                 12      /* callback function: ActivatePatternCB */
#define  BLOCKLIST_SINGLERUN             13
#define  BLOCKLIST_BATCHRUN              14
#define  BLOCKLIST_TRIGGERKEITHLEY       15
#define  BLOCKLIST_FLOATVLINE            16
#define  BLOCKLIST_BLOCKDELAY            17
#define  BLOCKLIST_TRIGGERVOLTAGE        18

#define  CHIPTEST                        5       /* callback function: ChipTestPanelCB */
#define  CHIPTEST_CHANNEL                2
#define  CHIPTEST_ADDRESSTEST            3       /* callback function: AddressTest */
#define  CHIPTEST_MINICHIP_4             4       /* callback function: MiniChipCanvasCB */
#define  CHIPTEST_MINICHIP_3             5       /* callback function: MiniChipCanvasCB */
#define  CHIPTEST_MINICHIP_2             6       /* callback function: MiniChipCanvasCB */
#define  CHIPTEST_MINICHIP_1             7       /* callback function: MiniChipCanvasCB */
#define  CHIPTEST_DELAY                  8       /* callback function: SetDelayCB */
#define  CHIPTEST_TEXTMSG                9
#define  CHIPTEST_STATUS                 10

#define  VOLTAGE                         6       /* callback function: VoltagePanelCB */
#define  VOLTAGE_ON                      2       /* callback function: StartVoltageCB */
#define  VOLTAGE_OFF                     3       /* callback function: StartVoltageCB */
#define  VOLTAGE_VB_2                    4
#define  VOLTAGE_DESCRIPTION             5
#define  VOLTAGE_VA_2                    6
#define  VOLTAGE_ID                      7       /* callback function: SelectVoltageCB */
#define  VOLTAGE_VSTART_2                8
#define  VOLTAGE_VB                      9
#define  VOLTAGE_VA                      10
#define  VOLTAGE_RAMPRATE_2              11
#define  VOLTAGE_VSTART                  12
#define  VOLTAGE_OFFDURATION_2           13
#define  VOLTAGE_ONDURATION_2            14
#define  VOLTAGE_OFFDURATION             15
#define  VOLTAGE_ONDURATION              16
#define  VOLTAGE_PROFILE_2               17
#define  VOLTAGE_CONTINOUS_2             18
#define  VOLTAGE_RAMPRATE                19
#define  VOLTAGE_TOTALDURATION_2         20
#define  VOLTAGE_TOTALDURATION           21
#define  VOLTAGE_CONTINOUS               22
#define  VOLTAGE_PROFILE                 23
#define  VOLTAGE_TEXTMSG                 24
#define  VOLTAGE_TEXTMSG_2               25
#define  VOLTAGE_TEXTMSG_3               26
#define  VOLTAGE_SETVOLTAGE_2            27      /* callback function: SetVoltageCB */
#define  VOLTAGE_SETVOLTAGE              28      /* callback function: SetVoltageCB */
#define  VOLTAGE_STATUS                  29
#define  VOLTAGE_FILENAME                30


     /* Menu Bars, Menus, and Menu Items: */

#define  PATMENU                         1
#define  PATMENU_FILE                    2
#define  PATMENU_FILE_OPEN               3
#define  PATMENU_FILE_OPEN_SUBMENU       4
#define  PATMENU_FILE_OPEN_OPENBLOCKDEF  5       /* callback function: BlockMenubarCB */
#define  PATMENU_FILE_OPEN_SEPARATOR_2   6
#define  PATMENU_FILE_OPEN_OPENBLOCKLIST 7       /* callback function: BlockMenubarCB */
#define  PATMENU_FILE_OPEN_SEPARATOR_3   8
#define  PATMENU_FILE_OPEN_OPENVCONFIG   9       /* callback function: BlockMenubarCB */
#define  PATMENU_FILE_SAVE               10
#define  PATMENU_FILE_SAVE_SUBMENU       11
#define  PATMENU_FILE_SAVE_SAVEBLOCKDEF  12      /* callback function: BlockMenubarCB */
#define  PATMENU_FILE_SAVE_SEPARATOR_4   13
#define  PATMENU_FILE_SAVE_SAVEBLOCKLIST 14      /* callback function: BlockMenubarCB */
#define  PATMENU_FILE_SAVE_SEPARATOR_5   15
#define  PATMENU_FILE_SAVE_SAVEVCONFIG   16      /* callback function: BlockMenubarCB */
#define  PATMENU_FILE_SEPARATOR          17
#define  PATMENU_FILE_CLOSE              18      /* callback function: BlockMenubarCB */
#define  PATMENU_EDIT                    19
#define  PATMENU_EDIT_COPY               20      /* callback function: BlockMenubarCB */
#define  PATMENU_EDIT_PASTE              21      /* callback function: BlockMenubarCB */
#define  PATMENU_VIEW                    22
#define  PATMENU_VIEW_CHIPMAP            23      /* callback function: BlockMenubarCB */
#define  PATMENU_VIEW_SEPARATOR_8        24
#define  PATMENU_VIEW_KE6485             25      /* callback function: BlockMenubarCB */
#define  PATMENU_VIEW_SEPARATOR_6        26
#define  PATMENU_VIEW_DEBUG              27      /* callback function: BlockMenubarCB */


     /* Callback Prototypes: */ 

int  CVICALLBACK ActivatePatternCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK AddBlockCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK AddressTest(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK BatchViewPanelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK BlockListPanelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK BlockMenubarCB(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK ChipImageCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ChipTestPanelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK EditPatternList(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ElectrodeTimer(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MiniChipCanvasCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK NewBlockCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PatternListCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PatternPanelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SelectPatternCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SelectVoltageCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SetDelayCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SetVoltageCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK StartVoltageCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TileBlockCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK UpdateBlockCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK VoltagePanelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
