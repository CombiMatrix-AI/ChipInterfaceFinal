#include <userint.h>
#include "pattern.h"
#include "patternuir.h"

//module globals
static int patternPanel = 0, blockDefinitionPanel = 0, blockListPanel = 0, chipTestPanel = 0,
	voltagePanel = 0, batchViewPanel = 0; 

voltageWatchThreadFunctionID = 0;

/**
Load user interface for pattern definition

Setup table popup menu
*/
int LoadPatternDefinitionPanel(char *version, Properties *defaults)
{
	int *pattern = NULL;
	int index = 0;
	char message[MAX_MESSAGE_SIZE] = {0};
	
	patternPanel = LoadPanel(0, "pattern.uir", BLOCKDEF);
    blockDefinitionPanel = LoadPanel(patternPanel, "pattern.uir", BLOCK);
    blockListPanel = LoadPanel(patternPanel, "pattern.uir", BLOCKLIST);
    chipTestPanel = LoadPanel(patternPanel, "pattern.uir", CHIPTEST);
    voltagePanel = LoadPanel(patternPanel, "pattern.uir", VOLTAGE);
    batchViewPanel = LoadPanel(0, "pattern.uir", BATCHVIEW);
    
    EasyTab_ConvertFromCanvas(patternPanel, BLOCKDEF_TABS);
	EasyTab_AddPanels (patternPanel, BLOCKDEF_TABS, 1,
                   blockDefinitionPanel, blockListPanel, voltagePanel, chipTestPanel, 0);        
                   
	//hide default popup menu items for table control
	HideBuiltInCtrlMenuItem (blockDefinitionPanel, BLOCK_TABLE, VAL_GOTO);
	HideBuiltInCtrlMenuItem (blockDefinitionPanel, BLOCK_TABLE, VAL_SEARCH);
	HideBuiltInCtrlMenuItem (blockDefinitionPanel, BLOCK_TABLE, VAL_SORT);
	
	//add my own popup menu to the table
	NewCtrlMenuItem (blockDefinitionPanel, BLOCK_TABLE, VLINE_0_STRING, -1, 
		(CtrlMenuCallbackPtr) SetVlineMenu, (void*)VLINE_0);
	NewCtrlMenuItem (blockDefinitionPanel, BLOCK_TABLE, VLINE_1_STRING, -1, 
		(CtrlMenuCallbackPtr) SetVlineMenu, (void*)VLINE_1);
	NewCtrlMenuItem (blockDefinitionPanel, BLOCK_TABLE, VLINE_2_STRING, -1, 
		(CtrlMenuCallbackPtr) SetVlineMenu, (void*)VLINE_2);
	NewCtrlMenuItem (blockDefinitionPanel, BLOCK_TABLE, VLINE_3_STRING, -1, 
		(CtrlMenuCallbackPtr) SetVlineMenu, (void*)VLINE_3);
		
	NewCtrlMenuSeparator (blockDefinitionPanel, BLOCK_TABLE, -1);
	
	NewCtrlMenuItem (blockDefinitionPanel, BLOCK_TABLE, CHECKERBOARD_STRING, -1, 
		(CtrlMenuCallbackPtr) SetBlockDesignMenu, (void*)CHECKERBOARD);
	NewCtrlMenuItem (blockDefinitionPanel, BLOCK_TABLE, INVCHECKERBOARD_STRING, -1, 
		(CtrlMenuCallbackPtr) SetBlockDesignMenu, (void*)INVCHECKERBOARD);
	
    pattern = calloc(sizeof(int), MAXIMUM_ELECTRODES);
    if(pattern == NULL)
    {
		sprintf(message, "Could not allocate memory for chip image.");
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;
	}
	
	for(index = 0; index < MAXIMUM_ELECTRODES; index++)
	{
		pattern[index] = FLOATVLINE;
	}
    UpdateChipView(pattern);
    free(pattern);
    
    SetCtrlVal(patternPanel, BLOCKDEF_VERSION, version);
    
    if(defaults->emulateChipHardware == 1)
    {
    	SetCtrlVal(patternPanel, BLOCKDEF_HARDWARESTATE, "EMULATED");
    }
    
    DisplayPanel(patternPanel);
    
    //set panel values from default properties
    SetCtrlVal(chipTestPanel, CHIPTEST_DELAY, defaults->chipHardwareDelay);
    return 0;
}

//code for menu bar picks
void CVICALLBACK BlockMenubarCB (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	int status = 0;
	int index = 0;
	int blockId = 0;
	int numberPatterns = 0;
	static char lastDirectory[512] = {0};
	char filename[512] = {0};
	char tempFilename[256] = {0};
	char message[MAX_MESSAGE_SIZE] = {0};
	
	int *newBlock = NULL;
	int *newPattern = NULL;
	PatternDefinition newItem;
	static PatternDefinition copyItem;   //this is used to hold a definition to be pasted
	VoltageHardware vProfile;
	
	GetCtrlVal(blockDefinitionPanel, BLOCK_ID, &blockId);
	if(lastDirectory[0] == NULL)
	{
		GetProjectDir(lastDirectory);
	}
	
	switch(menuItem)
	{
		case PATMENU_FILE_OPEN_OPENBLOCKDEF:
			status = FileSelectPopup (lastDirectory, "*.block", "*.block", LOAD_BLOCK_STRING, VAL_OK_BUTTON,
				0, 1, 1, 1, filename);
			if(status == VAL_NO_FILE_SELECTED)
			{
				return;
			}
			
			status = InitializePattern(&newItem, blockId);
			if(status != 0)
			{
				sprintf(message, "Could not initialize pattern definition (status %d).", status);
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return;
			}
			
			status = LoadPatternData(filename, &newItem);
			if(status != 0)
			{
				sprintf(message, "Could not load chip pattern definition (status %d).", status);
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return;
			}
			
			SetBlockPanelValues(&newItem);
			
			break;
		case PATMENU_FILE_SAVE_SAVEBLOCKDEF:
			status = GetPatternData(&newItem, blockId);
			if(status != 0)
			{
				sprintf(message, "Could not get list item (status %d).", status);
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return;
			}
			
			sprintf(tempFilename, "%s.block", newItem.description);
			status = FileSelectPopup (lastDirectory, tempFilename, "*.block", SAVE_BLOCK_STRING, VAL_SAVE_BUTTON,
				0, 1, 1, 1, filename);
			if(status == VAL_NO_FILE_SELECTED)
			{
				return;
			}
			
			strcpy(newItem.filename, filename);
			status = SavePatternData(filename, &newItem);
			if(status != 0)
			{
				sprintf(message, "Could not save chip pattern definition (status %d).", status);
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return;
			}
			//update list record to reflect filename saved
			InsertPattern(newItem, blockId);
			SetBlockPanelValues(&newItem);
			break;
		case PATMENU_FILE_OPEN_OPENBLOCKLIST:
			//if there is already a block list in memory, get rid of it... will check to make sure all
			//blocks are saved later...
			InitializePatternList();
			ClearListCtrl (blockListPanel, BLOCKLIST_PATTERNS);
			
			status = FileSelectPopup (lastDirectory, "*.list", "*.list", LOAD_BLOCK_LIST_STRING, VAL_OK_BUTTON,
				0, 1, 1, 1, filename);
			if(status == VAL_NO_FILE_SELECTED)
			{
				return;
			}
			
			numberPatterns = LoadBlockList(filename);
			if(numberPatterns <= 0)
			{
				sprintf(message, "Could not load pattern list (numberPatterns %d).", numberPatterns);
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return;
			}
			
			for(index = 0; index < numberPatterns; index++)
			{
				status = GetPatternData(&newItem, index+1);
				if(status != 0)
				{
					sprintf(message, "Could not get pattern definition from list (status %d).", status);
					AddErrorToLog(message, __FILE__, __LINE__);
					MessagePopup("ERROR", message);
					return;
				}
				
				//if there is no voltage configuration attached to this block, skip this
				if(strlen(newItem.voltageFilename) > 0)
				{
					//load voltage configuration to access voltage description
					LoadVoltageDataFile(newItem.voltageFilename, &vProfile);
			
					//create new tag containing voltage timing file
					sprintf(message, "%s\033p150l%s", newItem.description, vProfile.description);
					InsertListItem (blockListPanel, BLOCKLIST_PATTERNS, -1, message, newItem.patternId);
					
				}
				
				else
				{
					InsertListItem (blockListPanel, BLOCKLIST_PATTERNS, -1, newItem.description, 
						newItem.patternId);
				}
			}
			
			GetPatternData(&newItem, 1);
			SetBlockPanelValues(&newItem);
			break;
		case PATMENU_FILE_OPEN_OPENVCONFIG:
			//select a filename to load data
			status = FileSelectPopup (lastDirectory, "*.vcfg", "*.vcfg", LOAD_VOLTAGE_CONFIG_STRING, VAL_LOAD_BUTTON,
				0, 1, 1, 1, filename);
			if(status == VAL_NO_FILE_SELECTED)
			{
				return;
			}
			
			//load the data to a file
			status = LoadVoltageDataFile(filename, &vProfile);
			if(status != 0)
			{
				return;
			}
			
			//set voltage profile data from voltage panel
			status = SetVoltagePanelData(vProfile);
			if(status != 0)
			{
				return;
			}
			break;
		case PATMENU_FILE_SAVE_SAVEBLOCKLIST:
			GetNumListItems(blockListPanel, BLOCKLIST_PATTERNS, &numberPatterns);
			if(numberPatterns <= 0)
			{
				sprintf(message, "Could not save pattern list - emtpy.");
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return;
			}
			
			// make sure all block definitions are saved before trying to save list
			for(index = 0; index < numberPatterns; index++)
			{
				status = GetPatternData(&newItem, index+1);
				if(status != 0)
				{
					sprintf(message, "Could not get pattern definition from list (status %d).", status);
					AddErrorToLog(message, __FILE__, __LINE__);
					MessagePopup("ERROR", message);
					return;
				}	
				
				//this block definition was not saved, save it now
				if(strlen(newItem.filename) <= 0)
				{
					sprintf(message, "\"%s\" must be saved", newItem.description);
					sprintf(tempFilename, "%s.block", newItem.description);
					status = FileSelectPopup (lastDirectory, tempFilename, "*.block", message, VAL_SAVE_BUTTON,
						0, 1, 1, 1, filename);
					if(status == VAL_NO_FILE_SELECTED)
					{
						return;
					}
			
					strcpy(newItem.filename, filename);
					status = SavePatternData(filename, &newItem);
					if(status != 0)
					{
						sprintf(message, "Could not save chip pattern definition (status %d).", status);
						AddErrorToLog(message, __FILE__, __LINE__);
						MessagePopup("ERROR", message);
						return;
					}
					
					//update list item, incase filename field of pattern object was updated
					InsertPattern(newItem, index+1);
					SetBlockPanelValues(&newItem);
				}
			}
			status = FileSelectPopup (lastDirectory, "*.list", "*.list", SAVE_BLOCK_LIST_STRING, VAL_SAVE_BUTTON,
				0, 1, 1, 1, filename);
			if(status == VAL_NO_FILE_SELECTED)
			{
				return;
			}
			
			status = SavePatternList(filename, numberPatterns);
			if(status != 0)
			{
				sprintf(message, "Could not save chip pattern definition list (status %d).", status);
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return;
			}
			break;
		case PATMENU_FILE_SAVE_SAVEVCONFIG:
			//select a filename to place data
			status = FileSelectPopup (lastDirectory, "*.vcfg", "*.vcfg", SAVE_VOLTAGE_CONFIG_STRING, VAL_SAVE_BUTTON,
				0, 1, 1, 1, filename);
			if(status == VAL_NO_FILE_SELECTED)
			{
				return;
			}
			//get voltage profile data from voltage panel
			status = GetVoltagePanelData(&vProfile);
			if(status != 0)
			{
				return;
			}	
			
			//save the data to a file
			status = SaveVoltageDataFile(filename, &vProfile);
			if(status != 0)
			{
				return;
			}	
			
			break;
		case PATMENU_FILE_CLOSE:
			QuitUserInterface(0);
			break;
		case PATMENU_EDIT_COPY:
			InitializePattern(&copyItem, blockId);
			
			GetCtrlVal(blockDefinitionPanel, BLOCK_NUMCOLUMNS, &copyItem.numberColumns);
			GetCtrlVal(blockDefinitionPanel, BLOCK_NUMROWS, &copyItem.numberRows);
			GetCtrlVal(blockDefinitionPanel, BLOCK_STARTROW, &copyItem.startRow);
			GetCtrlVal(blockDefinitionPanel, BLOCK_STARTCOLUMN, &copyItem.startColumn);
			
			newBlock = calloc(sizeof(int), (copyItem.numberRows * copyItem.numberColumns));
			if(newBlock == NULL)
			{
				sprintf(message, "Could not allocate memory for block.");
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return;	
			}
			
			status = GetBlock(&(*newBlock), copyItem.numberRows, copyItem.numberColumns);
			if(status != 0)
			{
				sprintf(message, "Could not get block (status %d).", status);
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return;	
			}
			
			newPattern = calloc(sizeof(int), MAXIMUM_ELECTRODES);
			if(newPattern == NULL)
			{
				sprintf(message, "Could not allocate memory for chip pattern.");
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				free(newBlock);
				return;	
			}
			
			status = CreatePattern(&(*newPattern), newBlock, copyItem.startRow-1, 
				copyItem.startColumn-1, copyItem.numberRows, copyItem.numberColumns, FLOATVLINE);
			if(status != 0)
			{
				sprintf(message, "Could not create pattern (status %d).", status);
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				free(newBlock);
				free(newPattern);
				return;
			}
			
			status = GetBlockPanelValues(&copyItem, newPattern);
			if(status != 0)
			{
				sprintf(message, "Could not get block panel values (status %d).", status);
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				free(newBlock);
				free(newPattern);
				return;
			}
			
			free(newBlock);
			free(newPattern);
			break;
		case PATMENU_VIEW_CHIPMAP:
			LoadChipMapPanel();
			break;
		case PATMENU_VIEW_KE6485:
			LoadKeithleyPanel();
			break;
		case PATMENU_VIEW_DEBUG:
			DisplayDebugPanel();
			break;
		case PATMENU_EDIT_PASTE:
			copyItem.patternId = blockId;
			SetBlockPanelValues(&copyItem);
		default:
			break;
	}
	SplitPath (filename, NULL, lastDirectory, NULL);
}


int CVICALLBACK LoadBlockCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:

			break;
		}
	return 0;
}

int CVICALLBACK UpdateBlockCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int numberRows = 0;
	int numberColumns = 0;
	int startRow = 0;
	int startColumn = 0;
	char message[MAX_MESSAGE_SIZE] = {0};
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(panel, BLOCK_NUMCOLUMNS, &numberColumns);
			GetCtrlVal(panel, BLOCK_NUMROWS, &numberRows);
			GetCtrlVal(panel, BLOCK_STARTROW, &startRow);
			GetCtrlVal(panel, BLOCK_STARTCOLUMN, &startColumn);
			
			//do some error checking
			if(numberColumns == 0 || numberRows == 0)
			{
				sprintf(message, "Block parameters are invalid (Number columns and rows must be greater than 0).");
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return 0;
			}
			
			if((startRow-1)+numberRows > MAXIMUM_ROWS || (startColumn-1)+numberColumns > MAXIMUM_COLUMNS)
			{
				sprintf(message, "Block parameters are invalid (Block exceeds boundaries of chip).");
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return 0;
			}
			
			ResizeBlockTable(numberRows, numberColumns);
			break;
		}
	return 0;
}

int CVICALLBACK AddBlockCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int status = 0;
	
	int *newBlock = NULL;
	int *newPattern = NULL;
	int numberColumns = 0;
	int numberRows = 0;
	int startRow = 0;
	int startColumn = 0;
	
	int index = 0;
	int numberPatternsListed = 0;
	int patternId = 0;
	char message[MAX_MESSAGE_SIZE] = {0};
	PatternDefinition newItem;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(blockDefinitionPanel, BLOCK_NUMCOLUMNS, &numberColumns);
			GetCtrlVal(blockDefinitionPanel, BLOCK_NUMROWS, &numberRows);
			GetCtrlVal(blockDefinitionPanel, BLOCK_STARTROW, &startRow);
			GetCtrlVal(blockDefinitionPanel, BLOCK_STARTCOLUMN, &startColumn);
			
			//do some error checking
			if(numberColumns == 0 || numberRows == 0)
			{
				sprintf(message, "Block parameters are invalid (Number columns and rows must be greater than 0).");
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return -1;
			}
			
			if((startRow-1)+numberRows > MAXIMUM_ROWS || (startColumn-1)+numberColumns > MAXIMUM_COLUMNS)
			{
				sprintf(message, "Block parameters are invalid (Block exceeds boundaries of chip).");
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return -1;
			}
			
			GetNumTableRows (blockDefinitionPanel, BLOCK_TABLE, &numberRows);
			GetNumTableColumns (blockDefinitionPanel, BLOCK_TABLE, &numberColumns);
			if(numberColumns == 0 || numberRows == 0)
			{
				sprintf(message, "Table has not been created, press Update.");
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return -1;
			}
			
			newBlock = calloc(sizeof(int), (numberRows*numberColumns));
			if(newBlock == NULL)
			{
				sprintf(message, "Could not allocate memory for block.");
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return -1;	
			}
			
			status = GetBlock(&(*newBlock), numberRows, numberColumns);
			if(status != 0)
			{
				sprintf(message, "Could not get block (status %d).", status);
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return -1;	
			}
			
			newPattern = calloc(sizeof(int), MAXIMUM_ELECTRODES);
			if(newPattern == NULL)
			{
				sprintf(message, "Could not allocate memory for chip pattern.");
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return -1;	
			}
			
			status = CreatePattern(&(*newPattern), newBlock, startRow-1, startColumn-1, numberRows, 
				numberColumns, FLOATVLINE);
			if(status != 0)
			{
				sprintf(message, "Could not create chip pattern (status %d).", status);
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return -1;
			}
			
			UpdateChipView(newPattern);

			status = GetBlockPanelValues(&newItem, newPattern);
			if(status != 0)
			{
				sprintf(message, "Could not get block panel values (status %d).", status);
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return -1;
			}
			
			status = InsertPattern(newItem, newItem.patternId);
			if(status != 0)
			{
				sprintf(message, "Could not insert pattern definition into list (status %d).", status);
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return -1;
			}
			
			GetNumListItems (blockListPanel, BLOCKLIST_PATTERNS, &numberPatternsListed);
			for(index = 0; index < numberPatternsListed; index++)
			{
				GetValueFromIndex(blockListPanel, BLOCKLIST_PATTERNS, index, &patternId);
				if(patternId == newItem.patternId)
				{
					ReplaceListItem (blockListPanel, BLOCKLIST_PATTERNS, index, newItem.description, newItem.patternId);	
					return 0;
				}
			}
			InsertListItem (blockListPanel, BLOCKLIST_PATTERNS, numberPatternsListed, newItem.description, 
				newItem.patternId);
			
			free(newBlock);
			free(newPattern);
			break;
		}
	return 0;
}

int CVICALLBACK NewBlockCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int numberBlocks = 0;
	PatternDefinition tempItem;
	
	switch (event)
		{
		case EVENT_COMMIT:
			numberBlocks = GetNumberPatterns();
			SetCtrlVal (panel, BLOCK_ID, numberBlocks+1);
		    InitializePattern(&tempItem, numberBlocks+1);
			SetBlockPanelValues(&tempItem);    
	        break;
		}
	return 0;
}

int CVICALLBACK TileBlockCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int status = 0;
	
	int numberRows = 0;
	int numberColumns = 0;
	int startRow = 0;
	int startColumn = 0;
	
	int tiledWidth = 0;
	int tiledHeight = 0;
	
	int index = 0;
	
	int numberPatternsListed = 0;
	int patternId = 0;
	
	int *newBlock = NULL;
	int *newPattern = NULL;
	
	char message[MAX_MESSAGE_SIZE] = {0};
	PatternDefinition blockDef;
	
	switch (event)
		{
		case EVENT_COMMIT:
			//we want to tile this block across the chip, start with row, then increment column
			
			//first, check to see if it is possible
			//see if it has been added to the block list yet
			GetCtrlVal(blockDefinitionPanel, BLOCK_ID, &index);
			status = GetPatternData(&blockDef, index);
			
			if(status != 0)
			{
				sprintf(message, "Could not get pattern definition from list (status %d).\nMake sure block has been added to the list.", status);
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return -1;		
			}
			
			//see if it will fit on the chip
			GetCtrlVal(blockDefinitionPanel, BLOCK_NUMCOLUMNS, &numberColumns);
			GetCtrlVal(blockDefinitionPanel, BLOCK_NUMROWS, &numberRows);
			GetCtrlVal(blockDefinitionPanel, BLOCK_STARTROW, &startRow);
			GetCtrlVal(blockDefinitionPanel, BLOCK_STARTCOLUMN, &startColumn);
			
			tiledWidth = (startColumn + (numberColumns * 2)-1);
			tiledHeight = (startRow - numberRows);
			if(tiledHeight < 0 && tiledWidth > MAXIMUM_COLUMNS)
			{
				sprintf(message, "Cannot tile block - decrease size");
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return -1;
			}
			
			//now that we know it will fit somewhere, we have to deturmine where it will fit
			//check the columns first
			if(tiledWidth <= MAXIMUM_COLUMNS)
			{
				startColumn += numberColumns;
				SetCtrlVal(blockDefinitionPanel, BLOCK_STARTCOLUMN, startColumn);
			}
			//check the rows next if the columns fail
			else if(tiledHeight > 0)
			{
				startColumn = 1;
				startRow -= numberRows;
				SetCtrlVal(blockDefinitionPanel, BLOCK_STARTROW, startRow);
				SetCtrlVal(blockDefinitionPanel, BLOCK_STARTCOLUMN, startColumn);
				
			}
			//error condition
			else
			{
				sprintf(message, "Cannot tile block -  reached past end of chip.");
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return -1;	
			}
			
			/*
			//now check to see if we need to move up the chip to the next set of rows
			if(tiledWidth > MAXIMUM_COLUMNS && numberColumns * 2 <= MAXIMUM_COLUMNS)
			{
				startColumn = 1;
				// we are going up the chip, now down
				if(startRow - numberRows <= 0)
				{
					sprintf(message, "Cannot tile block -  reached past end of chip.");
					AddErrorToLog(message, __FILE__, __LINE__);
					MessagePopup("ERROR", message);
					return -1;
				}
				startRow -= numberRows;
			}
			*/
			newBlock = calloc(sizeof(int), (numberRows*numberColumns));
			if(newBlock == NULL)
			{
				sprintf(message, "Could not allocate memory for block.");
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return -1;	
			}
			
			status = GetBlock(&(*newBlock), numberRows, numberColumns);
			if(status != 0)
			{
				sprintf(message, "Could not get block (status %d).", status);
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return -1;	
			}
			
			newPattern = calloc(sizeof(int), MAXIMUM_ELECTRODES);
			if(newPattern == NULL)
			{
				sprintf(message, "Could not allocate memory for chip pattern.");
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return -1;	
			}
			
			status = CreatePattern(&(*newPattern), newBlock, startRow-1, startColumn-1, numberRows, 
				numberColumns, FLOATVLINE);
			if(status != 0)
			{
				sprintf(message, "Could not create chip pattern (status %d).", status);
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return -1;
			}
			
			UpdateChipView(newPattern);
			
			//Now that it was legit, add it to the block list
			GetCtrlVal(blockDefinitionPanel, BLOCK_ID, &index);
			SetCtrlVal(blockDefinitionPanel, BLOCK_ID, index+1);
			//SetTiledDescriptionName(&blockDef, tileRow, tileColumn);
			
			//reset the filename so it does not overwrite existing files
			SetCtrlVal(blockDefinitionPanel, BLOCK_FILENAME, "Unsaved Definition");
			
			status = GetBlockPanelValues(&blockDef, newPattern);
			if(status != 0)
			{
				sprintf(message, "Could not get block panel values (status %d).", status);
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return -1;
			}
			
			status = InsertPattern(blockDef, blockDef.patternId);
			if(status != 0)
			{
				sprintf(message, "Could not insert pattern definition into list (status %d).", status);
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return -1;
			}
			
			GetNumListItems (blockListPanel, BLOCKLIST_PATTERNS, &numberPatternsListed);
			for(index = 0; index < numberPatternsListed; index++)
			{
				GetValueFromIndex(blockListPanel, BLOCKLIST_PATTERNS, index, &patternId);
				if(patternId == blockDef.patternId)
				{
					ReplaceListItem (blockListPanel, BLOCKLIST_PATTERNS, index, blockDef.description, blockDef.patternId);	
					return 0;
				}
			}
			InsertListItem (blockListPanel, BLOCKLIST_PATTERNS, numberPatternsListed, blockDef.description, 
				blockDef.patternId);
				
			free(newBlock);
			free(newPattern);
			break;
		}
	return 0;
}

int CVICALLBACK PatternPanelCB (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_CLOSE:
			QuitUserInterface(0);
			break;
		}
	return 0;
}

int ResizeBlockTable(int numberRows, int numberColumns)
{
	int numberExistingRows = 0;
	int numberExistingColumns = 0;
	int index = 1;
	
	GetNumTableRows (blockDefinitionPanel, BLOCK_TABLE, &numberExistingRows);
	GetNumTableColumns (blockDefinitionPanel, BLOCK_TABLE, &numberExistingColumns);
	
	if(numberExistingRows > 0)
	{
		DeleteTableRows (blockDefinitionPanel, BLOCK_TABLE, 1, numberExistingRows);
	}
	
	if(numberExistingColumns > 0)
	{
		DeleteTableColumns (blockDefinitionPanel, BLOCK_TABLE, 1, numberExistingColumns);
	}
	
	InsertTableColumns (blockDefinitionPanel, BLOCK_TABLE, 1, numberColumns, VAL_USE_MASTER_CELL_TYPE);
	InsertTableRows (blockDefinitionPanel, BLOCK_TABLE, 1, numberRows, VAL_USE_MASTER_CELL_TYPE);
	
	
	for(index = 1; index <= numberColumns; index++)
	{
		SetTableColumnAttribute  (blockDefinitionPanel, BLOCK_TABLE, index, ATTR_SIZE_MODE, VAL_USE_EXPLICIT_SIZE);
		SetTableColumnAttribute  (blockDefinitionPanel, BLOCK_TABLE, index, ATTR_COLUMN_WIDTH, 18);
	}
	return 0;
}


int CVICALLBACK SelectPatternCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	static int previousValue = 0;
	int currentItem = 0;
	int status = 0;
	char message[MAX_MESSAGE_SIZE] = {0};
	PatternDefinition tempItem;
	
	switch (event)
		{
		case EVENT_LEFT_CLICK:
			GetCtrlVal(panel, BLOCK_ID, &previousValue);
			return 0;
		case EVENT_COMMIT:
			GetCtrlVal (panel, BLOCK_ID, &currentItem);
		    InitializePattern(&tempItem, currentItem);
			if (currentItem > 0) 
	        {
		        status = GetPatternData (&tempItem, currentItem);
				if(status != 0)
				{
					sprintf(message, "Could not get pattern definition from list (status %d).\nUse New Block when trying to create new block.", status);
					AddErrorToLog(message, __FILE__, __LINE__);
					MessagePopup("ERROR", message);
					SetCtrlVal(panel, BLOCK_ID, previousValue);
					return -1;
				}	
		        SetBlockPanelValues(&tempItem);    
	        }
			break;
		}
	return 0;
}

/**
custom menu for table, accessed by right clicking on table control
*/
void CVICALLBACK SetVlineMenu (int panelHandle, int controlID, int MenuItemID, void *callbackData)
{
	int index = 0;
	int vline = (int)callbackData;
	int *vlineArray = NULL;
	int arraySize = 0;
	char message[MAX_MESSAGE_SIZE] = {0};
	Rect selectionArea;
	Point cell;
	
	GetTableSelection (blockDefinitionPanel, BLOCK_TABLE, &selectionArea);
	
	arraySize = selectionArea.width * selectionArea.height;
	if(arraySize == 0)
	{
		return;
	}
	if(arraySize > 1)
	{
		vlineArray = calloc(sizeof(int), arraySize);
		if(vlineArray == NULL)
		{
			sprintf(message, "Could not allocate vline array.");
			AddErrorToLog(message, __FILE__, __LINE__);
			MessagePopup("ERROR", message);
			return;
		}
	
		for(index = 0; index < arraySize; index++)
		{
			vlineArray[index] = vline;
		}
	
		SetTableCellRangeVals (blockDefinitionPanel, BLOCK_TABLE, selectionArea, vlineArray, VAL_ROW_MAJOR);
	}
	else
	{
		GetActiveTableCell (blockDefinitionPanel, BLOCK_TABLE, &cell);
		SetTableCellVal (blockDefinitionPanel, BLOCK_TABLE, cell, vline);
	}
	return;
}

void CVICALLBACK SetBlockDesignMenu (int panelHandle, int controlID, int MenuItemID, void *callbackData)
{
	int patternType = (int)callbackData;
	int *vlineArray = NULL;
	int arraySize = 0;
	int checkerboardElectrode = 0;
	char buffer[2] = {0};
	char message[MAX_MESSAGE_SIZE] = {0};
	Rect selectionArea;

	GetTableSelection (blockDefinitionPanel, BLOCK_TABLE, &selectionArea);
	
	arraySize = selectionArea.width * selectionArea.height;
	if(arraySize == 0)
	{
		return;
	}
	vlineArray = calloc(sizeof(int), arraySize);
	if(vlineArray == NULL)
	{
		sprintf(message, "Could not allocate vline array.");
		AddErrorToLog(message, __FILE__, __LINE__);
		MessagePopup("ERROR", message);
		return;
	}
	
	switch(patternType)
	{
		case CHECKERBOARD:
			PromptPopup (NULL, "Enter electrode to checker", buffer, 1);
			checkerboardElectrode = atoi(buffer);
			GetTableCellRangeVals (blockDefinitionPanel, BLOCK_TABLE, selectionArea, 
				vlineArray, VAL_ROW_MAJOR);
			MakeCheckerboard(&(*vlineArray), selectionArea.height, selectionArea.width,
				0, checkerboardElectrode);
			break;
		case INVCHECKERBOARD:
			PromptPopup (NULL, "Enter electrode to checker", buffer, 1);
			checkerboardElectrode = atoi(buffer);
			GetTableCellRangeVals (blockDefinitionPanel, BLOCK_TABLE, selectionArea, 
				vlineArray, VAL_ROW_MAJOR);
			MakeCheckerboard(&(*vlineArray), selectionArea.height, selectionArea.width,
				1, checkerboardElectrode);
			break;
		default:
			return;
	}	
	
	SetTableCellRangeVals (blockDefinitionPanel, BLOCK_TABLE, selectionArea, vlineArray, VAL_ROW_MAJOR);
	
	return;
}

int GetBlock(int *newBlock, int numberRows, int numberColumns)
{
	int numberTableRows = 0;
	int numberTableColumns = 0;
	char message[MAX_MESSAGE_SIZE] = {0};
	
	GetNumTableRows (blockDefinitionPanel, BLOCK_TABLE, &numberTableRows);
	GetNumTableColumns (blockDefinitionPanel, BLOCK_TABLE, &numberTableColumns);
	
	if(numberRows != numberTableRows || numberColumns != numberTableColumns)
	{
		sprintf(message, "Number of rows and columns does not equal table size.");
		AddErrorToLog(message, __FILE__, __LINE__);
		return -1;	
	}
	
	GetTableCellRangeVals (blockDefinitionPanel, BLOCK_TABLE, MakeRect(1, 1, numberRows, numberColumns), 
		newBlock, VAL_ROW_MAJOR);

	return 0;
}

int UpdateChipView(int *pattern)
{
	int row = 0;
	int column = 0;
	int index = 0;
	int fillColor = 0;

	SetCtrlAttribute (patternPanel, BLOCKDEF_CHIPIMAGE, ATTR_PEN_COLOR, VAL_BLACK);

	CanvasStartBatchDraw (patternPanel, BLOCKDEF_CHIPIMAGE);

	for(row = 0; row < MAXIMUM_ROWS; row++)
	{
		for(column = 0; column < MAXIMUM_COLUMNS; column++)
		{
			if(pattern[index] == VLINE_0)
			{
				fillColor = VAL_DK_GRAY;
			}
			else if(pattern[index] == VLINE_1)
			{
				fillColor = VAL_BLUE;
			}
			else if(pattern[index] == VLINE_2)
			{
				fillColor = VAL_YELLOW;
			}
			else if(pattern[index] == VLINE_3)
			{
				fillColor = VAL_GREEN;
			}
			else
			{
				fillColor = VAL_RED;
			}
			
			SetCtrlAttribute (patternPanel, BLOCKDEF_CHIPIMAGE, ATTR_PEN_FILL_COLOR, fillColor);
		
			CanvasDrawRect (patternPanel, BLOCKDEF_CHIPIMAGE, 
				MakeRect((row*PIXELS_PER_ROW), (column*PIXELS_PER_COLUMN), PIXELS_PER_ROW, PIXELS_PER_COLUMN),
				VAL_DRAW_FRAME_AND_INTERIOR);
			
			//VAL_DRAW_FRAME
			/*CanvasDrawOval (patternPanel, BLOCKDEF_CHIPIMAGE,
				MakeRect((row*PIXELS_PER_ROW)+1, (column*PIXELS_PER_COLUMN)+1, PIXELS_PER_ROW-2, 
					PIXELS_PER_COLUMN-2),
				VAL_DRAW_FRAME_AND_INTERIOR);
			*/	
			index++;
		}	
	}

	CanvasEndBatchDraw (patternPanel, BLOCKDEF_CHIPIMAGE);

	return 0;
}

int GetBlockPanelValues(PatternDefinition *newPattern, int *pattern)
{
	int index = 0;
	
	GetCtrlVal(blockDefinitionPanel, BLOCK_ID, &index);
	
	InitializePattern(newPattern, index);
			
	GetCtrlVal(blockDefinitionPanel, BLOCK_DESCRIPTION, newPattern->description);
	GetCtrlVal(blockDefinitionPanel, BLOCK_FILENAME, newPattern->filename);
	if(strcmp(newPattern->filename, "Unsaved Definition") == 0)
	{
		strcpy(newPattern->filename, "");
	}
	
	GetCtrlVal(blockDefinitionPanel, BLOCK_NUMCOLUMNS, &newPattern->numberColumns);
	GetCtrlVal(blockDefinitionPanel, BLOCK_NUMROWS, &newPattern->numberRows);
	GetCtrlVal(blockDefinitionPanel, BLOCK_STARTROW, &newPattern->startRow);
	GetCtrlVal(blockDefinitionPanel, BLOCK_STARTCOLUMN, &newPattern->startColumn);
	GetCtrlVal(blockDefinitionPanel, BLOCK_COMMENTS, newPattern->comments);
	
	for(index = 0; index < MAXIMUM_ELECTRODES; index++)
	{
		newPattern->map[index] = pattern[index];
	}
	
	return 0;
}

int SetBlockPanelValues(PatternDefinition *newPattern)
{
	int *newBlock = NULL;
	char message[MAX_MESSAGE_SIZE] = {0};
	
	SetCtrlVal(blockDefinitionPanel, BLOCK_DESCRIPTION, newPattern->description);
	if(strcmp(newPattern->filename, "") == 0)
	{
		SetCtrlVal(blockDefinitionPanel, BLOCK_FILENAME, "Unsaved Definition");
	}
	else
	{
		SetCtrlVal(blockDefinitionPanel, BLOCK_FILENAME, newPattern->filename);
	}
	
	ResetTextBox (blockDefinitionPanel, BLOCK_COMMENTS, newPattern->comments);
	SetCtrlVal(blockDefinitionPanel, BLOCK_ID, newPattern->patternId);
	SetCtrlVal(blockDefinitionPanel, BLOCK_NUMCOLUMNS, newPattern->numberColumns);
	SetCtrlVal(blockDefinitionPanel, BLOCK_NUMROWS, newPattern->numberRows);
	SetCtrlVal(blockDefinitionPanel, BLOCK_STARTROW, newPattern->startRow);
	SetCtrlVal(blockDefinitionPanel, BLOCK_STARTCOLUMN, newPattern->startColumn);
		
	if(newPattern->numberRows > 0 && newPattern->numberColumns > 0)
	{
		ResizeBlockTable(newPattern->numberRows, newPattern->numberColumns);
		
		newBlock = calloc(sizeof(int), (newPattern->numberRows * newPattern->numberColumns));
		if(newBlock == NULL)
		{
			sprintf(message, "Could not allocate new block.");
			AddErrorToLog(message, __FILE__, __LINE__);
			MessagePopup("ERROR", message);
			return -1;	
		}
		
		GetBlockFromMap(newPattern, &(*newBlock));
		
		SetTableCellRangeVals (blockDefinitionPanel, BLOCK_TABLE, MakeRect(1, 1, newPattern->numberRows, 
			newPattern->numberColumns), newBlock, VAL_ROW_MAJOR);

	
		UpdateChipView(newPattern->map);
	}
	return 0;
}

/***************************
BlockList Panel functions

***************************/

int CVICALLBACK BlockListPanelCB (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:

			break;
		}
	return 0;
}

int CVICALLBACK PatternListCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int index = 0;
	PatternDefinition selectedBlock;
	VoltageHardware vProfile;
	switch (event)
		{
		case EVENT_COMMIT:

			break;
		case EVENT_LEFT_DOUBLE_CLICK:
		case EVENT_LEFT_CLICK:
			GetCtrlIndex(panel, control, &index);
			GetPatternData(&selectedBlock, index+1);
			SetBlockPanelValues(&selectedBlock);
			
			InitVoltageDataStructure(&vProfile);
			if(strlen(selectedBlock.voltageFilename) > 0)
			{
				LoadVoltageDataFile(selectedBlock.voltageFilename, &vProfile);
			}
			SetVoltagePanelData(vProfile);
			break;
		}
	return 0;
}

int CVICALLBACK ActivatePatternCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int index = 0;
	int electrodeIndex = 0;
	int status = 0;
	int patternSelected = 0;
	int numberPatternsListed = 0;
	int numberPatternsChecked = 0;
	int channel = 0;
	int channelSelected = 0;
	int numberChannelsChecked = 0;
	int batchRun = 0;
	int singleRun = 0;
	int triggerVoltage = 0;
	int triggerKeithley = 0;
	double longestDuration = 0.0;
	
	int *map = NULL;
	char message[MAX_MESSAGE_SIZE] = {0};
	PatternDefinition pattern;
	VoltageHardware profile;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetNumListItems (panel, BLOCKLIST_PATTERNS, &numberPatternsListed);
			if(numberPatternsListed != GetNumberPatterns())
			{
				sprintf(message, "Block pattern list is corrupt.");
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return -1;
			}	
			
			GetCtrlVal(panel, BLOCKLIST_BATCHRUN, &batchRun);
			GetCtrlVal(panel, BLOCKLIST_SINGLERUN, &singleRun);
			GetCtrlVal(panel, BLOCKLIST_TRIGGERVOLTAGE, &triggerVoltage);
			GetCtrlVal(panel, BLOCKLIST_TRIGGERKEITHLEY, &triggerKeithley);
			
			switch(control)
			{
				case BLOCKLIST_ALLON:
					for(index = 0; index < numberPatternsListed; index++)
					{
						CheckListItem (panel, BLOCKLIST_PATTERNS, index, 1);	
					}
					break;
				case BLOCKLIST_ALLOFF:
					for(index = 0; index < numberPatternsListed; index++)
					{
						CheckListItem (panel, BLOCKLIST_PATTERNS, index, 0);
					}
					break;
				case BLOCKLIST_ON:	//Set chip on
				
					//if batch run is selected, spawn a new thread to handle everything.
					if(batchRun == 1)
					{
						if(singleRun == 1)
						{
							StartBatchRunSingleThread();
						}
						else
						{
							StartBatchRunThread();
						}
						return 0;
					}
					
					map = calloc(sizeof(int), MAXIMUM_ELECTRODES);
					if(map == NULL)
					{
						sprintf(message, "Could not allocate map array.");
						AddErrorToLog(message, __FILE__, __LINE__);
						MessagePopup("ERROR", message);
						return -1;
					}
					
					GetNumCheckedItems (panel, BLOCKLIST_PATTERNS, &numberPatternsChecked);
					GetNumCheckedItems (panel, BLOCKLIST_CHANNEL, &numberChannelsChecked);
					if(numberPatternsChecked == 0)
					{
						sprintf(message, "There are no blocks selected.");
						AddErrorToLog(message, __FILE__, __LINE__);
						MessagePopup("ERROR", message);
						return -1;
					}
					if(numberChannelsChecked == 0)
					{
						sprintf(message, "There are no channels selected.");
						AddErrorToLog(message, __FILE__, __LINE__);
						MessagePopup("ERROR", message);
						return -1;
					}
					
					for(index = 1; index <= numberPatternsListed; index++)
					{
						IsListItemChecked (panel, BLOCKLIST_PATTERNS, index-1, &patternSelected);
						if(patternSelected == 1)
						{
							status = GetPatternData(&pattern, index);
							if(status != 0)
							{
								free(map);
								sprintf(message, "Could not get block pattern definition.");
								AddErrorToLog(message, __FILE__, __LINE__);
								MessagePopup("ERROR", message);
								return -1;
							}
							
							for(electrodeIndex = 0; electrodeIndex < MAXIMUM_ELECTRODES; electrodeIndex++)
							{
								map[electrodeIndex] |= pattern.map[electrodeIndex];
							}
						}
					}
					
					//output raw values read from chip
					//WriteMapToDebugWindow(channel, map);
					UpdateChipView(map);
					
					for(channel = 0; channel < NUMBER_CHANNELS; channel++)
					{
						IsListItemChecked (panel, BLOCKLIST_CHANNEL, channel, &channelSelected); 
						if(channelSelected == 1)
						{
							//output raw values being sent to chip
							sprintf(message, "Writing to channel %d", channel+1);
							AddToDebugOutput(message);
							WriteMapToDebugWindow(channel, map);

							status = SetChipMap(channel, map);
							if(status != 0)
							{
								free(map);
								sprintf(message, "Hardware error setting chip map (status %d).", status);
								AddErrorToLog(message, __FILE__, __LINE__);
								MessagePopup("ERROR", message);
								return -1;
							}
						}
					}
					
					//this will turn on the DACs and Keithley, if set to do so, but a channel
					//has to be selected, so this is to make sure there is.  It will only be
					//triggered once, then stop searching for an active channel.
					for(channel = 0; channel < NUMBER_CHANNELS; channel++)
					{
						IsListItemChecked (panel, BLOCKLIST_CHANNEL, channel, &channelSelected); 
						if(channelSelected == 1)
						{
							if(triggerKeithley == 1)
							{
								//get the voltage profile information for timing
								GetVoltagePanelData(&profile);
								//need to factor in the number of off time as well
								longestDuration = profile.dacChannel[0].totalDuration +
								   (profile.dacChannel[0].totalDuration / profile.dacChannel[0].onDuration) * profile.dacChannel[0].offDuration;

								//Start Keithley measurements
								StartSampleRead();	   //get measurements started
								StartSampleReadThread(longestDuration);	 //delay for acquisition
							}
							
							if(triggerVoltage == 1)
							{
								//set DACs to proper levels
								GetVoltagePanelData(&profile);
								if(status != 0)
								{
									return 0;	
								}
								status = StartVoltageSeries(profile);
								if(status != 0)
								{
									return 0;	
								}
								SetCtrlVal(voltagePanel, VOLTAGE_STATUS, "Running Voltage Series");
								MonitorVoltageThreads();
							}
							break;
						}
					}
					
					break;
				case BLOCKLIST_OFF:		//Set chip to float, stop voltage pulsing and keithley if necessary
					if(triggerKeithley == 1)
					{
						KE6485Abort();		 //stop measurements
						KE6485ClearBuffer(); //clear buffer of unwanted data
					}
					if(triggerVoltage == 1)
					{
						StopVoltageSeries(); //stop voltage series	
					}
					
					map = calloc(sizeof(int), MAXIMUM_ELECTRODES);
					if(map == NULL)
					{
						sprintf(message, "Could not allocate map array.");
						AddErrorToLog(message, __FILE__, __LINE__);
						MessagePopup("ERROR", message);
						return -1;
					}
					
					for(electrodeIndex = 0; electrodeIndex < MAXIMUM_ELECTRODES; electrodeIndex++)
					{
						map[electrodeIndex] = FLOATVLINE;
					}
					
					UpdateChipView(map);
					
					for(channel = 0; channel < NUMBER_CHANNELS; channel++)
					{
						IsListItemChecked (panel, BLOCKLIST_CHANNEL, channel, &channelSelected); 
						if(channelSelected == 1)
						{
							//output raw values being sent to chip
							sprintf(message, "Writing to channel %d", channel+1);
							AddToDebugOutput(message);
							WriteMapToDebugWindow(channel, map);
							
							status = SetChipMap(channel, map);
							if(status != 0)
							{
								free(map);
								sprintf(message, "Hardware error setting chip map (status %d).", status);
								AddErrorToLog(message, __FILE__, __LINE__);
								MessagePopup("ERROR", message);
								return -1;
							}
						}
					}
					break;
				case BLOCKLIST_READ:	//read chip state
					for(channel = 0; channel < NUMBER_CHANNELS; channel++)
					{
						//allocate space to store chip map
						map = calloc(sizeof(int), MAXIMUM_ELECTRODES);
						if(map == NULL)
						{
							sprintf(message, "Could not allocate map array.");
							AddErrorToLog(message, __FILE__, __LINE__);
							MessagePopup("ERROR", message);
							return -1;
						}
						
						IsListItemChecked (panel, BLOCKLIST_CHANNEL, channel, &channelSelected); 
						if(channelSelected == 1)
						{
							status = GetChipMap(channel, map);
							if(status != 0)
							{
								free(map);
								sprintf(message, "Hardware error reading chip map (status %d).", status);
								AddErrorToLog(message, __FILE__, __LINE__);
								MessagePopup("ERROR", message);
								return -1;
							}
							
							//output raw values being sent to chip
							sprintf(message, "Read from channel %d", channel+1);
							AddToDebugOutput(message);
							WriteMapToDebugWindow(channel, map);
	
							UpdateChipView(map);
						}
						
						free(map);
						map = NULL;
					}
					break;
			}
			if(map)
			{
				free(map);
			}
			break;
		}
	return 0;
}

int CVICALLBACK EditPatternList (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int index = 0;
	int status = 0;
	char message[MAX_MESSAGE_SIZE] = {0};
	char filename[512] = {0};
	static char lastDirectory[512] = {0};
	
	VoltageHardware vProfile;
	PatternDefinition selectedBlock;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlIndex(panel, BLOCKLIST_PATTERNS, &index);
		
			switch(control)
			{
				case BLOCKLIST_DELETE:
					if(index < 0)
					{
						return 0;
					}
					status = DeletePattern(index+1);
					if(status != 0)
					{
						sprintf(message, "Error deleting pattern from block definition list.", status);
						AddErrorToLog(message, __FILE__, __LINE__);
						MessagePopup("ERROR", message);
						return 0;
					}
			
					DeleteListItem (panel, BLOCKLIST_PATTERNS, index, 1);
					
					//still need to deal with definition panel, which could be displaying deleted data
					break;
				case BLOCKLIST_ADDVOLTAGE:
					//select a filename to load data
					status = FileSelectPopup (lastDirectory, "*.vcfg", "*.vcfg", LOAD_VOLTAGE_CONFIG_STRING, VAL_LOAD_BUTTON,
						0, 1, 1, 1, filename);
					if(status == VAL_NO_FILE_SELECTED)
					{
						return 0;
					}
					
					//load voltage configuration to access voltage description
					LoadVoltageDataFile(filename, &vProfile);
					
					//get pattern data to access block description
					if(GetPatternData(&selectedBlock, index+1) != 0)
					{
						return 0;
					}
					
					//replace pattern item in list to contain timing file
					strcpy(selectedBlock.voltageFilename, filename);
					status = InsertPattern(selectedBlock, selectedBlock.patternId);
					if(status != 0)
					{
						sprintf(message, "Error attaching timing file to block definition.", status);
						AddErrorToLog(message, __FILE__, __LINE__);
						MessagePopup("ERROR", message);
						return 0;
					}
					
					//create new tag containing voltage timing file
					sprintf(message, "%s\033p150l%s", selectedBlock.description, vProfile.description);
					ReplaceListItem (panel, BLOCKLIST_PATTERNS, index, message, selectedBlock.patternId);
					
					break;
				default:
					return 1;
			}	

			break;
		}
	return 0;
}

/***********************************

Block Definition Main panel functions

***********************************/

int CVICALLBACK ChipImageCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int row = 0;
	int column = 0;
	int leftButtonDown = 0;
	int rightButtonDown = 0;
	int keyModifiers = 0;
	
	switch (event)
		{
		case EVENT_LEFT_CLICK:
			GetRelativeMouseState (panel, control, &column, &row, &leftButtonDown, &rightButtonDown, &keyModifiers);
			
			row /= PIXELS_PER_ROW;
			row++;
			column /= PIXELS_PER_COLUMN;
			column++;
			SetCtrlVal(panel, BLOCKDEF_ROW, row);
			SetCtrlVal(panel, BLOCKDEF_COLUMN, column);
			return 1;
		}
	return 0;
}

int SetElectrodePositionTimer(double newInterval)
{
	SetCtrlAttribute(patternPanel, BLOCKDEF_ELECTRODETIMER, ATTR_INTERVAL, newInterval);
	return 1;
}

int CVICALLBACK ElectrodeTimer (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int row = 0;
	int column = 0;
	int leftButtonDown = 0;
	int rightButtonDown = 0;
	int keyModifiers = 0;
	
	switch (event)
		{
		case EVENT_TIMER_TICK:
			GetRelativeMouseState (patternPanel, BLOCKDEF_CHIPIMAGE, &column, &row, &leftButtonDown, &rightButtonDown, &keyModifiers);
			
			row /= PIXELS_PER_ROW;
			row++;
			column /= PIXELS_PER_COLUMN;
			column++;
			
			if(row < 1 || row > 64 || column < 1 || column > 16)
			{
				return 1;
			}
			SetCtrlVal(patternPanel, BLOCKDEF_ROW, row);
			SetCtrlVal(patternPanel, BLOCKDEF_COLUMN, column);
			
			break;
		}
	return 0;
}

/***********************************

Chip Test functions

***********************************/


int CVICALLBACK ChipTestPanelCB (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:

			break;
		}
	return 0;
}

/***********************

Chip Testing Functions

***********************/

int CVICALLBACK AddressTest (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int *map = NULL;
	int *readMap = NULL;
	int *resultMap = NULL;
	
	int channel = 0;
	int status = 0;
	int channelSelected = 0;
	int testIndex = 0;
	char message[128] = {0};
	
	switch (event)
		{
		case EVENT_COMMIT:
			map = calloc(sizeof(int), MAXIMUM_ELECTRODES);
			if(map == NULL)
			{
				sprintf(message, "Could not allocate map array.");
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return -1;
			}
			
			readMap = calloc(sizeof(int), MAXIMUM_ELECTRODES);
			if(readMap == NULL)
			{
				free(map);
				sprintf(message, "Could not allocate map array.");
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return -1;
			}
			
			resultMap = calloc(sizeof(int), MAXIMUM_ELECTRODES);
			if(readMap == NULL)
			{
				free(map);
				free(readMap);
				sprintf(message, "Could not allocate map array.");
				AddErrorToLog(message, __FILE__, __LINE__);
				MessagePopup("ERROR", message);
				return -1;
			}
			
			ResetMiniChipViews();
			
			//set patterns to chip, read them back.
			for(channel = 0; channel < NUMBER_CHANNELS; channel++)
			{
				IsListItemChecked (panel, CHIPTEST_CHANNEL, channel, &channelSelected); 
				if(channelSelected == 1)
				{
					for(testIndex = 0; testIndex < NUMBER_TEST_PATTERNS; testIndex++)
					{
						sprintf(message, "Channel %d Test %d/7", channel, testIndex+1);
						SetCtrlVal(panel, CHIPTEST_STATUS, message);

						switch(testIndex)
						{
							case 0:	//vline 0
								MakeSolidPattern(&(*map), MAXIMUM_ROWS, MAXIMUM_COLUMNS, testIndex);
								break;
							case 1:	//vline 1
								MakeSolidPattern(&(*map), MAXIMUM_ROWS, MAXIMUM_COLUMNS, testIndex);
								break;
							case 2:	//vline 2
								MakeSolidPattern(&(*map), MAXIMUM_ROWS, MAXIMUM_COLUMNS, testIndex);
								break;
							case 3:	//vline 3
								MakeSolidPattern(&(*map), MAXIMUM_ROWS, MAXIMUM_COLUMNS, testIndex);
								break;
							case 4:
								MakeSolidPattern(&(*map), MAXIMUM_ROWS, MAXIMUM_COLUMNS, 1);
								MakeCheckerboard(&(*map), MAXIMUM_ROWS, MAXIMUM_COLUMNS, 0, VLINE_2);
								break;
							case 5:
								MakeSolidPattern(&(*map), MAXIMUM_ROWS, MAXIMUM_COLUMNS, 1);
								MakeCheckerboard(&(*map), MAXIMUM_ROWS, MAXIMUM_COLUMNS, 1, VLINE_2);
								break;
							case 6:
								MakeRandomPattern(&(*map), MAXIMUM_ROWS, MAXIMUM_COLUMNS);
								break;
							default:
								MessagePopup("ERROR", "Invalid Test");
								return 0;
						}
						
						//set chip
						status = SetChipMap(channel, map);
						if(status != 0)
						{
							free(map);
							free(readMap);
							free(resultMap);
							sprintf(message, "Hardware error setting chip map (status %d).", status);
							AddErrorToLog(message, __FILE__, __LINE__);
							MessagePopup("ERROR", message);
							SetCtrlVal(panel, CHIPTEST_STATUS, message);
							return -1;
						}
					
						//read map
						status = GetChipMap(channel, readMap);
						if(status != 0)
						{
							free(map);
							free(readMap);
							free(resultMap);
							sprintf(message, "Hardware error reading chip map (status %d).", status);
							AddErrorToLog(message, __FILE__, __LINE__);
							MessagePopup("ERROR", message);
							SetCtrlVal(panel, CHIPTEST_STATUS, message);
							return -1;
						}
					
						//output raw values being sent to chip
						sprintf(message, "Writing to channel %d", channel+1);
						AddToDebugOutput(message);
						WriteMapToDebugWindow(channel, map);
						
						//output raw values being sent to chip
						sprintf(message, "Read from channel %d", channel+1);
						AddToDebugOutput(message);
						WriteMapToDebugWindow(channel, readMap);

						//compare maps
						CompareMaps(&(*resultMap), map, readMap, (MAXIMUM_ROWS*MAXIMUM_COLUMNS));
						UpdateMiniChipView(resultMap, channel);
					}
				}
			}
			SetCtrlVal(panel, CHIPTEST_STATUS, "Ready");
			break;
		}
	return 0;
}

int CVICALLBACK SetDelayCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int delay = 0;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(panel, CHIPTEST_DELAY, &delay);
			SetChipHardwareDelay(delay);

			break;
		}
	return 0;
}


int CVICALLBACK MiniChipCanvasCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			break;
		}
	return 0;
}

int ResetMiniChipViews(void)
{
	int channel = 0;
	int miniChip[] = {CHIPTEST_MINICHIP_1, CHIPTEST_MINICHIP_2, CHIPTEST_MINICHIP_3, CHIPTEST_MINICHIP_4};

	for(channel = 0; channel < NUMBER_CHANNELS; channel++)
	{
		CanvasClear (chipTestPanel, miniChip[channel], 
		MakeRect(0, 0, (MAXIMUM_ROWS*PIXELS_PER_ROW_MINI), (MAXIMUM_COLUMNS*PIXELS_PER_COLUMN_MINI)));
	}	
	return 0;
}	

int UpdateMiniChipView(int *pattern, int channel)
{
	int row = 0;
	int column = 0;
	int index = 0;
	int fillColor = 0;
	
	int miniChip[] = {CHIPTEST_MINICHIP_1, CHIPTEST_MINICHIP_2, CHIPTEST_MINICHIP_3, CHIPTEST_MINICHIP_4};

	SetCtrlAttribute (chipTestPanel, miniChip[channel], ATTR_PEN_COLOR, VAL_BLACK);

	CanvasStartBatchDraw (chipTestPanel, miniChip[channel]);

	for(row = 0; row < MAXIMUM_ROWS; row++)
	{
		for(column = 0; column < MAXIMUM_COLUMNS; column++)
		{
			if(pattern[index] == VLINE_0)
			{
				fillColor = VAL_DK_GRAY;
			}
			else if(pattern[index] == VLINE_1)
			{
				fillColor = VAL_BLUE;
			}
			else if(pattern[index] == VLINE_2)
			{
				fillColor = VAL_YELLOW;
			}
			else if(pattern[index] == VLINE_3)
			{
				fillColor = VAL_GREEN;
			}
			else
			{
				fillColor = VAL_RED;
			}
			
			SetCtrlAttribute (chipTestPanel, miniChip[channel], ATTR_PEN_FILL_COLOR, fillColor);
		
			CanvasDrawRect (chipTestPanel, miniChip[channel], 
				MakeRect((row*PIXELS_PER_ROW_MINI), (column*PIXELS_PER_COLUMN_MINI), PIXELS_PER_ROW_MINI, PIXELS_PER_COLUMN_MINI),
				VAL_DRAW_FRAME_AND_INTERIOR);
			
			index++;
		}	
	}

	CanvasEndBatchDraw (chipTestPanel, miniChip[channel]);

	return 0;
}

/****************************************

	Voltage Configuration
	
****************************************/

int CVICALLBACK VoltagePanelCB (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:

			break;
		}
	return 0;
}

int CVICALLBACK SetVoltageCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double voltage = 0.0;
	int channel = 0;
	char message[128] = {0};
	
	switch (event)
		{
		case EVENT_COMMIT:
			switch(control)
			{
				case VOLTAGE_SETVOLTAGE:
					GetCtrlVal(panel, VOLTAGE_VSTART, &voltage);
					channel = 0;
					SetDacVoltage(channel, voltage);
					sprintf(message, "Set Dac channel %d to %1.3fV", channel, voltage);
					AddToLog(message);
					AddToDebugOutput(message);
					break;
				case VOLTAGE_SETVOLTAGE_2:
					GetCtrlVal(panel, VOLTAGE_VSTART_2, &voltage);
					channel = 1;
					SetDacVoltage(channel, voltage);
					sprintf(message, "Set Dac channel %d to %1.3fV", channel, voltage);
					AddToLog(message);
					AddToDebugOutput(message);
					break;
			}
			break;
		}
	return 0;
}

int CVICALLBACK SelectVoltageCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:

			break;
		}
	return 0;
}

int CVICALLBACK StartVoltageCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int status = 0;
	char message[128] = {0};
	VoltageHardware profile;
	
	switch (event)
		{
		case EVENT_COMMIT:
			switch(control)
			{
				case VOLTAGE_ON:
					status = GetVoltagePanelData(&profile);
					if(status != 0)
					{
						return 0;	
					}
					status = StartVoltageSeries(profile);
					if(status != 0)
					{
						return 0;	
					}
					SetCtrlVal(voltagePanel, VOLTAGE_STATUS, "Running Voltage Series");
					MonitorVoltageThreads();
					break;
				case VOLTAGE_OFF:
					SetDacVoltage(0, 0.0);
					SetDacVoltage(1, 0.0);
					sprintf(message, "Setting all DAC channels off");
					AddToLog(message);
					AddToDebugOutput(message);
					StopVoltageSeries();
					break;
			}
			break;
		}
	return 0;
}

void MonitorVoltageThreads(void)
{
	CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, (ThreadFunctionPtr) WatchVoltageThreads, NULL, &voltageWatchThreadFunctionID);		
}

int CVICALLBACK WatchVoltageThreads (void *functionData)
{
	int index = 0;
	int functionStatus = 0;
	int status = -1;
	int checkThreadFlag[NUMBER_DAC_CHANNELS] = {0};
	int numberVoltageThreads = NUMBER_DAC_CHANNELS;
	int threadId = 0;
	char message[256] = {0};
	
	for(;;)
	{
		if(numberVoltageThreads == 0)
		{
			break;
		}
		
		for(index = 0; index < NUMBER_DAC_CHANNELS; index++)
		{
			if(numberVoltageThreads == 0)
			{
				break;
			}
			if(checkThreadFlag[index] != -1)
			{
				threadId = GetVoltageThreadID(index);
				if(threadId != -1)
				{
					functionStatus = CmtGetThreadPoolFunctionAttribute (DEFAULT_THREAD_POOL_HANDLE, threadId, ATTR_TP_FUNCTION_EXECUTION_STATUS, &status);					
					if(functionStatus != 0)
					{
						CmtGetErrorMessage (functionStatus, message);
						MessagePopup("ERROR", message);
					}
				}
				switch(status)
				{
					case 3:
					case 4:
					case 5:
						CmtReleaseThreadPoolFunctionID (DEFAULT_THREAD_POOL_HANDLE, threadId);
						checkThreadFlag[index] = -1;
						numberVoltageThreads--;
						sprintf(message, "Voltage thread %d was terminated with status %d", index, status);
						AddToLog(message);
						AddToDebugOutput(message);
						break;
					default:
						//thread still active
						continue;
				}
				/*if(status == 3 || status == 4 || status == 5 || threadId == -1)
				{
					checkThreadFlag[index] = -1;
					numberVoltageThreads--;
					sprintf(message, "Voltage thread %d was terminated with status %d", index, status);
					AddToLog(message);
					AddToDebugOutput(message);
				}*/
			}
		}
	}
	SetCtrlVal(voltagePanel, VOLTAGE_STATUS, "Ready");
	if(voltageWatchThreadFunctionID != 0)
	{
		CmtReleaseThreadPoolFunctionID (DEFAULT_THREAD_POOL_HANDLE, voltageWatchThreadFunctionID);
	}
	
	return 0;
}

int GetVoltagePanelData(VoltageHardware *profile)
{
	GetCtrlVal(voltagePanel, VOLTAGE_DESCRIPTION, profile->description);
	
	//Get data for channel 1;
	profile->dacChannel[0].id = 0;
	GetCtrlVal(voltagePanel, VOLTAGE_PROFILE, &profile->dacChannel[0].vProfile);
	GetCtrlVal(voltagePanel, VOLTAGE_VSTART, &profile->dacChannel[0].vStart);
	GetCtrlVal(voltagePanel, VOLTAGE_VA, &profile->dacChannel[0].vA);
	GetCtrlVal(voltagePanel, VOLTAGE_VB, &profile->dacChannel[0].vB);
	GetCtrlVal(voltagePanel, VOLTAGE_RAMPRATE, &profile->dacChannel[0].ramprate);
	GetCtrlVal(voltagePanel, VOLTAGE_ONDURATION, &profile->dacChannel[0].onDuration);
	GetCtrlVal(voltagePanel, VOLTAGE_OFFDURATION, &profile->dacChannel[0].offDuration);
	GetCtrlVal(voltagePanel, VOLTAGE_CONTINOUS, &profile->dacChannel[0].continous);	
	GetCtrlVal(voltagePanel, VOLTAGE_TOTALDURATION, &profile->dacChannel[0].totalDuration);
	
	//Get data for channel 2;
	profile->dacChannel[1].id = 1;
	GetCtrlVal(voltagePanel, VOLTAGE_PROFILE_2, &profile->dacChannel[1].vProfile);
	GetCtrlVal(voltagePanel, VOLTAGE_VSTART_2, &profile->dacChannel[1].vStart);
	GetCtrlVal(voltagePanel, VOLTAGE_VA_2, &profile->dacChannel[1].vA);
	GetCtrlVal(voltagePanel, VOLTAGE_VB_2, &profile->dacChannel[1].vB);
	GetCtrlVal(voltagePanel, VOLTAGE_RAMPRATE_2, &profile->dacChannel[1].ramprate);
	GetCtrlVal(voltagePanel, VOLTAGE_ONDURATION_2, &profile->dacChannel[1].onDuration);
	GetCtrlVal(voltagePanel, VOLTAGE_OFFDURATION_2, &profile->dacChannel[1].offDuration);
	GetCtrlVal(voltagePanel, VOLTAGE_CONTINOUS_2, &profile->dacChannel[1].continous);	
	GetCtrlVal(voltagePanel, VOLTAGE_TOTALDURATION_2, &profile->dacChannel[1].totalDuration);
	
	return 0;
}

int SetVoltagePanelData(VoltageHardware profile)
{
	SetCtrlVal(voltagePanel, VOLTAGE_DESCRIPTION, profile.description);
	
	//Get data for channel 1;
	SetCtrlVal(voltagePanel, VOLTAGE_PROFILE, profile.dacChannel[0].vProfile);
	SetCtrlVal(voltagePanel, VOLTAGE_VSTART, profile.dacChannel[0].vStart);
	SetCtrlVal(voltagePanel, VOLTAGE_VA, profile.dacChannel[0].vA);
	SetCtrlVal(voltagePanel, VOLTAGE_VB, profile.dacChannel[0].vB);
	SetCtrlVal(voltagePanel, VOLTAGE_RAMPRATE, profile.dacChannel[0].ramprate);
	SetCtrlVal(voltagePanel, VOLTAGE_ONDURATION, profile.dacChannel[0].onDuration);
	SetCtrlVal(voltagePanel, VOLTAGE_OFFDURATION, profile.dacChannel[0].offDuration);
	SetCtrlVal(voltagePanel, VOLTAGE_CONTINOUS, profile.dacChannel[0].continous);	
	SetCtrlVal(voltagePanel, VOLTAGE_TOTALDURATION, profile.dacChannel[0].totalDuration);
	
	//Get data for channel 2;
	SetCtrlVal(voltagePanel, VOLTAGE_PROFILE_2, profile.dacChannel[1].vProfile);
	SetCtrlVal(voltagePanel, VOLTAGE_VSTART_2, profile.dacChannel[1].vStart);
	SetCtrlVal(voltagePanel, VOLTAGE_VA_2, profile.dacChannel[1].vA);
	SetCtrlVal(voltagePanel, VOLTAGE_VB_2, profile.dacChannel[1].vB);
	SetCtrlVal(voltagePanel, VOLTAGE_RAMPRATE_2, profile.dacChannel[1].ramprate);
	SetCtrlVal(voltagePanel, VOLTAGE_ONDURATION_2, profile.dacChannel[1].onDuration);
	SetCtrlVal(voltagePanel, VOLTAGE_OFFDURATION_2, profile.dacChannel[1].offDuration);
	SetCtrlVal(voltagePanel, VOLTAGE_CONTINOUS_2, profile.dacChannel[1].continous);	
	SetCtrlVal(voltagePanel, VOLTAGE_TOTALDURATION_2, profile.dacChannel[1].totalDuration);
	
	return 0;
}

int SetTiledDescriptionName(PatternDefinition *block, int tileRow, int tileColumn)
{
	char *token = NULL;
	
	token = strtok(block->description, "_");
	if(token != NULL)
	{
		sprintf(block->description, "%s_%d_%c", token, tileRow, (int)'A' + tileColumn);	
	}
	return 1;
}

/*
  Batch run stuff.  It is here so there is easy access to all UI values
*/
int StartBatchRunThread(void)
{
	int status = 0;
	int threadFunctionID = 0;
	
	status = CmtScheduleThreadPoolFunctionAdv (DEFAULT_THREAD_POOL_HANDLE, 
		(ThreadFunctionPtr) batchRunThreadFunction, NULL, DEFAULT_THREAD_PRIORITY,
		(ThreadFunctionCallbackPtr) batchRunCallbackFunction, (EVENT_TP_THREAD_FUNCTION_BEGIN | EVENT_TP_THREAD_FUNCTION_END),
		NULL, RUN_IN_SCHEDULED_THREAD, &threadFunctionID);
	return status;
}

int StartBatchRunSingleThread(void)
{
	int status = 0;
	int threadFunctionID = 0;
	
	status = CmtScheduleThreadPoolFunctionAdv (DEFAULT_THREAD_POOL_HANDLE, 
		(ThreadFunctionPtr) batchRunSingleThreadFunction, NULL, DEFAULT_THREAD_PRIORITY,
		(ThreadFunctionCallbackPtr) batchRunSingleCallbackFunction, (EVENT_TP_THREAD_FUNCTION_BEGIN | EVENT_TP_THREAD_FUNCTION_END),
		NULL, RUN_IN_SCHEDULED_THREAD, &threadFunctionID);
	return status;
}

int CVICALLBACK batchRunThreadFunction (void *functionData)
{
	int status = 0;
	int index = 0;
	int temp = 0;
	
	int blockIndex = 0;				 	//index of block from block list control
	int numberBlocksSelected = 0;	 	//number of selected "checked" blocks
	int numberBlocksListed = 0;			//number of block in list box
	int *selectedBlockList = NULL;		//list of block indicies that are selected
	
	int channelIndex = 0;				//channel index
	int numberChannelsSelected = 0;   	//get number of channels selected
	int numberChannelsListed = 0;		//number of channels listed, will always be the same
	int *selectedChannelList = NULL;	//list of channel indicies that are selected
	
	int abortFlag = 0;					//flag which will abort process if it is set
	
	int voltageWatchThreadFunctionID = 0; //thread ID to wait for voltage channels to finish pulsing
	
	int numberKeithleySamples = 0;		//actual number of samples gathered during acquisition
	int dataIndex = 0;					//index into keithleyDataBuffer
	
	int floatVline = 0;					//state to set entire chip to between blocks
	
	double longestDuration = 0.0;		//longest pulsing duration for keithley sampling time
	double keithleyDataBuffer[2500] = {0};		//buffer containing the acquired keithley data
	
	double blockDelay = 0.0;			//delay between blocks
	
	char dataDirectory[512] = {0};
	char dataFilename[512] = {0};
	char tempstr[256] = {0};
	char message[256] = {0};
	
	FILE *fp = NULL;					//file pointer for data files
	PatternDefinition block;			//container for block definition
	VoltageHardware   vConfig;			//container for voltage/timing definition
	
	//this will handle going through each block and voltage/timing configuration pair
	//that is selected
	
	//get number of blocks that are selected
	GetNumCheckedItems (blockListPanel, BLOCKLIST_PATTERNS, &numberBlocksSelected);
	if(numberBlocksSelected <= 0)
	{
		abortFlag = 1;
		sprintf(message, "No blocks were selected");
		goto Error;
	}
	
	//allocate memory for selected block list
	selectedBlockList = calloc(sizeof(int), numberBlocksSelected);
	if(selectedBlockList == NULL)
	{
		sprintf(message, "Could not allocate 'selectedBlockList' array.");
		AddErrorToLog(message, __FILE__, __LINE__);
		abortFlag = 1;
		goto Error;
	}
	
	//get list of blocks that are selected
	GetNumListItems (blockListPanel, BLOCKLIST_PATTERNS, &numberBlocksListed);
	temp = 0;
	for(index = 0; index < numberBlocksListed; index++)
	{
		//if list item is checked, place it in our selected block list
		IsListItemChecked (blockListPanel, BLOCKLIST_PATTERNS, index, &status);
		if(status == 1)
		{
			selectedBlockList[temp] = index;
			temp++;
		}

	}
	
	//get number of channel selected
	GetNumCheckedItems (blockListPanel, BLOCKLIST_CHANNEL, &numberChannelsSelected);
	if(numberChannelsSelected <= 0)
	{
		abortFlag = 1;
		sprintf(message, "No channels were selected");
		goto Error;
	}
	
	//allocate memory for selected channel list
	selectedChannelList = calloc(sizeof(int), numberChannelsSelected);
	if(selectedChannelList == NULL)
	{
		sprintf(message, "Could not allocate 'selectedChannelList' array.");
		AddErrorToLog(message, __FILE__, __LINE__);
		abortFlag = 1;
		goto Error;
	}
	
	//get list of channels that are selected
	GetNumListItems (blockListPanel, BLOCKLIST_CHANNEL, &numberChannelsListed);
	temp = 0;
	for(index = 0; index < numberChannelsListed; index++)
	{
		//if list item is checked, place it in our selected block list
		IsListItemChecked (blockListPanel, BLOCKLIST_CHANNEL, index, &status);
		if(status == 1)
		{
			selectedChannelList[temp] = index;
			temp++;
		}

	}
	
	//get directory for data to be placed
	status = DirSelectPopup ("", BLOCK_DATA_DIRECTORY, 1, 1, dataDirectory);
	if(status == VAL_NO_DIRECTORY_SELECTED)
	{
		goto Error;
	}
	
	//get block delay and float vline
	GetCtrlVal(blockListPanel, BLOCKLIST_BLOCKDELAY, &blockDelay);
	GetCtrlVal(blockListPanel, BLOCKLIST_FLOATVLINE, &floatVline);
	
	//update batch overview
	//clear any plots from a previous run
	DeleteGraphPlot (batchViewPanel, BATCHVIEW_GRAPH, -1, VAL_IMMEDIATE_DRAW );
			
	//iterate through blocks and channels to be done
	for(blockIndex = 0; blockIndex < numberBlocksSelected; blockIndex++)
	{
		for(channelIndex = 0; channelIndex < numberChannelsSelected; channelIndex++)
		{
			//load the block definition
			//NOTE: the block definition list is 1-indexed
			status = GetPatternData(&block, selectedBlockList[blockIndex]+1);
			if(status != 0)
			{
				sprintf(message, "Could not get block %d pattern definition.", selectedBlockList[blockIndex]+1);
				AddErrorToLog(message, __FILE__, __LINE__);
				abortFlag = 1;
				goto Error;
			}
			
			//load voltage/timing configuration
			status = LoadVoltageDataFile(block.voltageFilename, &vConfig);
			if(status != 0)
			{
				sprintf(message, "Could not load voltage configuration from block pattern definition.\nBlock id: %d, description: %s",
					block.patternId, block.description);
				
				AddErrorToLog(message, __FILE__, __LINE__);
				abortFlag = 1;
				goto Error;
			}
			
			//update batch overview
			SetCtrlVal(batchViewPanel, BATCHVIEW_CURRENTBLOCK, block.description);
			sprintf(tempstr, "%d (write in progress)", selectedChannelList[channelIndex]+1);
			SetCtrlVal(batchViewPanel, BATCHVIEW_CURRENTCHANNEL, tempstr);
			SetCtrlVal(batchViewPanel, BATCHVIEW_TIMINGCFG, vConfig.description);
			SetCtrlVal(batchViewPanel, BATCHVIEW_STARTTIME, "");
			SetCtrlVal(batchViewPanel, BATCHVIEW_DURATION, "");
			DisplayPanel(batchViewPanel);
			
			//set chip
			//output raw values being sent to chip
			sprintf(message, "Writing to channel %d", selectedChannelList[channelIndex]+1);
			AddToDebugOutput(message);
			WriteMapToDebugWindow(selectedChannelList[channelIndex], block.map);
			UpdateChipView(block.map);
			status = SetChipMap(selectedChannelList[channelIndex], block.map);
			if(status != 0)
			{
				sprintf(message, "Hardware error setting chip map on channel %d (status %d).",
					selectedChannelList[channelIndex]+1, status);
				AddErrorToLog(message, __FILE__, __LINE__);
				abortFlag = 1;
				goto Error;
			}
			
			//update batch overview
			sprintf(tempstr, "%d", selectedChannelList[channelIndex]+1);
			SetCtrlVal(batchViewPanel, BATCHVIEW_CURRENTCHANNEL, tempstr);
			SetCtrlVal(batchViewPanel, BATCHVIEW_STARTTIME, TimeStr());
			
			//start Keithley
			//get the longest duration for Keithley sampling (need to factor in the off duration)
			longestDuration = vConfig.dacChannel[0].totalDuration +
			   (vConfig.dacChannel[0].totalDuration / vConfig.dacChannel[0].onDuration) * vConfig.dacChannel[0].offDuration;

			
			//update batch overview
			sprintf(tempstr, "%2.3f seconds", longestDuration);
			SetCtrlVal(batchViewPanel, BATCHVIEW_DURATION, tempstr);
			
			//get measurements started 
			status = StartSampleRead();	   
			if(status != 0)
			{
				GetLastResult(status, message);
				abortFlag = 1;
				goto Error;
			}
			
			//start voltage
			status = StartVoltageSeries(vConfig);
			if(status != 0)
			{
				abortFlag = 1;
				goto Error;	
			}
			SetCtrlVal(voltagePanel, VOLTAGE_STATUS, "Running Voltage Series");
			
			//wait for voltage to complete
			CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, (ThreadFunctionPtr) WatchVoltageThreads, NULL, &voltageWatchThreadFunctionID);		
			CmtWaitForThreadPoolFunctionCompletion (DEFAULT_THREAD_POOL_HANDLE, voltageWatchThreadFunctionID, OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
			CmtReleaseThreadPoolFunctionID (DEFAULT_THREAD_POOL_HANDLE, voltageWatchThreadFunctionID);
			
			//get Keithley data
			StopAndFetchSampleRead(keithleyDataBuffer, &numberKeithleySamples);
			
			//save Keithley data
			sprintf(dataFilename, "%s\\%s_ch%d_block%d_%s_%s.log", dataDirectory, DateStr(), selectedChannelList[channelIndex]+1,
				selectedBlockList[blockIndex]+1, block.description, vConfig.description);
			
			fp = fopen(dataFilename, "wt");
			if(fp != NULL)
			{
				for(dataIndex = 0; dataIndex < numberKeithleySamples; dataIndex++)
				{
					fprintf(fp, "%15.15e\n", keithleyDataBuffer[dataIndex]);
				}
				//fprintf(fp, "%d\n", status);
			   	fflush(fp);
				fclose(fp);
			}
			
			//update batch overview
			//plot graph with data gathered
			DeleteGraphPlot (batchViewPanel, BATCHVIEW_GRAPH, -1, VAL_IMMEDIATE_DRAW );
			if(numberKeithleySamples > 0)
			{
				PlotY (batchViewPanel, BATCHVIEW_GRAPH, keithleyDataBuffer, numberKeithleySamples-10, VAL_DOUBLE,
					VAL_THIN_LINE, VAL_CONNECTED_POINTS, VAL_SOLID, VAL_CONNECTED_POINTS,
					VAL_RED);
			}
			
			SetCtrlVal(batchViewPanel, BATCHVIEW_CURRENTBLOCK, "Block complete -- Setting chip to float");
			
			sprintf(tempstr, "%d (write in progress)", selectedChannelList[channelIndex]+1);
			SetCtrlVal(batchViewPanel, BATCHVIEW_CURRENTCHANNEL, tempstr);
			SetCtrlVal(batchViewPanel, BATCHVIEW_TIMINGCFG, "");
			SetCtrlVal(batchViewPanel, BATCHVIEW_STARTTIME, "");
			SetCtrlVal(batchViewPanel, BATCHVIEW_DURATION, "");
			
		
			//set chip to float
			MakeSolidPattern(block.map, MAXIMUM_ROWS, MAXIMUM_COLUMNS, floatVline);
			UpdateChipView(block.map);
			status = SetChipMap(selectedChannelList[channelIndex], block.map);
			if(status != 0)
			{
				sprintf(message, "Hardware error setting chip map on channel %d (status %d).",
					selectedChannelList[channelIndex]+1, status);
				AddErrorToLog(message, __FILE__, __LINE__);
				abortFlag = 1;
				goto Error;
			}
			
			//update batch overview
			sprintf(tempstr, "%d", selectedChannelList[channelIndex]+1);
			SetCtrlVal(batchViewPanel, BATCHVIEW_CURRENTCHANNEL, tempstr);
			sprintf(tempstr, "Delaying for %2.3f seconds", blockDelay);
			SetCtrlVal(batchViewPanel, BATCHVIEW_DURATION, tempstr);
			
			//wait for a little bit
			Delay(blockDelay);
			//do next channel
			
			
			
		}
	}

Error:
	if(abortFlag == 1)
	{
		//display error message if there is one
		if(strlen(message) > 0)
		{
			MessagePopup("ERROR", message);
			AddErrorToLog(message, __FILE__, __LINE__);
		}
		
		//free any allocated memory
		if(selectedBlockList) free(selectedBlockList);
		if(selectedChannelList) free(selectedChannelList);
		
		//turn off voltage
		
		//turn off Keithley
	}
	
	CmtExitThreadPoolThread (1);
	return 1;
}

void CVICALLBACK batchRunCallbackFunction (int poolHandle, int functionID, 
	unsigned int event, int value, void *callbackData)
{
	switch(event)
	{
		case EVENT_TP_THREAD_FUNCTION_BEGIN:
			break;
		case EVENT_TP_THREAD_FUNCTION_END:
			SetCtrlVal(batchViewPanel, BATCHVIEW_CURRENTBLOCK, "Batch Complete");
			SetCtrlVal(batchViewPanel, BATCHVIEW_CURRENTCHANNEL, "");
			SetCtrlVal(batchViewPanel, BATCHVIEW_TIMINGCFG, "");
			SetCtrlVal(batchViewPanel, BATCHVIEW_STARTTIME, "");
			SetCtrlVal(batchViewPanel, BATCHVIEW_DURATION, "");
			
			//free thread resources
			CmtReleaseThreadPoolFunctionID (poolHandle, functionID);
			
			break;
	}
}

int CVICALLBACK batchRunSingleThreadFunction (void *functionData)
{
	int status = 0;
	int index = 0;
	int temp = 0;
	
	int blockIndex = 0;				 	//index of block from block list control
	int numberBlocksSelected = 0;	 	//number of selected "checked" blocks
	int numberBlocksListed = 0;			//number of block in list box
	int *selectedBlockList = NULL;		//list of block indicies that are selected
	
	int channelIndex = 0;				//channel index
	int numberChannelsSelected = 0;   	//get number of channels selected
	int numberChannelsListed = 0;		//number of channels listed, will always be the same
	int *selectedChannelList = NULL;	//list of channel indicies that are selected
	
	int abortFlag = 0;					//flag which will abort process if it is set
	
	int voltageWatchThreadFunctionID = 0; //thread ID to wait for voltage channels to finish pulsing
	
	int numberKeithleySamples = 0;		//actual number of samples gathered during acquisition
	int dataIndex = 0;					//index into keithleyDataBuffer
	
	int floatVline = 0;					//state to set entire chip to between blocks
	
	int electrodeIndex = 0;				//index of electrode within block
	int numberElectrodes = 0;			//area of block in electrodes
	int row = 0;						//step through row of block map
	int column = 0;						//step through column of block map
	int chip[MAXIMUM_ELECTRODES] = {0};	//holds a chip map
	
	double longestDuration = 0.0;		//longest pulsing duration for keithley sampling time
	double keithleyDataBuffer[2500] = {0};		//buffer containing the acquired keithley data
	
	double blockDelay = 0.0;			//delay between blocks
	
	char dataDirectory[512] = {0};
	char dataFilename[512] = {0};
	char tempstr[256] = {0};
	char message[256] = {0};
	
	FILE *fp = NULL;					//file pointer for data files
	PatternDefinition block;			//container for block definition
	VoltageHardware   vConfig;			//container for voltage/timing definition
	
	//this will handle going through each block and voltage/timing configuration pair
	//that is selected
	
	//get number of blocks that are selected
	GetNumCheckedItems (blockListPanel, BLOCKLIST_PATTERNS, &numberBlocksSelected);
	if(numberBlocksSelected <= 0)
	{
		abortFlag = 1;
		sprintf(message, "No blocks were selected");
		goto Error;
	}
	
	//allocate memory for selected block list
	selectedBlockList = calloc(sizeof(int), numberBlocksSelected);
	if(selectedBlockList == NULL)
	{
		sprintf(message, "Could not allocate 'selectedBlockList' array.");
		AddErrorToLog(message, __FILE__, __LINE__);
		abortFlag = 1;
		goto Error;
	}
	
	//get list of blocks that are selected
	GetNumListItems (blockListPanel, BLOCKLIST_PATTERNS, &numberBlocksListed);
	temp = 0;
	for(index = 0; index < numberBlocksListed; index++)
	{
		//if list item is checked, place it in our selected block list
		IsListItemChecked (blockListPanel, BLOCKLIST_PATTERNS, index, &status);
		if(status == 1)
		{
			selectedBlockList[temp] = index;
			temp++;
		}

	}
	
	//get number of channel selected
	GetNumCheckedItems (blockListPanel, BLOCKLIST_CHANNEL, &numberChannelsSelected);
	if(numberChannelsSelected <= 0)
	{
		abortFlag = 1;
		sprintf(message, "No channels were selected");
		goto Error;
	}
	
	//allocate memory for selected channel list
	selectedChannelList = calloc(sizeof(int), numberChannelsSelected);
	if(selectedChannelList == NULL)
	{
		sprintf(message, "Could not allocate 'selectedChannelList' array.");
		AddErrorToLog(message, __FILE__, __LINE__);
		abortFlag = 1;
		goto Error;
	}
	
	//get list of channels that are selected
	GetNumListItems (blockListPanel, BLOCKLIST_CHANNEL, &numberChannelsListed);
	temp = 0;
	for(index = 0; index < numberChannelsListed; index++)
	{
		//if list item is checked, place it in our selected block list
		IsListItemChecked (blockListPanel, BLOCKLIST_CHANNEL, index, &status);
		if(status == 1)
		{
			selectedChannelList[temp] = index;
			temp++;
		}

	}
	
	//get directory for data to be placed
	status = DirSelectPopup ("", BLOCK_DATA_DIRECTORY, 1, 1, dataDirectory);
	if(status == VAL_NO_DIRECTORY_SELECTED)
	{
		goto Error;
	}
	
	//get block delay and float vline
	GetCtrlVal(blockListPanel, BLOCKLIST_BLOCKDELAY, &blockDelay);
	GetCtrlVal(blockListPanel, BLOCKLIST_FLOATVLINE, &floatVline);
	
	//iterate through blocks and channels to be done
	for(blockIndex = 0; blockIndex < numberBlocksSelected; blockIndex++)
	{
		for(channelIndex = 0; channelIndex < numberChannelsSelected; channelIndex++)
		{
			//clear chip
			MakeSolidPattern(chip, MAXIMUM_ROWS, MAXIMUM_COLUMNS, floatVline);
			UpdateChipView(chip);
			status = SetChipMap(selectedChannelList[channelIndex], chip);
			if(status != 0)
			{
				sprintf(message, "Hardware error setting chip map on channel %d (status %d).",
					selectedChannelList[channelIndex]+1, status);
				AddErrorToLog(message, __FILE__, __LINE__);
				abortFlag = 1;
				goto Error;
			}
			
			//load the block definition
			//NOTE: the block definition list is 1-indexed
			status = GetPatternData(&block, selectedBlockList[blockIndex]+1);
			if(status != 0)
			{
				sprintf(message, "Could not get block %d pattern definition.", selectedBlockList[blockIndex]+1);
				AddErrorToLog(message, __FILE__, __LINE__);
				abortFlag = 1;
				goto Error;
			}
			
			//load voltage/timing configuration
			status = LoadVoltageDataFile(block.voltageFilename, &vConfig);
			if(status != 0)
			{
				sprintf(message, "Could not load voltage configuration from block pattern definition.\nBlock id: %d, description: %s",
					block.patternId, block.description);
				
				AddErrorToLog(message, __FILE__, __LINE__);
				abortFlag = 1;
				goto Error;
			}
			
			//deturmine area of block in electodes
			numberElectrodes = block.numberRows * block.numberColumns;
			row = block.startRow-1;
			column = block.startColumn-1;
			
			//increment through the electrodes
			for(electrodeIndex = 0; electrodeIndex < numberElectrodes; electrodeIndex++)
			{
				//increment row and column index to move through block
				if(electrodeIndex > 0)
				{
					row++;
					if(row >= (block.startRow-1) + block.numberRows)
					{
						row = block.startRow-1;
						column++;
					}	
				}
				
				//see if this location is equal to a float value, if so, move on.
				if(block.map[MAXIMUM_COLUMNS * row + column] == floatVline)
				{
					continue;
				}	
				
				//update batch overview
				SetCtrlVal(batchViewPanel, BATCHVIEW_CURRENTBLOCK, block.description);
				sprintf(tempstr, "%d (write in progress)", selectedChannelList[channelIndex]+1);
				SetCtrlVal(batchViewPanel, BATCHVIEW_CURRENTCHANNEL, tempstr);
				SetCtrlVal(batchViewPanel, BATCHVIEW_TIMINGCFG, vConfig.description);
				SetCtrlVal(batchViewPanel, BATCHVIEW_STARTTIME, "");
				SetCtrlVal(batchViewPanel, BATCHVIEW_DURATION, "");
				DisplayPanel(batchViewPanel);
			
				//set chip
				//output raw values being sent to chip
				sprintf(message, "Writing to channel %d", selectedChannelList[channelIndex]+1);
				AddToDebugOutput(message);
				SetNextMeasurementSite(chip, channelIndex, row, column, block.map[MAXIMUM_COLUMNS * row + column], floatVline);
				UpdateChipView(chip);
				
				if(status != 0)
				{
					sprintf(message, "Hardware error setting chip map on channel %d (status %d).",
						selectedChannelList[channelIndex]+1, status);
					AddErrorToLog(message, __FILE__, __LINE__);
					abortFlag = 1;
					goto Error;
				}
			
				//update batch overview
				sprintf(tempstr, "%d", selectedChannelList[channelIndex]+1);
				SetCtrlVal(batchViewPanel, BATCHVIEW_CURRENTCHANNEL, tempstr);
				SetCtrlVal(batchViewPanel, BATCHVIEW_STARTTIME, TimeStr());
			
				//start Keithley
				//get the longest duration for Keithley sampling (need to factor in the off duration)
				longestDuration = vConfig.dacChannel[0].totalDuration +
				   (vConfig.dacChannel[0].totalDuration / vConfig.dacChannel[0].onDuration) * vConfig.dacChannel[0].offDuration;

			
				//update batch overview
				sprintf(tempstr, "%2.3f seconds", longestDuration);
				SetCtrlVal(batchViewPanel, BATCHVIEW_DURATION, tempstr);
			
				//get measurements started 
				status = StartSampleRead();	   
				if(status != 0)
				{
					GetLastResult(status, message);
					abortFlag = 1;
					goto Error;
				}
			
				//start voltage
				status = StartVoltageSeries(vConfig);
				if(status != 0)
				{
					abortFlag = 1;
					goto Error;	
				}
				SetCtrlVal(voltagePanel, VOLTAGE_STATUS, "Running Voltage Series");
			
				//wait for voltage to complete
				CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, (ThreadFunctionPtr) WatchVoltageThreads, NULL, &voltageWatchThreadFunctionID);		
				CmtWaitForThreadPoolFunctionCompletion (DEFAULT_THREAD_POOL_HANDLE, voltageWatchThreadFunctionID, OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
				CmtReleaseThreadPoolFunctionID (DEFAULT_THREAD_POOL_HANDLE, voltageWatchThreadFunctionID);
			
				//get Keithley data
				StopAndFetchSampleRead(keithleyDataBuffer, &numberKeithleySamples);
			
				//save Keithley data
				sprintf(dataFilename, "%s\\%s_ch%d_block%d_%s_%s_row%d_col%d.log", dataDirectory, DateStr(), selectedChannelList[channelIndex]+1,
					selectedBlockList[blockIndex]+1, block.description, vConfig.description, row, column);
			
				fp = fopen(dataFilename, "wt");
				if(fp != NULL)
				{
					for(dataIndex = 0; dataIndex < numberKeithleySamples; dataIndex++)
					{
						fprintf(fp, "%15.15e\n", keithleyDataBuffer[dataIndex]);
					}
					//fprintf(fp, "%d\n", status);
				   	fflush(fp);
					fclose(fp);
				}
			
				//update batch overview
				//plot graph with data gathered
				DeleteGraphPlot (batchViewPanel, BATCHVIEW_GRAPH, -1, VAL_IMMEDIATE_DRAW );
				if(numberKeithleySamples > 0)
				{
					PlotY (batchViewPanel, BATCHVIEW_GRAPH, keithleyDataBuffer, numberKeithleySamples, VAL_DOUBLE,
						VAL_THIN_LINE, VAL_CONNECTED_POINTS, VAL_SOLID, VAL_CONNECTED_POINTS,
						VAL_RED);
				}
			
				SetCtrlVal(batchViewPanel, BATCHVIEW_CURRENTBLOCK, "Block complete -- Setting chip to float");
			
				sprintf(tempstr, "%d (write in progress)", selectedChannelList[channelIndex]+1);
				SetCtrlVal(batchViewPanel, BATCHVIEW_CURRENTCHANNEL, tempstr);
				SetCtrlVal(batchViewPanel, BATCHVIEW_TIMINGCFG, "");
				SetCtrlVal(batchViewPanel, BATCHVIEW_STARTTIME, "");
				SetCtrlVal(batchViewPanel, BATCHVIEW_DURATION, "");
			
		
				//set chip to float
				MakeSolidPattern(chip, MAXIMUM_ROWS, MAXIMUM_COLUMNS, floatVline);
				UpdateChipView(chip);
				status = SetChipMap(selectedChannelList[channelIndex], chip);
				if(status != 0)
				{
					sprintf(message, "Hardware error setting chip map on channel %d (status %d).",
						selectedChannelList[channelIndex]+1, status);
					AddErrorToLog(message, __FILE__, __LINE__);
					abortFlag = 1;
					goto Error;
				}
			
				//update batch overview
				sprintf(tempstr, "%d", selectedChannelList[channelIndex]+1);
				SetCtrlVal(batchViewPanel, BATCHVIEW_CURRENTCHANNEL, tempstr);
				sprintf(tempstr, "Delaying for %2.3f seconds", blockDelay);
				SetCtrlVal(batchViewPanel, BATCHVIEW_DURATION, tempstr);
			
				//wait for a little bit
				Delay(blockDelay);
				//do next channel
			}
		}
	}

Error:
	if(abortFlag == 1)
	{
		//display error message if there is one
		if(strlen(message) > 0)
		{
			MessagePopup("ERROR", message);
			AddErrorToLog(message, __FILE__, __LINE__);
		}
		
		//free any allocated memory
		if(selectedBlockList) free(selectedBlockList);
		if(selectedChannelList) free(selectedChannelList);
		
		//turn off voltage
		
		//turn off Keithley
	}
	
	CmtExitThreadPoolThread (1);
	return 1;
}

void CVICALLBACK batchRunSingleCallbackFunction (int poolHandle, int functionID, 
	unsigned int event, int value, void *callbackData)
{
	switch(event)
	{
		case EVENT_TP_THREAD_FUNCTION_BEGIN:
			break;
		case EVENT_TP_THREAD_FUNCTION_END:
			SetCtrlVal(batchViewPanel, BATCHVIEW_CURRENTBLOCK, "Batch Complete");
			SetCtrlVal(batchViewPanel, BATCHVIEW_CURRENTCHANNEL, "");
			SetCtrlVal(batchViewPanel, BATCHVIEW_TIMINGCFG, "");
			SetCtrlVal(batchViewPanel, BATCHVIEW_STARTTIME, "");
			SetCtrlVal(batchViewPanel, BATCHVIEW_DURATION, "");
			
			//free thread resources
			CmtReleaseThreadPoolFunctionID (poolHandle, functionID);
			
			break;
	}
}

int CVICALLBACK BatchViewPanelCB (int panel, int event, void *callbackData,
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
