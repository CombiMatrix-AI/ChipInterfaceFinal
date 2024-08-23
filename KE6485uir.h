//KE6485uir.h

#ifndef _KE6485UIR_H
	#define _KE6485UIR_H
	
	#include <cvirte.h>		
	#include <userint.h>
	#include "KE6485Driver.h"
	#include "KE6485TestPanel.h"

	int LoadKeithleyPanel(void);
	void CVICALLBACK InsertNewMeasurement (WinMsgWParam wParam, WinMsgLParam lParam, void *callbackData);

#endif
