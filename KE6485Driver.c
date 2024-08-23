#include <userint.h>
#include "KE6485TestPanel.h"

/*
  KE6485Driver.c
  
  Keithley Instruments 6485 Driver
  
  Interface with Keithley Instrument Driver (VXI) to control the KE6485
  through the GPIB interface (GPIB-USB-B)


*/
extern int keithley;

#include "KE6485Driver.h"

int acquire = 0;
ViSession vi = VI_NULL;  //VI Session handle
double sampleDuration = 0.0;

/*
  Establish connection with device and initialize it.
*/
int InitKE6485(double nplc, int autorange, double resolution, double range, int simulate)
{
	ViStatus error = VI_SUCCESS;
	if((int)nplc == -1)
	{
		nplc = 1;
	}
	if(autorange == -1)
	{
		autorange = 1;
	}
	if((int)resolution == -1)
	{
		resolution = 10e-15;	
	}
	if((int)range == -1)
	{
		range = 2e-9;	
	}
	if(simulate == -1)
	{
		simulate = 0;
	}
	
	//Call KE6485_init to initialize the driver and get a vi session handle
	checkErr (KE6485_init (INSTRUMENT, VI_TRUE, VI_TRUE, &vi)); 
	
	if(simulate == 1)
	{
		checkErr( KE6485_SetAttributeViBoolean( vi, VI_NULL, KE6485_ATTR_SIMULATE, VI_TRUE) );
	}
	else
	{
		checkErr( KE6485_SetAttributeViBoolean( vi, VI_NULL, KE6485_ATTR_SIMULATE, VI_FALSE) );
	}
	
	//Configure measurement, autorange, range
    if(autorange == 1)
    {
    	//enable autoranging
    	checkErr (KE6485_ConfigureMeasurement (vi, KE6485_VAL_DC_CURRENT, KE6485_VAL_AUTO_RANGE_ON, resolution));

		//set nplc (integration speed), autorange on
		checkErr( KE6485_ConfigureDCISense ( vi, 0, nplc, VI_TRUE, VI_FALSE) );

	}
	else
	{
		//disable autoranging
		checkErr (KE6485_ConfigureMeasurement (vi, KE6485_VAL_DC_CURRENT, KE6485_VAL_AUTO_RANGE_OFF, resolution));

		//set nplc (integration speed), autorange off
		checkErr( KE6485_ConfigureDCISense ( vi, 0, nplc, VI_FALSE, VI_FALSE) );

		//set range
		checkErr (KE6485_SetAttributeViReal64 (vi, NULL, KE6485_ATTR_RANGE, range));
		
	}
	
	
	// Turn off zero checking
    checkErr( KE6485_SetAttributeViBoolean( vi, VI_NULL, KE6485_ATTR_ZEROCHECK_ENABLED, VI_FALSE ) );
	
    // Turn off auto zeroing
    checkErr( KE6485_SetAttributeViInt32( vi, VI_NULL, KE6485_ATTR_AUTO_ZERO, VI_FALSE ) );
	
    // disable front panel lockout
    //checkErr( KE6485_SetAttributeViBoolean( vi, VI_NULL, KE6485_ATTR_FRONT_PANEL_LOCKOUT, VI_FALSE ) );
    
    
Error:
	if ( error != VI_SUCCESS )
	{
		HandleError(vi, error);
	}
	
	return (int)error;
}

void CloseKE6485(void)
{
	if (vi) KE6485_close(vi);
}

int StartSampleReadThread(double duration)
{
	int threadFunctionID = 0;
	sampleDuration = duration;
	CmtScheduleThreadPoolFunctionAdv (DEFAULT_THREAD_POOL_HANDLE, (ThreadFunctionPtr) StartSampleReadThreadFunction, 
		(void*)&duration, DEFAULT_THREAD_PRIORITY, (ThreadFunctionCallbackPtr) SampleReadThreadFunctionCallback, 
		(EVENT_TP_THREAD_FUNCTION_BEGIN | EVENT_TP_THREAD_FUNCTION_END), NULL, RUN_IN_SCHEDULED_THREAD, 
		&threadFunctionID);
		
	return threadFunctionID;
}

int CVICALLBACK StartSampleReadThreadFunction (void *functionData)
{
	char message[256] = {0};
	
	Delay(sampleDuration);
	
	sprintf(message, "Finished Keithley acquisition %s", TimeStr());
	AddToDebugOutput(message);
	CmtExitThreadPoolThread (1);
	return 1;
}

void CVICALLBACK SampleReadThreadFunctionCallback (int poolHandle, int functionID, unsigned int event, 
	int value, void *callbackData)
{
	double buffer[2500];
	int numberSamples = 0;
	int numberRows = 0;
	Rect cellRange;
	
	switch(event)
	{
		case EVENT_TP_THREAD_FUNCTION_BEGIN:
			break;
		case EVENT_TP_THREAD_FUNCTION_END:
			//get the samples in the Keithley's buffer
			StopAndFetchSampleRead(buffer, &numberSamples);
			//clear the table
			if(numberSamples > 0)
			{
				GetNumTableRows (keithley, KETEST_SAMPLETABLE, &numberRows);
				if(numberRows > 0)
				{
					DeleteTableRows (keithley, KETEST_SAMPLETABLE, 1, -1);
				}
				//fill in the table
				InsertTableRows (keithley, KETEST_SAMPLETABLE, 1, numberSamples, VAL_USE_MASTER_CELL_TYPE);
			
				cellRange.top = 1;
				cellRange.left = 1;
				cellRange.width = 1;
				cellRange.height = numberSamples;
			
				SetTableCellRangeVals (keithley, KETEST_SAMPLETABLE, cellRange, buffer, VAL_COLUMN_MAJOR);
			
				//release the thread back to the thread pool
				CmtReleaseThreadPoolFunctionID (poolHandle, functionID);
			}
			else
			{
				MessagePopup("Warning", "There were no samples taken from the Keithley");
				return;
			}
			break;
		default:
			break;
			
	}
}

int StartSampleRead(void)
{
	ViStatus error = VI_SUCCESS;
	ViInt32		bufferSize;
	ViInt32		sampleCount = -1;
    ViInt32		sampleTrigger = KE6485_VAL_IMMEDIATE;
    ViInt32		triggerCount = 1;
    ViReal64	sampleDuration = 0.0;
    char message[256] = {0};
    
	//Call configure multi-point to configure the measurement
	checkErr (KE6485_ConfigureMultiPoint (vi, triggerCount, sampleCount, sampleTrigger, sampleDuration));

	// 	(1) use the instrument-specific KE6485_ConfigureBuffer function to return binay data.
	//  Configure buffer
	bufferSize=2500;
	checkErr (KE6485_ConfigureBuffer (vi, KE6485_VAL_ELEMENT_READING, KE6485_VAL_FORMAT_SINGLE, 
		bufferSize, KE6485_VAL_TIMESTAMP_ABS));
	checkErr (KE6485_Initiate (vi));
	sprintf(message, "Started Keithley acquisition %s", TimeStr());
    AddToDebugOutput(message);
Error:
	if ( error != VI_SUCCESS )
	{
		HandleError( vi, error);
	}	
	return error;
}

void KE6485Abort(void)
{
 	KE6485_Abort(vi);			
}

void KE6485ClearBuffer(void)
{
	KE6485_ClearBuffer(vi);
}	

int StopAndFetchSampleRead(double *buffer, int *numberSamples)
{
	ViStatus error = VI_SUCCESS;
	KE6485_Abort(vi);
	checkErr( KE6485_FetchMultiPoint(vi, 1000, 2500, buffer, numberSamples) );
Error:
	if ( error != VI_SUCCESS )
	{
		HandleError( vi, error);
	}	
	return error;
}

/*
  Use "Fetch Statistics" function to read statistics data from buffer
*/
int ReadStatistics(int numberSamples, int maxTime, double *mean, double *stdev, double *min, double *max,
	double *pk2pk)
{
	ViStatus error = VI_SUCCESS;
	ViInt32		triggerCount;
    ViInt32		sampleCount;
    ViInt32		sampleTrigger;
    ViReal64	sampleInterval_sec;
    ViInt32		bufferSize;
	ViBoolean	srq;
	FILE *fp = VI_NULL;
	
	if(vi == VI_NULL)
	{
		error = InitKE6485(-1, -1, -1, -1, 0);
		if(error != VI_SUCCESS)
		{
			return 0;
		}	
	}
	//Open a file for output results
	fp=fopen(".\\stats_output.txt", "a");
	
    /*Call configure multi-point to configure the measurement
	  The sample count set to numberSamples and trigger count set to 1. The total
	  measurement readings are numberSamples. Trigger source set to immediate */
	triggerCount=1, sampleCount=numberSamples;
	sampleTrigger=KE6485_VAL_IMMEDIATE;
	sampleInterval_sec=0;  
	checkErr (KE6485_ConfigureMultiPoint (vi, triggerCount, sampleCount, sampleTrigger,
                                              sampleInterval_sec));
	bufferSize=triggerCount * sampleCount;
    
    //We will use SRQ to determine that measurement has completed and data is available
    //Configure SQR event to detect idle state or buffer full
	checkErr (KE6485_ConfigureSRQEvents(vi, KE6485_SRQ_ON_IDLE|KE6485_SRQ_ON_BUFFER_FULL));
	//Enable SQR event
	checkErr (KE6485_EnableSRQEvents(vi, VI_TRUE));
     
    //Start measurements
    checkErr( KE6485_Initiate (vi));
    
    checkErr (KE6485_Spoll(vi, &srq));//Check SRQ
	while(!srq){ //If SRQ is true, measurements are finished  
		checkErr (KE6485_Spoll(vi, &srq));//Check SRQ   
	}
	//Make sure the instrument is in idle state
	checkErr( KE6485_Abort(vi));

	//Select statistics function "Mean"
    checkErr (KE6485_ConfigureStatistics(vi, KE6485_VAL_STATS_MEAN));
	checkErr (KE6485_FetchStatistics(vi, maxTime, mean));
	//Output Mean
	fprintf(fp, "%s %s Mean  = %-15.15f\n", DateStr(), TimeStr(), *mean);
	
	//Select statistics function "Standard Deviation"
    checkErr (KE6485_ConfigureStatistics(vi, KE6485_VAL_STATS_SDEV));
    checkErr (KE6485_FetchStatistics(vi, maxTime, stdev));  
    //Output standard deviation
    fprintf(fp, "%s %s SDEV  = %-15.15f\n", DateStr(), TimeStr(), *stdev); 
    
    //Select statistics function "Minimum"
    checkErr (KE6485_ConfigureStatistics(vi, KE6485_VAL_STATS_MIN));
    checkErr (KE6485_FetchStatistics(vi, maxTime, min));  
    //Output minimum
    fprintf(fp, "%s %s Min   = %-15.15f\n", DateStr(), TimeStr(), *min); 
    
    //Select statistics function "Maximum"
    checkErr (KE6485_ConfigureStatistics(vi, KE6485_VAL_STATS_MAX));
    checkErr (KE6485_FetchStatistics(vi, maxTime, max));  
    //Output maximum
    fprintf(fp, "%s %s Max   = %-15.15f\n", DateStr(), TimeStr(), *max); 
    
    //Select statistics function "Peak to Peak Distance"
    checkErr (KE6485_ConfigureStatistics(vi, KE6485_VAL_STATS_PK2PK));
    checkErr (KE6485_FetchStatistics(vi, maxTime, pk2pk));  
    //Output Peak to Peak Distance
    fprintf(fp, "%s %s PK2PK = %-15.15f\n", DateStr(), TimeStr(), *pk2pk); 
    

Error:
	if ( error != VI_SUCCESS )
	{
		HandleError( vi, error);
	}

	if (fp) fclose(fp);
	
	return error;
}

/*int ReadSamples(int numberSamples, double totalDuration, double *sampleBuffer)
{
	ViStatus error = VI_SUCCESS;
	int index = 0;
	int triggerDelay = (totalDuration/(double)numberSamples)*1000.0;
	clock_t startTrigger;
	clock_t elapsedTrigger;
	clock_t start;
	
	checkErr (KE6485_ConfigureMeasurement (vi, KE6485_VAL_DC_CURRENT, KE6485_VAL_AUTO_RANGE_ON, 1E-18));
	checkErr (KE6485_ConfigureTrigger (vi, KE6485_VAL_SOFTWARE_TRIG, 0.0));
	checkErr (KE6485_ConfigureBuffer (vi, 3, KE6485_VAL_TIMESTAMP_DELTA, numberSamples*2, KE6485_VAL_TIMESTAMP_DELTA));
	checkErr (KE6485_ClearBuffer (vi));
	checkErr (KE6485_EnableBuffer (vi, KE6485_VAL_CONTROL_NEXT));
	
	for(index = 0; index < numberSamples; index++)
    {
    	start = clock();
    	checkErr (KE6485_Initiate(vi));
		checkErr ( KE6485_SendSoftwareTrigger(vi));
    	checkErr ( KE6485_Fetch(vi, 100, &sampleBuffer[index]));
    	startTrigger = clock();
    	
    	do
    	{
    		elapsedTrigger = clock() - startTrigger;
    	}while(elapsedTrigger < triggerDelay);
    	//printf("Trigger time: %d Sample time: %d\n\r", elapsedTrigger, clock()-start);
    }
Error:

	return error;
}*/


/*int ReadSamples(int numberSamples, double totalDuration, double *sampleBuffer)
{
	ViStatus error = VI_SUCCESS;
	ViInt32		measFunction;
	ViReal64	range, resolution;
    ViInt32		triggerCount = 1;
    ViInt32		sampleCount = numberSamples;
    ViInt32		sampleTrigger = KE6485_VAL_IMMEDIATE;
    ViInt32		dataElements, dataFormat, bufferSize, timeStampFormat;
	ViInt32		maxTime, arraySize, actualPts, i;
	FILE*		fp = VI_NULL;
	int triggerDelay = totalDuration/(double)numberSamples;
	int index = 0;
	
	clock_t start;
	clock_t startTrigger;
	clock_t elapsedTrigger;
	clock_t elapsed;
	
	if(vi == VI_NULL)
	{
		return 0;
	}
	
	fp=fopen(".\\sample.txt", "wt");

	//Call configure measurements to set measurement function. 
	//  Model 6485 only supports the DC current measurement. In this example,
	//  measurement function set to DC current 
	checkErr (KE6485_ConfigureMeasurement (vi, KE6485_VAL_DC_CURRENT, KE6485_VAL_AUTO_RANGE_ON, 1E-18)); 
    
    //Call configure multi-point to configure the measurement
	checkErr (KE6485_ConfigureMultiPoint (vi, 1, numberSamples, sampleTrigger, totalDuration));

// 	(1) use the instrument-specific KE6485_ConfigureBuffer function to return binay data.
	//Configure buffer
	dataElements=3;//KE6485_VAL_ELEMENT_READING; //Only store readings in the buffer
	dataFormat=3;// KE6485_VAL_FORMAT_SINGLE; //Set data format to single to do the binary data transfer
	bufferSize=triggerCount * sampleCount;
	timeStampFormat=KE6485_VAL_TIMESTAMP_DELTA;  
	checkErr (KE6485_ConfigureBuffer (vi, dataElements, dataFormat, bufferSize, timeStampFormat));
    
    //Locate the memory
	arraySize=bufferSize;
  	
  	memset(sampleBuffer, arraySize, 0);


// 	(2) call low level functions "Initiate" and "Fetch Multi-Point" to read buffer data
   //Start measurements
    start = clock();
    checkErr( KE6485_Initiate (vi));
    elapsed = clock() - start;
    
	printf("elapsed time: %d\n\r", elapsed);
	
    //try sitting in a loop, issuing software triggers at a rate
    for(index = 0; index < triggerCount; index++)
    {
    	checkErr ( KE6485_SendSoftwareTrigger(vi));
    	startTrigger = clock();
    	
    	do
    	{
    		elapsedTrigger = clock() - startTrigger;
    	}while(elapsedTrigger < triggerDelay);
    	printf("Trigger time: %d\n\r", elapsedTrigger);
    }
    
    //Transfer data from the buffer 
    maxTime=1000 * (1 + bufferSize * 0.1); // milliseconds * (one second margin + number of samples * default aperature time)
	checkErr( KE6485_FetchMultiPoint(vi, maxTime, arraySize, sampleBuffer, &actualPts) );
	//Output results
	for(i=0; i<actualPts; i++){
		fprintf(fp, "%15.15f\n", sampleBuffer[i]);
	}
	

Error:
	if ( error != VI_SUCCESS )
	{
		HandleError( vi, error);
	}

	if (fp) fclose(fp);
	
	return error;
}*/

int ReadSamples(int numberSamples, double temp, double *sampleBuffer)
{
	ViStatus error = VI_SUCCESS;
	ViInt32		measFunction;
	ViReal64	range, resolution;
    ViInt32		triggerCount;
    ViInt32		sampleCount;
    ViInt32		sampleTrigger;
    ViReal64	sampleInterval_sec;
    ViInt32		dataElements, dataFormat, bufferSize, timeStampFormat;
	ViInt32		maxTime, arraySize, actualPts, i;
	clock_t start;
	
	start = clock();
	if(vi == VI_NULL)
	{
		return 0;
	}
	
	//Call configure multi-point to configure the measurement
	//  The sample count set to numberSamples and trigger count set to 1. The total
	//  measurement readings are numberSamples. Sample trigger source set to immediate
	triggerCount=1, sampleCount=numberSamples;
	sampleTrigger=KE6485_VAL_IMMEDIATE;
	sampleInterval_sec=0;  
	checkErr (KE6485_ConfigureMultiPoint (vi, triggerCount, sampleCount, sampleTrigger,
                                              sampleInterval_sec));

// 	(1) use the instrument-specific KE6485_ConfigureBuffer function to return binay data.
	//Configure buffer
	dataElements=KE6485_VAL_ELEMENT_READING; //Only store readings in the buffer
	dataFormat= KE6485_VAL_FORMAT_SINGLE; //Set data format to single to do the binary data transfer
	//dataElements = 3;	// Reading and Time
	//dataFormat = 3;		// Reading and Time
	bufferSize=triggerCount * (sampleCount);
	timeStampFormat=KE6485_VAL_TIMESTAMP_DELTA;  
	checkErr (KE6485_ConfigureBuffer (vi, dataElements, dataFormat, bufferSize, timeStampFormat));
    
    //Locate the memory
	arraySize=bufferSize;
  	
  	memset(sampleBuffer, arraySize, 0);


// 	(2) call low level functions "Initiate" and "Fetch Multi-Point" to read buffer data
   //Start measurements
    checkErr( KE6485_Initiate (vi));
    //Transfer data from the buffer 
    maxTime=1000 * (1 + bufferSize * 0.1); // milliseconds * (one second margin + number of samples * default aperature time)
	checkErr( KE6485_FetchMultiPoint(vi, maxTime, arraySize, sampleBuffer, &actualPts) );
	//printf("Sample time: %d\n\r", clock()-start);  
	//Output results
	/*for(i=0; i<actualPts; i+=2){
		printf("%15.15f %15.15f\n", sampleBuffer[i], sampleBuffer[i+1]);
	}*/
	/*for(i=0; i<actualPts; i++){
		printf("%15.15f\n", sampleBuffer[i]);
	}*/
	

Error:
	if ( error != VI_SUCCESS )
	{
		HandleError( vi, error);
		return error;
	}

	return clock()-start;
}

int ReadSingleSample(double *sample)
{
	ViStatus error = VI_SUCCESS;
	ViInt32		sampleTrigger;
    ViReal64	sampleInterval_sec;
    
	if(vi == VI_NULL)
	{
		error = InitKE6485(-1, -1, -1, -1, 0);
		if(error != VI_SUCCESS)
		{
			return 0;
		}	
	}
	
	if(vi == VI_NULL)
	{
		error = InitKE6485(-1, -1, -1, -1, 0);
		if(error != VI_SUCCESS)
		{
			return 0;
		}	
	}
	
	sampleTrigger=KE6485_VAL_IMMEDIATE;
	sampleInterval_sec=KE6485_VAL_AUTO_DELAY_OFF;  
	checkErr( KE6485_ConfigureTrigger(vi, sampleTrigger, sampleInterval_sec));
    
    checkErr ( KE6485_Read(vi, 100, sample));

Error:
	if ( error != VI_SUCCESS )
	{
		HandleError( vi, error);
	}

	return error;
}

int InitiateAndFetch(int maxTime, double *sample)
{
	ViStatus error = VI_SUCCESS;
	
	checkErr ( KE6485_Initiate(vi));
	checkErr ( KE6485_Fetch(vi, maxTime, sample));

Error:
	if ( error != VI_SUCCESS )
	{
		HandleError( vi, error);
	}

	return error;
}

void GetLastResult(int status, char *msg)
{
	KE6485_error_message( vi, status, msg );
}

void HandleError( ViSession vi, ViStatus error)
{
	ViChar		primaryError[ BUFFER_SIZE ] = "";
	ViChar		secondaryError[ BUFFER_SIZE ] = "";
	ViChar		errorElaboration[ BUFFER_SIZE ] = "";
	ViInt32		primaryErrorCode;
	ViInt32		secondaryErrorCode;

	FILE *fp = VI_NULL;
	fp=fopen(".\\error.txt", "a");
	
	KE6485_GetErrorInfo( vi, &primaryErrorCode, &secondaryErrorCode, errorElaboration );
	KE6485_error_message( vi, primaryErrorCode, primaryError );
	if (secondaryErrorCode != VI_SUCCESS) 
	{
		KE6485_error_message( vi, secondaryErrorCode, secondaryError);
	}
	DebugPrintf( "%s\n%s\n%s\n", primaryError, secondaryError, errorElaboration );
	if (fp) 
	{
		fprintf( fp, "%s %s %s\n%s\n%s\n", DateStr(), TimeStr(), primaryError, secondaryError, errorElaboration );
	}
	else
	{
		printf( "%s\n%s\n%s\n", primaryError, secondaryError, errorElaboration );
	}
	
	fclose(fp); 
}

/*
  Control the infiniate acquisition thread
*/ 
int InfinateAcquire(int state)
{
	static int threadFunctionID = 0;
	
	//kill the thread and return its resources back to the thread pool
	if(state == 0)
	{
		acquire = 0;
		threadFunctionID = 0;
	}
	
	//make sure we are not already doing the tread
	if(threadFunctionID != 0)
	{
		return -100; // aquisition already in process
	}
	
	//start thread if state is 1, or kill it if the state is 0
	if(state == 1)
	{
		acquire = 1;
		CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, (ThreadFunctionPtr) InfinateAcquireThread,
			NULL, &threadFunctionID);
		
	}
	
	return 1;
}

/*
  Infinate Acquisition thread
*/
int CVICALLBACK InfinateAcquireThread (void *functionData)
{
	double sample = 0.0;
	int index = 1;
	Point cell;
	
	cell.x = 1;
	GetNumTableRows (keithley, KETEST_SAMPLETABLE, &index);
	index+=1;
	
	
	while(acquire == 1)
	{
		cell.y = index;
		InitiateAndFetch(500, &sample);
		/*printf("%15.15f\n", sample);
		status = PostMessage(
		  NULL,      // handle to destination window
		  measurementAvailable,       // message
		  sample,  // first message parameter
		  0   // second message parameter
		);*/
		// This SUCKS!!!!!
		if(acquire == 1)  // make sure it is still going since an aquisition takes a little while
		{
			InsertTableRows (keithley, KETEST_SAMPLETABLE, index, 1, VAL_USE_MASTER_CELL_TYPE);
			SetTableCellVal (keithley, KETEST_SAMPLETABLE, cell, sample);
			index++;
		}
	}
	
	CmtExitThreadPoolThread (1);

	return 1;	
}


