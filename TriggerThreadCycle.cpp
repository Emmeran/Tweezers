#include "Creep F9.h"
#include "Tweezers.h"

void TriggerThreadCycle(void* lpParam)
{
	/*int32		written;
	float64 data[2][1];
	float64 zero[2];*/
	char        errBuff[2048];

	// get thread params
	threadinfo* tinfo = (threadinfo*)lpParam;

	TaskHandle DOtaskHandle;

	// initialize device
	DAQmxResetDevice("Dev1");

	//while(!_kbhit())
	//{
		DAQmxCreateTask("DO",&DOtaskHandle);
		DAQmxCreateCOPulseChanTime(DOtaskHandle,"Dev1/Ctr0","",DAQmx_Val_Seconds,DAQmx_Val_Low,0.1,float64(200)*1E-3,float64(tinfo->extime)*1E-3);
		DAQmxCfgImplicitTiming(DOtaskHandle, DAQmx_Val_FiniteSamps, 1);
		DAQmxStartTask(DOtaskHandle);
		DAQmxWaitUntilTaskDone(DOtaskHandle,-1);
		DAQmxClearTask(DOtaskHandle);
	//}
	return;
}

