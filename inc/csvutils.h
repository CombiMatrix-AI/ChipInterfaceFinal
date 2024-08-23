// csvutils.h

#ifndef CSVUTILS_H_
	#define CSVUTILS_H_
	
	// includes
	#include "log.h"
	#include <utility.h>

	// prototypes
	int LoadCSVFile (char *filename, int *map, int maxRows, int maxColumns);
	int DisplayCSVMap(int *map, int numberRows, int numberColumns);
	int TestCSVFunctions(int numberRows, int numberColumns);

	// definitions
	#define MAXIMUM_LINE_LENGTH 	80
	#define CSV_OPEN_FILE_CAPTION 	"Open CSV File"
	
#endif
