#include "Tweezers_CL.h"

void SaveThreadCalib(void* lpParam)
{
	// get thread params
	threadinfo* t = (threadinfo*)lpParam;

	// emmeran@gmx.net, 22. 5. 2012: set capturing to false, video cannot be recorded with this thread
	//    use SaveThreadCalibVideo instead for video save
	t->capturing = false;

	// thread handle
	HANDLE hThread;

	BeadCoord* bc;

	// filenames
	ostringstream fnamestream;
	char fname[128];
	char cellnum[12];
	
    // frame counters
	unsigned long lastSavedFrame = 0,lastCapturedFrame = 0,llatestFrame = 0;

	// create cellnum string for directory and go there
	sprintf(cellnum,"%03d",t->cell);
	CreateAndGotoDir(cellnum);

	// move overview image to right folder
	DeleteFile("overview.tif");
	MoveFileEx("../overview.tif","overview.tif",MOVEFILE_WRITE_THROUGH);

	cerr<<"array size: "<<*t->array_size<<"\n";

	// write needle file
	ofstream nf;
	nf.open("needle.txt");
	for(unsigned x=0;x<1344;x++)
		for(unsigned y=0;y<*t->array_size;y++)
			if(t->needleframe->get_Pointer()[(y+t->array_top)*1344+x])
				nf<<x<<"\t"<<y<<endl;
	nf.close();

	// write time.txt
	ofstream tf;
	tf.open("time.txt");
	tf.precision(10);
	for(unsigned i=0;i<*t->zframes+*t->fframes+*t->rframes;i++)
		tf<<*t->delta*1E-3*i+*t->wait*1E-3<<"\n";
	tf.close();

	// copy config file to new location
	CopyFile(t->oldconf.c_str(),"config.txt",0);

	// open eval file
	ofstream of;
	of.precision(10);
	of.open("eval.txt");

	// initialize saver frame
	SBwinSaveFrame2D<unsigned short> cframe(1344,1024,SB_BUFFER_SIZE);
	cframe.zero();

	// initialize capture frame
	t->sframe = new SBwinCaptureFrame2D<unsigned short>(1344,1024,SB_BUFFER_SIZE);
	t->sframe->set_autoBcst(FALSE);
	t->sframe->set_AlwaysDisplay(FALSE);

	// counter variables
	int i=0,j=0;

	t->calib_running = TRUE;

	// start cam thread
	hThread = (HANDLE)_beginthread(CamThreadCalib,0,t);

	while(t->calib_running)
	{
		if(cframe.get_latestSavedFrame() < cframe.get_latestCapturedFrame()) // ahh - new frame!
		{
			// buffer overflow
			if(cframe.get_latestCapturedFrame()-cframe.get_latestSavedFrame() > SB_BUFFER_SIZE) t->error = 1;

			cerr<<cframe.get_latestCapturedFrame()<<"\r";
			// initial find beads, store original bead position for needle movement
			// maybe replace by pure trackbeads based on cycle thread?
			if(!i)
				t->ba.findBeads(cframe);
			else // track beads - normal case
				t->ba.trackBeads(cframe);
			
			bc = cframe.get_BeadInfo()->bc;
			t->nofBeads = cframe.get_BeadInfo()->nofBeads;

			// write beads to eval.txt
			for(j=0;j<cframe.get_BeadInfo()->nofBeads;j++){
				of<<bc[j].x<<"\t"<<bc[j].y<<"\t";
				if(bc[j].valid)
					of<<bc[j].sum<<"\t";
				else
					of<<bc[j].valid<<"\t";
				of<<t->distmat[int(bc[j].y+t->array_top)*1344+int(bc[j].x)]<<"\t";
			}
		
			of<<endl;
		
			cframe++;				
			i++;
		}
	}

	of.close();

	// wait for camera to return
	WaitForSingleObject(hThread, INFINITE);

	delete t->sframe;

	return;
}