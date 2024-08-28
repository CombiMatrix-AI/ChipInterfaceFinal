// ChipControl.c
//
// Handle all low level functions of the chip and interface with
// ADLink PCI-9112 (PCIDAQ1410)

#include "chipcontrol.h"

// global variable holding card id value.
int cardId = -1;
int emulateChipHardware = 0;
int chipHardwareDelay = 0;
int emulatedChipMap[1024] = {0};
/**
Emulate hardware if it is not present

emulationState - 1 or 0
*/
void SetChipHardwareEmulation(int emulationState)
{
	char message[128] = {0};
	
	emulateChipHardware = emulationState;
	sprintf(message, "Chip hardware emulation state: %d", emulateChipHardware);
	AddToLog(message);
}

/**
Register card with system, make sure it exists and initializes properly

returns 0 on success
*/
int SetupChipControlCard(Properties *defaults)
{
	int status = 0;
	char message[128] = {0};
	
	SetChipHardwareEmulation(defaults->emulateChipHardware); 
	SetChipHardwareDelay(defaults->chipHardwareDelay);
	
	if(defaults->emulateChipHardware == 1)
	{
		sprintf(message, "All chip functions will be emulated");
		AddToLog(message);
		return 0;
	}
	// Register card with system.  Only one can be registered at a time
	// (this includes other programs)
	cardId = Register_Card (PCIe_9101, CHIP_CARD_NUMBER);
	if(cardId < 0)
	{
		sprintf(message, "Could not register PCIe_9101: Error %d", cardId);
		MessagePopup(ERROR, message);	
		AddErrorToLog(message, __FILE__, __LINE__);
	}
	else
	{
		sprintf(message, "Successfully registered PCIe_9101.");
		AddToLog(message);
	}
	
	//status = AO_9112_Config (CHIP_CARD_NUMBER, 0, -5.0);
	AO_relay_EN(CHIP_CARD_NUMBER,1);
	//status = AO_Config (CHIP_CARD_NUMBER, P91xx_AO_TIMEBASE_INT, P91xx_AO_TRGMOD_POST | P91xx_AO_TRGSRC_SOFT, 1, TRUE);
	if(status < 0)
	{
		sprintf(message, "Could not setup PCIe_9101 DAC: Error %d", status);
		MessagePopup(ERROR, message);
		AddErrorToLog(message, __FILE__, __LINE__);
	}
	
	//Comment this out i guess because now one line does both channels?
	
	//status = AO_9112_Config (CHIP_CARD_NUMBER, 1, -5.0);
	//if(status < 0)
	//{
	//  	sprintf(message, "Could not setup PCIe_9101 DAC Channel 1: Error %d", status);
	//  	MessagePopup(ERROR, message);
	//  	AddErrorToLog(message, __FILE__, __LINE__);
	//}
	
	
	return cardId;
}

int SetDacVoltage(int channel, double voltage)
{
	int status = 0;
	char message[128] = {0};
	
	if(emulateChipHardware == 1)
	{
		sprintf(message, "(emulated) set voltage to % 1.3f on PCIe_9101 DAC Channel %d", voltage, channel);
		AddToDebugOutput(message);
		return 0;
	}
	
	status = AO_VWriteChannel (CHIP_CARD_NUMBER, channel, voltage);
	if(status < 0)
	{
		sprintf(message, "Could not set voltage to % 1.3f on PCIe_9101 DAC Channel %d: Error %d", voltage, status, channel);
		MessagePopup(ERROR, message);
		AddErrorToLog(message, __FILE__, __LINE__);
	}
	return 0;
}

/**
Frees resources associated with registering card

returns 0 on success
*/
int ShutdownChipControlCard(void) 
{
	int status = 0;
	char message[128] = {0};
	
	// release resources associated with registering card
	status = Release_Card  (CHIP_CARD_NUMBER);
	if(status < 0)
	{
		sprintf(message, "Could not unregister PCIe_9101: Error %d", cardId);
		MessagePopup(ERROR, message);
		AddErrorToLog(message, __FILE__, __LINE__);
	}
	else
	{
		sprintf(message, "Successfully unregistered PCIe_9101.");
		AddToLog(message);
	}
	return status;	  
}

/**
Sets the entire chip state
channel		- channel to be addressed (0 indexed)
map			- data to be written

returns 0 on success
*/
int SetChipMap(int channel, int *map)
{
	int row = 0;
	int column = 0;
	int status = 0;
	int address = 0x0;
	int value = 0;
	int wr = 0;
	int dataToWrite = 0x0000;
	char message[128] = {0};
	/** 
		address space definition
			bits	function
			0-5 	row address
			6-9 	column address
			10-11   value to be set (0-3)
			12		write strobe
			13 		channel
			14-15	data read		(ignored on write)
	*/
	
	//setup signals
	channel <<=13;
	
	sprintf(message, "%s Setting chip map", TimeStr()); 
	AddToDebugOutput(message);
	
	//for(row = 0; row < MAXIMUM_ROWS; row++) 
	for(column = 0; column < MAXIMUM_COLUMNS; column++)
	{	
		//for(column = 0; column < MAXIMUM_COLUMNS; column++)
		for(row = 0; row < MAXIMUM_ROWS; row++)
		{
			value = map[MAXIMUM_COLUMNS * row + column];
			value <<= 10;
			
			address = column;
			address <<= 6;
			address |= row;
			
			wr = 0x0;
			wr <<= 12;
			dataToWrite = address | value | wr | channel;
			if(emulateChipHardware == 0)
			{
				status = DO_WritePort (cardId, 0, dataToWrite);
				if(chipHardwareDelay > 0)
				{
					Wait(chipHardwareDelay);
				}	
			}
			else
			{
				address = (row*MAXIMUM_COLUMNS) + column;
				emulatedChipMap[address] = map[address];					
			}
			
			wr = 0x1;
			wr <<= 12;
			dataToWrite = address | value | wr | channel;
			
			if(emulateChipHardware == 0)
			{
				status = DO_WritePort (cardId, 0, dataToWrite);
				if(chipHardwareDelay > 0)
				{
					Wait(chipHardwareDelay);
				}	
			}
			
			dataToWrite = 0;
			address = 0;
		}
	}
	return status;
}

/**
Reads the entire chip state
channel		- channel to be addressed (0 indexed)
map			- array that will hold map read

returns 0 on success
*/
int GetChipMap(int channel, int *map)
{
	int status = 0;
	int column = 0;
	int row = 0;
	int address = 0x0;
	int value = 0;
	int wr = 0x1;
	int dataToWrite = 0x0000;
	int dataRead = 0x0;
	char message[128] = {0};
	
	/** 
		address space definition
			bits	function
			0-5 	row address
			6-9 	column address
			10-11   value to be set (0-3)  (ignored on read)
			12		write strobe
			13 		channel
			14-15	data read
	*/
	
	channel <<= 13;
	value <<= 10;
	wr <<= 12;
	
	sprintf(message, "%s Reading chip map", TimeStr()); 
	AddToDebugOutput(message);
	
	//for(row = 0; row < MAXIMUM_ROWS; row++) 
	for(column = 0; column < MAXIMUM_COLUMNS; column++)
	{
		//for(column = 0; column < MAXIMUM_COLUMNS; column++)
		for(row = 0; row < MAXIMUM_ROWS; row++)
		{
			address = column;
			address <<= 6;
			address += row;
			
			dataToWrite = address | value | wr | channel;
	
			if(emulateChipHardware == 0)
			{
				status = DO_WritePort (cardId, 0, dataToWrite);
				if(chipHardwareDelay > 0)
				{
					Wait(chipHardwareDelay);
				}	
				status = DI_ReadPort(cardId, 0, &dataRead);
				if(chipHardwareDelay > 0)
				{
					Wait(chipHardwareDelay);
				}	
			}
			else
			{
				address = (row*MAXIMUM_COLUMNS) + column;
				dataRead = emulatedChipMap[address] << 14;					
			}
			dataRead >>= 14;
			map[MAXIMUM_COLUMNS * row + column] = dataRead;
		}
	}
	
	return status;
}

/**
Sets single electrode chip state
returns 0 on success
*/
int SetChipState(int channel, int row, int column, int value)
{
	int status = 0;
	int address = 0x0;
	int wr = 0;
	int dataToWrite = 0x0000;
	char message[128] = {0};
	/** 
		address space definition
			bits	function
			0-5 	row address
			6-9 	column address
			10-11   value to be set (0-3)
			12		write strobe
			13 		channel
			14-15	data read		(ignored on write)
	*/
	
	//setup signals
	channel <<=13;
	
	sprintf(message, "%s Setting chip state row %d column %d vline %d", TimeStr(), row, column, value); 
	AddToDebugOutput(message);
	
	value <<= 10;
	
	address = column;
	address <<= 6;
	address |= row;
	
	wr = 0x0;
	wr <<= 12;
	dataToWrite = address | value | wr | channel;
	if(emulateChipHardware == 0)
	{
		status = DO_WritePort (cardId, 0, dataToWrite);
		if(chipHardwareDelay > 0)
		{
			Wait(chipHardwareDelay);
		}	
	}
	
	wr = 0x1;
	wr <<= 12;
	dataToWrite = address | value | wr | channel;
	
	if(emulateChipHardware == 0)
	{
		status = DO_WritePort (cardId, 0, dataToWrite);
		if(chipHardwareDelay > 0)
		{
			Wait(chipHardwareDelay);
		}	
	}
	
	dataToWrite = 0;
	address = 0;
	
	return status;
}

/**
Reads single electrode chip state
returns 0 on success
*/
int GetChipState(int channel, int row, int column, int *value)
{
	int status = 0;
	int address = 0x0;
	int wr = 0x1;
	int dataToWrite = 0x0000;
	int dataRead = 0x0;
	char message[128] = {0};
	
	/** 
		address space definition
			bits	function
			0-5 	row address
			6-9 	column address
			10-11   value to be set (0-3)  (ignored on read)
			12		write strobe
			13 		channel
			14-15	data read
	*/
	
	channel <<= 13;
	wr <<= 12;
	
	sprintf(message, "%s Reading chip state row %d column %d vline %d", TimeStr(), row, column, value); 
	AddToDebugOutput(message);
	
	address = column;
	address <<= 6;
	address += row;
	
	dataToWrite = address | wr | channel;

	if(emulateChipHardware == 0)
	{
		status = DO_WritePort (cardId, 0, dataToWrite);
		if(chipHardwareDelay > 0)
		{
			Wait(chipHardwareDelay);
		}	
		status = DI_ReadPort(cardId, 0, &dataRead);
		if(chipHardwareDelay > 0)
		{
			Wait(chipHardwareDelay);
		}	
	}
	dataRead >>= 14;
 	*value = dataRead;
	
	return status;
}

void SetChipHardwareDelay(int newDelay)
{
	char message[128] = {0};
	
	chipHardwareDelay = newDelay;
	sprintf(message, "Chip hardware delay counts: %d", chipHardwareDelay);
	AddToLog(message);
}

int GetChipHardwareDelay(void)
{
	return chipHardwareDelay;
}

int Wait(int counts)
{
	//int threadFunctionID = 0;
	
	//CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, (ThreadFunctionPtr) WaitThread, (void*)counts, 
	//	&threadFunctionID);
	//CmtWaitForThreadPoolFunctionCompletion (DEFAULT_THREAD_POOL_HANDLE, threadFunctionID, OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
	//CmtReleaseThreadPoolFunctionID (DEFAULT_THREAD_POOL_HANDLE, threadFunctionID);
	//return 1;	
	
	//Delay((double)counts / 5000.0);
	
	for (register int i = 0; i<10000*counts; i++);
	
	return 1;
}

int CVICALLBACK WaitThread (void *functionData)
{
	int counts = (int) functionData;
	int index = 0;
	
	for(index = 0; index < counts; index++);
	
	return 1;
}
