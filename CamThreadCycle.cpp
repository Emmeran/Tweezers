#include "Tweezers_CL.h"
#include "Tweezers.h"

void CamThreadCycle(void* lpParam)
{
	// thread handle
	HANDLE hThread;

	char        errBuff[2048];

	BeadCoord* bc;

	unsigned xn_min = 1344;
	unsigned yn_min = 0;

	char cc = '0';

	bool needle = TRUE;
	bool adjust_array = false;

	// get thread params
	threadinfo* t = (threadinfo*)lpParam;

	// light mode
	long lm=1;
	t->cam->setSimple_Lightmode(lm);

	// trigger
	t->cam->setSave_Trigger_Mode(HamamatsuCamera::CAP::TRIGGER_EDGE);
	t->cam->setSave_Trigger_Polarity(HamamatsuCamera::CAP::TRIGGER_POS);

	// subarray
	t->cam->set_SubArray(0,0,1344,1024);

	// exposure
	double ext = *t->extime*1E-3;
	t->cam->setSimple_ExposureTime(ext);

	// init save frame for tracking
	SBwinSaveFrame2D<unsigned short> frame2(1344,1024,SB_BUFFER_SIZE);
	frame2.zero();

	// init capture frame
	t->sframe = new SBwinCaptureFrame2D<unsigned short>(1344,1024,SB_BUFFER_SIZE);
	t->sframe->set_autoBcst(FALSE);
	t->sframe->set_AlwaysDisplay(TRUE);

	// show key definitions
	cerr<<"\n\n(n) = next measurement (#"<<t->cell+1<<")";
	if (t->cell>0) cerr<<"\n(r) = repeat this measurement (#"<<t->cell<<")";
	cerr<<"\n\n(g/b) = Bead Window\t(j/m) = BeadDistance";
	cerr<<"\n(6/7) = extime +/-\t(5) = Needle update";
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
			if(t->mt->TriggerFrames(1))
			{
				DAQmxGetExtendedErrorInfo(errBuff,2048);
				cerr<<errBuff;
			}
			WaitForSingleObject(hThread, INFINITE);

			// find beads in image
			t->ba.findBeads(frame2);

			// copy bead info
			bc = frame2.get_BeadInfo()->bc;
			t->nofBeads = frame2.get_BeadInfo()->nofBeads;

			// find needle
			if(needle){
				delete t->needleframe;
				t->needleframe = &frame2.find_Needle();
				needle = FALSE;
			}

			// Adjust array position acording to the needle tip
			if(adjust_array){
				xn_min = 1344;
				yn_min = 0;

				// find position of needle tip
				for(unsigned xn=0;xn<1344;xn++) {
					for(unsigned yn=0;yn<1024;yn++) {
						if(t->needleframe->get_Pointer()[yn*1344+xn] && xn<xn_min)
						{
							xn_min = xn;
							yn_min = yn;
						}
					}
				}

				t->array_top = yn_min-(*t->array_size)/2;
				if(t->array_top < 0) t->array_top = 0;
				if(t->array_top > 1024-(*t->array_size)-8) t->array_top = 1024-(*t->array_size)-8;
				t->array_top = ( (int)(t->array_top/8) ) * 8;
				adjust_array = false;
			}

			// draw array borders
			for(unsigned i=0;i<1344;i++)
			{
				frame2.get_Pointer()[1344*t->array_top+i] = 0;
				frame2.get_Pointer()[1344*(t->array_top+*t->array_size)+i] = 0;
			}

			// mark needle red
			for(unsigned i=0;i<1344*1024;i++)
				if((t->needleframe->get_Pointer())[i])
					frame2.get_Pointer()[i] = 4096;

			xn_min = 1344;
			yn_min = 0;

			// find position of needle tip in subframe
			for(unsigned xn=0;xn<1344;xn++) {
				for(unsigned yn=0;yn<*t->array_size;yn++) {
					if(t->needleframe->get_Pointer()[(yn+t->array_top)*1344+xn] && xn<xn_min)
					{
						xn_min = xn;
						yn_min = yn + t->array_top;
					}
				}
			}

			// get number of beads and distance of closest bead (for ramp)
			t->dist = 999;
			double temp_dist = 0;
			for(unsigned j=0;j<t->nofBeads;j++)
			{
				// dist of this bead in µm
				temp_dist = sqrt(pow((int)(xn_min-bc[j].x),2) + pow((int)(yn_min-bc[j].y),2))* 6.45/(*t->magnification);
				// if closer than the bead before => use this one
				if (temp_dist < t->dist && bc[j].y > t->array_top && bc[j].y < *t->array_size + t->array_top)
				{
					t->closest_idx = i;
					t->dist = temp_dist;
				}
			}

			// now broadcast paint signal and increase pointer manually
			t->sframe->Bcst();
			frame2.inc_FramePointer();

			// output live disp information
			cerr<<"exp="<<*t->extime<<" BTHi="<<t->ba.BeadThreshHigh<<" BTLo="<<t->ba.BeadThreshLow<<" BDia="<<t->ba.BeadDiameter<<" BWin="<<t->ba.BeadWindow<<" BDist="<<t->ba.BeadDistance<<" dist="<<(int)t->dist<<" um ("<<t->nofBeads<<") MT="<<cc<<"  \r";
		}

		// get key and react
		t->ch = _getch();

		// array position
		if(t->ch=='+' && t->array_top<=1015)
			t->array_top+=8;

		if(t->ch=='-' && t->array_top>=8)
			t->array_top-=8;

		// high threshold
		if(t->ch=='a')
			t->ba.BeadThreshHigh+=50;
		if(t->ch=='y')
			t->ba.BeadThreshHigh-=50;

		// low threshold
		if(t->ch=='s')
			t->ba.BeadThreshLow+=50;
		if(t->ch=='x')
			t->ba.BeadThreshLow-=50;

		// diameter
		if(t->ch=='d')
			t->ba.BeadDiameter+=1;
		if(t->ch=='c')
			t->ba.BeadDiameter-=1;

		// distance
		if(t->ch=='j')
			t->ba.BeadDistance+=1;
		if(t->ch=='m')
			t->ba.BeadDistance-=1;

		// window
		if(t->ch=='g')
			t->ba.BeadWindow+=1;
		if(t->ch=='b')
			t->ba.BeadWindow-=1;

		// find needle
		if(t->ch=='5')
		{
			needle = TRUE;
			adjust_array = true;
		}

		// find needle
		if(t->ch=='9')
		{			
			if(t->mt->Degauss())
		{
			DAQmxGetExtendedErrorInfo(errBuff,2048);
			cerr<<errBuff;
			t->mt->RecoverFromError();
		}

		cerr<<"\nresetting...\n"<<endl;

		if(t->mt->Reset())
		{
			DAQmxGetExtendedErrorInfo(errBuff,2048);
			cerr<<errBuff;
			t->mt->RecoverFromError();
		}

		t->mt->SetCurrent(*t->anti_voltage);

}
		// exposure time
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

		// continue
		if(t->ch=='q' || t->ch=='n' || t->ch=='r') break;
	}

	// get another frame for needle recognition
	hThread = (HANDLE)_beginthread(CaptureCycle,0,t);
	Sleep(100);
	if(t->mt->TriggerFrames(1))
	{
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		cerr<<errBuff;
	}
	WaitForSingleObject(hThread, INFINITE);

	// find beads
	cerr<<"\nfound "<<t->ba.findBeads(frame2)<<" beads!\n";

	// copy bead info
	bc = frame2.get_BeadInfo()->bc;
	t->nofBeads = frame2.get_BeadInfo()->nofBeads;

	// find needle
	delete t->needleframe;
	t->needleframe = &frame2.find_Needle();

	// find position of needle tip
	xn_min = 1344;
	yn_min = 0;
	for(unsigned xn=0;xn<1344;xn++) {
		for(unsigned yn=0;yn<1024;yn++) {
			if(t->needleframe->get_Pointer()[yn*1344+xn] && xn<xn_min)
			{
				xn_min = xn;
				yn_min = yn;
			}
		}
	}

	t->array_top = yn_min-(*t->array_size)/2;
	if(t->array_top < 0) t->array_top = 0;
	if(t->array_top > 1023-(*t->array_size)) t->array_top = 1023-(*t->array_size);
	t->array_top = ( (int)(t->array_top/8) ) * 8;

	// draw again array borders
	for(unsigned i=0;i<1344;i++)
	{
		frame2.get_Pointer()[1344*t->array_top+i] = 0;
		frame2.get_Pointer()[1344*(t->array_top+*t->array_size)+i] = 0;
	}

	// mark needle red
	for(unsigned i=0;i<1344*1024;i++)
		if((t->needleframe->get_Pointer())[i])
			frame2.get_Pointer()[i] = 4096;

	// send paint signal and save image
	t->sframe->Bcst();
	DeleteFile("overview.tif");
	frame2.saveCapturedImage("overview.tif");

	// find position of needle tip in subframe
	xn_min = 1344;
	yn_min = 0;

	for(unsigned xn=0;xn<1344;xn++) {
		for(unsigned yn=0;yn<*t->array_size;yn++) {
			if(t->needleframe->get_Pointer()[(yn+t->array_top)*1344+xn] && xn<xn_min)
			{
				xn_min = xn;
				yn_min = yn + t->array_top;
			}
		}
	}

	// create distance matrix from needle tip
	t->distmat = new double[1344*1024];
	for(unsigned x=0;x<1344;x++){
		for(unsigned y=0;y<1024;y++){
			if(t->needleframe->get_Pointer()[y*1344+x]){
				t->distmat[y*1344+x] = 0;
			}
			else
				t->distmat[y*1344+x] = sqrt(pow((int)(xn_min-x),2) + pow((int)(yn_min-y),2));
			//t->distmat[y*1344+x] = sqrt(pow(xn_min-x,2.0));
		}
	}

	// get number of beads and distance of closest bead (for ramp)
	t->dist = 999;
	double temp_dist = 0;
	for(unsigned j=0;j<t->nofBeads;j++)
	{
		// dist of this bead in µm
		temp_dist = sqrt(pow((int)(xn_min-bc[j].x),2) + pow((int)(yn_min-bc[j].y),2))* 6.45/(*t->magnification);
		// if closer than the bead before => use this one
		if (temp_dist < t->dist && bc[j].y > t->array_top && bc[j].y < *t->array_size + t->array_top)
		{
			t->closest_idx = i;
			t->dist = temp_dist;
		}
	}

	delete t->sframe;

	t->cam->Image_Idle();
	t->cam->Image_Free();

	return;
}

void CaptureCycle(void* lpParam)
{
	threadinfo* t = (threadinfo*)lpParam;

	if( (*t->cam>>*t->sframe) == FALSE ) cerr<<"ERROR!"<<endl;
}

void TriggerFrames(unsigned int nr, double extime)
{

	char        errBuff[2048];
//	int32       error;

	TaskHandle DOtaskHandle;
	DAQmxCreateTask("DO",&DOtaskHandle);
	//cerr<<"sending trigger pulse: "<<extime*1E-3<<" secs!\n";
	DAQmxCreateCOPulseChanTime(DOtaskHandle,"Dev1/Ctr0","",DAQmx_Val_Seconds,DAQmx_Val_Low,0.0,extime*1E-3,extime*1E-3);
	//DAQmxCfgImplicitTiming(DOtaskHandle, DAQmx_Val_ContSamps, 1);
	DAQmxCfgImplicitTiming(DOtaskHandle, DAQmx_Val_FiniteSamps, 1);
	DAQmxStartTask(DOtaskHandle);
	DAQmxWaitUntilTaskDone(DOtaskHandle,-1);
	DAQmxStopTask(DOtaskHandle);
	DAQmxClearTask(DOtaskHandle);

	DAQmxGetExtendedErrorInfo(errBuff,2048);
	cerr<<errBuff;
}