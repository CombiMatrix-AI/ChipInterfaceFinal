/*
  KE6485Driver.c
  
  Keithley Instruments 6485 Driver
  
  Interface with Keithley Instrument Driver (VXI) to control the KE6485
  through the GPIB interface (GPIB-USB-B)
*/
#include <cvirte.h>		
#include <userint.h>
#include "KE6485TestPanel.h"
#include "KE6485Driver.h"

int keithley;
double sample;			 //global for receiving measurements

void CVICALLBACK InsertNewMeasurement (WinMsgWParam wParam, WinMsgLParam lParam, void *callbackData);

int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)
{
	int measurementAvailableHandle = 0;
	if (InitCVIRTE (hInstance, 0, 0) == 0)
		return -1;	/* out of memory */
	if ((keithley = LoadPanel (0, "KE6485TestPanel.uir", KETEST)) < 0)
		return -1;
	SetSystemPopupsAttribute (ATTR_POPUP_STYLE, VAL_CLASSIC);	
	
	//Register windows message for new measurement available (Infinate Acquire).
	//RegisterWinMsgCallback ((WinMsgCallbackPtr) InsertNewMeasurement, MEASUREMENT_AVAILABLE, (void*)&sample, 
	//	0, &measurementAvailableHandle, 1);	
	
	DisplayPanel (keithley);
	RunUserInterface ();
	DiscardPanel (keithley);
	CloseKE6485();
	//UnRegisterWinMsgCallback (measurementAvailableHandle);
	return 0;
}

int CVICALLBACK KeithleyTestPanelCB (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_CLOSE:
			QuitUserInterface (0);
			break;
		}
	return 0;
}

int CVICALLBACK InitKeithleyCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int status = 0;
	char msg[100] = {0};
	
	switch (event)
		{
		case EVENT_COMMIT:
			SetCtrlVal(panel, KETEST_LASTRESULT, "Operation Pending...");
			
			//Initialize the instrument
			status = InitKE6485();
			if(status != 0)
			{
				sprintf(msg, "Failed to initialize instrument (error %d)", status);
				MessagePopup("Failure", msg);
			}
			else
			{
				MessagePopup("Success", "KE6485 is initialized");
			}
			GetLastResult(status, msg);
			SetCtrlVal(panel, KETEST_LASTRESULT, msg); 
			break;
		}
	return 0;
}

int CVICALLBACK AquireStatisticsCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int status = 0;
	char msg[256] = {0};
	
	int numberSamples = 0;
	int maxTime = 0;
	double mean = 0.0;
	double stdev = 0.0;
	double min = 0.0;
	double max = 0.0;
	double pk2pk = 0.0;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(panel, KETEST_NUMSAMPLESSTATS, &numberSamples);
			GetCtrlVal(panel, KETEST_MAXTIMESTATS, &maxTime);
			//do some error checking in the future
			
			//do the read
			SetCtrlVal(panel, KETEST_LASTRESULT, "Operation Pending...");
			
			status = ReadStatistics(numberSamples, maxTime, &mean, &stdev, &min, &max, &pk2pk);
			if(status != 0)
			{
				SetCtrlVal(panel, KETEST_MEAN, -1.0);
				SetCtrlVal(panel, KETEST_STDEV, -1.0);
				SetCtrlVal(panel, KETEST_MIN, -1.0);
				SetCtrlVal(panel, KETEST_MAX, -1.0);
				SetCtrlVal(panel, KETEST_PK2PK, -1.0);
				
				sprintf(msg, "Error reading statistics from instrument (error %d)", status);
				MessagePopup("Failure", msg);
			}
			else
			{
				SetCtrlVal(panel, KETEST_MEAN, mean);
				SetCtrlVal(panel, KETEST_STDEV, stdev);
				SetCtrlVal(panel, KETEST_MIN, min);
				SetCtrlVal(panel, KETEST_MAX, max);
				SetCtrlVal(panel, KETEST_PK2PK, pk2pk);
			}
			GetLastResult(status, msg);
			SetCtrlVal(panel, KETEST_LASTRESULT, msg);
			break;
		}
	return 0;
}

int CVICALLBACK AquireSamplesCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int status = 0;
	char msg[256] = {0};
	
	int numberSamples = 0;
	double *sampleBuffer = NULL;
	
	int numberRows = 0;
	Rect cellRange;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(panel, KETEST_NUMSAMPLESSAMP, &numberSamples);
			if(numberSamples <= 0)
			{
				sprintf(msg, "Number Samples must be greater than zero!");
				MessagePopup("Failure", msg);
				SetCtrlVal(panel, KETEST_LASTRESULT, msg);
				return 0;
			}
			sampleBuffer = calloc(sizeof(double), numberSamples);
			if(sampleBuffer == NULL)
			{
				sprintf(msg, "Failed to allocate sampleBuffer (%d elements)", numberSamples);
				MessagePopup("Failure", msg);
				SetCtrlVal(panel, KETEST_LASTRESULT, msg);
				return 0;
			}
			
			//do the read
			SetCtrlVal(panel, KETEST_LASTRESULT, "Operation Pending...");
			
			status = ReadSamples(numberSamples, sampleBuffer);
			
			GetNumTableRows (panel, KETEST_SAMPLETABLE, &numberRows);
			if(numberRows > 0)
			{
				DeleteTableRows (panel, KETEST_SAMPLETABLE, 1, -1);	
			}
			if(status != 0)
			{
				sprintf(msg, "Error aquiring readings from instrument (error %d)", status);
				MessagePopup("Failure", msg);
			}
			else
			{
				InsertTableRows (panel, KETEST_SAMPLETABLE, 1, numberSamples, VAL_USE_MASTER_CELL_TYPE);
				
				cellRange.top = 1;
				cellRange.left = 1;
				cellRange.width = 1;
				cellRange.height = numberSamples;
				
				SetTableCellRangeVals (panel, KETEST_SAMPLETABLE, cellRange, sampleBuffer, VAL_COLUMN_MAJOR);
			}
			
			GetLastResult(status, msg);
			SetCtrlVal(panel, KETEST_LASTRESULT, msg);
			
			break;
		}
	return 0;
}

int CVICALLBACK AquireSampleCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int status = 0;
	char msg[256];
	
	double sample = 0.0;
	
	switch (event)
		{
		case EVENT_COMMIT:
			
			//do the read
			SetCtrlVal(panel, KETEST_LASTRESULT, "Operation Pending...");
			
			status = ReadSingleSample(&sample);
			if(status != 0)
			{
				sprintf(msg, "Error aquiring reading from instrument (error %d)", status);
				MessagePopup("Failure", msg);
			}
			else
			{
				SetCtrlVal(panel, KETEST_SAMPLE, sample);
			}
			
			GetLastResult(status, msg);
			SetCtrlVal(panel, KETEST_LASTRESULT, msg);
			
			break;
		}
	return 0;
}

int CVICALLBACK InfinateAcquireCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int state = 0;
	int numberRows = 0;
	int status = 0;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(panel, control, &state);
			status = InfinateAcquire(state);
			break;
		}
	return 0;
}

/*
  Handle windows message and post new measurement
*/
void CVICALLBACK InsertNewMeasurement (WinMsgWParam wParam, WinMsgLParam lParam, void *callbackData)
{
	int status = 0;
	
	return;
}

int CVICALLBACK ClearTableCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int numberRows = 0;
	
	switch (event)
		{
		case EVENT_COMMIT:
			//clear the table
			GetNumTableRows (panel, KETEST_SAMPLETABLE, &numberRows);
			if(numberRows > 0)
			{
				DeleteTableRows (panel, KETEST_SAMPLETABLE, 1, -1);
			}
			break;
		}
	return 0;
}
