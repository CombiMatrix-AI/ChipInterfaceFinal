//patterndef.c
#include "patterndef.h"

//module globals
static ListType gPatternList = {0};

/**
Initialize list
*/
int InitializePatternList(void)
{
	int status = 0;
	char message[128] = {0};
	
	if(gPatternList != 0)
	{
		status = DeletePatternList(gPatternList);
		if(status != 0)
		{
			sprintf(message, "Could not initialize pattern list (error %d).", status);
			AddErrorToLog(message, __FILE__, __LINE__);
			return -1;
		}
	}	
	gPatternList = ListCreate(sizeof(PatternDefinition));
	
	return 0;
}

/**
Initialize pattern

newPattern - object to be initialized
index - block id

returns 0 on success
*/
int InitializePattern(PatternDefinition *newPattern, int index)
{
	int i = 0;
	char message[MAX_MESSAGE_SIZE] = {0};
	
	if(newPattern == NULL || index <= 0)
	{
		sprintf(message, "Could not initialize pattern object.");
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;
	}
	
	strcpy(newPattern->description, "");
	strcpy(newPattern->filename, "");
	strcpy(newPattern->comments, "");
	strcpy(newPattern->voltageFilename, "");
	
	newPattern->patternId = index;
	newPattern->numberRows = 1;
	newPattern->numberColumns = 1;
	newPattern->startRow = 1;
	newPattern->startColumn = 1;
	newPattern->tileRow = 1;
	newPattern->tileColumn = 1;
	
	for(i = 0; i < MAXIMUM_ELECTRODES; i++)
	{
		newPattern->map[i] = FLOATVLINE;
	}
	
	return 0;
}

/**
Insert pattern into list

newPattern - object to be added to list
index - object id, placement in list

returns 0 on success
*/
int InsertPattern(PatternDefinition newPattern, int index)
{
	int numberItems = 0;
	
	numberItems = ListNumItems(gPatternList);
	
	//if there is already an entry at this index, it is to be replaced with new object
	if(numberItems > 0 && numberItems >= index)
	{
		ListReplaceItem(gPatternList, &newPattern, index);
	}
	else
	{
		ListInsertItem(gPatternList, &newPattern, index);
	}
	return 0;
}

/**
Delete pattern from list

index - object id, placement in list

returns 0 on success
*/
int DeletePattern(int index)
{
	int numberItems = 0;
	int patternIndex = 0;
	char message[MAX_MESSAGE_SIZE] = {0};
	PatternDefinition nextPattern;
	
	numberItems = ListNumItems(gPatternList);
	if(numberItems == 0)
	{
		sprintf(message, "Could not delete anything from list, it is already empty.");
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;
	}
	
	ListRemoveItem(gPatternList, 0, index);
	
	// need to update pattern Id
	if(index-1 <= 0)  // we are at the first pattern
	{
		GetPatternData(&nextPattern, 1);
		nextPattern.patternId = 1;
		InsertPattern(nextPattern, 1); // put the updated pattern back in list
	}
	else
	{
		for(patternIndex = index; patternIndex <= numberItems; patternIndex++)
		{
			GetPatternData(&nextPattern, patternIndex);
			nextPattern.patternId = patternIndex;
			InsertPattern(nextPattern, nextPattern.patternId);
		}
	}
	return 0;
}

/**
Delete entire pattern list
*/
int DeletePatternList(ListType listToDelete)
{
	int numberItems = 0;
	
	numberItems = ListNumItems(gPatternList);
	ListRemoveItems (gPatternList, 0, 1, numberItems);

	return 0;
}


/**
Return number of patterns in list
*/
int GetNumberPatterns(void)
{
	return ListNumItems(gPatternList);
}

/**
Return copy of object in list
*/
int GetPatternData(PatternDefinition *newPattern, int index)
{
	int numberItems = 0;
	char message[MAX_MESSAGE_SIZE] = {0};
	
	numberItems = ListNumItems(gPatternList);
	if(numberItems < index)
	{
		sprintf(message, "Could not get pattern data from list, it is empty.");
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;
	}
	
	ListGetItem (gPatternList, newPattern, index);
	
	if(newPattern == NULL)
	{
		sprintf(message, "Could not get pattern data from list, list is corrupt.");
		AddErrorToLog(message, __FILE__, __LINE__);
		return -2;
	}
	
	//this will fail when a block has been deleted and pattern id not updated
	/*if(newPattern->patternId != index)
	{
		sprintf(message, "Could not get pattern data from list, index invalid.\n(passed %d, got %d)",
			index, newPattern->patternId);
		MessagePopup("ERROR", message);
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;
	}*/
	
	return 0;
}

/**
make solid pattern
*/

int MakeSolidPattern(int *map, int numberRows, int numberColumns, int vline)
{
	int index = 0;
	
	if(map == NULL)
	{
		return -1;
	}
	
	for(index = 0; index < (numberRows*numberColumns); index++)
	{
		map[index] = vline;
	}
	return 0;
}

/**
make random pattern
*/

int MakeRandomPattern(int *map, int numberRows, int numberColumns)
{
	int index = 0;
	
	if(map == NULL)
	{
		return -1;
	}
	
	for(index = 0; index < (numberRows*numberColumns); index++)
	{
		map[index] = Random(0, 4);	// 4 is not included
	}
	return 0;
}

/**
Make checkerboard pattern from defined parameters
*/
int MakeCheckerboard(int *newBlock, int numberRows, int numberColumns, int inverse, int active)
{
	int row = 0;
	int column = 0;
	int index = 0;
	int on = 0;
	char message[MAX_MESSAGE_SIZE] = {0};
	
	if(newBlock == NULL)
	{
		sprintf(message, "Could not create pattern (block passed was not defined)");
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;
	}
	
	if(inverse == 1)
	{
		on = 1;
	}
	
	for(row = 0; row < numberRows; row++)
	{
		for(column = 0; column < numberColumns; column++)
		{
			if(on == 1)
			{
				newBlock[index] = active;
				on = 0;
			}
			else
			{
				on = 1;
			}
			index++;
		}
		if(newBlock[index-numberColumns] == active)
		{
			on = 0;
		}
		else
		{
			on = 1;
		}
	}	
	
	return 0;
}

/**
With a defined block, create the entire chip pattern
*/
int CreatePattern(int *newPattern, int *block, int startingRow, int startingColumn,
	int numberRows, int numberColumns, int floating)
{
	int index = 0;
	int row = 0;
	int column = 0;
	char message[MAX_MESSAGE_SIZE] = {0};
	
	//do some error checking
	if(newPattern == NULL)
	{
		sprintf(message, "Could not create pattern (pattern passed was not defined)");
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;
	}
	
	if(block == NULL)
	{
		sprintf(message, "Could not create pattern (block passed was not defined)");
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;
	}
	
	//first, initialize list with FLOATVLINE
	for(index = 0; index < MAXIMUM_ELECTRODES; index++)
	{
		newPattern[index] = FLOATVLINE;
	}	
	
	index = 0;
	
	//put block values into pattern
	for(row = startingRow; row < (startingRow+numberRows); row++)
	{
		for(column = startingColumn; column < (startingColumn+numberColumns); column++)
		{
			newPattern[MAXIMUM_COLUMNS * row + column] = block[index];
			index++;
		}
	}
	return 0;
}

/**
Compare maps
*/
int CompareMaps(int *resultsMap, int *wroteMap, int *readMap, int numberElectrodes)
{
	int index = 0;
	
	if(resultsMap == NULL)
	{
		return -1;
	}
	if(wroteMap == NULL)
	{
		return -1;
	}
	if(readMap == NULL)
	{
		return -1;
	}
	
	for(index = 0; index < numberElectrodes; index++)
	{
		if(wroteMap[index] == readMap[index] && resultsMap[index] != -1)
		{
			resultsMap[index] = readMap[index];
		}
		else
		{
			resultsMap[index] = -1;
		}
	}
	
	return 0;
}	

/**
Load pattern definition from file
*/
int LoadPatternData(char *filename, PatternDefinition *newPattern)
{
	int status = 0;
	int index = 0;
	int blockSize = 0;
	int dirHasChanged = 0;
	int *newBlock = NULL;
	char *data = NULL;
	char message[MAX_MESSAGE_SIZE] = {0};
	IniText patternFileHandle;
	
	//allocate handle for file
	patternFileHandle = Ini_New(0);
	if(patternFileHandle == 0)
	{
		sprintf(message, "Could not create pattern data file handle.");
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;
	}
	
	//Read block file
	if(Ini_ReadFromFile(patternFileHandle, filename) != 0)
	{
		sprintf(message, "Could not read block file.");
		AddErrorToLog(message, __FILE__, __LINE__);
		MessagePopup(ERROR, message);
		Ini_Dispose(patternFileHandle);
		return -1;
	}
	
	//Get data items from file
	Ini_GetStringIntoBuffer(patternFileHandle, "Block", "Description", newPattern->description, MAX_LABEL_SIZE);
	Ini_GetStringIntoBuffer(patternFileHandle, "Block", "Filename", newPattern->filename, MAX_FILENAME_SIZE);
	Ini_GetStringIntoBuffer(patternFileHandle, "Block", "Comments", newPattern->comments, MAX_COMMENTS_SIZE);
	
	//check to see if the file has been moved
	if(strcmp(newPattern->filename, filename) != 0)
	{
		dirHasChanged = 1;
	}
	
	if(strlen(newPattern->filename) <= 0)
	{
		strcpy(newPattern->filename, filename);
	}
	
	Ini_GetInt(patternFileHandle, "Block", "Number rows", &newPattern->numberRows);
	Ini_GetInt(patternFileHandle, "Block", "Number columns", &newPattern->numberColumns);
	Ini_GetInt(patternFileHandle, "Block", "Start row", &newPattern->startRow);
	Ini_GetInt(patternFileHandle, "Block", "Start column", &newPattern->startColumn);
	
	//get the map into a format we can save
	blockSize = newPattern->numberRows * newPattern->numberColumns;
	if(blockSize <= 0)
	{
		sprintf(message, "Could not create pattern (block size is invalid: %d)", blockSize);
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;
	}
	
	//allocate memory for buffers
	newBlock = calloc(sizeof(int), blockSize);
	if(newBlock == NULL)
	{
		sprintf(message, "Could not allocate memory for new block");
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;
	}
	
	data = calloc(sizeof(char), blockSize+1);
	if(data == NULL)
	{
		free(newBlock);
		sprintf(message, "Could not allocate memory for new pattern.");
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;
	}
	
	//read block data from file, put into allocated memory
	Ini_GetStringIntoBuffer(patternFileHandle, "Block", "Definition", data, blockSize+1);
	
	//convert ascii data to raw vline
	for(index = 0; index < blockSize; index++)
	{
		newBlock[index] = data[index] & 0x3;	
	}
	
	//with block read, put it into pattern buffer
	CreatePattern(newPattern->map, newBlock, newPattern->startRow-1, newPattern->startColumn-1,
		newPattern->numberRows, newPattern->numberColumns, FLOATVLINE);
	
	free(newBlock);
	free(data);
	data = NULL;
	
	//alert the user that the file has been moved and ask to resave it
	if(dirHasChanged == 1)
	{
		data = calloc(sizeof(char), 1024);
		sprintf(data, "The block with description '%s' has been moved\nWould you like to save it with this new filename?\n(%s->%s)",
			newPattern->description, newPattern->filename, filename);
		status = ConfirmPopup("WARNING!  Block file has been moved", data);
		if(status == 1)
		{
			//update it
			strcpy(newPattern->filename, filename);
			status = SavePatternData(filename, newPattern);
			AddToLog(data);
		}
		free(data);
	}
	Ini_Dispose(patternFileHandle);
	
	return 0;
}	

/**
Save pattern definition to file
*/
int SavePatternData(char *filename, PatternDefinition *newPattern)
{
	int index = 0;
	int blockSize = 0;
	int *newBlock = NULL;
	char *data = NULL;
	char message[MAX_MESSAGE_SIZE] = {0};
	IniText patternFileHandle;
	
	//allocate handle for file
	patternFileHandle = Ini_New(0);
	if(patternFileHandle == 0)
	{
		sprintf(message, "Could not create pattern data file handle.");
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;
	}
	
	//put date from object into file
	Ini_PutString(patternFileHandle, "Block", "Description", newPattern->description);
	Ini_PutString(patternFileHandle, "Block", "Filename", filename);
	Ini_PutString(patternFileHandle, "Block", "Comments", newPattern->comments);
	
	Ini_PutInt(patternFileHandle, "Block", "Number rows", newPattern->numberRows);
	Ini_PutInt(patternFileHandle, "Block", "Number columns", newPattern->numberColumns);
	Ini_PutInt(patternFileHandle, "Block", "Start row", newPattern->startRow);
	Ini_PutInt(patternFileHandle, "Block", "Start column", newPattern->startColumn);
	
	blockSize = newPattern->numberRows * newPattern->numberColumns;
	if(blockSize <= 0)
	{
		sprintf(message, "Could not create pattern (block size is invalid: %d)", blockSize);
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;
	}
	
	newBlock = calloc(sizeof(int), blockSize);
	if(newBlock == NULL)
	{
		sprintf(message, "Could not allocate memory for new block");
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;
	}
	
	data = calloc(sizeof(char), blockSize+1);
	if(data == NULL)
	{
		free(newBlock);
		sprintf(message, "Could not allocate memory for new pattern.");
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;
	}
	
	//get the map into a format we can save
	GetBlockFromMap(newPattern, &(*newBlock));
	
	//convert raw vline into ascii value
	for(index = 0; index < blockSize; index++)
	{
		data[index] = (char) newBlock[index] | 0x30;	
	}
	
	//save the block 
	Ini_PutString(patternFileHandle, "Block", "Definition", data);
	
	free(newBlock);
	free(data);
	
	Ini_WriteToFile(patternFileHandle, filename);
	Ini_Dispose(patternFileHandle);
	
	return 0;
}

/**
Save pattern list to file
file contains number of pattern definitions, and their filenames
*/
int SavePatternList(char *filename, int numberPatterns)
{
	int status = 0;
	int index = 0;
	char section[128] = {0};
	char message[MAX_MESSAGE_SIZE] = {0};
	
	PatternDefinition tempItem;
	IniText patternFileHandle;
	
	//allocate handle for file
	patternFileHandle = Ini_New(0);
	if(patternFileHandle == 0)
	{
		sprintf(message, "Could not create pattern list file handle.");
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;
	}
	
	if(numberPatterns == 0)
	{
		sprintf(message, "Pattern list is empty.");
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;
	}
	
	Ini_PutInt(patternFileHandle, "Block List Properties", "Number blocks", numberPatterns);
	
	for(index = 0; index < numberPatterns; index++)
	{
		sprintf(section, "Pattern_%d", index);
		status = GetPatternData(&tempItem, index+1);
		if(status != 0)
		{
			sprintf(message, "Could not get data from pattern list (index: %d).", index);
			AddErrorToLog(message, __FILE__, __LINE__);
			return -1;
		}
		Ini_PutString(patternFileHandle, section, "Pattern definition file", tempItem.filename);
		Ini_PutString(patternFileHandle, section, "Voltage definition file", tempItem.voltageFilename);
	}
	
	Ini_WriteToFile(patternFileHandle, filename);
	Ini_Dispose(patternFileHandle);
	
	return 0;
}

/**
Load block list from file.

When each filename is read from the list, the pattern definition file is the read, and inserted into
ths list

Important to note that the existing list will be cleared if there is one
*/
int LoadBlockList(char *filename)
{
	int status = 0;
	int index = 0;
	int numberPatterns = 0;
	char message[256] = {0};
	char section[128] = {0};
	
	PatternDefinition tempItem;
	
	IniText patternFileHandle;
	
	patternFileHandle = Ini_New(0);
	if(patternFileHandle == 0)
	{
		sprintf(message, "Could not create pattern list file handle.");
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;
	}
	
	//Read block file
	if(Ini_ReadFromFile(patternFileHandle, filename) != 0)
	{
		sprintf(message, "Could not read block list file.");
		AddErrorToLog(message, __FILE__, __LINE__);
		MessagePopup(ERROR, message);
		Ini_Dispose(patternFileHandle);
		return -1;
	}
	
	Ini_GetInt(patternFileHandle, "Block List Properties", "Number blocks", &numberPatterns);
	
	if(numberPatterns <= 0)
	{
		sprintf(message, "Pattern list is invalid (Number blocks: %d).", numberPatterns);
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;
	}
	
	//Load pattern definition files, put data into list
	for(index = 0; index < numberPatterns; index++)
	{
		sprintf(section, "Pattern_%d", index);
		
		Ini_GetStringIntoBuffer(patternFileHandle, section, "Pattern definition file", 
			tempItem.filename, MAX_FILENAME_SIZE);

		status = LoadPatternData(tempItem.filename, &tempItem);
		if(status != 0)
		{
			sprintf(message, "Could not load pattern data file.");
			AddErrorToLog(message, __FILE__, __LINE__);
			return -1;
		}
		
		
		Ini_GetStringIntoBuffer(patternFileHandle, section, "Voltage definition file", 
			tempItem.voltageFilename, MAX_FILENAME_SIZE);

		tempItem.patternId = index+1;
		
		status = InsertPattern(tempItem, tempItem.patternId);
		if(status != 0)
		{
			sprintf(message, "Could not insert pattern data into list.");
			AddErrorToLog(message, __FILE__, __LINE__);
			return -1;
		}
	}
	
	Ini_Dispose(patternFileHandle);
	
	return numberPatterns;
}

/**
Extract block from chip map
*/
int GetBlockFromMap(PatternDefinition *newPattern, int *newBlock)
{
	int index = 0;
	int row = 0;
	int column = 0;
	
	for(row = newPattern->startRow-1; row < (newPattern->startRow + newPattern->numberRows)-1; row++)
	{
		for(column = newPattern->startColumn-1; column < (newPattern->startColumn + newPattern->numberColumns)-1; column++)
		{
			newBlock[index] = newPattern->map[MAXIMUM_COLUMNS * row + column];
			index++;
		}
	}
	
	return 0;
}

	






