//debuguir.h
#ifndef DEBUGUIR_H_
	#define DEBUGUIR_H_
	//includes
	#include "chipcontrol.h"
	
	//prototypes
	int LoadDebugPanel(void);
	int DisplayDebugPanel(void);
	int AddToDebugOutput(char *message);
	int WriteMapToDebugWindow(int channel, int *map);
#endif
