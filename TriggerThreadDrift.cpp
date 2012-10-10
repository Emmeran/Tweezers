#include "Tweezers_CL.h"
#include "Tweezers.h"

void TriggerThreadDrift(void* lpParam)
{
	char        errBuff[2048];

	TaskHandle taskHandle, DOtaskHandle;
	float64 data[2],i, voltage;
	data[0]=data[1]=0;
	int32 read;
	char cellnum[12];

	// get thread params
	threadinfo* t = (threadinfo*)lpParam;

	// initialize device
	DAQmxResetDevice("Dev1");
	DAQmxCreateTask("",&taskHandle);
	DAQmxCreateAOVoltageChan(taskHandle,"Dev1/ao0","",-10.0,10.0,DAQmx_Val_Volts,NULL);
	// create pulse channel with parameters set by user
	DAQmxCreateTask("DO",&DOtaskHandle);
	DAQmxCreateCOPulseChanTime(DOtaskHandle,"Dev1/Ctr0","",DAQmx_Val_Seconds,DAQmx_Val_Low,float64(*t->wait)*1E-3,(float(*t->delta)/2)*1E-3,(float(*t->delta)/2)*1E-3);
	DAQmxCfgImplicitTiming(DOtaskHandle, DAQmx_Val_ContSamps, 1);
 
	DAQmxStartTask(DOtaskHandle);
	
	DAQmxGetExtendedErrorInfo(errBuff,2048);
	cerr<<errBuff;

	while(*t->bead_lost==0)
	{
		//cerr<<*t->rframes;
				if (*t->rframes==*t->fframes){
				ConstVoltageOut(taskHandle, 2.0);
				Sleep(*t->duration);
				ConstVoltageOut(taskHandle, -0.164);
		}
			
	Sleep(*t->delta);
	}


	DAQmxStopTask(DOtaskHandle);
	
	DAQmxClearTask(DOtaskHandle);

	cerr<<"trigger done!\n";

	return;
}
