#include "Tweezers_CL.h"

void SaveThreadTwisting(void* lpParam)
{
	// get thread params
	threadinfo* t = (threadinfo*)lpParam;
	
	int x = 0;

	if (*t->freq<=1.0)
		x=0;
	else
    	x=ceil(*t->freq/8.0);

	double dt = 1/(*t->freq) * (x+(double)1/8); //zeitintervall zw. 2 pkt

	t->nofFrames = floor(*t->duration / dt);
	*t->cycles = (int)(t->nofFrames / 8);

	cerr<<"t->nofFrames: "<<t->nofFrames<<endl;

	*t->bead_lost = 0;

	// thread handle
	HANDLE hThread;

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

	// write time.txt
	ofstream tf;
	tf.open("time.txt");
	tf.precision(10);
	for(unsigned i=0;i<t->nofFrames;i++)
		tf<<dt<<"\n";
	tf.close();

	// open eval and dim files
	ofstream of;
	of.precision(10);
	of.open("eval.txt");

	ofstream dimf;
	dimf.precision(3);
	dimf.open("dim.txt");

	// initialize saver frame
	SBwinSaveFrame2D<unsigned short> cframe(1344,1024,SB_BUFFER_SIZE);
	cframe.zero();
	
	// counter variables
	int i=0,j=0;

	// start cam thread
	hThread = (HANDLE)_beginthread(CamThread,0,t);

	// output captured and saved counters
	cerr<<"cap: "<<cframe.get_latestCapturedFrame()<<" sav: "<<cframe.get_latestSavedFrame()<<endl;

	// record zero frames
	while(i<t->nofFrames)
	{
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
				dimf.close();
			}
			else t->ba.trackBeads(cframe);
			
			// write bead info to eval.txt
			for(j=0;j<cframe.get_BeadInfo()->nofBeads;j++){
				of<<cframe.get_BeadInfo()->bc[j].x<<"\t"<<cframe.get_BeadInfo()->bc[j].y<<"\t\t";
			}
			of<<endl;
			
			// output information	
			cerr<<"cap: "<<cframe.get_latestCapturedFrame()<<" sav: "<<cframe.get_latestSavedFrame()<<" diff: "<<cframe.get_latestCapturedFrame()-cframe.get_latestSavedFrame()<<"\r";

			// save image or increase pointer
			if(i % *t->save_freq == 0)
			{
				sprintf(fname,"frame_%06d.tif",i+1);
				cframe.saveCapturedImage(fname);
			}
			else cframe++;
			
			i++;

			// replace with user input
			if(*t->bead_lost) break;
		}
		if(*t->bead_lost) break;
	}

	of.close();

	ofstream gp;
	gp.open("plot.txt");
	gp<<"plot \"eval.txt\" using 1";
	gp.close();
	

	WaitForSingleObject(hThread, INFINITE);
	return;
}