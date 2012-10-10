#include "Tweezers.h"
#include "Tweezers_CL.h"
#include <iostream>
#include <limits>

// constructor
Tweezers::Tweezers(double iF0,double id0,double ic1,double ic2,double ic3,double Anti_Voltage, void* lpParam)
{

	threadinfo* t = (threadinfo*)lpParam;

	// set calibration parameters
	F0 = iF0;
	d0 = id0;
	c1 = ic1;
	c2 = ic2;
	c3 = ic3;
	AV = Anti_Voltage;

	x4 = *t->x4;
	x3 = *t->x3;
	x2 = *t->x2;
	x1 = *t->x1;
	x0 = *t->x0;

	done = false;
	islate = false;

	residual = AV;
	tmp_residual = AV;

	trigdata[0][0] = 0;
	trigdata[1][0] = 1;

	// initialize device
	DAQmxResetDevice("Dev1");

	// create and setup analog task
	DAQmxCreateTask("AO",&AOtaskHandle);
	DAQmxCreateAOVoltageChan(AOtaskHandle,"Dev1/ao0","",-10.0,10.0,DAQmx_Val_Volts,NULL);

	DAQmxGetExtendedErrorInfo(errBuff,2048);
	cerr<<errBuff;

	// create and reset start trigger
	// implicit timing => no start task... (?)

	DAQmxCreateTask("TR",&TRtaskHandle);
	DAQmxCreateDOChan(TRtaskHandle,"Dev1/port0/line7","TR",DAQmx_Val_ChanForAllLines);
	DAQmxWriteDigitalLines(TRtaskHandle,1,1,10,DAQmx_Val_GroupByChannel,trigdata[0],&written,NULL);

	// create pulse channel for trigger
	DAQmxCreateTask("DO",&DOtaskHandle);
	DAQmxCreateCOPulseChanTime(DOtaskHandle,"Dev1/Ctr0","",DAQmx_Val_Seconds,DAQmx_Val_Low,1E-3*(*t->wait),1E-3*(*t->delta)/2.0,1E-3*(*t->delta/2.0));

	DAQmxCreateTask("AI",&AItaskHandle);
	DAQmxCreateAIVoltageChan(AItaskHandle,"Dev1/ai0,Dev1/ai2","",DAQmx_Val_RSE,-10,+10,DAQmx_Val_Volts,NULL);

	DAQmxGetExtendedErrorInfo(errBuff,2048);
	cerr<<errBuff;

	// set to zero force
	Reset();
}

// destructor
Tweezers::~Tweezers()
{
	cerr<<"reset...";
	Reset();
	DAQmxClearTask(AItaskHandle);
	DAQmxClearTask(AOtaskHandle);
	DAQmxClearTask(DOtaskHandle);
	DAQmxClearTask(TRtaskHandle);
	DAQmxResetDevice("Dev1");
	
	DAQmxGetExtendedErrorInfo(errBuff,2048);
	cerr<<errBuff;
	cerr<<"delete.\n";
}

int32 Tweezers::Degauss(float U, float duration)
{
	clock_t		startTime;
	float		freq = 100;
	int			bufferSize = (int)(5000 * duration);
	float64     *data = new float64[bufferSize];

	// fill buffer with data
	for(int i=0;i<bufferSize;i++)
	{
		float64 amp = U * (1 - (double)i/(double)(bufferSize-1));
		data[i] = amp * cos( 2.0 * PI * freq * (double)i/5000.0); // degauss Srate must be independent of run!!
	}

	DAQmxErrRtn(DAQmxClearTask(AOtaskHandle));

	// create and setup analog task
	DAQmxErrRtn(DAQmxCreateTask("AO",&AOtaskHandle));
	DAQmxErrRtn(DAQmxCreateAOVoltageChan(AOtaskHandle,"Dev1/ao0","",-10.0,10.0,DAQmx_Val_Volts,NULL));

	// configure sample clock for buffered write
	DAQmxErrRtn(DAQmxCfgSampClkTiming(AOtaskHandle,"",5000,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,(int)(duration*5000)));

	// write data and start task
	DAQmxErrRtn(DAQmxWriteAnalogF64(AOtaskHandle,bufferSize,0,10.0,DAQmx_Val_GroupByChannel,data,&written,NULL));

	startTime = clock();

	DAQmxErrRtn(DAQmxStartTask(AOtaskHandle));

	while(!done) {
		DAQmxErrRtn(DAQmxIsTaskDone(AOtaskHandle,&done));
		if (!done) Sleep(100);
	}

	done = false;

    DAQmxErrRtn(DAQmxStopTask(AOtaskHandle));

	residual = 0.0;

	delete data;
}

int32 Tweezers::SetCurrent(float current)
{
	// init output data array
	float64 out_data[1];
	out_data[0] = current;

	// setup HTSP timing and deactivate onboard memory
	DAQmxErrRtn(DAQmxCfgSampClkTiming(AOtaskHandle,"",Srate_HTSP,DAQmx_Val_Rising,DAQmx_Val_HWTimedSinglePoint,1));

	// start task
	DAQmxErrRtn(DAQmxStartTask(AOtaskHandle));

	// write single sample
	DAQmxErrRtn(DAQmxWriteAnalogF64(AOtaskHandle,1,true,-1,DAQmx_Val_GroupByChannel,out_data,&written,NULL));

	// stop task
	DAQmxErrRtn(DAQmxStopTask(AOtaskHandle));

	residual = (abs(current) > abs(residual) ? current : residual);
}

int32 Tweezers::Run(float64* force, float64* indata, void* lpParam)
{
	threadinfo* t = (threadinfo*)lpParam;

	// init output data array
	float64 out_data[1];
	out_data[0] = 0.0;

	register int i = 0,j = 0;

	// # of samples to be generated per cycle
	register int nofSamples = (int)(t->nofFrames/(*t->cycles)) * (*t->delta)*1E-3 * Srate_HTSP;
	//cerr<<nofSamples<<" samples\n";

	// setup HTSP timing and deactivate onboard memory
	DAQmxErrRtn(DAQmxCfgSampClkTiming(AOtaskHandle,"OnboardClock",Srate_HTSP,DAQmx_Val_Rising,DAQmx_Val_HWTimedSinglePoint,nofSamples));
	DAQmxSetRealTimeConvLateErrorsToWarnings(AOtaskHandle, TRUE);

	// setup start trigger
	DAQmxErrRtn(DAQmxCfgDigEdgeStartTrig(AOtaskHandle, "PFI0", DAQmx_Val_Rising));

	// setup counter task and start trigger
	// maybe change trigger to digital line? need to use ContSamps, otherwise limited frame #
	//DAQmxErrRtn(DAQmxCreateCOPulseChanTime(DOtaskHandle,"Dev1/Ctr0","",DAQmx_Val_Seconds,DAQmx_Val_Low,0,1E-3*(*t->delta)/2,1E-3*(*t->delta/2)));
	DAQmxErrRtn(DAQmxCfgImplicitTiming(DOtaskHandle, DAQmx_Val_ContSamps, 1));

	// setup start trigger
	DAQmxErrRtn(DAQmxCfgDigEdgeStartTrig(DOtaskHandle, "PFI0", DAQmx_Val_Rising));

	// setup analog in task
	DAQmxErrRtn(DAQmxCfgSampClkTiming(AItaskHandle,"Ctr0Out",100,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,*t->cycles * t->nofFrames));
	//DAQmxErrRtn(DAQmxCfgDigEdgeStartTrig(AItaskHandle, "PFI0", DAQmx_Val_Rising));

	// start tasks (still waiting for edge trigger from TRtask)
	DAQmxErrRtn(DAQmxStartTask(DOtaskHandle));
	DAQmxErrRtn(DAQmxStartTask(AOtaskHandle));
	DAQmxErrRtn(DAQmxStartTask(AItaskHandle));

	// precalulate number of loop runs to speed up loop
	int samps = (nofSamples * (*t->cycles) - 1);
	
	// write first sample outside loop, so there is enough time to send the TR start trigger
	out_data[0] = current(force[0],t->dist);

	DAQmxErrRtn(DAQmxWriteAnalogF64(AOtaskHandle,1,true,-1,DAQmx_Val_GroupByChannel,out_data,&written,NULL));

	// GO!
	DAQmxErrRtn(DAQmxWriteDigitalLines(TRtaskHandle,1,1,10,DAQmx_Val_GroupByChannel,trigdata[1],&written,NULL));

	// 1 kHz sample generation loop
	while(!(*t->bead_lost &&
			// emmeran@gmx.net: 05/10/2012 16:16 - dont stop recording if bead is lost in Creep-noint protocol
			*t->protocol != "Creep-noint"
			// end of change
		) && i < samps)
	{
		j = (++i % nofSamples);
		//cerr<<j<<"\r";
		out_data[0] = current(force[j],t->dist);
		DAQmxErrRtn(DAQmxWaitForNextSampleClock(AOtaskHandle, 1.0, &islate));
		DAQmxErrRtn(DAQmxWriteAnalogF64(AOtaskHandle,1,true,-1,DAQmx_Val_GroupByChannel,out_data,&written,NULL));
		if(islate)
		{
			cerr<<i++<<"\n****** late write - out of sync! *******\n";
			*t->bead_lost = TRUE;
			t->syncerror = TRUE;
		}
	}

	printf("DEBUG: bead_lost(%d) protocol(%s) i(%d) samps(%d)\n",
		*t->bead_lost, *t->protocol, i, samps);

	// wait for last sample to be generated
	DAQmxErrRtn(DAQmxWaitForNextSampleClock(AOtaskHandle, 1.0, &islate));

	// make sure enough trigger pulses are generated
	Sleep(*t->delta * 5);

	// stop tasks
	DAQmxErrRtn(DAQmxStopTask(DOtaskHandle));
	DAQmxErrRtn(DAQmxStopTask(AOtaskHandle));

	// number of frames actually taken
	t->FramesTaken = 1 + floor(t->nofFrames * (double)i/(double)(nofSamples * (*t->cycles)));

	// read protocol samples
	DAQmxReadAnalogF64(AItaskHandle,t->FramesTaken,5.0,DAQmx_Val_GroupByChannel,indata,t->FramesTaken*2,&read,NULL);

	DAQmxErrRtn(DAQmxStopTask(AItaskHandle));
	
	// disable start triggers
	DAQmxErrRtn(DAQmxDisableStartTrig(DOtaskHandle));
	DAQmxErrRtn(DAQmxDisableStartTrig(AOtaskHandle));
	
	// reset trigger line
	DAQmxErrRtn(DAQmxWriteDigitalLines(TRtaskHandle,1,1,10,DAQmx_Val_GroupByChannel,trigdata[0],&written,NULL));

}

int32 Tweezers::CurrentRun(float64* curr, float64* indata, void* lpParam)
{
	threadinfo* t = (threadinfo*)lpParam;

	// config and start analog tast
	DAQmxErrRtn(DAQmxCfgSampClkTiming(AOtaskHandle,"",10*float64(1/(*t->delta*1E-3)),DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,(*t->cycles*10*t->nofFrames)));
	DAQmxErrRtn(DAQmxCfgOutputBuffer(AOtaskHandle, t->nofFrames*10));
	DAQmxErrRtn(DAQmxCfgDigEdgeStartTrig(AOtaskHandle, "PFI0", DAQmx_Val_Rising));
	DAQmxErrRtn(DAQmxWriteAnalogF64(AOtaskHandle,t->nofFrames*10,0,10.0,DAQmx_Val_GroupByChannel,curr,&written,NULL));
	DAQmxErrRtn(DAQmxStartTask(AOtaskHandle));

	cerr << "\nTrying to allocate buffer for " << (*t->cycles * t->nofFrames) << " frames\n";
	// config and start pulse channel with parameters set by user
	//DAQmxErrRtn(DAQmxCfgImplicitTiming(DOtaskHandle, DAQmx_Val_FiniteSamps,*t->cycles * t->nofFrames));
	DAQmxErrRtn(DAQmxCfgImplicitTiming(DOtaskHandle, DAQmx_Val_ContSamps,1));
	DAQmxErrRtn(DAQmxCfgDigEdgeStartTrig(DOtaskHandle, "PFI0", DAQmx_Val_Rising));
	DAQmxErrRtn(DAQmxStartTask(DOtaskHandle));

	// config analog in task
	DAQmxErrRtn(DAQmxCfgSampClkTiming(AItaskHandle,"Ctr0Out",100,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,*t->cycles * t->nofFrames));
	DAQmxErrRtn(DAQmxStartTask(AItaskHandle));

	// set trigger line to 1 - GO!
	DAQmxWriteDigitalLines(TRtaskHandle,1,1,10,DAQmx_Val_GroupByChannel,trigdata[1],&written,NULL);
	
	// wait until cam trigger task is done
	DAQmxWaitUntilTaskDone(AOtaskHandle,-1);
	
	// reset trigger line
	DAQmxWriteDigitalLines(TRtaskHandle,1,1,10,DAQmx_Val_GroupByChannel,trigdata[0],&written,NULL);

    // read protocol samples
	DAQmxReadAnalogF64(AItaskHandle,*t->cycles * t->nofFrames,5.0,DAQmx_Val_GroupByChannel,indata,*t->cycles * t->nofFrames*2,&read,NULL);

	//read indeed current
	ofstream strom;
	strom.open("newcurrent.txt", ofstream::app);
	for(int i=0;i<t->nofFrames;i++)
		strom<<i*(*t->delta*1E-3)<<"\t"<<indata[i]<<endl;
		strom.close();
	
	// stop tasks, disable trigger
	DAQmxStopTask(AOtaskHandle);
	DAQmxStopTask(DOtaskHandle);
	DAQmxStopTask(AItaskHandle);
	DAQmxDisableStartTrig(AOtaskHandle);
	DAQmxDisableStartTrig(DOtaskHandle);
}


double Tweezers::ResidualCurrent(double c)
{
	return x4*pow(c,4)+x3*pow(c,3)+x2*pow(c,2)+x1*pow(c,1)+x0;
}

double Tweezers::current(double f,double d)
{
	double current = 0.0;
	
	// zero force:
	if(f==0.0) {
		// update actual residual only when force returns to zero
		residual = tmp_residual;

		// antivoltage: no hysteresis yet
		if(residual==AV)
			return AV;
		// residual as calibrated for zero force
		else
			return x4*pow(residual,4)+x3*pow(residual,3)+x2*pow(residual,2)+x1*pow(residual,1)+x0;
	}
	// end zero force

	// not zero force
	else
	{
		if(f<0.5*1E-9)
			f=0.5*1E-9;

		current = AV + 1/c3 * log( 1/c2 * ( c1 * ( log(d/d0) / log(f/F0) ) - 1 ) );
	//cout << current << endl << AV << endl << c3 << endl << c2 << endl << c1 << endl << d << endl<< d0<< endl << f<< endl << F0 << endl;
	// if force too small => calib not valid!

	// update temporary residual and return current value
	if(-3.0 < current < 3.0)
	{
		tmp_residual = (abs(current) > abs(tmp_residual) ? current : tmp_residual);
		return current;
	}
	else
		return 0.0;
	}
	return 0.0;
}


double Tweezers::force(double I,double d)
{
	double force = 0.0;
	force = F0 * pow( d/d0 , c1/( 1 + c2*exp(c3*I) ) );

	if (force)
		return force;
	else
		return 0.0;
}

int32 Tweezers::TriggerFrames(unsigned int nr)
{
	//DAQmxErrRtn(DAQmxStopTask(DOtaskHandle));
	DAQmxErrRtn(DAQmxCfgImplicitTiming(DOtaskHandle, DAQmx_Val_FiniteSamps, 1));
	DAQmxErrRtn(DAQmxStartTask(DOtaskHandle));
	DAQmxErrRtn(DAQmxWaitUntilTaskDone(DOtaskHandle,-1));
	DAQmxErrRtn(DAQmxStopTask(DOtaskHandle));
}

int32 Tweezers::RecoverFromError()
{
	// stop running tasks
	DAQmxErrRtn(DAQmxStopTask(DOtaskHandle));
	DAQmxErrRtn(DAQmxStopTask(AOtaskHandle));
	DAQmxErrRtn(DAQmxStopTask(AItaskHandle));

	// disable start triggers
	DAQmxErrRtn(DAQmxDisableStartTrig(DOtaskHandle));
	DAQmxErrRtn(DAQmxDisableStartTrig(AOtaskHandle));
	
	// reset trigger line
	DAQmxErrRtn(DAQmxWriteDigitalLines(TRtaskHandle,1,1,10,DAQmx_Val_GroupByChannel,trigdata[0],&written,NULL));

	// reset to zero
	DAQmxErrRtn(SetCurrent(AV));
}
