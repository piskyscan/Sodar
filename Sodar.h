/*
 * Sodar.h
 *
 *  Created on: 1 Jan 2018
 *      Author: aron
 */

#ifndef SODAR_H_
#define SODAR_H_

#define SPEED_OF_SOUND_MS 331.0
/* Used by main to communicate with parse_opt. */
struct arguments
{
  char *arg1;                   /* arg1 */
  char **strings;               /* [string…] */
  int silent, verbose, abort;   /* ‘-s’, ‘-v’, ‘--abort’ */
  char *output_file;            /* file arg to ‘--output’ */
  int repeat_count;             /* count arg to ‘--repeat’ */
    char *device;
    int hertz;
    int frames;
    double correlation;
    int width;
    double time;
    double ignore;
    double minStd;
};

typedef struct
{
	double correlation;
	int n;
	int offset;
	double sx;
	double correlationStd;
} Results;

typedef struct
{
	double correlation;
	double sx;
	double sy;
	double sxy;
} CorrResults;


extern int main_process(struct arguments *args);

extern void estimatePhaseShift(double *re1, double *im1, double *re2, double *im2, int N);
extern void estimatePhaseShift2(double *re1, double *im1, double *re2, double *im2, int N);
extern double correlation(double *ptr1, double *ptr2, int n, CorrResults* res);
extern void estimatePhaseShift3(double *raw1, double *raw2, int n, Results*resPtr);
extern void estimatePhaseShiftRaw2(double *data1, double *data2, int N);

#endif /* SODAR_H_ */
