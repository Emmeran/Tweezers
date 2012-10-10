#include "Tweezers_CL.h"

void SaveImgThread(void* lpParam)
{
	char fname[128];

	HANDLE SaveEvent = OpenEvent(EVENT_ALL_ACCESS,FALSE,"SaveEvent");
		
	DWORD WaitResult;

	// get thread params
	threadinfo* t = (threadinfo*)lpParam;

	unsigned frame_num = 0;

	SBwinSaveFrame2D<unsigned short> sframe(1344,1024,SB_BUFFER_SIZE);

	// while new images are to come
	while(t->capturing)
	{
		// wait up to 1 sec for signal to save
		WaitResult = WaitForSingleObject(SaveEvent,1000);
		// check result
		switch(WaitResult)
		{
			//save
			case WAIT_OBJECT_0:
				// create filename
				sprintf(fname,"frame_%06d.tif",t->framecounter);
				// save image
				sframe.saveCapturedImage((t->framecounter % 120),fname);
				ResetEvent(SaveEvent);
				break;
			default:
				break;
		}
	}
	return;
}