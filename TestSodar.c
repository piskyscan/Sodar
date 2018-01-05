/*
 * TestSodar.c
 *
 *  Created on: 28 Dec 2017
 *      Author: aron
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <fftw3.h>
#include "Sodar.h"
#include "sodar_fft.h"



#define BUFFSIZE 4096

extern double testData[40960][2];
static void testEndian();

extern int32_t getBufferVal(char *src, int offset, int channel, int channels);

int testDatacolumn = sizeof(testData[0])/sizeof(testData[0][0]);
int testDatarow = sizeof(testData) / sizeof(testData[0]);





int main()
{
	int i;
	int j;
	double re1[BUFFSIZE];
	double re2[BUFFSIZE];

	double im1[BUFFSIZE];
	double im2[BUFFSIZE];

	double raw[2][BUFFSIZE];
	Results results;
	double sintheta;
	double theta;

	testEndian();

	FftHnd *fftHnd = initialiseFFT(BUFFSIZE);

	for (j = 0;j < testDatarow/BUFFSIZE;j++)
	{
	// Do first channel
	for (i = 0; i < BUFFSIZE;i++)
	{
		fftHnd->input_buffer[i] = testData[i+j*BUFFSIZE][0];
		raw[0][i] = fftHnd->input_buffer[i];
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
		raw[1][i] = fftHnd->input_buffer[i];
	}

	FFT_c(fftHnd);

	for (i = 0; i < BUFFSIZE;i++)
	{
		re2[i] = fftHnd->output_buffer[i][0];
		im2[i] = fftHnd->output_buffer[i][1];
	}

	estimatePhaseShift3(&raw[0][0],&raw[1][0], BUFFSIZE, &results);

	if (results.correlation > 0.95)
	{
		sintheta = 1000*(results.offset * SPEED_OF_SOUND_MS/44000.0)/70.0;

		if (sintheta >= 1)
		{
			theta = 90.0;
		} else
			if (sintheta <= -1)
			{
				theta = -90.0;
			} else
			{
				theta = asin(sintheta) * 360.0/(2.0*M_PI);
			}

		printf("Angle %f, Correlation %f\n",theta, results.correlation);
	}


	// estimatePhaseShift2(re1, im1,re2,im2, BUFFSIZE/4);
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
