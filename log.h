// log.h

#ifndef LOG_H_
	#define LOG_H_
	
	// includes
	#include "debuguir.h"
	#include <ansi_c.h>
	#include <userint.h>


	// prototypes
	int StartLog(void);
	int CloseLog(void);
	int AddErrorToLog(char *message, char *file, int line);
	int AddToLog(char *message);
	
	// definitions
	#define ERRORSTR "ERROR"
	
#endif
