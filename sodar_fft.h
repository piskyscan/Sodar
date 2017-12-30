/*
 * sodar_fft.h
 *
 *  Created on: 30 Dec 2017
 *      Author: piskyscan
 */

#ifndef SODAR_FFT_H_
#define SODAR_FFT_H_

typedef struct _FftHnd
{
double *input_buffer;
fftw_complex* output_buffer;
fftw_plan plan;
} FftHnd;

#ifdef __cplusplus
extern "C" {
#endif

extern FftHnd *initialiseFFT(int N);
extern void terminateFFT(FftHnd *hndPtr);
extern void FFT_c(FftHnd *hndPtr);

#ifdef __cplusplus
}
#endif

#endif /* SODAR_FFT_H_ */
