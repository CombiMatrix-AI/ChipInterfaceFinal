//ChipMapUir.h
#ifndef _CHIPMAPUIR_H
	#define _CHIPMAPUIR_H
	
	#include <userint.h> 
	#include <cvirte.h>		
	#include <userint.h>
	#include "chipcontrol.h"
	#include "chipmap.h"
	#include "log.h"
	#include "KE6485driver.h"
	
	#define CHIPMAP_PIXELS_PER_ROW 4
	#define CHIPMAP_PIXELS_PER_COLUMN 8
	
	typedef struct 
	{
		int channel;
		
		int startRow;
		int startColumn;
		int endRow;
		int endColumn;
		
		int workingVline;
		int neighborVline;
		int floatVline;
		
		int numberSamples;
		double sampleDuration;
		double relaxDuration;
		
		double ch1Voltage;
		double ch2Voltage;
		
		int disableKeithley;
	}ChipMapperDefinition;
	
	int LoadChipMapPanel(void);
	int GetChipMapPanelValues(ChipMapperDefinition *panelValues);
	int StartChipMappingThread(ChipMapperDefinition *panelValues); 
	int CVICALLBACK ChipMapperThread (void *functionData);
	int SetNextMeasurementSite(int *map, int channel, int row, int column, int workingVline, int neighborVline);
	int UpdateChipMapView(int *pattern, int workingVline, int neighborVline, int floatVline);
	
#endif
