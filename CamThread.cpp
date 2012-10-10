#include "Tweezers_CL.h"

void CamThread(void* lpParam)
{
	// thread handle
	HANDLE hThread;

	// get thread params
	threadinfo* t = (threadinfo*)lpParam;

	// light mode
	long lm=1;
	t->cam->setSimple_Lightmode(lm);

	// trigger mode
	t->cam->setSave_Trigger_Mode(HamamatsuCamera::CAP::TRIGGER_EDGE);
	t->cam->setSave_Trigger_Polarity(HamamatsuCamera::CAP::TRIGGER_POS);

	// subarray
	t->cam->set_SubArray(0,t->array_top,1344,*t->array_size);

	// exposure
	double ext = *t->extime*1E-3;
	t->cam->setSimple_ExposureTime(ext);

	// write subarray info into file
	ofstream array;
	array.open("array.txt");
	array<<t->array_top<<endl<<*t->array_size<<endl<<"MOVE_NEEDLE: ";

#ifdef MOVE_NEEDLE
	array<<"TRUE";
#else
	array<<"FALSE";
#endif

	array<<endl<<"ANTI_VOLTAGE: "<<*t->anti_voltage<<endl;
	array.close();

	// initialize capture frame
	t->sframe = new SBwinCaptureFrame2D<unsigned short>(1344,1024,SB_BUFFER_SIZE);
	t->sframe->set_autoBcst(FALSE);
	t->sframe->set_AlwaysDisplay(FALSE);
	
	int dfreq = 0;

	// invoke trigger thread
	if(*t->protocol=="Creep" || *t->protocol=="Creep-noint" || *t->protocol=="Ramp-down" || *t->protocol=="Ramp" || *t->protocol=="Ramp-Step" || *t->protocol=="Ramp-LogStep" || *t->protocol=="Ramp-firststep") {
			hThread = (HANDLE)_beginthread(TriggerThread,0,t);
			SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
			dfreq = ceil(250.0 / *t->delta);
	}

	if(*t->protocol=="Twisting") {
		hThread = (HANDLE)_beginthread(TriggerThreadTwisting,0,t);
		dfreq = 2;
	}

	if(*t->protocol=="Nerve") {
		hThread = (HANDLE)_beginthread(TriggerThreadDrift,0,t);
		dfreq = 2;
	}

	// emmeran@gmx.net: 05/10/2012 16:16 - dont stop recording if bead is lost in Creep-noint protocol
	bool displayedMessage = false;
	// end of change

	// get frames for one cycle
	for(int i=0;i < (t->nofFrames);i++)
	{
		*t->cam>>*t->sframe;
	
		if ((i+1)%dfreq==0) t->sframe->Bcst();

		if(*t->bead_lost){
			// emmeran@gmx.net: 05/10/2012 16:16 - dont stop recording if bead is lost in Creep-noint protocol
			if (*t->protocol != "Creep-noint" || (*t->protocol == "Creep-noint" && !displayedMessage))
			{
				displayedMessage = true;
			// end of change
			cerr<<"\nbead lost!\n";
			// emmeran@gmx.net: 05/10/2012 16:16 - dont stop recording if bead is lost in Creep-noint protocol
			}

			if (*t->protocol != "Creep-noint")
			// end of change

			break;
		}
	}

	// wait for trigger thread to return
	WaitForSingleObject(hThread, INFINITE);

	cerr<<"cam done!\n";
	delete t->sframe;

	// reset camera
	t->cam->Image_Idle();
	t->cam->Image_Free();

	return;
}
