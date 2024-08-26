// ChipControl.h

#ifndef CHIPCONTROL_H_
	#define CHIPCONTROL_H_
	
	//includes	
	#include "dask.h"
	#include <ansi_c.h>
	#include <userint.h>

	#include "properties.h"
	#include "log.h"
	#include <utility.h>
	
	//definitions
	#define CHIP_CARD_NUMBER 	0
	#define MAXIMUM_COLUMNS		16
	#define MAXIMUM_ROWS 		64
	#define MAXIMUM_ELECTRODES	1024
	#define NUMBER_CHANNELS		4
	
	#define NUMBER_DAC_CHANNELS	1
	#define DAC_VOLTAGE_REFERENCE AD_U_5_V 

	//prototypes
	void SetChipHardwareEmulation(int emulationState);
	int SetupChipControlCard(Properties *defaults);
	int SetDacVoltage(int channel, double voltage);
	int ShutdownChipControlCard(void);
	int SetChipMap(int channel, int *map);
	int GetChipMap(int channel, int *map);
	int SetChipState(int channel, int row, int column, int value);
	int GetChipState(int channel, int row, int column, int *value);
	void SetChipHardwareDelay(int newDelay);
	int GetChipHardwareDelay(void);
	int Wait(int counts);
	int CVICALLBACK WaitThread (void *functionData);
	
#endif
