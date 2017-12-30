/*
 * sodar_processing.c
 *
 *  Created on: 30 Dec 2017
 *      Author: piskyscan
 */

#include <stdio.h>
#include <math.h>
#include <fftw3.h>
#include <stdint.h>

#include "sodar_fft.h"


void estimatePhaseShift(double *re1, double *im1, double *re2, double *im2, int N)
{
	double phase1;
	double phase2;
	double phaseShift[N];
	double abs1[N];
	double abs2[N];
	double phaseDiff;
	int i;
	double phaseSum = 0;
	double weightSum = 0;
	double weight = 0;

	for (i = 1;i < N;i++)
	{
		phase1 = atan2(re1[i],im1[i]);
		phase2 = atan2(re2[i], im2[i]);
		phaseDiff = phase1 - phase2;
		if (phaseDiff < - M_PI)
		{
			phaseDiff += 2* M_PI;
		}

		if (phaseDiff > M_PI)
		{
			phaseDiff = phaseDiff - 2 * M_PI;
		}
		phaseShift[i] = phaseDiff;

		abs1[i] = sqrt(re1[i]*re1[i] + im1[i]*im1[i]);
		abs2[i] = sqrt(re2[i]*re2[i] + im2[i]*im2[i]);

		weight = abs1[i]*abs2[i];

		phaseSum += phaseDiff/i * weight;
		weightSum += weight;
	}

	printf("phase diff %f, weight %f \n", phaseSum/weightSum * 4096,weightSum);
}

void estimatePhaseShiftRaw(double *data1, double *data2, int N,FftHnd *fftHnd)
{
	int i;
	double re1[N];
	double re2[N];

	double im1[N];
	double im2[N];


	for (i = 0; i < N;i++)
	{
		fftHnd->input_buffer[i] = data1[i];
	}

	FFT_c(fftHnd);

	for (i = 0; i < N;i++)
	{
		re1[i] = fftHnd->output_buffer[i][0];
		im1[i] = fftHnd->output_buffer[i][1];
	}

	// next do other channel.
	for (i = 0; i < N;i++)
	{
		fftHnd->input_buffer[i] = data2[i];
	}

	FFT_c(fftHnd);

	for (i = 0; i < N;i++)
	{
		re2[i] = fftHnd->output_buffer[i][0];
		im2[i] = fftHnd->output_buffer[i][1];
	}

	estimatePhaseShift(re1, im1,re2,im2, N/4);

}


int32_t getBufferVal(char *src, int offset, int channel, int channels)
{
	return ((int32_t *)src)[(offset * channels) + channel];
}

