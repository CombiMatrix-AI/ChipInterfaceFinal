//properties.h
#ifndef PROPERTIES_H_
	#define PROPERTIES_H_
	
	//definitions
	#define MAX_STRING_LENGTH 512
	
	typedef struct Properties_Tag
	{
		char filename[MAX_STRING_LENGTH];
		int emulateChipHardware;
		int floatVline;
		double electrodePositionTimer;
		int chipHardwareDelay;
	}Properties;
	
	#define FILENAME_SECTION 		"Properties Filename"
	#define FILENAME_TAG			"Filename"

	#define PROPERTIES_SECTION		"Properties"
	#define EMULATE_CHIP_HARDWARE	"Emulate chip hardware"
	#define VLINE_FLOAT_TAG			"Float Vline"
	#define ELECTRODE_TIMER_TAG		"Electrode map position timer"
	#define CHIPHARDWARE_DELAY_TAG  "Chip Hardware Delay Counts"
	
	//includes
	#include "log.h"
	#include "inifile.h"

	#include <utility.h>
	
	//prototypes
	int LoadProperties(char *propertiesFilename, Properties *defaults);
	int SetProperties(IniText propertiesFileHandle, Properties *defaults);
	int SaveProperties(char *propertiesFilename, Properties *defaults);
	
#endif
