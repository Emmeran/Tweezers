#include "Tweezers_CL.h"

void CamThreadCalib(void* lpParam)
{
	// get thread params
	threadinfo* t = (threadinfo*)lpParam;

	// lightmode
	long lm=1;
	t->cam->setSimple_Lightmode(lm);

	// trigger
	t->cam->setSave_Trigger_Mode(HamamatsuCamera::CAP::TRIGGER_EDGE);
	t->cam->setSave_Trigger_Polarity(HamamatsuCamera::CAP::TRIGGER_POS);

	// subarray
	t->cam->set_SubArray(0,t->array_top,1344,*t->array_size);

	// exposure
	double ext = *t->extime*1E-3;
	t->cam->setSimple_ExposureTime(ext);

	// start trigger thread
	HANDLE hThread = (HANDLE)_beginthread(TriggerThreadCalib,0,t);
	SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);

	int i=0;

	// get frames for one cycle
	while(t->calib_running)
	{
		*t->cam>>*t->sframe;
		if ((i++)%10==0) t->sframe->Bcst();
	}

	// wait for trigger thread
	WaitForSingleObject(hThread, INFINITE);

	// reset cam
	t->cam->Image_Idle();
	t->cam->Image_Free();

	return;
}
