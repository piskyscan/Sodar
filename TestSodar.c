/*
 * TestSodar.c
 *
 *  Created on: 28 Dec 2017
 *      Author: aron
 */

#include <stdio.h>
#include <stdint.h>

#include <fftw3.h>
#include "sodar_fft.h"

#define BUFFSIZE 4096

extern double testData[40960][2];
static void testEndian();

extern int32_t getBufferVal(char *src, int offset, int channel, int channels);

int testDatacolumn = sizeof(testData[0])/sizeof(testData[0][0]);
int testDatarow = sizeof(testData) / sizeof(testData[0]);

extern void estimatePhaseShift(double *re1, double *im1, double *re2, double *im2, int N);

int main()
{
	int i;
	int j;
	double re1[BUFFSIZE];
	double re2[BUFFSIZE];

	double im1[BUFFSIZE];
	double im2[BUFFSIZE];

	testEndian();

	FftHnd *fftHnd = initialiseFFT(BUFFSIZE);

	for (j = 0;j < testDatarow/BUFFSIZE;j++)
	{
	// Do first channel
	for (i = 0; i < BUFFSIZE;i++)
	{
		fftHnd->input_buffer[i] = testData[i+j*BUFFSIZE][0];
	}

	FFT_c(fftHnd);

	for (i = 0; i < BUFFSIZE;i++)
	{
		re1[i] = fftHnd->output_buffer[i][0];
		im1[i] = fftHnd->output_buffer[i][1];
	}

	// next do other channel.
	for (i = 0; i < BUFFSIZE;i++)
	{
		fftHnd->input_buffer[i] = testData[i+j*BUFFSIZE][1];
	}

	FFT_c(fftHnd);

	for (i = 0; i < BUFFSIZE;i++)
	{
		re2[i] = fftHnd->output_buffer[i][0];
		im2[i] = fftHnd->output_buffer[i][1];
	}

	estimatePhaseShift(re1, im1,re2,im2, BUFFSIZE/4);
	}

	terminateFFT(fftHnd);
}


char test[] = {0x00,0xC4,0xFF,0xFF,0x00,0x88,0xFF,0xFF};

void testEndian()
{
	int32_t v1 = getBufferVal(test,0,0,2);
	int32_t v2 = getBufferVal(test,0,1,2);

	if (v1 != -15360 || v2 != -30720)
	{
		fprintf(stderr,
				"Endian test failed\n");
	}
}
