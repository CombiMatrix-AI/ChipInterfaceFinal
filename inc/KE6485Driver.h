/*
  KE6485Driver.h
  
*/

#include "KE6485.h" //VXI Instrument Driver
#include "debuguir.h"
#include <windows.h>
#include <ansi_c.h>
#include <utility.h>



#define INSTRUMENT "GPIB0::14::INSTR"
#define BUFFER_SIZE 256
#define MEASUREMENT_AVAILABLE "MEASUREMENT_AVAILABLE"

int measurementAvailable;

int InitKE6485(double nplc, int autorange, double resolution, double range, int simulate);
void CloseKE6485(void);
int StartSampleReadThread(double duration);
int CVICALLBACK StartSampleReadThreadFunction (void *functionData);
void CVICALLBACK SampleReadThreadFunctionCallback (int poolHandle, int functionID, unsigned int event, 
	int value, void *callbackData);
int StartSampleRead(void);
void KE6485Abort(void);
void KE6485ClearBuffer(void);
int StopAndFetchSampleRead(double *buffer, int *numberSamples);
int ReadStatistics(int numberSamples, int maxTime, double *mean, double *stdev,
	double *min, double *max, double *pk2pk);
//int ReadSamples(int numberSamples, double *sampleBuffer);
int ReadSamples(int numberSamples, double totalDuration, double *sampleBuffer);
int ReadSingleSample(double *sample);
int InitiateAndFetch(int maxTime, double *sample);
void GetLastResult(int status, char *msg);
void HandleError(ViSession vi, ViStatus error);
int InfinateAcquire(int state);
int CVICALLBACK InfinateAcquireThread (void *functionData);
