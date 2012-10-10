#include "Tweezers_CL.h"
#include "Tweezers.h"

void TriggerThread(void* lpParam)
{
	// get thread params
	threadinfo* t = (threadinfo*)lpParam;

	char        errBuff[2048];

	double duration		= t->nofFrames/(*t->cycles) * (*t->delta)*1E-3;
	double FPS			= 1E3 / (*t->delta);
	int nofSamples		= Srate_HTSP * duration;
	double SampsPerFrame =  (double)Srate_HTSP / FPS;

	float64* force	= new float64[nofSamples];
	float64* indata	= new float64[*t->cycles * t->nofFrames * 2];

	// generate buffer with force values (Ramp case)
	if(*t->protocol=="Ramp")
	{
		for (int i=0; i < (int)(*t->zframes * SampsPerFrame); i++)
			force[i] = 0.0;

		for (int i=0;i<*t->rampframes*SampsPerFrame;i++)
			force[i + (int)(*t->zframes * SampsPerFrame)] = *t->force1 * 1E-9 * (float)i/(*t->rampframes*SampsPerFrame);

		for (int i=0; i < *t->rframes * SampsPerFrame; i++)
			force[i + (int)((*t->rampframes + *t->zframes)  * SampsPerFrame)] = 0.0;

		for (int i=0;i<*t->fframes*SampsPerFrame;i++)
			force[i  + (int)((*t->rampframes + *t->zframes + *t->rframes)  * SampsPerFrame)] = *t->force2 * 1E-9;

		for (int i=0;i<*t->nframes*SampsPerFrame;i++)
			force[i + (int)((*t->zframes + *t->fframes + *t->rampframes + *t->rframes)  * SampsPerFrame)] = 0.0;
	}

		// generate buffer with force values (Ramp-down case)
	if(*t->protocol=="Ramp-firststep")
	{
		for (int i=0; i < (int)(*t->zframes * SampsPerFrame); i++)
			force[i] = 0.0;

		for (int i=0;i<*t->fframes*SampsPerFrame;i++)
			force[i + (int)(*t->zframes * SampsPerFrame)] = 2 * 1E-9;

		for (int i=0;i<*t->nframes*SampsPerFrame;i++)
			force[i + (int)((*t->zframes + *t->fframes)  * SampsPerFrame)] = 0.0;

		for (int i=0;i<*t->rampframes*SampsPerFrame;i++)
			force[i + (int)((*t->fframes + *t->zframes + *t->nframes)  * SampsPerFrame)] = 2* 1E-9 + *t->force1 * 1E-9 * ((float)i+1)/(*t->rampframes*SampsPerFrame);

		for (int i=0; i < *t->rframes * SampsPerFrame; i++)
			force[i + (int)((*t->fframes + *t->zframes + *t->nframes + *t->rampframes)  * SampsPerFrame)] = 0.0;

	}

/*

if(*t->protocol=="Ramp-Step")
	{
		for (int i=0; i < (int)(*t->zframes * SampsPerFrame); i++)
			force[i] = 0.0;

		unsigned binsize = (int)(*t->rampframes*SampsPerFrame*0.0125);

		for (int i=0;i<binsize;i++) {
			force[i + (int)(*t->zframes * SampsPerFrame)] = 1E-9 * 2.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize] = 1E-9 * 2.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*2] = 1E-9 * 4.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*3] = 1E-9 * 4.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*4] = 1E-9 * 6.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*5] = 1E-9 * 6.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*6] = 1E-9 * 8.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*7] = 1E-9 * 8.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*8] = 1E-9 * 10.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*9] = 1E-9 * 10.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*10] = 1E-9 * 12.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*11] = 1E-9 * 12.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*12] = 1E-9 * 14.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*13] = 1E-9 * 14.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*14] = 1E-9 * 16.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*15] = 1E-9 * 16.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*16] = 1E-9 * 18.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*17] = 1E-9 * 18.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*18] = 1E-9 * 20.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*19] = 1E-9 * 20.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*20] = 1E-9 * 22.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*21] = 1E-9 * 22.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*22] = 1E-9 * 24.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*23] = 1E-9 * 24.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*24] = 1E-9 * 26.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*25] = 1E-9 * 26.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*26] = 1E-9 * 28.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*27] = 1E-9 * 28.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*28] = 1E-9 * 30.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*29] = 1E-9 * 30.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*30] = 1E-9 * 32.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*31] = 1E-9 * 32.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*32] = 1E-9 * 34.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*33] = 1E-9 * 34.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*34] = 1E-9 * 36.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*35] = 1E-9 * 36.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*36] = 1E-9 * 38.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*37] = 1E-9 * 38.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*38] = 1E-9 * 40.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*39] = 1E-9 * 40.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*40] = 1E-9 * 42.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*41] = 1E-9 * 42.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*42] = 1E-9 * 44.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*43] = 1E-9 * 44.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*44] = 1E-9 * 46.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*45] = 1E-9 * 46.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*46] = 1E-9 * 48.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*47] = 1E-9 * 48.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*48] = 1E-9 * 50.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*49] = 1E-9 * 50.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*50] = 1E-9 * 52.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*51] = 1E-9 * 52.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*52] = 1E-9 * 54.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*53] = 1E-9 * 54.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*54] = 1E-9 * 56.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*55] = 1E-9 * 56.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*56] = 1E-9 * 58.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*57] = 1E-9 * 58.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*58] = 1E-9 * 60.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*59] = 1E-9 * 60.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*60] = 1E-9 * 62.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*61] = 1E-9 * 62.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*62] = 1E-9 * 64.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*63] = 1E-9 * 64.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*64] = 1E-9 * 66.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*65] = 1E-9 * 66.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*66] = 1E-9 * 68.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*67] = 1E-9 * 68.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*68] = 1E-9 * 70.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*69] = 1E-9 * 70.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*70] = 1E-9 * 72.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*71] = 1E-9 * 72.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*72] = 1E-9 * 74.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*73] = 1E-9 * 74.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*74] = 1E-9 * 76.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*75] = 1E-9 * 76.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*76] = 1E-9 * 78.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*77] = 1E-9 * 78.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*78] = 1E-9 * 80.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*79] = 1E-9 * 80.0;
		}

		for (int i=0; i < *t->rframes * SampsPerFrame; i++)
			force[i + (int)((*t->rampframes + *t->zframes)  * SampsPerFrame)] = 0.0;

		for (int i=0;i<*t->fframes*SampsPerFrame;i++)
			force[i  + (int)((*t->rampframes + *t->zframes + *t->rframes)  * SampsPerFrame)] = *t->force2 * 1E-9;

		for (int i=0;i<*t->nframes*SampsPerFrame;i++)
			force[i + (int)((*t->zframes + *t->fframes + *t->rampframes + *t->rframes)  * SampsPerFrame)] = 0.0;
	}

*/

	// 2nN Ramp-Step
	if(*t->protocol=="Ramp-Step")
	{
		for (int i=0; i < (int)(*t->zframes * SampsPerFrame); i++)
			force[i] = 0.0;

		unsigned binsize = (int)(*t->rampframes*SampsPerFrame*0.0125);

		for (int i=0;i<binsize;i++) {
			force[i + (int)(*t->zframes * SampsPerFrame)] = 1E-9 * 2.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*2] = 1E-9 * 4.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*3] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*4] = 1E-9 * 6.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*5] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*6] = 1E-9 * 8.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*7] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*8] = 1E-9 * 10.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*9] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*10] = 1E-9 * 12.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*11] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*12] = 1E-9 * 14.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*13] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*14] = 1E-9 * 16.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*15] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*16] = 1E-9 * 18.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*17] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*18] = 1E-9 * 20.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*19] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*20] = 1E-9 * 22.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*21] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*22] = 1E-9 * 24.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*23] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*24] = 1E-9 * 26.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*25] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*26] = 1E-9 * 28.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*27] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*28] = 1E-9 * 30.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*29] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*30] = 1E-9 * 32.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*31] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*32] = 1E-9 * 34.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*33] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*34] = 1E-9 * 36.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*35] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*36] = 1E-9 * 38.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*37] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*38] = 1E-9 * 40.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*39] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*40] = 1E-9 * 42.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*41] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*42] = 1E-9 * 44.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*43] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*44] = 1E-9 * 46.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*45] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*46] = 1E-9 * 48.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*47] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*48] = 1E-9 * 50.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*49] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*50] = 1E-9 * 52.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*51] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*52] = 1E-9 * 54.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*53] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*54] = 1E-9 * 56.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*55] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*56] = 1E-9 * 58.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*57] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*58] = 1E-9 * 60.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*59] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*60] = 1E-9 * 62.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*61] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*62] = 1E-9 * 64.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*63] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*64] = 1E-9 * 66.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*65] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*66] = 1E-9 * 68.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*67] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*68] = 1E-9 * 70.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*69] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*70] = 1E-9 * 72.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*71] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*72] = 1E-9 * 74.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*73] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*74] = 1E-9 * 76.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*75] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*76] = 1E-9 * 78.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*77] = 1E-9 * 0.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*78] = 1E-9 * 80.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*79] = 1E-9 * 0.0;
		}

		for (int i=0; i < *t->rframes * SampsPerFrame; i++)
			force[i + (int)((*t->rampframes + *t->zframes)  * SampsPerFrame)] = 0.0;

		for (int i=0;i<*t->fframes*SampsPerFrame;i++)
			force[i  + (int)((*t->rampframes + *t->zframes + *t->rframes)  * SampsPerFrame)] = *t->force2 * 1E-9;

		for (int i=0;i<*t->nframes*SampsPerFrame;i++)
			force[i + (int)((*t->zframes + *t->fframes + *t->rampframes + *t->rframes)  * SampsPerFrame)] = 0.0;
	}


	// 8nN Ramp-Step
/*	if(*t->protocol=="Ramp-Step")
	{
		for (int i=0; i < (int)(*t->zframes * SampsPerFrame); i++)
			force[i] = 0.0;

		unsigned binsize = (int)(*t->rampframes*SampsPerFrame*0.025);

		for (int i=0;i<binsize;i++) {
			force[i + (int)(*t->zframes * SampsPerFrame)] = 1E-9 * 8.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize] = 1E-9 * 8.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*2] = 1E-9 * 8.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*3] = 1E-9 * 8.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*4] = 1E-9 * 16.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*5] = 1E-9 * 16.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*6] = 1E-9 * 16.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*7] = 1E-9 * 16.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*8] = 1E-9 * 24.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*9] = 1E-9 * 24.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*10] = 1E-9 * 24.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*11] = 1E-9 * 24.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*12] = 1E-9 * 32.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*13] = 1E-9 * 32.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*14] = 1E-9 * 32.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*15] = 1E-9 * 32.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*16] = 1E-9 * 40.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*17] = 1E-9 * 40.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*18] = 1E-9 * 40.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*19] = 1E-9 * 40.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*20] = 1E-9 * 48.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*21] = 1E-9 * 48.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*22] = 1E-9 * 48.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*23] = 1E-9 * 48.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*24] = 1E-9 * 56.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*25] = 1E-9 * 56.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*26] = 1E-9 * 56.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*27] = 1E-9 * 56.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*28] = 1E-9 * 64.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*29] = 1E-9 * 64.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*30] = 1E-9 * 64.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*31] = 1E-9 * 64.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*32] = 1E-9 * 72.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*33] = 1E-9 * 72.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*34] = 1E-9 * 72.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*35] = 1E-9 * 72.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*36] = 1E-9 * 80.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*37] = 1E-9 * 80.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*38] = 1E-9 * 80.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*39] = 1E-9 * 80.0;
		}

		for (int i=0; i < *t->rframes * SampsPerFrame; i++)
			force[i + (int)((*t->rampframes + *t->zframes)  * SampsPerFrame)] = 0.0;

		for (int i=0;i<*t->fframes*SampsPerFrame;i++)
			force[i  + (int)((*t->rampframes + *t->zframes + *t->rframes)  * SampsPerFrame)] = *t->force2 * 1E-9;

		for (int i=0;i<*t->nframes*SampsPerFrame;i++)
			force[i + (int)((*t->zframes + *t->fframes + *t->rampframes + *t->rframes)  * SampsPerFrame)] = 0.0;
	}
*/


/*	// 4nN Ramp-Step
	if(*t->protocol=="Ramp-Step")
	{
		for (int i=0; i < (int)(*t->zframes * SampsPerFrame); i++)
			force[i] = 0.0;

		unsigned binsize = (int)(*t->rampframes*SampsPerFrame*0.025);

		for (int i=0;i<binsize;i++) {
			force[i + (int)(*t->zframes * SampsPerFrame)] = 1E-9 * 4.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize] = 1E-9 * 4.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*2] = 1E-9 * 8.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*3] = 1E-9 * 8.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*4] = 1E-9 * 12.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*5] = 1E-9 * 12.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*6] = 1E-9 * 16.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*7] = 1E-9 * 16.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*8] = 1E-9 * 20.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*9] = 1E-9 * 20.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*10] = 1E-9 * 24.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*11] = 1E-9 * 24.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*12] = 1E-9 * 28.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*13] = 1E-9 * 28.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*14] = 1E-9 * 32.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*15] = 1E-9 * 32.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*16] = 1E-9 * 36.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*17] = 1E-9 * 36.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*18] = 1E-9 * 40.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*19] = 1E-9 * 40.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*20] = 1E-9 * 44.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*21] = 1E-9 * 44.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*22] = 1E-9 * 48.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*23] = 1E-9 * 48.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*24] = 1E-9 * 52.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*25] = 1E-9 * 52.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*26] = 1E-9 * 56.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*27] = 1E-9 * 56.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*28] = 1E-9 * 60.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*29] = 1E-9 * 60.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*30] = 1E-9 * 64.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*31] = 1E-9 * 64.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*32] = 1E-9 * 68.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*33] = 1E-9 * 68.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*34] = 1E-9 * 72.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*35] = 1E-9 * 72.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*36] = 1E-9 * 76.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*37] = 1E-9 * 76.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*38] = 1E-9 * 80.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*39] = 1E-9 * 80.0;
		}

		for (int i=0; i < *t->rframes * SampsPerFrame; i++)
			force[i + (int)((*t->rampframes + *t->zframes)  * SampsPerFrame)] = 0.0;

		for (int i=0;i<*t->fframes*SampsPerFrame;i++)
			force[i  + (int)((*t->rampframes + *t->zframes + *t->rframes)  * SampsPerFrame)] = *t->force2 * 1E-9;

		for (int i=0;i<*t->nframes*SampsPerFrame;i++)
			force[i + (int)((*t->zframes + *t->fframes + *t->rampframes + *t->rframes)  * SampsPerFrame)] = 0.0;
	}
*/
	/*

	if(*t->protocol=="Ramp-Step")
	{
		for (int i=0; i < (int)(*t->zframes * SampsPerFrame); i++)
			force[i] = 0.0;

		unsigned binsize = (int)(*t->rampframes*SampsPerFrame*0.025);

		for (int i=0;i<binsize;i++) {
			force[i + (int)(*t->zframes * SampsPerFrame)] = 1E-9 * 20.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize] = 1E-9 * 20.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*2] = 1E-9 * 20.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*3] = 1E-9 * 20.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*4] = 1E-9 * 20.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*5] = 1E-9 * 20.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*6] = 1E-9 * 20.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*7] = 1E-9 * 20.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*8] = 1E-9 * 20.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*9] = 1E-9 * 20.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*10] = 1E-9 * 40.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*11] = 1E-9 * 40.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*12] = 1E-9 * 40.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*13] = 1E-9 * 40.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*14] = 1E-9 * 40.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*15] = 1E-9 * 40.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*16] = 1E-9 * 40.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*17] = 1E-9 * 40.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*18] = 1E-9 * 40.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*19] = 1E-9 * 40.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*20] = 1E-9 * 60.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*21] = 1E-9 * 60.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*22] = 1E-9 * 60.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*23] = 1E-9 * 60.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*24] = 1E-9 * 60.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*25] = 1E-9 * 60.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*26] = 1E-9 * 60.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*27] = 1E-9 * 60.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*28] = 1E-9 * 60.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*29] = 1E-9 * 60.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*30] = 1E-9 * 80.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*31] = 1E-9 * 80.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*32] = 1E-9 * 80.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*33] = 1E-9 * 80.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*34] = 1E-9 * 80.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*35] = 1E-9 * 80.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*36] = 1E-9 * 80.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*37] = 1E-9 * 80.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*38] = 1E-9 * 80.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*39] = 1E-9 * 80.0;
		}

		for (int i=0; i < *t->rframes * SampsPerFrame; i++)
			force[i + (int)((*t->rampframes + *t->zframes)  * SampsPerFrame)] = 0.0;

		for (int i=0;i<*t->fframes*SampsPerFrame;i++)
			force[i  + (int)((*t->rampframes + *t->zframes + *t->rframes)  * SampsPerFrame)] = *t->force2 * 1E-9;

		for (int i=0;i<*t->nframes*SampsPerFrame;i++)
			force[i + (int)((*t->zframes + *t->fframes + *t->rampframes + *t->rframes)  * SampsPerFrame)] = 0.0;
	}

*/

	if(*t->protocol=="Ramp-LogStep")
	{
		for (int i=0; i < (int)(*t->zframes * SampsPerFrame); i++)
			force[i] = 0.0;

		unsigned binsize = (int)(*t->rampframes*SampsPerFrame*0.1);

		for (int i=0;i<binsize;i++) {
			force[i + (int)(*t->zframes * SampsPerFrame)] = 1E-9 * 0.5;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize] = 1E-9 * 1.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*2] = 1E-9 * 1.5;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*3] = 1E-9 * 2.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*4] = 1E-9 * 3.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*5] = 1E-9 * 4.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*6] = 1E-9 * 5.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*7] = 1E-9 * 6.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*8] = 1E-9 * 8.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*9] = 1E-9 * 10.0;
/*			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*10] = 1E-9 * 12.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*11] = 1E-9 * 14.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*12] = 1E-9 * 16.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*13] = 1E-9 * 18.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*14] = 1E-9 * 20.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*15] = 1E-9 * 22.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*16] = 1E-9 * 24.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*17] = 1E-9 * 26.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*18] = 1E-9 * 28.0;
			force[i + (int)(*t->zframes * SampsPerFrame) + binsize*19] = 1E-9 * 30.0;*/
		}

		for (int i=0; i < *t->rframes * SampsPerFrame; i++)
			force[i + (int)((*t->rampframes + *t->zframes)  * SampsPerFrame)] = 0.0;

		for (int i=0;i<*t->fframes*SampsPerFrame;i++)
			force[i  + (int)((*t->rampframes + *t->zframes + *t->rframes)  * SampsPerFrame)] = *t->force2 * 1E-9;

		for (int i=0;i<*t->nframes*SampsPerFrame;i++)
			force[i + (int)((*t->zframes + *t->fframes + *t->rampframes + *t->rframes)  * SampsPerFrame)] = 0.0;
	}

	// generate buffer with force values (Creep case)
	if(*t->protocol=="Creep" || *t->protocol=="Creep-noint")
	{
		cerr << "\ncreep protocol\n";
		for (int i=0;i<(int)*t->zframes*SampsPerFrame;i++)
			force[i] = 0.0;

		for (int i=0;i<*t->fframes*SampsPerFrame;i++)
			force[i + (int)(*t->zframes*SampsPerFrame)] = *t->force1 * 1E-9;

		for (int i=0;i<*t->rframes*SampsPerFrame;i++)
			force[i + (int)((*t->zframes + *t->fframes) * SampsPerFrame)] = 0.0;
	}

	ofstream f;
	f.open("force.txt");
	for(int i=0;i<nofSamples;i++)
		f<<force[i]<<endl;
	f.close();

	//t->mt->~Tweezers();
	delete t->mt;
	t->mt = new Tweezers(*t->F0,*t->d0,*t->c1,*t->c2,*t->c3,*t->anti_voltage,t);

	if(t->mt->Reset())
	{
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		cerr<<errBuff;
		t->mt->RecoverFromError();
	}
	
	t->mt->SetCurrent(*t->anti_voltage);

	//cerr<<"\nrunning...\n";

	if(t->mt->Run(force,indata,t))
	{
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		cerr<<errBuff;
		t->mt->RecoverFromError();
	}

	cerr<<"\ndegaussing...\n";

//if(*t->protocol!="Ramp" && *t->protocol!="Creep" && *t->protocol!="Ramp-Step" && *t->protocol!="Ramp-firststep")
if(*t->protocol!="Ramp" && *t->protocol!="Ramp-Step" && *t->protocol!="Ramp-firststep")
{
	if(t->mt->Degauss())
	{
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		cerr<<errBuff;
		t->mt->RecoverFromError();
	}

	if(t->mt->Reset())
	{
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		cerr<<errBuff;
		t->mt->RecoverFromError();
	}

	t->mt->SetCurrent(*t->anti_voltage);
}
	ofstream tf;
	tf.open("current.txt");
	for(int i=0;i < t->FramesTaken; i++)
		tf<<i*(*t->delta*1E-3)<<"\t"<<indata[i]<<endl;
	tf.close();

	ofstream mf;
	mf.open("mag.txt");
	for(int i=0;i< t->FramesTaken;i++)
		mf<<i*(*t->delta*1E-3)<<"\t"<<indata[i+t->FramesTaken]<<endl;
	mf.close();

	delete force;
	delete indata;

	cerr<<"trigger done!\n";
	return;
}
