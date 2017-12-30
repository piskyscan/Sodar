/*
 * FFT_code.cpp
 *
 *  Created on: 28 Dec 2017
 *
 *
 */

#include <cstdlib>
#include <complex>
#include <malloc.h>
#include <fftw3.h>

#include "sodar_fft.h"

using namespace std;

extern "C"
{
// setup FFT
FftHnd *initialiseFFT(int N)
{
	FftHnd *hndPtr = (FftHnd *)malloc(sizeof(FftHnd));
	hndPtr->input_buffer = (double *)fftw_malloc(sizeof(double)* N);

	hndPtr->output_buffer = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * (N/2+1));
	hndPtr->plan =  fftw_plan_dft_r2c_1d(N, hndPtr->input_buffer , hndPtr->output_buffer ,FFTW_ESTIMATE);

	return hndPtr;
}

// close FFT
void terminateFFT(FftHnd *hndPtr)
{
	fftw_free(hndPtr->input_buffer);
	fftw_free(hndPtr->output_buffer);
	fftw_destroy_plan(hndPtr->plan);
	free(hndPtr);
}
}

extern "C"
{
// calculate FFT
void FFT_c(FftHnd *hndPtr)
{
	fftw_execute(hndPtr->plan);
}
}
