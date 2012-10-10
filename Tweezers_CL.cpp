#include "Tweezers_CL.h"
#include "Tweezers.h"
#include "nidaqmx.h"

// main function
int main(int argc, char* argv[])
{
	// output version info
	cerr<<endl<<"Tweezers/Twisting ver.2006-06-22"<<endl;

	// thread handle for subthread
	HANDLE hThread;

	// global thread variables
	threadinfo t;
	t.stopCalibVideo = new bool(false);

	// error buffer
	char errBuff[2048];

	// set process priority to "realtime" (to be replaced by system call)
	HANDLE hProcess;
	int stat;
	hProcess = GetCurrentProcess();
	stat = SetPriorityClass(hProcess, REALTIME_PRIORITY_CLASS);

	// if no config file specified, use standard filename
	string config_str = "config.txt";
	GetConfig(config_str,&t);

	// where are we?
	char curdir[1024];
	_getcwd(curdir,1024);

	// generate original location of config file
	stringstream stream;
	stream << curdir << "\\" << config_str;
	t.oldconf = stream.str();

	// create and enter directories
	if( CreateAndGotoDir(t.location->c_str()) ) return 1;
	if( CreateAndGotoDir(t.identifier->c_str()) ) return 1;

	// where are we now?
	_getcwd(curdir,1024);

	// if not twisting, initialize MT object + MM
	if(*t.protocol!="Twisting" && *t.protocol!="Nerve" && *t.protocol!="Calib-Rem")
	{
		// initialize MT
		t.mt = new Tweezers(*t.F0,*t.d0,*t.c1,*t.c2,*t.c3,*t.anti_voltage,&t);

		// initial degauss and reset
		cerr<<"degaussing..."<<endl;
		if(t.mt->Degauss())
		{
			DAQmxGetExtendedErrorInfo(errBuff,2048);
			cerr<<errBuff;
		}
		cerr<<"resetting..."<<endl;
		if(t.mt->Reset())
		{
			DAQmxGetExtendedErrorInfo(errBuff,2048);
			cerr<<errBuff;
		}
		cerr<<"setting current...";
		t.mt->SetCurrent(*t.anti_voltage);

		cerr<<"done!"<<endl;

		// init micromanipulator
		cerr<<"* Looking for micromanipulator...";
		t.mm = new MikroEppendorf(wxCOM16);
		t.mm->ActivateHandcontrol();
		cerr<<t.mm->Version();	
	}

	if(*t.protocol=="Twisting")
		t.ba = BeadAlgorithm(7,20,40,1600,1800); // 10x
	else
		t.ba = BeadAlgorithm(25,40,40,1200,1200); // 40x
//		t.ba = BeadAlgorithm_BF(1800,1900,20,20,3,2);

	// initialize cam
	cerr<<"* looking for cam...";
	t.cp = new Camera_Pool();
	t.cam = new HamamatsuCamera(*t.cp, "Hamamatsu", "C4742-95-12ER", "CORECO PC-DIG");
	cerr<<"..found!"<<endl;

	// infinite input loop
	while(1)
	{
		// return to identifier directory
		if(CreateAndGotoDir(curdir)) return 1;

		// start live display thread
		if(*t.protocol=="Twisting" || *t.protocol=="Nerve")
			hThread = (HANDLE)_beginthread(CamThreadCycleDrift,0,&t);
		else
			if(*t.protocol != "Calib-Rem")
				hThread = (HANDLE)_beginthread(CamThreadCycle,0,&t);
			else
			{
				cerr<<"starting calibration cycle...";
				hThread = (HANDLE)_beginthread(SaveThreadCalibRem,0,&t);
				WaitForSingleObject(hThread, INFINITE);
				cerr<<"finished!"<<endl;
				t.ch='q';
			}

		// wait for live display thread to return (user input)
		WaitForSingleObject(hThread, INFINITE);

		// quit
		if(t.ch=='q') break;
	
		// new measurement
		if(t.ch=='n' ||t.ch=='r')
		{
			// increase counter if new measurement
			if(t.ch=='n'){
				cerr<<"increasing cell counter from "<<t.cell;
				t.cell++;
				cerr<<" to "<<t.cell<<endl;
			}
			else
				cerr<<"not increasing cell counter!"<<endl;

			// for creep or ramp -> check force-distance and start threads
			if(*t.protocol == "Creep" || *t.protocol == "Creep-noint" || *t.protocol == "Ramp-down" || *t.protocol == "Ramp" || *t.protocol == "Ramp-Step" || *t.protocol=="Ramp-LogStep" || *t.protocol=="Ramp-firststep")
			{
				// check if force protocol makes sense at distance bead-needle
				/*if((t.mt->current(*t.force1*1E-9,t.dist)<3.0) && (t.mt->current(*t.force1*1E-9,t.dist)>0.0) && (t.mt->current(*t.force2*1E-9,t.dist)<3.0) && (t.mt->current(*t.force2*1E-9,t.dist)>0.0)){
					cerr<<"\nforce "<<*t.force1<<" nN at "<<t.dist<<" um: I = "<<t.mt->current(*t.force1*1E-9,t.dist)<<"\n";*/
					if((t.mt->current(*t.force2*1E-9,t.dist)<3.0) && (t.mt->current(*t.force2*1E-9,t.dist)>0.0)){
					cerr<<"\nforce "<<*t.force2<<" nN at "<<t.dist<<" um: I = "<<t.mt->current(*t.force2*1E-9,t.dist)<<"\n";

					cerr << "\nProtocol: " << *t.protocol << "\n";

					// start saver and wait...
					cerr<<"starting measurement...";
					hThread = (HANDLE)_beginthread(SaveThread,0,&t);
					WaitForSingleObject(hThread, INFINITE);
					cerr<<"finished!"<<endl;
				}
				else{
					// output error and return to old counter
					cerr<<"\nforce "<<(*t.force1 > *t.force2 ? *t.force1 : *t.force2)<<" nN not possible for dist = "<<t.dist<<" um: I = "<<t.mt->current(*t.force1*1E-9,t.dist)<<"\n";
					if (t.ch=='n') t.cell--;
				}
			}

			// calibration
			if(*t.protocol == "Calib-Visc")
			{
				cerr<<"starting calibration cycle...";
				hThread = (HANDLE)_beginthread(SaveThreadCalib,0,&t);
				WaitForSingleObject(hThread, INFINITE);
				cerr<<"finished!"<<endl;
			}

			// calibration with video
			if(*t.protocol == "Calib-Visc-Video")
			{
				cerr<<"starting calibration cycle (with video)...";
				hThread = (HANDLE)_beginthread(SaveThreadCalibVideo,0,&t);
				WaitForSingleObject(hThread, INFINITE);
				cerr<<"finished!"<<endl;
			}

			// twisting: start corresponding saver thread
			if(*t.protocol == "Twisting")
			{
				cerr<<"starting magnetic twisting...";
				hThread = (HANDLE)_beginthread(SaveThreadTwisting,0,&t);
				WaitForSingleObject(hThread, INFINITE);
				cerr<<"finished!"<<endl;
			}

			if(*t.protocol == "Nerve")
			{
				cerr<<endl<<"starting measurement...";
				hThread = (HANDLE)_beginthread(SaveThreadDrift,0,&t);
				WaitForSingleObject(hThread, INFINITE);
				cerr<<"finished!"<<endl;
			}

		}

		// in case of an error, complain
		if(t.error)
		{
			Beep(2000,200);
			cerr<<"\n\n*** ERROR CAPTURING IMAGES! PLEASE REPEAT MEASUREMENT! ***\n";
			t.error = 0;
		}
		
		if(t.syncerror)
		{
			Beep(2000,200);
			cerr<<"\n\n*** OUT OF SYNC ! NOT SAVING MEASUREMENT! ***\n";
			if (t.ch=='n') t.cell--;
			t.syncerror = 0;
		}
	}


	delete t.cam;
	delete t.cp;

	cerr<<"\nExit...";
	return 0;
}


unsigned int CreateAndGotoDir(const char* directory)
{
	_mkdir(directory);

	// check if directory can be entered
	if (_chdir(directory)!=0)
	{
		cerr << "\n\nERROR CREATING DIRECTORY! CHECK LOCATION!\n";
		Beep(2000,1000);
		return 1;
	}
	else
	{
		// free space?
		__int64 freespace = 0;

		char curdir[1024];
		_getcwd(curdir,1024);

		GetDiskFreeSpaceEx(curdir,(PULARGE_INTEGER)&freespace,NULL,NULL);

		if(freespace < 500000000)	// 500 MB
		{
			cerr<<freespace<<endl;
			Beep(2000,200);
			Beep(2000,200);
			Beep(2000,200);
			cerr<<"\n\n:-O !!!! DISK IS ALMOST FULL !!!! \n";
			cerr<<"************************************\n\n";
			return 1;
		}
		else return 0;
	}
}
