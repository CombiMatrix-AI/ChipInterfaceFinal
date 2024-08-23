//properties.c
#include "properties.h"

//global modules

static IniText gPropertiesFileHandle = 0;
static Properties gProperties;

int LoadProperties(char *propertiesFilename, Properties *defaults)
{
	char filename[MAX_STRING_LENGTH] = {0};
	char message[128] = {0};
	
	//see if we are going to use default filename or supply our own
	if(propertiesFilename == NULL)
	{
		GetProjectDir(filename);
		strcat(filename, "\\properties\\properties.ini");
	}
	else
	{
		strcpy(filename, propertiesFilename);
	}
	
	//create object that will hold properties in memory
	gPropertiesFileHandle = Ini_New (0);
	if(gPropertiesFileHandle == 0)
	{
		sprintf(message, "Could not create handle to read properties file.");
		AddErrorToLog(message, __FILE__, __LINE__);
		MessagePopup(ERROR, message);
		return -1;	
	}
	
	//Read properties file
	if(Ini_ReadFromFile(gPropertiesFileHandle, filename) != 0)
	{
		sprintf(message, "Could not read properties file.");
		AddErrorToLog(message, __FILE__, __LINE__);
		MessagePopup(ERROR, message);
		Ini_Dispose(gPropertiesFileHandle);
		return -1;
	}
	
	//apply the properties
	if(SetProperties(gPropertiesFileHandle, defaults) != 0)
	{
		sprintf(message, "Could not set properties.");
		AddErrorToLog(message, __FILE__, __LINE__);
		MessagePopup(ERROR, message);
		Ini_Dispose(gPropertiesFileHandle);
		return -1;
	}
	
	Ini_Dispose(gPropertiesFileHandle);
	return 0;	   
}

int SetProperties(IniText propertiesFileHandle, Properties *defaults)
{
	Ini_GetInt(gPropertiesFileHandle, PROPERTIES_SECTION, EMULATE_CHIP_HARDWARE, &defaults->emulateChipHardware);
	Ini_GetDouble(gPropertiesFileHandle, PROPERTIES_SECTION, ELECTRODE_TIMER_TAG, &defaults->electrodePositionTimer);
	Ini_GetInt(gPropertiesFileHandle, PROPERTIES_SECTION, CHIPHARDWARE_DELAY_TAG, &defaults->chipHardwareDelay);
	
	if(defaults->electrodePositionTimer == 0.0)
	{
		defaults->electrodePositionTimer = 0.1;
	}
	//Ini_GetInt(gPropertiesFileHandle, PROPERTIES_SECTION, VLINE_FLOAT_TAG, &defaults->floatVline);
	
	
	return 0;
}

int SaveProperties(char *propertiesFilename, Properties *defaults)
{
	int status = 0;
	char filename[MAX_STRING_LENGTH] = {0};
	char message[128] = {0};

	// check to see if we are renaming this set of properties
	if(propertiesFilename == NULL)
	{
		GetProjectDir(filename);
		strcat(filename, "\\properties");
		
		//see if the properties directory exists, if not, make it
		GetFileWritability(filename, &status);
		if(status == 1)
		{
			MakeDir(filename);
		}
		strcat(filename, "\\properties.ini");
	}
	else
	{
		strcpy(filename, propertiesFilename);
	}
	
	//create object that will hold properties in memory
	gPropertiesFileHandle = Ini_New (0);
	if(gPropertiesFileHandle == 0)
	{
		sprintf(message, "Could not create handle to read properties file.");
		AddErrorToLog(message, __FILE__, __LINE__);
		MessagePopup(ERROR, message);
		return -1;	
	}
	
	defaults->chipHardwareDelay = GetChipHardwareDelay();
	
	
	Ini_PutString(gPropertiesFileHandle, FILENAME_SECTION, FILENAME_TAG, filename);
	Ini_PutInt(gPropertiesFileHandle, PROPERTIES_SECTION, EMULATE_CHIP_HARDWARE, defaults->emulateChipHardware);
	Ini_PutDouble(gPropertiesFileHandle, PROPERTIES_SECTION, ELECTRODE_TIMER_TAG, defaults->electrodePositionTimer);
	Ini_PutInt(gPropertiesFileHandle, PROPERTIES_SECTION, CHIPHARDWARE_DELAY_TAG, defaults->chipHardwareDelay);
	//Ini_PutInt(gPropertiesFileHandle, PROPERTIES_SECTION, VLINE_FLOAT_TAG, defaults->floatVline);
	
	status = Ini_WriteToFile(gPropertiesFileHandle, filename);
	
	Ini_Dispose(gPropertiesFileHandle);
	return 0;
}
