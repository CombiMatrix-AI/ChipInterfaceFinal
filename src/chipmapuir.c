#include <userint.h>
#include "chipmap.h"
#include <analysis.h>
//chipmapuir.c

#include "chipmapuir.h"

int chipmap = -1;

int chipmapAbortFlag = 0;
int chipmapInProcess = 0;
	
ChipMapperDefinition *gPanelValues = NULL;

int LoadChipMapPanel(void)
{
	if(chipmap == -1)
	{
		if ((chipmap = LoadPanel (0, "chipmap.uir", CHIPMAP)) < 0)
			return -1;
	}
	DisplayPanel(chipmap);
	return chipmap;	
}

int CVICALLBACK ChipMapPanelCB (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_CLOSE:
			if(chipmapInProcess == 1)
			{
				MessagePopup("ERROR", "Cannot close panel while measurement is in process");
				return 0;
			}
			DiscardPanel(chipmap);
			chipmap = -1;
			//do any other necessary cleanup
			break;
		}
	return 0;
}

void CVICALLBACK ChipTestMenuBarCB (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	switch(menuItem)
	{
	
		default:
			return;
	}	
}

int CVICALLBACK StartMappingCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int status = 0;
	
	ChipMapperDefinition panelValues;
	
	switch (event)
		{
		case EVENT_COMMIT:
			if(chipmapInProcess == 1)
			{
				MessagePopup("ERROR", "Chip map operation already in progress");
				return 0;
			}
			chipmapAbortFlag = 0;
			status = GetChipMapPanelValues(&panelValues);
			if(status != 1)
			{
				//add error message
				return 0;
			}
			chipmapInProcess = 1;
			status = StartChipMappingThread(&panelValues);
			break;
		}
	return 0;
}

int CVICALLBACK AbortMappingCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			chipmapAbortFlag = 1;
			break;
		}
	return 0;
}

int GetChipMapPanelValues(ChipMapperDefinition *panelValues)
{
	GetCtrlVal(chipmap, CHIPMAP_CHANNEL, &panelValues->channel); 
	panelValues->channel -= 1; //our channel is zero indexed
	GetCtrlVal(chipmap, CHIPMAP_STARTROW, &panelValues->startRow);
	GetCtrlVal(chipmap, CHIPMAP_STARTCOLUMN, &panelValues->startColumn);
	GetCtrlVal(chipmap, CHIPMAP_ENDROW, &panelValues->endRow);
	GetCtrlVal(chipmap, CHIPMAP_ENDCOLUMN, &panelValues->endColumn);
	GetCtrlVal(chipmap, CHIPMAP_WORKING, &panelValues->workingVline);
	GetCtrlVal(chipmap, CHIPMAP_NEIGHBOR, &panelValues->neighborVline);
	GetCtrlVal(chipmap, CHIPMAP_FLOAT, &panelValues->floatVline);
	GetCtrlVal(chipmap, CHIPMAP_VOLTAGE_1, &panelValues->ch1Voltage);
	GetCtrlVal(chipmap, CHIPMAP_VOLTAGE_2, &panelValues->ch2Voltage);
	GetCtrlVal(chipmap, CHIPMAP_NUMBERSAMPLES, &panelValues->numberSamples);
	GetCtrlVal(chipmap, CHIPMAP_SAMPLEDURATION, &panelValues->sampleDuration);
	GetCtrlVal(chipmap, CHIPMAP_RELAXDURATION, &panelValues->relaxDuration);
	GetCtrlVal(chipmap, CHIPMAP_DISABLEKEITHLEY, &panelValues->disableKeithley);
	
	//do some error checking when there is time
	return 1;
}

/*
  Chip Map Thread
*/ 
int StartChipMappingThread(ChipMapperDefinition *panelValues) 
{
	int status = 0;
	int threadFunctionID = 0;
	
	gPanelValues = malloc(sizeof(ChipMapperDefinition));
	if(gPanelValues == NULL)
	{
		return 0;
	}
	
	gPanelValues->channel = panelValues->channel; 
	gPanelValues->startRow = panelValues->startRow;
	gPanelValues->startColumn = panelValues->startColumn;
	gPanelValues->endRow = panelValues->endRow;
	gPanelValues->endColumn= panelValues->endColumn;
	gPanelValues->workingVline = panelValues->workingVline;
	gPanelValues->neighborVline = panelValues->neighborVline;
	gPanelValues->floatVline = panelValues->floatVline;
	gPanelValues->ch1Voltage = panelValues->ch1Voltage;
	gPanelValues->ch2Voltage = panelValues->ch2Voltage;
	gPanelValues->numberSamples = panelValues->numberSamples;
	gPanelValues->sampleDuration = panelValues->sampleDuration;
	gPanelValues->relaxDuration = panelValues->relaxDuration;
	gPanelValues->disableKeithley = panelValues->disableKeithley;
	
	status = CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, (ThreadFunctionPtr) ChipMapperThread, (void*)gPanelValues,
		&threadFunctionID);
		
	return 1;
}

int CVICALLBACK ChipMapperThread (void *functionData)
{
	int logEnabled = 0;
	int status = 0;
	int row = 0;
	int column = 0;
	int index = 0;
	int dataIndex = 0;
	int numberPoints = 0;
	int *map = NULL;
	double *data = NULL;
	double mean = 0.0;
	double stdev = 0.0;
	char folder[384] = {0};
	char filename[512] = {0};
	Point cell;
	FILE *fp = NULL;
	
	ChipMapperDefinition *chipDef = (ChipMapperDefinition*) gPanelValues;
	
	map = calloc(sizeof(int), MAXIMUM_ELECTRODES);
	if(map == NULL)
	{
		goto Error;
	}
	
	data = calloc(sizeof(double), chipDef->numberSamples);
	if(data == NULL)
	{
		goto Error;
	}
	
	//get the folder name, if it does not exist, disable file logging
	GetCtrlVal(chipmap, CHIPMAP_DATAFOLDER, folder);
	if(strlen(folder) > 0)
	{
		logEnabled = 1;
	}	
	
	//first, set the entire chip to float
	//create a map that is all float
	for(index = 0; index < MAXIMUM_ELECTRODES; index++)
	{
		map[index] = chipDef->floatVline;	
	}
	
	//send the pattern to the chip
	status = SetChipMap(chipDef->channel, map);
	if(status != 0)
	{
		goto Error;
	}
	
	status = UpdateChipMapView(map, chipDef->workingVline, chipDef->neighborVline, chipDef->floatVline);
	if(status != 1)
	{
		goto Error;
	}
	
	//update table with blank rows
	GetNumTableRows (chipmap, CHIPMAP_TABLE, &row);
	if(row > 0)
	{
		DeleteTableRows (chipmap, CHIPMAP_TABLE, 1, row);	
	}
	numberPoints = ((chipDef->startRow - chipDef->endRow) + 1) * 
				   ((chipDef->startColumn - chipDef->endColumn) + 1);
	InsertTableRows (chipmap, CHIPMAP_TABLE, -1, numberPoints, VAL_USE_MASTER_CELL_TYPE );
	
	//next, set electrode state for measurement sites
	index = 1;
	
	for(row = chipDef->startRow; row >= chipDef->endRow; row--)
	{
		for(column = chipDef->startColumn; column >= chipDef->endColumn; column--)
		{
			cell.y = index;
			cell.x = 1;
			SetTableCellVal (chipmap, CHIPMAP_TABLE, cell, row);
			cell.x++;
			SetTableCellVal (chipmap, CHIPMAP_TABLE, cell, column);
			cell.x++;
			
			status = SetNextMeasurementSite(map, chipDef->channel, row, column, chipDef->workingVline, chipDef->neighborVline);
			status = SetDacVoltage(0, chipDef->ch1Voltage);
			status = SetDacVoltage(1, chipDef->ch2Voltage);
			status = UpdateChipMapView(map, chipDef->workingVline, chipDef->neighborVline, chipDef->floatVline);
			
			if(chipDef->disableKeithley == 0)
			{
				//do measurements, save data for table in preallocated tables
				status = ReadSamples(chipDef->numberSamples, chipDef->sampleDuration, data);
				if(status < 0)
				{
					MessagePopup("ERROR", "Error fetching data from Keithley");
					goto Error;
				}
				//for a graph control
				DeleteGraphPlot (chipmap, CHIPMAP_GRAPH, -1, VAL_IMMEDIATE_DRAW );
				PlotY (chipmap, CHIPMAP_GRAPH, data, chipDef->numberSamples, VAL_DOUBLE,
					VAL_THIN_LINE, VAL_CONNECTED_POINTS, VAL_SOLID, VAL_CONNECTED_POINTS,
					VAL_RED);
				SetCtrlVal(chipmap, CHIPMAP_ELAPSEDSAMPLETIME, status);
			}
			else
			{
				//wait to gather measurements externally
				if(chipDef->sampleDuration > 0.0)
				{
					Delay(chipDef->sampleDuration);
				}
			}
			
			//save data if a folder is specified
			if(logEnabled == 1)
			{
				sprintf(filename, "%s\\%s_%d_%d.log", folder, DateStr(), row, column);
				fp = fopen(filename, "wt");
				if(fp != NULL)
				{
					for(dataIndex = 0; dataIndex < chipDef->numberSamples; dataIndex++)
					{
						fprintf(fp, "%15.15e\n", data[dataIndex]);
					}
					//fprintf(fp, "%d\n", status);
				   	fflush(fp);
					fclose(fp);
				}
				else  // if there is an error logging, disable it.
				{
					logEnabled = 0;
				}
			}
			
			//next, we need to set the last electrodes used to a float state
			status = SetDacVoltage(0, 0.0);
			status = SetDacVoltage(1, 0.0);
			status = SetNextMeasurementSite(map, chipDef->channel, row, column, chipDef->floatVline, chipDef->floatVline);
			status = UpdateChipMapView(map, chipDef->workingVline, chipDef->neighborVline, chipDef->floatVline);
			if(chipmapAbortFlag == 1)
			{
				MessagePopup("ABORTED", "This session has been terminated");
				goto Error;
			}
			
			//wait for things to settle before moving to the next if necessary
			if(chipDef->relaxDuration > 0.0)
			{
				Delay(chipDef->relaxDuration);
			}
			
			//update table with data
			StdDev (data, chipDef->numberSamples, &mean, &stdev);
			SetTableCellVal (chipmap, CHIPMAP_TABLE, cell, mean);
			cell.x++;
			SetTableCellVal (chipmap, CHIPMAP_TABLE, cell, stdev); 
			index++;
		}
	}
	
Error:
	if(map)
		free(map);
	if(data)
		free(data);
	if(chipDef)
		free(chipDef);
	chipmapInProcess = 0;
	
	return 1;
}

int SetNextMeasurementSite(int *map, int channel, int row, int column, int workingVline, int neighborVline)
{
	int status = 0;
	int workingIndex = 0;
	int neighborIndex = 0;
	int workingRow = 0;
	int workingColumn = 0;
	int neighborRow = 0;
	int neighborColumn = 0;
	
	//first, check to see if we are on any of the edges, we have to do special stuff for that.
	if(column == 0 || column == MAXIMUM_COLUMNS-1)
	{
		if(row == 0) // we're up at the top of the chip, put the neighbor below the working
		{
			neighborRow = row+1;
			neighborColumn = column;
		}
		else
		{
			neighborRow = row-1;
			neighborColumn = column;
		}
	}
	else
	{
		neighborRow = row;
		neighborColumn = column-1;
	}
	
	workingRow = row;
	workingColumn = column;
	
	workingIndex = (workingRow * MAXIMUM_COLUMNS) + workingColumn;
	neighborIndex = (neighborRow * MAXIMUM_COLUMNS) + neighborColumn;
	
	if(workingIndex > MAXIMUM_ELECTRODES-1 || neighborIndex > MAXIMUM_ELECTRODES-1)
	{
		return 0;
	}
	map[workingIndex] = workingVline;
	map[neighborIndex] = neighborVline;
	
	status = SetChipState(channel, workingRow, workingColumn, workingVline);
	if(status != 0)
	{
		return status;
	}
	status = SetChipState(channel, neighborRow, neighborColumn, neighborVline);
	if(status != 0)
	{
		return status;
	}
	
	return 1;
}

int UpdateChipMapView(int *pattern, int workingVline, int neighborVline, int floatVline)
{
	int row = 0;
	int column = 0;
	int index = 0;
	int fillColor = 0;

	SetCtrlAttribute (chipmap, CHIPMAP_CANVAS, ATTR_PEN_COLOR, VAL_BLACK);

	CanvasStartBatchDraw (chipmap, CHIPMAP_CANVAS);

	for(row = 0; row < MAXIMUM_ROWS; row++)
	{
		for(column = 0; column < MAXIMUM_COLUMNS; column++)
		{
			if(pattern[index] == floatVline)
			{
				fillColor = VAL_DK_GRAY;
			}
			else if(pattern[index] == workingVline)
			{
				fillColor = VAL_BLUE;
			}
			else if(pattern[index] == neighborVline)
			{
				fillColor = VAL_YELLOW;
			}
			else
			{
				fillColor = VAL_RED;
			}
			
			SetCtrlAttribute (chipmap, CHIPMAP_CANVAS, ATTR_PEN_FILL_COLOR, fillColor);
		
			CanvasDrawRect (chipmap, CHIPMAP_CANVAS, 
				MakeRect((row*CHIPMAP_PIXELS_PER_ROW), (column*CHIPMAP_PIXELS_PER_COLUMN), CHIPMAP_PIXELS_PER_ROW, CHIPMAP_PIXELS_PER_COLUMN),
				VAL_DRAW_FRAME_AND_INTERIOR);
			//VAL_DRAW_FRAME
			/*CanvasDrawOval (patternPanel, BLOCKDEF_CHIPIMAGE,
				MakeRect((row*PIXELS_PER_ROW)+1, (column*PIXELS_PER_COLUMN)+1, PIXELS_PER_ROW-2, 
					PIXELS_PER_COLUMN-2),
				VAL_DRAW_FRAME_AND_INTERIOR);
			*/	
			index++;
		}	
	}

	CanvasEndBatchDraw (chipmap, CHIPMAP_CANVAS);

	return 1;
}



int CVICALLBACK SelectFolderCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int status = 0;
	char pathName[512] = {0};
	
	switch (event)
		{
		case EVENT_COMMIT:
			status = DirSelectPopup ("", "Select Data Folder", 1, 1, pathName);
			if(status != 1)
			{
				return 0;
			}
			SetCtrlVal(panel, CHIPMAP_DATAFOLDER, pathName);
			break;
		}
	return 0;
}
