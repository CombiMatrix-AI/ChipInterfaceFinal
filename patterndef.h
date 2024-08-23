//patterndef.h
#ifndef PATTERNDEF_H_
	#define PATTERNDEF_H_
	
	//includes
	#include "chipcontrol.h"
	#include "toolbox.h"
	#include "inifile.h"
	#include "log.h"
	#include "voltage.h"

	//definitions
	#define MAX_LABEL_SIZE		128
	#define MAX_MESSAGE_SIZE	512
	#define MAX_COMMENTS_SIZE	1024
	#define MAX_FILENAME_SIZE	512
	
	#define CHECKERBOARD 		0

	#define FLOATVLINE			0
	
	typedef struct
	{
		char description[MAX_LABEL_SIZE];
		int patternId;
		
		int numberRows;
		int numberColumns;
		int startRow;
		int startColumn;
		int tileRow;
		int tileColumn;
		int map[MAXIMUM_ELECTRODES];
		char comments[MAX_COMMENTS_SIZE];
		char filename[MAX_FILENAME_SIZE];
		char voltageFilename[MAX_FILENAME_SIZE];
	}PatternDefinition;
	
	//prototypes
	int InitializePatternList(void);
	int InitializePattern(PatternDefinition *newPattern, int index);
	int InsertPattern(PatternDefinition newPattern, int index);
	int DeletePattern(int index);
	int GetNumberPatterns(void);
	int GetPatternData(PatternDefinition *pPattern, int index);
	int DeletePatternList(ListType listToDelete);
	
	int CreateBlock(int*newBlock, int patternType, int numberRows, int numberColumns);
	int MakeSolidPattern(int *map, int numberRows, int numberColumns, int vline);
	int MakeRandomPattern(int *map, int numberRows, int numberColumns);
	int MakeCheckerboard(int*newBlock, int numberRows, int numberColumns, int inverse,
		int active);
	int CreatePattern(int *newPattern, int *block, int startingRow, int startingColumn,
		int numberRows, int numberColumns, int floating);
	int CompareMaps(int *resultsMap, int *wroteMap, int *readMap, int numberElectrodes);
	int LoadPatternData(char *filename, PatternDefinition *newPattern);
	int SavePatternData(char *filename, PatternDefinition *newPattern);
	int SavePatternList(char *filename, int numberPatterns);
	int LoadBlockList(char *filename);
	
	int GetBlockFromMap(PatternDefinition *newPattern, int *newBlock);
	
#endif
