#include "Tweezers_CL.h"

void SaveThreadCalibRem(void* lpParam)
{
	TaskHandle taskHandle, taskHandleRead;
	float64 data[1],i, voltage;
	data[0]=0;
	int32 read;
	char cellnum[12];

	// get thread params
	threadinfo* t = (threadinfo*)lpParam;

	DAQmxResetDevice("Dev1");

	// create tasks
	DAQmxCreateTask("",&taskHandle);
	DAQmxCreateTask("",&taskHandleRead);

	// create channels
	DAQmxCreateAOVoltageChan(taskHandle,"Dev1/ao0","",-10.0,10.0,DAQmx_Val_Volts,NULL);
	DAQmxCreateAIVoltageChan(taskHandleRead,"Dev1/ai2","",DAQmx_Val_RSE,-1.0,1.0,DAQmx_Val_Volts,NULL);

	// create cellnum string for directory and go there
	sprintf(cellnum,"%03d",t->cell);
	CreateAndGotoDir(cellnum);

	ofstream of;
	of.open("eval.txt");

	// copy config file to new location
	CopyFile(t->oldconf.c_str(),"config.txt",0);

	for (i = 0.0; i <= 3; i+=0.25)
	{

	cerr << i << "V" << endl;
	DegaussCycle(taskHandle, 3.0, 1.0);
	ConstVoltageOut(taskHandle, 0.09);

	Sleep(1000);
	DAQmxReadAnalogF64(taskHandleRead,1,10.0,DAQmx_Val_GroupByChannel,data,1,&read,NULL);
	cerr << "antivoltage: " << data[0]*1000 << " mT" << endl;
//	of << data[0]*1000 << ";" << ";";


	for (voltage = -0.3; voltage < 0.1; voltage=voltage+0.05)  // loop for different offsets
	{
		ConstVoltageOut(taskHandle, i);

		Sleep(1000);
		DAQmxReadAnalogF64(taskHandleRead,1,10.0,DAQmx_Val_GroupByChannel,data,1,&read,NULL);
		
		cerr << endl << "field on: " << data[0]*1000 << " mT" << endl;
	
		of << i << "\t" << data[0]*1000 << "\t";
		
		ConstVoltageOut(taskHandle, voltage);

		Sleep(1000);
		DAQmxReadAnalogF64(taskHandleRead,1,10.0,DAQmx_Val_GroupByChannel,data,1,&read,NULL);
		cerr << "antivoltage: " << voltage << ", field: "<<data[0]*1000 << " mT" << endl;

		of << voltage << "\t" << data[0]*1000 << "\n";

		DegaussCycle(taskHandle, 3.0, 1.0);
	}

	of << endl;

	}

	DAQmxClearTask(taskHandle);

	of.close();

	return;
}

int32 DegaussCycle(TaskHandle taskHandle, float U, float duration)
{

	#define Srate 5000

	int32		written;
	clock_t		startTime;
	bool32      done=0;
	int32       error=0;
	char        errBuff[2048]={'\0'};

	float		f1 = 100;
	float		f2 = 100;

	int			bufferSize = (int)(Srate * duration);

	float64     *data = new float64[bufferSize];

	// fill buffer with data
	for(int i=0;i<bufferSize;i++)
	{
		float64 freq = f1 + (f2-f1)*((double)i/(double)bufferSize);
		float64 amp = U * (1 - (double)i/(double)(bufferSize-1));
		
		data[i] = amp*cos((double)i/(double)(Srate)*2.0*PI*freq);
	}

	// configure sample clock
	DAQmxErrRtn(DAQmxCfgSampClkTiming(taskHandle,"",Srate,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,(int)(duration*Srate)));

	// write data and start task
	DAQmxErrRtn(DAQmxWriteAnalogF64(taskHandle,bufferSize,0,10.0,DAQmx_Val_GroupByChannel,data,&written,NULL));

	DAQmxErrRtn(DAQmxStartTask(taskHandle));

	startTime = clock();

	while( !done && clock()<startTime+duration*1000) {
		DAQmxErrRtn(DAQmxIsTaskDone(taskHandle,&done));
		if (!done) Sleep(100);
	}
    DAQmxErrRtn(DAQmxStopTask(taskHandle));
}
