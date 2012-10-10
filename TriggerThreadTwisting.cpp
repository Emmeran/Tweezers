#include "Tweezers_CL.h"
#include "Tweezers.h"

#define PI	3.1415926535
#define Srate 5000

#include <iostream>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fstream>

using namespace std;


void TriggerThreadTwisting(void* lpParam)
{
    int32 written;
	int32 read;

	bool32		done = 0;

	threadinfo* t = (threadinfo*)lpParam;

	uInt8 trigdata[2][1] = {{0},{1}};

	char errBuff[2048];
	int x;
    int ir;

	char key = '0';

	cerr<<endl<<"press q to stop measurement..."<<endl;

	int b=(int)(100000.0/(*t->freq));  // b=Dauer mit 100000 Pkt. pro Sekunde fur magnetfeldsinus
	
	float64 *curr = new float64[b];
	
	// initialize device
	DAQmxResetDevice("Dev1");

	if (*t->freq<=1.0)
		x=0;
	else
    	x=ceil(*t->freq/8.0);

	double dt = 1/(*t->freq) * (x+(double)1/8); //zeitintervall zw. 2 bilder

	ir = ceil(*t->duration / dt); //bilder pro angegebene zeit in sek

	// array für eingelesene daten
	float64 *indata = new float64[2*ir];

	curr=sinus(*t->amplitude, *t->freq, b);

	TaskHandle AOtaskHandle,TRtaskHandle,DOtaskHandle,AItaskHandle;

	// create and setup start trigger
	DAQmxCreateTask("TR",&TRtaskHandle);
	DAQmxCreateDOChan(TRtaskHandle,"Dev1/port0/line7","TR",DAQmx_Val_ChanForAllLines);
	DAQmxWriteDigitalLines(TRtaskHandle,1,1,0,DAQmx_Val_GroupByChannel,trigdata[0],&written,NULL);//!!!



	// create and setup analog task
	DAQmxCreateTask("AO",&AOtaskHandle);
	DAQmxCreateAOVoltageChan(AOtaskHandle,"Dev1/ao0","",-10.0,10.0,DAQmx_Val_Volts,NULL);

	DAQmxGetExtendedErrorInfo(errBuff,2048);
	printf("%s\n",errBuff);

	DAQmxCfgSampClkTiming(AOtaskHandle,"",100000,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,*t->duration*100000);
	DAQmxCfgDigEdgeStartTrig(AOtaskHandle, "PFI0", DAQmx_Val_Rising);
	DAQmxWriteAnalogF64(AOtaskHandle,b,0,0,DAQmx_Val_GroupByChannel,curr,&written,NULL);//!!!!!!
	DAQmxStartTask(AOtaskHandle);


	// create pulse channel with parameters set by user
	DAQmxCreateTask("DO",&DOtaskHandle);
	DAQmxCreateCOPulseChanTime(DOtaskHandle,"Dev1/Ctr0","",DAQmx_Val_Seconds,DAQmx_Val_Low ,0,dt-(*t->extime*1E-3),(*t->extime*1E-3));

	DAQmxGetExtendedErrorInfo(errBuff,2048);
	printf("%s\n",errBuff);

	DAQmxCfgImplicitTiming(DOtaskHandle, DAQmx_Val_ContSamps,1);
	DAQmxCfgDigEdgeStartTrig(DOtaskHandle, "PFI0", DAQmx_Val_Rising);
	DAQmxStartTask(DOtaskHandle);


	
	// create analog read task 
	DAQmxCreateTask("AI",&AItaskHandle);
    //DAQmxCreateAOVoltageChan(AOtaskHandle,"Dev1/ao0","",-10.0,10.0,DAQmx_Val_Volts,NULL);
	DAQmxCreateAIVoltageChan(AItaskHandle,"Dev1/ai0:1","",DAQmx_Val_RSE,-10,+10,DAQmx_Val_Volts,NULL);
	DAQmxCfgSampClkTiming(AItaskHandle,"Ctr0Out",10,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,ir);
	DAQmxCfgDigEdgeStartTrig(AItaskHandle, "PFI0", DAQmx_Val_Rising);
	DAQmxStartTask(AItaskHandle);


	
  //DAQmxWriteDigitalLines(TRtaskHandle,1,1,10,DAQmx_Val_GroupByChannel,trigdata[0],&written,NULL);
	DAQmxWriteDigitalLines(TRtaskHandle,1,1,10,DAQmx_Val_GroupByChannel,trigdata[1],&written,NULL);

	DAQmxGetExtendedErrorInfo(errBuff,2048);
	printf("%s\n",errBuff);

	// is bead lost or task done?
	while(!done && *t->bead_lost==FALSE)
	{
		DAQmxIsTaskDone(AOtaskHandle,&done);
		if(_kbhit())
		{
			key = _getch();
			cerr<<"key pressed! ";
			if(key=='q')
			{
				*t->bead_lost=TRUE;
				cerr<<"stopping!"<<endl;
			}
		}
		Sleep(500);
	}

	Sleep(500);

	if(*t->bead_lost) DAQmxStopTask(AOtaskHandle);

	DAQmxStopTask(DOtaskHandle);

	int FramesTotal = 0;

	// get analog read-in data
	if(*t->bead_lost==0)
	{
		DAQmxReadAnalogF64(AItaskHandle,ir,-1,DAQmx_Val_GroupByChannel,indata,2*ir,&read,NULL);
	}

	DAQmxStopTask (AOtaskHandle);

	ConstVoltageOut(AOtaskHandle, 0.0);

	// get analog read-in data

	ofstream ks; //sinus für kamera
	ks.open("kamerasin1.txt");  //ir = ceil(1/dt)*t;
	for(int i=0;i<ir;i++)
	    ks << (float)i/(1/dt) << "\t" << indata[i] << endl;
	ks.close();

	ofstream tf; // trigger
	tf.open("trigger1.txt");
	for(int i=0;i<ir;i++)
       tf <<(float)i/(1/dt)<< "\t" << indata[i+ir] << endl;
    tf.close();

	delete curr;
	delete indata;

	DAQmxClearTask(DOtaskHandle);
	DAQmxClearTask(AOtaskHandle);
	DAQmxClearTask(TRtaskHandle);
	DAQmxClearTask(AItaskHandle);

	cerr<<"trigger done!\n";
	return;
}


float64 *sinus(float amplitude, float frequenz, int b)
{

	//float t=3.0;       // t=Zeit in Sekunden
	float64 *result;
	result = new float64[b];
	for(int i=0; i < b; i++)
	{
		result[i] = amplitude*sin(i*2*PI*frequenz/100000);
	}
	return result;
}

int32 ConstVoltageOut(TaskHandle taskHandle, float U)
{
	int32		written;
	clock_t		startTime;
	bool32      done=0;
	int32       error=0;
	char        errBuff[2048]={'\0'};
	float64		data[] = {U,U};

	// configure sample clock
	DAQmxErrRtn(DAQmxCfgSampClkTiming(taskHandle,"",Srate_HTSP,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,2));

	// write data and start task
	DAQmxErrRtn(DAQmxWriteAnalogF64(taskHandle,2,0,10.0,DAQmx_Val_GroupByChannel,data,&written,NULL));
	DAQmxErrRtn(DAQmxStartTask(taskHandle));

	startTime = clock();

	while( !done && clock()<startTime+1000) {
		DAQmxErrRtn(DAQmxIsTaskDone(taskHandle,&done));
		if (!done) Sleep(100);
	}
	DAQmxErrRtn(DAQmxStopTask(taskHandle));

}