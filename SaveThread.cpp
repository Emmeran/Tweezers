#include "Tweezers_CL.h"

void SaveThread(void* lpParam)
{

	bool save_postponed = FALSE;

	// get thread params
	threadinfo* t = (threadinfo*)lpParam;

	t->nofFrames = *t->cycles * (*t->rampframes + *t->nframes + *t->rframes + *t->fframes + *t->zframes);

	*t->bead_lost = 0;

	t->capturing = true;
	t->saveFlag = (HANDLE)0;

	BeadCoord* bc;

	// thread handle
	HANDLE hThread;
	HANDLE sThread;

	HANDLE SaveEvent = CreateEvent(NULL,TRUE,FALSE,"SaveEvent");

	// filenames
	ostringstream fnamestream;
	char cellnum[12];
	
    // frame counters
	unsigned long lastSavedFrame = 0,lastCapturedFrame = 0,llatestFrame = 0;

	// create cellnum string for directory and go there
	sprintf(cellnum,"%03d",t->cell);
	CreateAndGotoDir(cellnum);

	// move overview image to right folder
	DeleteFile("overview.tif");
	MoveFileEx("../overview.tif","overview.tif",MOVEFILE_WRITE_THROUGH);

	// write needle file
	ofstream nf;
	nf.open("needle.txt");
	for(unsigned x=0;x<1344;x++)
		for(unsigned y=0;y<*t->array_size;y++)
			if(t->needleframe->get_Pointer()[(y+t->array_top)*1344+x])
				nf<<x<<"\t"<<y<<endl;
	nf.close();

	// copy config file to new location
	DeleteFile("config.txt");
	CopyFile(t->oldconf.c_str(),"config.txt",0);

	// write time.txt
	ofstream tf;
	tf.open("time.txt");
	tf.precision(10);
	for(unsigned k=0;k<t->nofFrames;k++)
		tf<<*t->delta*1E-3*k+*t->wait*1E-3<<"\n";
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
	int i=0,j=0,frame_to_save = 0;

	t->framecounter = 0;

	// maybe sometime MM doesn't answer??

	// get initial position from MM and activate remote control
	double posx=0, posy=0, posz=0;
	cerr<<"activating MM remote...";
	t->mm->Position_mustep(&posx,&posy,&posz);
	t->mm->ActivateRemote();
	cerr<<"done."<<endl;

	// original position of bead
	double orig_x = 0, orig_y = 0;

	// position of needle (mm coordinates)
	double mm_x = posx, mm_y = posy;
	
	// amount of pixels needle has moved from original position
	int n_dx = 0, n_dy = 0, old_n_dx = 0, old_n_dy = 0;

	int bead_delta_x = 0, bead_delta_y = 0, bead_old_x = 0, bead_old_y = 0, bead_x = 0, bead_y = 0;

	// start cam thread
	hThread = (HANDLE)_beginthread(CamThread,0,t);
	if(*t->save_freq)
	{
		sThread = (HANDLE)_beginthread(SaveImgThread,0,t);
		SetThreadPriority(sThread, THREAD_PRIORITY_LOWEST);
	}

	// record  frames
	while(i < (t->nofFrames))
	{
		if(cframe.get_latestSavedFrame() < cframe.get_latestCapturedFrame()) // >= Saved
		{
			// buffer overflow
			if(cframe.get_latestCapturedFrame()-cframe.get_latestSavedFrame() > SB_BUFFER_SIZE) t->error = 1;
			
			// initial find beads, store original bead position for needle movement
			if(!i){
				// find beads
				cerr<<"found "<<t->ba.findBeads(cframe)<<" beads!\n";
				bc = cframe.get_BeadInfo()->bc;
				t->nofBeads = cframe.get_BeadInfo()->nofBeads;
				
				// write dim.txt
//				for(unsigned j=0;j<t->nofBeads;j++) dimf<<t->ba.BeadSize(cframe)<<endl;
				dimf.close();
				
				// get number of beads and distance of closest bead (for ramp)
				t->dist = 999;
				double temp_dist = 0;
				for(unsigned k=0;k<t->nofBeads;k++)
				{
					temp_dist = t->distmat[ int(bc[k].y+t->array_top)*1344 + int(bc[k].x) ] * 6.45/(*t->magnification);
					if (temp_dist < t->dist)
					{
						t->closest_idx = k;
						t->dist = temp_dist;
					}
				}

				// sotre original bead position for needle movement
				orig_x = bc[t->closest_idx].x;
				orig_y = bc[t->closest_idx].y;

			}
			else
			{
				bead_old_x = bc[t->closest_idx].x;
				bead_old_y = bc[t->closest_idx].y;

				t->ba.trackBeads(cframe);

				bead_x = bc[t->closest_idx].x;
				bead_y = bc[t->closest_idx].y;
				
				bead_delta_x = bead_x - bead_old_x;
				bead_delta_y = bead_y - bead_old_y;

				// new dist before moving needle again
				t->dist = t->distmat[ int(bead_y + t->array_top - n_dy)*1344 + int(bead_x - n_dx) ] * 6.45/(*t->magnification);
			}
		
			old_n_dx = n_dx;
			old_n_dy = n_dy;

			// if needle feedback on, adapt needle position

	if(*t->move_needle)
			{
			if( (bc[t->closest_idx].valid) && (t->dist < 15.0) )
			{
				// calculate new position relative to old position
				mm_x = mm_x + 25.6*(bead_delta_x)*6.45/float(*t->magnification);
				mm_y = mm_y - 25.6*(bead_delta_y)*6.45/float(*t->magnification);

				// send signal to COM
				t->mm->gotoAbs_mustep_fast(mm_x,mm_y,posz);
				
				// new needle position for distance calculation
				n_dx += bead_delta_x;
				n_dy += bead_delta_y;
			}
			}

			// write beads to eval.txt - use old needle position data
			for(j=0;j<cframe.get_BeadInfo()->nofBeads;j++){
				of<<bc[j].x<<"\t"<<bc[j].y<<"\t";
				if(bc[j].valid)
					of<<bc[j].sum<<"\t";
				else
					of<<bc[j].valid<<"\t";
					of<<t->distmat[int(bc[j].y+t->array_top-old_n_dy)*1344+int(bc[j].x-old_n_dx)]<<"\t";
			}
		
			of<<endl;
					
			// time to save? send signal
			if( (*t->save_freq)) 
			{
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

				// output information	
			cerr<<"cap: "<<cframe.get_latestCapturedFrame()<<" sav: "<<cframe.get_latestSavedFrame()<<" diff: "<<cframe.get_latestCapturedFrame()-cframe.get_latestSavedFrame()<<"\r";

			cframe++;
// Temporaer fuer Abriss video	die beiden naechsten Zeilen auskommentieren		
// emmeran@gmx.net: 3. July 2012: removed commenting from Navid(?) to make video stop if bead rips again
			if (bc[t->closest_idx].sum < 30 || bc[t->closest_idx].valid==0)
				*t->bead_lost = i-1;

			i++;		

			if(*t->bead_lost
// emmeran@gmx.net: 05/10/2012 16:16 - dont stop recording if bead is lost in Creep-noint protocol
				&& *t->protocol != "Creep-noint"
// end of change
				)
				break;
		}
		if(*t->bead_lost
// emmeran@gmx.net: 05/10/2012 16:16 - dont stop recording if bead is lost in Creep-noint protocol
			&& *t->protocol != "Creep-noint"
// end of change
		)
			break;
	}

	t->mm->ActivateHandcontrol();

	of.close();

	ofstream gp;
	gp.open("plot.txt");
	gp<<"plot \"eval.txt\" using 1";
	gp.close();
	system("wgnuplot.exe plot.txt -");

	t->capturing = false;

	WaitForSingleObject(hThread, INFINITE);
	if(*t->save_freq)
		WaitForSingleObject(sThread, INFINITE);


	return;
}