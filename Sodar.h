/*
 * Sodar.h
 *
 *  Created on: 1 Jan 2018
 *      Author: aron
 */

#ifndef SODAR_H_
#define SODAR_H_

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
};

extern int main_process(struct arguments *args);


#endif /* SODAR_H_ */
