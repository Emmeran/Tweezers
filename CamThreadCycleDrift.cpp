#include "Tweezers_CL.h"
#include "Tweezers.h"

void CamThreadCycleDrift(void* lpParam)
{
	// thread handle
	HANDLE hThread;

	BeadCoord* bc;

	unsigned xn_min = 1344;
	unsigned yn_min = 0;

	char cc = '0';

	// get thread params
	threadinfo* t = (threadinfo*)lpParam;

	// set cam parameters
	long lm=1;
	t->cam->setSimple_Lightmode(lm);
	//t->cam->setSave_Trigger_Mode(HamamatsuCamera::CAP::TRIGGER_INTERNAL);
	t->cam->setSave_Trigger_Mode(HamamatsuCamera::CAP::TRIGGER_EDGE);
	t->cam->setSave_Trigger_Polarity(HamamatsuCamera::CAP::TRIGGER_POS);
	t->cam->set_SubArray(0,0,1344,1024);
	double ext = *t->extime*1E-3;
	t->cam->setSimple_ExposureTime(ext);

	// init capture frame
	t->sframe = new SBwinCaptureFrame2D<unsigned short>(1344,1024,SB_BUFFER_SIZE);
	t->sframe->set_autoBcst(FALSE);
	t->sframe->set_AlwaysDisplay(TRUE);

	// init save frame for tracking
	SBwinSaveFrame2D<unsigned short> frame2(1344,1024,SB_BUFFER_SIZE);
	frame2.zero();

	// show key definitions
	cerr<<"\n\n(n) = next measurement (#"<<t->cell+1<<")";
	if (t->cell>0) cerr<<"\n(r) = repeat this measurement (#"<<t->cell<<")";
	cerr<<"\n\n(g/b) = Bead Window\t(j/m) = BeadDistance";
	cerr<<"\n(6/7) = extime +/-";
	cerr<<"\n(s/x) = Low Threshold\t(d/c) = Bead Diameter";
	cerr<<"\n(a/y) = High Threshold\t(q) = quit program\n\n";

	int j=0;

	t->dist = 999;

	// cycle
	while(1){

		// loop while no key pressed
		while(!_kbhit()){

			// let camera wait for image
			hThread = (HANDLE)_beginthread(CaptureCycle,0,t);
			Sleep(100);
			TriggerFrames(1, *t->extime);
			WaitForSingleObject(hThread, INFINITE);

			//*t->cam>>sframe;

			// find beads in image
			t->ba.findBeads(frame2);

			bc = frame2.get_BeadInfo()->bc;
			t->nofBeads = frame2.get_BeadInfo()->nofBeads;

			// send paint signal and update frame pointer
			// t->sframe->Bcst();
			// frame2.inc_FramePointer();

			// now broadcast paint signal and increase pointer manually
			t->sframe->Bcst();
			frame2.inc_FramePointer();

			// output live disp information
			cerr<<"exp="<<*t->extime<<" BTHi="<<t->ba.BeadThreshHigh<<" BTLo="<<t->ba.BeadThreshLow<<" BDia="<<t->ba.BeadDiameter<<" BWin="<<t->ba.BeadWindow<<" BDist="<<t->ba.BeadDistance<<" nOfBeads = "<<t->nofBeads<<"  \r";
		}


		// get key and react
		t->ch = _getch();

		if(t->ch=='a')
			t->ba.BeadThreshHigh+=50;
		if(t->ch=='y')
			t->ba.BeadThreshHigh-=50;

		if(t->ch=='s')
			t->ba.BeadThreshLow+=50;
		if(t->ch=='x')
			t->ba.BeadThreshLow-=50;

		if(t->ch=='d')
			t->ba.BeadDiameter+=1;
		if(t->ch=='c')
			t->ba.BeadDiameter-=1;

		if(t->ch=='j')
			t->ba.BeadDistance+=1;
		if(t->ch=='m')
			t->ba.BeadDistance-=1;

		if(t->ch=='g')
			t->ba.BeadWindow+=1;
		if(t->ch=='b')
			t->ba.BeadWindow-=1;



		if(t->ch=='6')
		{
			*t->extime=*t->extime+0.5;
			double ext = *t->extime*1E-3;
			t->cam->setSimple_ExposureTime(ext);
		}

		if(t->ch=='7')
		{
			*t->extime=*t->extime-0.5;
			if(*t->extime<0.5) *t->extime = 0.5;
			double ext = *t->extime*1E-3;
			t->cam->setSimple_ExposureTime(ext);
		}

		if(t->ch=='q' || t->ch=='n' || t->ch=='r') break;
	}

	delete t->sframe;

	t->cam->Image_Idle();
	t->cam->Image_Free();

	return;
}
