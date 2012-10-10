#ifndef TWEEZERS_H
#define TWEEZERS_H

#include "NIDAQmx.h"

#include <math.h>
#include <time.h>
#include <windows.h>
#include <stdio.h>
#include <iostream>

// pi
#define PI	3.1415926535

// sampling rate (1kHz)
#define Srate_HTSP 200

// NIDAQmx error handler
#define DAQmxErrRtn(functionCall) { if( DAQmxFailed(error=(functionCall)) ){ return error; }}

class Tweezers
{
public:

	// constructor
	Tweezers(double F0,double d0,double c1,double c2,double c3,double Anti_Voltage, void* lpParam);

	// destructor
	Tweezers::~Tweezers();

	// demagnetize core (default: +/- 3.0 V for 1 sec.)
	int32 Degauss(float U = 3.0, float duration = 1.0);

	// set constant current (independent of force)
	int32 SetCurrent(float current);

	// reset to zero force (after degauss)
	int32 Reset() { DAQmxErrRtn(SetCurrent(AV)); };

	// run measurement
	//
	// needs # of frames, pointer to force and indata array
	int32 Run(float64* force, float64* indata, void* t);

	// current run (buffered write)
	int32 Tweezers::CurrentRun(float64* current, float64* indata, void* lpParam);

	int32 TriggerFrames(unsigned int nr);

	int32 RecoverFromError();

		// return current for given force and distance
	double current(double force, double dist);

	// return force for given current and distance
	double force(double current, double dist);

	// returns needed residual current for given current
	double ResidualCurrent(double current);

protected:

	// highest current core has seen
	double residual, tmp_residual;

	// data array for start trigger
	uInt8 trigdata[2][1];
	
	// vector of force values
	//float64		*force_data;

	// # of samples written or read
	int32	written,read;

	// error handling
	char        errBuff[2048];
	int32       error;

	// DAQmx task done and late write flag
	bool32		done,islate;

	// DAQmx task handles
	TaskHandle	AOtaskHandle,TRtaskHandle,DOtaskHandle,AItaskHandle;

	//**********************
	// calibration constants
	//**********************
	float F0,d0,c1,c2,c3; // for force/current/distance relation

	double AV; // residual current after complete degauss

	float x1,x2,x3,x4,x0; // for residual current after given current (before degauss)
	//
	//**********************



};

#endif // TWEEZERS_H