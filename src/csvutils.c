// csvutils.c
//
// Load, save, parse .CSV files

#include "csvutils.h"

/**
Loads a comma dilimited file and puts it into an array to be easier to use

filename 	- CSV file to open
map      	- pointer to an integer array that will contain the map from the CSV file
maxRows  	- number rows in CSV file
maxColumns 	- number columns in CSV file

returns 0 on success
*/
int LoadCSVFile (char *filename, int *map, int maxRows, int maxColumns) 
{
	// Load our CSV file and put it into map
	FILE *fp = NULL;
	char message[512] = {0};
	char line[MAXIMUM_LINE_LENGTH] = {0};
	char *temp = NULL;
	
	int index = 0;
	int column = 0;
	int row = 0;
	
	// do some error checking first
	if(maxColumns >= MAXIMUM_LINE_LENGTH-1)
	{
		sprintf(message, "File: %s has too many columns (Max is: %d).", filename, MAXIMUM_LINE_LENGTH);
		AddErrorToLog(message, __FILE__, __LINE__);
		MessagePopup(ERROR, message);
		return -1;
	}	
	
	// make sure we can open the file
	fp = fopen(filename, "r");
	if(fp == NULL)
	{
		sprintf(message, "Could not open CSV file: %s", filename);
		AddErrorToLog(message, __FILE__, __LINE__);
		MessagePopup(ERROR, message);
		return -1;
	}
	else
	{
		sprintf(message, "Successfully loaded CSV file: %s", filename);
		AddToLog(message);
	}
	
	// Read file one line at a time, and then tokenize the input
	for(row = 0; row < maxRows; row++)
	{
		// get the line
		fgets(line, MAXIMUM_LINE_LENGTH-1, fp);
		if(line != NULL)
		{
			// tokenize it, do some error checking
			temp = strtok(line, ",");
			if(temp == NULL)
			{
				sprintf(message, "Unexpected 1st try error while parsing CSV file: %s (index: %d, row: %d)", filename, index, row);
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup(ERROR, message);
				return -1;
			}
			map[index] = atoi(temp);
			index++;
			
			// read the rest of the tokens to the end of the column
			for(column = 1; column < maxColumns; column++, index++)
			{
				temp = strtok(NULL, ",\n\r");
				if(temp == NULL)
				{
					sprintf(message, "Unexpected loop error while parsing CSV file: %s (index: %d, row: %d)", filename, index, row);
					AddErrorToLog(message, __FILE__, __LINE__);
					MessagePopup(ERROR, message);
					return -1;
				}
				map[index] = atoi(temp);	
			}
		}	
	}
	
	// finished
	sprintf(message, "Successfully parsed CSV file: %s", filename);
	AddToLog(message);
	
	fclose(fp);
	return 0;
}

/**
Displays a map that has been converted from a CSV file for debugging
map      	- pointer to an integer array that contains the map from the CSV file
maxRows  	- number rows in the map
maxColumns 	- number columns in the map

returns 0 on success
*/
int DisplayCSVMap(int *map, int numberRows, int numberColumns)
{
	char *charMap = NULL;
	char message[128] = {0};
	int index = 0;
	int row = 0;
	int column = 0;
	
	// allocate memory for "char" representation of map
	charMap = calloc(sizeof(char), 2000);
	if(charMap == NULL)
	{
		sprintf(message, "Could not allocate memory for charMap.  Aborting test.");
		AddToLog(message);
		MessagePopup(ERROR, message);
		return -1;	
	}
	
	charMap[index] = '\n';
	
	// populate char array with integer values, converting to ascii
	for(row = 0, index = 1; row < numberRows; row++) 
	{
		for(column = 0; column < numberColumns; column++)
		{
			// convert integer to printable ascii character
			charMap[index] = (char)(map[(row * numberColumns) + column] | 0x30);
			index++;
		}
		strcat(charMap, "\n");
		index++;
	}
	
	// finished
	AddToLog("CSV File Display:");
	AddToLog(charMap);
	
	
	free(charMap);
	return 0;
}

/**
Test the CSV functions
numberRows 		- number rows CSV file should have
numberColumns 	- number columns CSV file should have

returns 0 on success
*/
int TestCSVFunctions(int numberRows, int numberColumns) // returns 0 on success
{
	char projectDirectory[384] = {0};
	char filename[512] = {0};
	char message[128] = {0};
	int status = 0;
	int *map = NULL;
	
	sprintf(message, "Started CSV Functions Test.");
	AddToLog(message);
	
	// Get a file to test
	GetProjectDir (projectDirectory);
	status = FileSelectPopup (projectDirectory, "*.csv", "*.csv", CSV_OPEN_FILE_CAPTION, VAL_SELECT_BUTTON, 0, 0, 1, 1, filename);
	
	if(status == VAL_NO_FILE_SELECTED) 
	{
		sprintf(message, "No CSV file was selected.  Aborting test.");
		AddToLog(message);
		MessagePopup(ERROR, message);
		return -1;
	}
	
	// Allocate memory for converted map
	map = calloc(sizeof(int), (numberRows*numberColumns));
	if(map == NULL)
	{
		sprintf(message, "Could not allocate memory for map.  Aborting test.");
		AddToLog(message);
		MessagePopup(ERROR, message);
		return -1;	
	}
	
	// Load CSV file and perform conversion
	status = LoadCSVFile(filename, &(*map), numberRows, numberColumns);
	
	if(status != 0) 
	{
		sprintf(message, "LoadCSVFile test failed.");
		AddToLog(message);
		MessagePopup(ERROR, message);
		free(map);
		return -1;
	}
	
	// Display converted map in log for debugging
	status = DisplayCSVMap(map, numberRows, numberColumns);
	if(status != 0) 
	{
		sprintf(message, "DisplayCSVMap test failed.");
		AddToLog(message);
		MessagePopup(ERROR, message);
		free(map);
		return -1;
	}
	
	// Done with tests
	sprintf(message, "Finished CSV Functions Test.");
	AddToLog(message);
	
	free(map);
	return 0;
}
