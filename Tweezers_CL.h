/***************************************************

include file for Creep F9

****************************************************/

#include "HamamatsuCamera.h"
//#include "CameraSimulatorNoise.h"
#include "time.h"
#include "direct.h"

#include "nidaqmx.h"

#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <conio.h>
#include "mikroepp.h"

#include "stdafx.h"

#include "El_timer.h"
#include "BeadAlgorithm.h"
#include "Tweezers.h"

#define SB_BUFFER_SIZE 120
//#define MOVE_NEEDLE

class threadinfo
{

public:

	threadinfo() {
		cell = 0;
		error = 0;
		syncerror = 0;
		location = new string;
		identifier = new string;
		cur = new double;
		force1 = new double;
		force2 = new double;
		delta = new int;
		extime = new double;
		fframes = new int;
		zframes = new int;
		rframes = new int;
		nframes = new int;
		rampframes = new int;
		wait = new int;
		magnification = new int;
		c1 = new double;
		c2 = new double;
		c3 = new double;
		F0 = new double;
		d0 = new double;
		array_top = 0;
		x0 = new double;
		x1 = new double;
		x2 = new double;
		x3 = new double;
		x4 = new double;
		array_size = new int;
		anti_voltage = new double;
		error = 0;
		closest_idx = 0;
		protocol = new string;
		visc = new double;
		beaddiam = new double;
		cycles = new int;
		freq = new double;
		amplitude = new double; 
		duration = new double;
		move_needle = new int;

		dist = 0;

		calib_running = false;
		
		// init needleframe
		needleframe = new Frame2D<unsigned short>(1344,1024);
		needleframe->zero();

		bead_lost = new unsigned;

		nofFrames = 0;

		save_freq = new int;

		framecounter = 0;
	};

	HANDLE saveFlag;

	bool capturing;

	unsigned int closest_idx;

	string* location; // location to store images
	string* identifier; // subdirectory
	string* date; // date

	string* protocol; // protocol type
	double* visc; // viscosity (for calibration)

	double* cur; // current
	
	double* force1; // max force
	double* force2; // max force

	double* extime; // exposition time
	int* delta; // time between frames
	int* wait; //ms;

	int* zframes;
	int* fframes;
	int* rframes;
	int* nframes;
	int* rampframes;

	int* cycles;
	int* move_needle;
	double* freq;
	double* amplitude;

	Camera_Pool* cp; // Camera Pool
	//CameraSimulatorNoise<unsigned short> *cam;
	HamamatsuCamera* cam;
	SBwinCaptureFrame2D<unsigned short> *sframe;
	
	char ch;

	MikroEppendorf* mm;
	
	int current_frame; // frame counter variable
	int cell; // number of cell
	
	bool error; // error handler
	bool syncerror;
	HANDLE CycleReadyEvent;

	string oldconf;

	unsigned array_top;
	int *array_size;
	double *anti_voltage;

	BeadAlgorithm ba;
	//BeadAlgorithm_BF ba;
	unsigned nofBeads;
	double* beaddiam;

	double *distmat;
	double dist;

	int *magnification;

	double *c1,*c2,*c3,*F0,*d0;

	double *x0,*x1,*x2,*x3,*x4;

	Frame2D<unsigned short> *needleframe;

	unsigned *bead_lost;

	double* duration;

	unsigned nofFrames;
	int FramesTaken;

	Tweezers* mt;

	int* save_freq;
	bool *stopCalibVideo;

	bool calib_running;

	int framecounter;

	Frame2D<unsigned short>* saveframe;

};

// sub threads
void CamThread(void *args);
void SaveThread(void *args);
void TriggerThread(void *args);
void DegaussThread(void *args);
void CaptureCycle(void* args);
void CamThreadCycle(void *args);

void CamThreadCalib(void *args);
void SaveThreadCalib(void *args);
void SaveThreadCalibVideo(void *args);
void TriggerThreadCalib(void* args);

void SaveThreadCalibRem(void *args);

void SaveThreadTwisting(void *args);
void TriggerThreadTwisting(void* args);

void SaveThreadDrift(void *args);
void TriggerThreadDrift(void* args);
void CamThreadCycleDrift(void* args);

void SaveImgThread(void* lpParam);


// read in config file
void GetConfig(string filename, threadinfo* tlinfo);

double current_15(double d, void* lpParam);
double current(double force, double dist, void *args);
double force(double current, double dist, void *args);
float64* sinus(float amplitude, float frequenz, int b);
int32 ConstVoltageOut(TaskHandle taskHandle, float U);

int32 DegaussCycle(TaskHandle taskHandle, float U, float duration);

// create and goto directory
unsigned int CreateAndGotoDir(const char* directory);

// trigger certain number of frames
void TriggerFrames(unsigned int nr, double extime);
