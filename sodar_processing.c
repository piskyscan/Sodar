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
#include "Sodar.h"

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
	double re1val, re2val, im1val, im2val;

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

		re1val = re1[i];
		re2val = re2[i];

		im1val = im1[i];
		im2val = im2[i];


		abs1[i] = sqrt(re1val*re1val + im1val * im1val);
		abs2[i] = sqrt(re2val*re2val + im2val * im2val);

		weight = abs1[i]*abs2[i];

		phaseSum += phaseDiff/i * weight;
		weightSum += weight;
	}

	printf("phase diff %f, weight %f \n", phaseSum/weightSum * 4096,weightSum/N);
}

void estimatePhaseShiftRaw2(double *data1, double *data2, int N)
{
	Results results;
	estimatePhaseShift3(data1, data2, N, &results);
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

	estimatePhaseShift2(re1, im1,re2,im2, N/4);

}

void estimatePhaseShift2(double *re1, double *im1, double *re2, double *im2, int N)
{
	double phase1;
	double phase2;
	double phaseShift[N];
	double abs1[N];
	double abs2[N];
	double phaseDiff;
	int i;
	double isum = 0;
	double isum2 = 0;
	double phaseSum = 0;
	double phaseSum2 = 0;
	double xysum = 0;

	double weightSum = 0;
	double weight = 0;
	double re1val, re2val, im1val, im2val;

	double sx2, sy2, sxy,corr;

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

		re1val = re1[i];
		re2val = re2[i];

		im1val = im1[i];
		im2val = im2[i];


		abs1[i] = sqrt(re1val*re1val + im1val * im1val);
		abs2[i] = sqrt(re2val*re2val + im2val * im2val);

		weight = abs1[i]*abs2[i]*abs1[i]*abs2[i];

		phaseSum += phaseDiff * weight;
		phaseSum2 += phaseDiff * phaseDiff  * weight;

		isum  += i *  weight;
		isum2 += i*i *  weight;

		weightSum += weight;
		xysum += weight * (i * phaseDiff);
	}

	sx2 = isum2/weightSum - (isum/ weightSum)*(isum/ weightSum);
	sy2 = phaseSum2/weightSum - (phaseSum/ weightSum)*(phaseSum/ weightSum);
	sxy = xysum/weightSum - (isum/ weightSum)*(phaseSum/ weightSum);

	corr = sxy/sqrt(sx2*sy2);

	printf("phase diff, %f, corr, %f, weight %f \n", 4096*(xysum/weightSum)/(isum2/weightSum),corr, weightSum/N);
}


double correlation(double *ptr1, double *ptr2, int n,CorrResults* resPtr)
{
	int i;
	double sumx = 0;
	double sumx2 = 0;
	double sumy = 0;
	double sumy2 = 0;
	double sumxy = 0;
	double sx2, sy2, sxy;
	double corr;

	for (i = 0;i<n;i++)
	{
		sumx += ptr1[i];
		sumx2 += ptr1[i]*ptr1[i];
		sumy += ptr2[i];
		sumy2 += ptr2[i]*ptr2[i];
		sumxy += ptr1[i]*ptr2[i];
	}

	sx2 = sumx2/n - (sumx/i)*(sumx/i);

	sy2 = sumy2/n - (sumy/i)*(sumy/i);

	sxy = sumxy/n - (sumx/i)*(sumy/i);

	corr =  sxy/sqrt(sx2*sy2);

	if (resPtr != NULL)
	{
		resPtr->correlation = corr;
		resPtr->sx = sqrt(sx2);
		resPtr->sy = sqrt(sy2);
	}

	return corr;

}

void estimatePhaseShift3(double *raw1, double *raw2, int n,Results* resPtr)
{
	int i;
	int j;
	int size = 40;
	double corr;
	double maxCorrelation = -1;
	int index = 0;
	double sumCorr = 0;
	double sumCorr2 = 0;
	double meanCorr;
	CorrResults corrResults;
	double correlations[2*size];
	double a,b,c;
	int count = 2*size;

	// double correlations[n - 2 * size];

	double sumx = 0, sumy = 0, sumxy = 0, sumx2y = 0, sumx2,sumxx2,sumx22;
	double s11, s12, s22, sy1, sy2;

	int nToUse = n - 2 * size;

	for (i = -size;i<size;i++)
	{
		corr = correlation(&raw1[size],&raw2[size + i],nToUse,&corrResults);
		sumCorr += corr;
		sumCorr2 += corr * corr;

		if (corr >maxCorrelation )
		{
			maxCorrelation = corr;
			index = i;
		}
		correlations[i+size] = corr;
	}

	meanCorr = sumCorr/(2*size);

	resPtr->n = n;
	resPtr->correlation = maxCorrelation;
	resPtr->offset = index;
	resPtr->correlationStd = sqrt(sumCorr2/(2*size) -meanCorr*meanCorr );
	resPtr->sx = corrResults.sx;
	resPtr->sy = corrResults.sy;

	// now fit quadratic to correlations

	for (i = 0; i < 2*size;i++)
	{
		sumx += i;
		sumx2 += i*i;
		sumxy += i * correlations[i];
		sumxx2 += i*i*i;
		sumy += correlations[i];
		sumx22 += i*i*i*i;
		sumx2y += i*i*correlations[i];
//		printf("Corr, %d, %f\n", i,correlations[i]);
	}

	s11 = sumx2 - sumx * sumx/count;
	s12 = sumxx2 - sumx * sumx2/count;
	s22 = sumx22 -sumx2 *sumx2 /count;
	sy1 = sumxy - sumx * sumy/count;
	sy2 = sumx2y - sumy * sumx2 /count;

	if (s22 * s11 - s12*s12 != 0)
	{
	a = (sy2*s11 - sy1*s12)/(s22 * s11 - s12*s12);
	b = (sy1*s22 - sy2 *s12)/(s22* s11 - s12*s12);
	resPtr->bestFitIndex = -b/(2*a) - size;
	}
	else
	{
		resPtr->bestFitIndex = 0;
	}
}



int32_t getBufferVal(char *src, int offset, int channel, int channels)
{
	return ((int32_t *)src)[(offset * channels) + channel];
}

