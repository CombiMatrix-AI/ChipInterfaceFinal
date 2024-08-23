//voltage.h
#ifndef VOLTAGE_H_
	#define VOLTAGE_H_
	
	//includes
	#include "chipcontrol.h"
	#include <userint.h>
	#include "inifile.h"
	#include "pattern.h"

	#include "log.h"
	#include "debuguir.h"
	
	//definitions
	
	typedef struct
	{
		int id;
		int vProfile;
		double vStart;
		double vA;
		double vB;
		double ramprate;		//millivolts Per Second
		double onDuration;		//in seconds
		double offDuration;		//in seconds
		double totalDuration;	//in seconds
		int continous;			//ignore on/off duration
		
	}VChannel;
	
	typedef struct
	{
		
		char description[256];
		VChannel dacChannel[NUMBER_DAC_CHANNELS];
	}VoltageHardware;
	
	//prototypes
	int StartVoltageSeries(VoltageHardware profile);
	int CVICALLBACK VoltageProfileThread (void *functionData);
	int CopyVoltageProfile(VoltageHardware *source, VoltageHardware *destination);
	int CVICALLBACK WatchVoltageThreads (void *functionData);
	int GetVoltageThreadID(int channel);
	void StopVoltageSeries(void);
	void InitVoltageDataStructure(VoltageHardware *vProfile);
	int SaveVoltageDataFile(char *filename, VoltageHardware *vProfile);
	int LoadVoltageDataFile(char *filename, VoltageHardware *vProfile);
#endif
