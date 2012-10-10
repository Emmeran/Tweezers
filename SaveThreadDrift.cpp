#include "Tweezers_CL.h"

void SaveThreadDrift(void* lpParam)
{
	
	bool save_postponed = FALSE;
	
	// get thread params
	threadinfo* t = (threadinfo*)lpParam;
	
	t->nofFrames = *t->rampframes;

	*t->bead_lost = 0;

	t->capturing = true;

	int frame_to_save = 0;

	t->framecounter = 0;

	// thread handle
	HANDLE hThread;
	HANDLE sThread;

	HANDLE SaveEvent = CreateEvent(NULL,TRUE,FALSE,"SaveEvent");

	// filenames
	ostringstream fnamestream;
	char fname[128];
	char cellnum[12];
	
    // frame counters
	unsigned long lastSavedFrame = 0,lastCapturedFrame = 0,llatestFrame = 0;

	// create cellnum string for directory and go there
	sprintf(cellnum,"%03d",t->cell);
	CreateAndGotoDir(cellnum);

	// copy config file to new location
	CopyFile(t->oldconf.c_str(),"config.txt",0);

	// open eval and dim files
	ofstream of;
	of.precision(10);
	of.open("eval.txt");

//	ofstream dimf;
//	dimf.precision(3);
//	dimf.open("dim.txt");

	// initialize saver frame
	SBwinSaveFrame2D<unsigned short> cframe(1344,1024,SB_BUFFER_SIZE);
	cframe.zero();
	
	// counter variables
	int i=0,j=0;
	//*t->rframes = 0;
	// start cam thread
	hThread = (HANDLE)_beginthread(CamThread,0,t);
	if(*t->save_freq)
	{
		sThread = (HANDLE)_beginthread(SaveImgThread,0,t);
		SetThreadPriority(sThread, THREAD_PRIORITY_LOWEST);
	}
	
	// output captured and saved counters
	cerr<<"cap: "<<cframe.get_latestCapturedFrame()<<" sav: "<<cframe.get_latestSavedFrame()<<endl;

	// record zero frames
	while(i<t->nofFrames)
	{
		*t->rframes=i;
		
		if(cframe.get_latestSavedFrame() < cframe.get_latestCapturedFrame()) // >= Saved
		{
			// buffer overflow
			if(cframe.get_latestCapturedFrame()-cframe.get_latestSavedFrame() > SB_BUFFER_SIZE) t->error = 1;
			
			// initial find beads, store original bead position for needle movement
			if(!i){
				// find beads
				cerr<<"found "<<t->ba.findBeads(cframe)<<" beads!\n";
				BeadCoord *bc = cframe.get_BeadInfo()->bc;
				t->nofBeads = cframe.get_BeadInfo()->nofBeads;
				
				// write dim.txt
//				for(int j=0;j<t->nofBeads;j++) dimf<<t->ba.BeadSize(cframe)<<endl;
//				dimf.close();
			}
			else t->ba.trackBeads(cframe);

			// write bead info to eval.txt
			for(j=0;j<cframe.get_BeadInfo()->nofBeads;j++){
				of<<cframe.get_BeadInfo()->bc[j].x<<"\t"<<cframe.get_BeadInfo()->bc[j].y<<"\t\t";
			}
			of<<endl;
			
			// output information	
			cerr<<"cap: "<<cframe.get_latestCapturedFrame()<<" sav: "<<cframe.get_latestSavedFrame()<<" diff: "<<cframe.get_latestCapturedFrame()-cframe.get_latestSavedFrame()<<"\r";

			// time to save? send signal
			if( (*t->save_freq)) {
				if((i % *t->save_freq)==0)
				{
				// this frame should be saved
				frame_to_save = i;

				if(WaitForSingleObject(SaveEvent,0)==WAIT_OBJECT_0)
					// still set -> postpone
					save_postponed = true;
				else
				{
					// set framecounter
					t->framecounter = frame_to_save;
					// not set -> set to start saving
					SetEvent(SaveEvent);				
				}
			}
			else
			{
				if(save_postponed)
				{
					if(WaitForSingleObject(SaveEvent,0)==WAIT_OBJECT_0)
						// still set -> postpone
						save_postponed = true;
					else
					{
						// set framecounter
						t->framecounter = frame_to_save;
						// not set -> set to start saving
						SetEvent(SaveEvent);				
						// currently not saving? set event.
						save_postponed = false;
					}
				}
			}
			}

			cframe++;
			
			i++;
		}
	}

	*t->bead_lost = 1;

	of.close();

//	ofstream gp;
//	gp.open("plot.txt");
//	gp<<"plot \"eval.txt\" using 1";
//	gp.close();
	
	t->capturing = false;

	WaitForSingleObject(hThread, INFINITE);
	if(*t->save_freq)
		WaitForSingleObject(sThread, INFINITE);
	return;
}