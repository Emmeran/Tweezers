#include "Tweezers_CL.h"
#include "Tweezers.h"
#include "nidaqmx.h"

// main function
int main(int argc, char* argv[])
{

	char        errBuff[2048];
	TaskHandle taskHandle, taskHandleRead;
	float64 data[1],i, voltage;
	data[0]=0;
	int32 read;

	DAQmxResetDevice("Dev1");

	// create tasks
	DAQmxCreateTask("",&taskHandle);
	DAQmxCreateTask("",&taskHandleRead);

	// create channels
	DAQmxCreateAOVoltageChan(taskHandle,"Dev1/ao0","",-10.0,10.0,DAQmx_Val_Volts,NULL);
	DAQmxCreateAIVoltageChan(taskHandleRead,"Dev1/ai2","",DAQmx_Val_RSE,-1.0,1.0,DAQmx_Val_Volts,NULL);

	ofstream of;
	of.open("eval.txt");

//////////////////////////////

	for (i = 0.5; i <= 3; i+=0.5)
	{

	cerr << i << "V" << endl;
	//of << i << ";";
/*	ConstVoltageOut(taskHandle, 3.0);

	Sleep(1000);
	DAQmxReadAnalogF64(taskHandleRead,1,10.0,DAQmx_Val_GroupByChannel,data,1,&read,NULL);
	cerr << endl << "field on: " << data[0]*1000 << " mT" << endl;

	of << data[0]*1000 << ";";
/*
/*	DegaussCycle(taskHandle, 3.0, 1.0);

	Sleep(1000);
	DAQmxReadAnalogF64(taskHandleRead,1,10.0,DAQmx_Val_GroupByChannel,data,1,&read,NULL);
	cerr<<"degauss: "<<data[0]*1000<<" mT"<<endl;
*/

	DegaussCycle(taskHandle, 3.0, 1.0);
	ConstVoltageOut(taskHandle, 0.0);

	Sleep(1000);
	DAQmxReadAnalogF64(taskHandleRead,1,10.0,DAQmx_Val_GroupByChannel,data,1,&read,NULL);
	cerr << "antivoltage: " << data[0]*1000 << " mT" << endl;
	//of << data[0]*1000 << ";" << ";";


	for (voltage = -0.2; voltage < -0.1; voltage=voltage+0.02)  // loop for different offsets
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
	}

	of << endl;

	}
	DAQmxClearTask(taskHandle);

	of.close();

}















	// thread handle
/*	HANDLE hThread;

	// input character
	char ch;

	// central variables
	threadinfo tinfo;

	// if no config file specified, use standard filename
	string config_str;
	if(argc!=2)
		config_str = "config.txt";
	else
		config_str = argv[1];

	// read in config file
	GetConfig(config_str,&tinfo);

	// where are we?
	char curdir[1024];
	_getcwd(curdir,1024);

	// generate original location of config file
	stringstream stream;
	stream << curdir << "\\" << config_str;
	tinfo.oldconf = stream.str();

	// create and enter directories
	if( CreateAndGotoDir(tinfo.location->c_str()) ) return 1;
	if( CreateAndGotoDir(tinfo.identifier->c_str()) ) return 1;

	// where are we now?
	_getcwd(curdir,1024);

	// initial degauss
	cerr<<"Degaussing...";
	TaskHandle taskHandle;
	DAQmxResetDevice("Dev1");
	DAQmxCreateTask("",&taskHandle);
	DAQmxCreateAOVoltageChan(taskHandle,"Dev1/ao0","",-10.0,10.0,DAQmx_Val_Volts,NULL);
	DegaussCycle(taskHandle, 3.0, 1.0);
	ConstVoltageOut(taskHandle, ANTI_VOLTAGE);
	DAQmxClearTask(taskHandle);
	cerr<<"done!\n";

	// init micromanipulator
	cerr<<"Looking for micromanipulator...";
	tinfo.mm = new MikroEppendorf(wxCOM15);
	tinfo.mm->ActivateHandcontrol();
	cerr<<tinfo.mm->Version()<<" found!\n";	

	// infinite input loop
	while(1)
	{
		// return to identifier directory
		if(CreateAndGotoDir(curdir)) return 1;

		// start live display thread
		hThread = (HANDLE)_beginthread(CamThreadCycle,0,&tinfo);
		WaitForSingleObject(hThread, INFINITE);

        if(tinfo.ch=='q') break;
		if(tinfo.ch=='n' ||tinfo.ch=='r')
		{
			// increase counter if new measurement
			if(tinfo.ch=='n') tinfo.cell++;

			if(*tinfo.protocol == "Creep")
			{
				cerr<<"starting Creep measurement...";
				hThread = (HANDLE)_beginthread(SaveThread,0,&tinfo);
				WaitForSingleObject(hThread, INFINITE);
				cerr<<"finished!"<<endl;
			}

			if(*tinfo.protocol == "Calib-Visc")
			{
				cerr<<"starting calibration cycle...";
				hThread = (HANDLE)_beginthread(SaveThreadCalib,0,&tinfo);
				WaitForSingleObject(hThread, INFINITE);
				cerr<<"finished!"<<endl;
			}

		}

		// in case of an error, complain
		if(tinfo.error)
		{
			Beep(2000,200);Beep(2000,200);Beep(2000,200);
			cerr<<"\n\n*** ERROR CAPTURING FRAMES! PLEASE REPEAT MEASUREMENT! ***\n";
		}
	}

	cerr<<"\nExit...";
	return 0;
}
*/

unsigned int CreateAndGotoDir(const char* directory)
{
	_mkdir(directory);

	// check if directory can be entered
	if (_chdir(directory))
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
