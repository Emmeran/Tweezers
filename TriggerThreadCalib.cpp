#include "Tweezers_CL.h"
#include "Tweezers.h"
#include <iostream>

void TriggerThreadCalib(void* lpParam)
{
	// get thread params
	threadinfo* t = (threadinfo*)lpParam;

	char        errBuff[2048];

	// array for current values
	float64* curr = new float64[t->nofFrames*10];
	float64* indata	= new float64[*t->cycles * t->nofFrames * 2];
	
	double z_cur = *t->anti_voltage;
	double r_cur = t->mt->ResidualCurrent(*t->cur);
    
	for(int i=0;i<(*t->zframes)*10;i++)
		curr[i] = z_cur;

	for(int i=0;i<(*t->fframes)*10;i++)
		curr[i+*t->zframes*10] = *t->cur;

	for(int i=0;i<(*t->rframes)*10;i++)
		curr[i + *t->fframes*10 + *t->zframes*10] = r_cur;

	if(t->mt->Reset())
	{
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		cerr<<errBuff;
		t->mt->RecoverFromError();
	}
	
	cerr<<endl<<"--- press any key to stop ---"<<endl;

	t->calib_running = TRUE;

	Sleep(1000);
	
	// Anlegen einer neuen leeren Datei von newcurrent.txt um evtl existierende zu löschen
	ofstream strom;
	strom.open("newcurrent.txt");
	strom<<"";
	strom.close();
	
	int cycles = 0;
	while(true)
	{
		if (_kbhit())
			break;

		// emmeran@gmx.net, 22. 5. 2012: stop after number of cycles if using SaveThreadCalibVideo ^= capturing = false
		if (t->capturing && ++cycles > *t->cycles)
			break;

		cerr<<endl<<"new cycle..."<<endl;

		if(t->mt->CurrentRun(curr,indata,t))
		{
			DAQmxGetExtendedErrorInfo(errBuff,2048);
			cerr<<errBuff;
			t->mt->RecoverFromError();
		}

		// after first cycle: zframes need residual current
		if(curr[0]==*t->anti_voltage)
			for(int i=0;i<(*t->zframes)*10;i++)
				curr[i] = r_cur;
	}
	t->ch = _kbhit() ? _getch() : 0;

	t->calib_running = FALSE;

	t->mt->TriggerFrames(1); // to release cam...

/*	cerr<<"\ndegaussing...\n";

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
*/
	ofstream tf;
	tf.open("current.txt");
	for(int i=0;i<t->nofFrames - 1;i++)
		tf<<i*(*t->delta*1E-3)<<"\t"<<indata[i+1]<<endl;
	tf.close();

	ofstream mf;
	mf.open("mag.txt");
	for(int i=0;i<t->nofFrames;i++)
		mf<<i*(*t->delta*1E-3)<<"\t"<<indata[i+t->nofFrames]<<endl;
	mf.close();

	cerr<<"trigger done!\n";

	delete curr;
	delete indata;

	return;
}
