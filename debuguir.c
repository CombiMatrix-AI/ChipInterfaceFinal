#include <userint.h>
#include "debug.h"

//debuguir.c
#include "debuguir.h"

//global
int debugPanel = 0;

int LoadDebugPanel(void)
{
	debugPanel = LoadPanel(0, "debug.uir", DEBUG);
	return 0;	
}

int DisplayDebugPanel(void)
{
	if(debugPanel != 0)
	{
		DisplayPanel(debugPanel);
	}
	return 0;
}

int AddToDebugOutput(char *message)
{
	InsertTextBoxLine(debugPanel, DEBUG_OUTPUT, -1, message);
	return 0;
}

//still need to create a mirror of this
int WriteMapToDebugWindow(int channel, int *map)
{
	char *buffer = NULL;
	int row = 0;
	int column = 0;
	int index = 0;
	buffer = calloc (sizeof(char), MAXIMUM_ELECTRODES + MAXIMUM_ROWS + 1);
	
	for(column = 0; column < MAXIMUM_COLUMNS; column++)
	{
		for(row = 0; row < MAXIMUM_ROWS; row++) 
		{
			buffer[index] = map[MAXIMUM_COLUMNS * row + column] | 0x30;
			index++;
		}
		buffer[index] = '\n';
		index++;
	}
	
	AddToDebugOutput(buffer);
	free(buffer);
	return 0;
}

int CVICALLBACK DebugPanelCB (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_CLOSE:
			HidePanel(panel);
			break;
		}
	return 0;
}

int CVICALLBACK ClearDebugOutputCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			ResetTextBox(panel, DEBUG_OUTPUT, "");
			break;
		}
	return 0;
}

