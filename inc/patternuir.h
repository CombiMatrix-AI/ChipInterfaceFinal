//patterndefuir.h
#ifndef PATTERNUIR_H_
	#define PATTERNUIR_H_
	
	//includes
	#include "patterndef.h"
	#include <userint.h>
	#include "easytab.h" 
	#include "pattern.h"

	#include "chipcontrol.h"
	#include "debuguir.h"
	#include "KE6485uir.h"
	#include "chipmapuir.h"
	
	#include "properties.h"
	#include "voltage.h"
	#include <utility.h>
	
	//definitions
	#define NUMBER_CHANNELS 4
	#define PIXELS_PER_ROW 	8
	#define PIXELS_PER_COLUMN 	8
	
	#define PIXELS_PER_ROW_MINI 4
	#define PIXELS_PER_COLUMN_MINI 4
	
	#define VLINE_0_STRING	"Vline 0"
	#define VLINE_1_STRING	"Vline 1"
	#define VLINE_2_STRING	"Vline 2"
	#define VLINE_3_STRING	"Vline 3"
	#define CHECKERBOARD_STRING	"Checkerboard"
	#define INVCHECKERBOARD_STRING	"Inv. Checkerboard"
	
	#define BLOCKLIST_DELETE_STRING "Delete"
	
	#define VLINE_0			0
	#define VLINE_1			1
	#define VLINE_2			2
	#define VLINE_3			3
	
	#define CHECKERBOARD	0
	#define INVCHECKERBOARD 1
	
	#define NUMBER_TEST_PATTERNS 7
	
	#define SAVE_BLOCK_STRING	"Save Block"
	#define LOAD_BLOCK_STRING	"Load Block"
	
	#define SAVE_BLOCK_LIST_STRING "Save Block List"
	#define LOAD_BLOCK_LIST_STRING "Load Block List"
	
	#define SAVE_VOLTAGE_CONFIG_STRING "Save Voltage Configuration"
	#define LOAD_VOLTAGE_CONFIG_STRING "Load Voltage Configuration"
	
	#define BLOCK_DATA_DIRECTORY "Block Data Directory"
	//prototypes
	int LoadPatternDefinitionPanel(char *version, Properties *defaults);
	int ResizeBlockTable(int numberRows, int numberColumns);
	int GetBlock(int *newBlock, int numberRows, int numberColumns);
	int UpdateChipView(int *pattern);
	void CVICALLBACK SetVlineMenu (int panelHandle, int controlID, int MenuItemID, void *callbackData);
	void CVICALLBACK SetBlockDesignMenu (int panelHandle, int controlID, int MenuItemID, void *callbackData);
	void CVICALLBACK EditPatternListMenu (int panelHandle, int controlID, int MenuItemID, void *callbackData); 
	int GetBlockPanelValues(PatternDefinition *newPattern, int *pattern);
	int SetBlockPanelValues(PatternDefinition *newPattern);
	int SetElectrodePositionTimer(double newInterval);
	int ResetMiniChipViews(void);
	int UpdateMiniChipView(int *pattern, int channel);
	int GetVoltagePanelData(VoltageHardware *profile);
	int SetVoltagePanelData(VoltageHardware profile);
	void MonitorVoltageThreads(void);
	int CVICALLBACK WatchVoltageThreads (void *functionData);
	int SetTiledDescriptionName(PatternDefinition *block, int tileRow, int tileColumn);
	int StartBatchRunThread(void);
	int CVICALLBACK batchRunThreadFunction (void *functionData);
	void CVICALLBACK batchRunCallbackFunction (int poolHandle, int functionID, 
		unsigned int event, int value, void *callbackData);
	int StartBatchRunSingleThread(void);
	int CVICALLBACK batchRunSingleThreadFunction (void *functionData);
	void CVICALLBACK batchRunSingleCallbackFunction (int poolHandle, int functionID, 
		unsigned int event, int value, void *callbackData);
#endif
