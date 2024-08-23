// main.c
//
// Main function to start app

#include "app.h"

#define VERSION "082424S"

/*
  Revision History ... Should have started this long ago :(
  
  071304  -- Started
  	Bug 1 - Block read crashes program - FIXED
  	Bug 2 - Deleting of block causes problems - FIXED
  	Bug 3 - Pulsed blocks ran under batch do not save - FIXED
  	
  	Enhancement 1 - Delete individual timing parameters from block def
	Enhancement 2 - Keithley window must stay open to keep it initialized
  
  082104
    Enhancement 1 - added ability to step through electrodes in blocks sequentially
    				during batch mode.
  082404
  	Bug 4 - Fixed minor issue where program would not iterate electrodes in a block

  082124  -- From Loren
	Updated to Windows 11 and to PCIe-9101 card

  082424  -- From Loren
	Keithley 6485 finally works with Windows 11 port of software
	

*/
static Properties gDefaults;

int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
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

