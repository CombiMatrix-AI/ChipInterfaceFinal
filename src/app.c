// main.c
//
// Main function to start app

#include "app.h"

#define VERSION "082424S"
static Properties gDefaults;

int main (HINSTANCE hInstance, HINSTANCE hPrevInstance,
					   LPSTR lpszCmdLine, int nCmdShow)
{
	if (InitCVIRTE (hInstance, 0, 0) == 0)
		return -1;    /* out of memory */
	
	LoadDebugPanel();
	
	StartLog();
	LoadProperties(NULL, &gDefaults);
	SetupChipControlCard(&gDefaults);

	InitializePatternList();
	LoadPatternDefinitionPanel(VERSION, &gDefaults);
	SetElectrodePositionTimer(gDefaults.electrodePositionTimer);
	
	SetSystemPopupsAttribute (ATTR_POPUP_STYLE, VAL_CLASSIC);	
	
	//start GUI event loop
	RunUserInterface();
	
	//shutdown program
	SaveProperties(NULL, &gDefaults);
	ShutdownChipControlCard();
	CloseLog();
	return 0;
}

