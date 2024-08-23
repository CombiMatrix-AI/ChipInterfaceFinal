// log.c
//
// perform logging functions for debugging purposes.

#include "log.h"

// global file pointer for log file
FILE *logfile = NULL;

/**
Start logging (open file for log)

returns 0 on success
*/
int StartLog(void)  
{
	char logFilename[] = "log.txt";
	
	// start logging of events as they happen
	if(logfile != NULL)
	{
		// log is already open
		AddToLog("Tried to start log when it was already started!");
		return -1;
	}
	
	logfile = fopen(logFilename, "a+");
	if(log == NULL)
	{
		MessagePopup(ERRORSTR, "Could not create temporary file for log");
		return -1;
	}
	
	AddToLog("\n********\nStarted Log.");
	
	return 0;
}

/**
Close logging (close file for log)

returns 0 on success
*/
int CloseLog(void)
{
	// close logging
	AddToLog("Logging has terminated.");
	fclose(logfile);
	return 0;
}

/**
Add ERRORSTR message to log
message 	- message to be added to log
file		- file error occured in
line		- line number close to error

returns 0 on success
*/
int AddErrorToLog(char *message, char *file, int line)
{
	if(logfile == NULL)
	{
		return -1;
	}
	fprintf(logfile, "%s %s %s (trace: file %s @line %d)\n", DateStr(), TimeStr(), message, file, line);
	fflush(logfile);
	return 0;
}

/**
Add message to log
message 	- message to be added to log

returns 0 on success
*/
int AddToLog(char *message) // returns 0 for success
{
	if(logfile == NULL)
	{
		return -1;
	}
	fprintf(logfile, "%s %s %s\n", DateStr(), TimeStr(), message);
	fflush(logfile);
	return 0;
}
