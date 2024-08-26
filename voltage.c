//voltage.c
//
//setup and control voltage to electrodes

#include "voltage.h"

int threadFunctionID[NUMBER_DAC_CHANNELS];
VoltageHardware gProfile;
int stopFlag = 0;

int StartVoltageSeries(VoltageHardware profile)
{
	int status = 0;
	
	CopyVoltageProfile(&profile, &gProfile);
	stopFlag = 0;
	
	//schedule these threads at a higher priority
	status = CmtScheduleThreadPoolFunctionAdv (DEFAULT_THREAD_POOL_HANDLE, (ThreadFunctionPtr) VoltageProfileThread, 
		(void*)&gProfile.dacChannel[0], 1, NULL, NULL, NULL, NULL, &threadFunctionID[0]);
	if(status != 0)
	{
		MessagePopup("ERROR", "Could not start pulsing thread!");
		StopVoltageSeries();
		return 0;
	}

	//CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, (ThreadFunctionPtr) VoltageProfileThread, (void*)&gProfile.dacChannel[0], &threadFunctionID[0]);
	//CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, (ThreadFunctionPtr) VoltageProfileThread, (void*)&gProfile.dacChannel[1], &threadFunctionID[1]);
	//CmtWaitForThreadPoolFunctionCompletion (DEFAULT_THREAD_POOL_HANDLE, threadFunctionID[0], OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
	//CmtWaitForThreadPoolFunctionCompletion (DEFAULT_THREAD_POOL_HANDLE, threadFunctionID[1], OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
	//CmtReleaseThreadPoolFunctionID (DEFAULT_THREAD_POOL_HANDLE, threadFunctionID[0]);
	//CmtReleaseThreadPoolFunctionID (DEFAULT_THREAD_POOL_HANDLE, threadFunctionID[1]);
	return 0;
}

int CVICALLBACK VoltageProfileThread (void *functionData)
{
	VChannel *channel = (VChannel*) functionData;
	char message[256] = {0};
	int lockHandle = 0;
	int iteration = 0;
	int numberIterations = -1;
	
	
	if(channel->continous == 0)
	{
		numberIterations = channel->totalDuration / channel->onDuration;
	}
	
	CmtNewLock(NULL, OPT_TL_PROCESS_EVENTS_WHILE_WAITING, &lockHandle);
	sprintf(message, "%s Starting pulsing thread on channel %d\nStart V: %1.3f for %1.3f seconds\nVa: %1.3f for %1.3f %s\n", 
		TimeStr(), channel->id, channel->vStart, channel->onDuration, channel->vA, channel->offDuration, TimeStr());
	
	CmtGetLock(lockHandle);
	AddToLog(message);
	AddToDebugOutput(message);
	CmtReleaseLock(lockHandle);

	while(iteration != numberIterations)
	{
		iteration++;
		
		CmtGetLock(lockHandle);
		SetDacVoltage(channel->id, channel->vStart);
		CmtReleaseLock(lockHandle);
	
		if(stopFlag == 1)
		{
			break;
		}
		Delay(channel->onDuration);
		
		CmtGetLock(lockHandle);
		SetDacVoltage(channel->id, channel->vA);
		CmtReleaseLock(lockHandle);
		
		if(stopFlag == 1)
		{
			break;
		}
		
		Delay(channel->offDuration);

	}
	
	sprintf(message, "Finished pulsing thread on channel %d (%d iterations) %s\n", channel->id, iteration, TimeStr());
	CmtGetLock (lockHandle);
	AddToLog(message);
	AddToDebugOutput(message);
	CmtReleaseLock(lockHandle);
	CmtDiscardLock(lockHandle);
	
	//CmtReleaseThreadPoolFunctionID (DEFAULT_THREAD_POOL_HANDLE, threadFunctionID[channel->id]);
	//threadFunctionID[channel->id] = -1;
	//CmtExitThreadPoolThread (0);
	return 0;
}

int CopyVoltageProfile(VoltageHardware *source, VoltageHardware *destination)
{
	int index = 0;
	
	for(index = 0; index < NUMBER_DAC_CHANNELS; index++)
	{
		destination->dacChannel[index].id = source->dacChannel[index].id;
		destination->dacChannel[index].vProfile = source->dacChannel[index].vProfile;
		destination->dacChannel[index].vStart = source->dacChannel[index].vStart;
		destination->dacChannel[index].vA = source->dacChannel[index].vA;
		destination->dacChannel[index].vB = source->dacChannel[index].vB;
		destination->dacChannel[index].ramprate = source->dacChannel[index].ramprate;
		destination->dacChannel[index].onDuration = source->dacChannel[index].onDuration;
		destination->dacChannel[index].offDuration = source->dacChannel[index].offDuration;
		destination->dacChannel[index].totalDuration = source->dacChannel[index].totalDuration;
		destination->dacChannel[index].continous = source->dacChannel[index].continous;
	}  
	return 0;
}

void StopVoltageSeries(void)
{
	stopFlag = 1;
}	

int GetVoltageThreadID(int channel)
{
	return threadFunctionID[channel];
}

void InitVoltageDataStructure(VoltageHardware *vProfile)
{
	int index = 0;
	
	sprintf(vProfile->description, "");
	
	for(index = 0; index < NUMBER_DAC_CHANNELS; index++)
	{
		vProfile->dacChannel[index].id = index;
		vProfile->dacChannel[index].vProfile = 0;
		vProfile->dacChannel[index].vStart = 0.0;
		vProfile->dacChannel[index].vA = 0.0;
		vProfile->dacChannel[index].vB = 0.0;
		vProfile->dacChannel[index].ramprate = 0.0;
		vProfile->dacChannel[index].onDuration = 0.0;
		vProfile->dacChannel[index].offDuration = 0.0;
		vProfile->dacChannel[index].totalDuration = 0.0;
		vProfile->dacChannel[index].continous = 0.0;
	}
}

/**
Save voltage definition to file
*/
int SaveVoltageDataFile(char *filename, VoltageHardware *vProfile)
{
	int index = 0;
	char section[256] = {0};
	char message[256] = {0};
	IniText vProfileFileHandle;
	
	//allocate handle for file
	vProfileFileHandle = Ini_New(0);
	if(vProfileFileHandle == 0)
	{
		sprintf(message, "Could not create voltage data file handle.");
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;
	}
	
	//put data from object into file
	Ini_PutString(vProfileFileHandle, "Voltage Profile", "Description", vProfile->description);
	Ini_PutString(vProfileFileHandle, "Voltage Profile", "Filename", filename);
	//Ini_PutString(vProfileFileHandle, "Voltage Profile", "Comments", vProfile->comments);
	
	//save data for each channel
	for(index = 0; index < NUMBER_DAC_CHANNELS; index++)
	{
		/*int id;
		int vProfile;
		double vStart;
		double vA;
		double vB;
		double ramprate;		//millivolts Per Second
		double onDuration;		//in seconds
		double offDuration;		//in seconds
		double totalDuration;	//in seconds
		int continous;			//ignore on/off duration*/
		
		sprintf(section, "DAC Channel %d", index);
		Ini_PutInt   (vProfileFileHandle, section, "ID", vProfile->dacChannel[index].id);
		Ini_PutDouble(vProfileFileHandle, section, "vStart", vProfile->dacChannel[index].vStart);
		Ini_PutDouble(vProfileFileHandle, section, "vA", vProfile->dacChannel[index].vA);
		Ini_PutDouble(vProfileFileHandle, section, "vB", vProfile->dacChannel[index].vB);
		Ini_PutDouble(vProfileFileHandle, section, "Ramp Rate", vProfile->dacChannel[index].ramprate);
		Ini_PutDouble(vProfileFileHandle, section, "On Duration", vProfile->dacChannel[index].onDuration);
		Ini_PutDouble(vProfileFileHandle, section, "Off Duration", vProfile->dacChannel[index].offDuration);
		Ini_PutDouble(vProfileFileHandle, section, "Total On Duration", vProfile->dacChannel[index].totalDuration);
		Ini_PutInt   (vProfileFileHandle, section, "Constant On", vProfile->dacChannel[index].continous);
		
	}
	
	Ini_WriteToFile(vProfileFileHandle, filename);
	Ini_Dispose(vProfileFileHandle);
	
	return 0;
}

/**
Load voltage definition from file
*/
int LoadVoltageDataFile(char *filename, VoltageHardware *vProfile)   
{
	int index = 0;
	char section[256] = {0};
	char message[256] = {0};
	IniText vProfileFileHandle;
	
	//allocate handle for file
	vProfileFileHandle = Ini_New(0);
	if(vProfileFileHandle == 0)
	{
		sprintf(message, "Could not create voltage data file handle.");
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;
	}
	
	//Read voltage file
	if(Ini_ReadFromFile(vProfileFileHandle, filename) != 0)
	{
		sprintf(message, "Could not read voltage file.");
		AddErrorToLog(message, __FILE__, __LINE__);
		MessagePopup(ERROR, message);
		Ini_Dispose(vProfileFileHandle);
		return -1;
	}
	
	
	//get data from object into file
	Ini_GetStringIntoBuffer(vProfileFileHandle, "Voltage Profile", "Description", vProfile->description, 255);
	//Ini_GetStringIntoBuffer(vProfileFileHandle, "Voltage Profile", "Comments", vProfile->comments);
	
	//load data for each channel
	for(index = 0; index < NUMBER_DAC_CHANNELS; index++)
	{
		/*int id;
		int vProfile;
		double vStart;
		double vA;
		double vB;
		double ramprate;		//millivolts Per Second
		double onDuration;		//in seconds
		double offDuration;		//in seconds
		double totalDuration;	//in seconds
		int continous;			//ignore on/off duration*/
		
		sprintf(section, "DAC Channel %d", index);
		Ini_GetInt   (vProfileFileHandle, section, "ID", &vProfile->dacChannel[index].id);
		Ini_GetDouble(vProfileFileHandle, section, "vStart", &vProfile->dacChannel[index].vStart);
		Ini_GetDouble(vProfileFileHandle, section, "vA", &vProfile->dacChannel[index].vA);
		Ini_GetDouble(vProfileFileHandle, section, "vB", &vProfile->dacChannel[index].vB);
		Ini_GetDouble(vProfileFileHandle, section, "Ramp Rate", &vProfile->dacChannel[index].ramprate);
		Ini_GetDouble(vProfileFileHandle, section, "On Duration", &vProfile->dacChannel[index].onDuration);
		Ini_GetDouble(vProfileFileHandle, section, "Off Duration", &vProfile->dacChannel[index].offDuration);
		Ini_GetDouble(vProfileFileHandle, section, "Total On Duration", &vProfile->dacChannel[index].totalDuration);
		Ini_GetInt   (vProfileFileHandle, section, "Constant On", &vProfile->dacChannel[index].continous);
		
	}
	
	Ini_Dispose(vProfileFileHandle);
	
	return 0;
}
