
/* Use the newer ALSA API */

#define ALSA_PCM_NEW_HW_PARAMS_API

#include <math.h>
#include <alsa/asoundlib.h>
#include <fftw3.h>
#include "sodar_fft.h"

#define JITTER_RANGE 10  // range to check for correlation in.  -10 to +10 samples.
#define IGNORE_SECONDS 0.3  // Microphone take time to warm up.
#define BUFFSIZE 4096

snd_pcm_t *initialiseSoundCard(int hertz, int frames);
extern int32_t getBufferVal(char *src, int offset, int channel, int channels);
extern void estimatePhaseShiftRaw(double *data1, double *data2, int N,FftHnd *fftHnd);


FftHnd *fftHnd = NULL;

void runAtExit(void)
{
	if (fftHnd != NULL)
	{
		terminateFFT(fftHnd);
	}
}



int main()
{
  long loops;
  int rc;
  int size;
  unsigned int val;
  int dir;
  double position = 0;
  double varianceSum = 0;
  char output[100];
  int len;
  char *buffer;
  int ignore_count;
  int k;
  int countSum = 0;
  char temp[8];
  snd_pcm_t *handle;
  int hertz = 44000; // just about maximum.
  int i;

  double raw1[BUFFSIZE];
  double raw2[BUFFSIZE];

  int frames = BUFFSIZE;

  if (atexit(runAtExit))
  {

	  fprintf(stderr, "Unable to setup atexit function");
	    exit(1);
  }

  handle = initialiseSoundCard(hertz,frames);

  FftHnd *fftHnd = initialiseFFT(BUFFSIZE);


  loops = 100000 / hertz;

  ignore_count = IGNORE_SECONDS/val;

  k = 0;

  size = frames * 4; /* 2 bytes/sample, 2 channels */

  buffer = (char *) malloc(size);

  while (loops > 0)
  {
    loops--;k++;

    rc = snd_pcm_readi(handle, buffer, frames);
    if (rc == -EPIPE)
    {
      /* EPIPE means overrun */
      fprintf(stderr, "overrun occurred\n");
      snd_pcm_prepare(handle);
    }
    else
    {
    	if (rc < 0)
    	{
    		fprintf(stderr,
              "error from read: %s\n",
              snd_strerror(rc));

    	}
    	else
    	{
    		if (rc != (int)frames)
    		{
    			fprintf(stderr, "short read, read %d frames\n", rc);
    		}
    		else
    		{
    			// we have a buffer to process
    			for (i = 0;i < frames;i++)
    			{
    				raw1[i] = (double)getBufferVal(buffer,i,0,2);
    				raw2[i] = (double)getBufferVal(buffer,i,1,2);
    				estimatePhaseShiftRaw(raw1, raw2,frames, fftHnd);
    			}
    		}
    	}
    }
  }

  snd_pcm_drain(handle);
  snd_pcm_close(handle);

  // free(buffer);

  return 0;
}




snd_pcm_t *initialiseSoundCard(int hertz, int tframes)
{
	  int rc;
	  unsigned int val;
	  char *buffer;
	  int size;
	  int dir;

	  snd_pcm_uframes_t frames;
	  snd_pcm_t *handle;
	  snd_pcm_hw_params_t *params;

	  frames = tframes;

	  /* Open PCM device for recording (capture). */
	  rc = snd_pcm_open(&handle, "default",
	                    SND_PCM_STREAM_CAPTURE, 0);
	  if (rc < 0) {
	    fprintf(stderr,
	            "unable to open pcm device: %s\n",
	            snd_strerror(rc));
	    exit(1);
	  }

	  /* Allocate a hardware parameters object. */
	  snd_pcm_hw_params_alloca(&params);

	  /* Fill it in with default values. */
	  snd_pcm_hw_params_any(handle, params);

	  /* Set the desired hardware parameters. */

	  /* Interleaved mode */
	  rc = snd_pcm_hw_params_set_access(handle, params,
	                      SND_PCM_ACCESS_RW_INTERLEAVED);
	  if (rc != 0) {
	    fprintf(stderr,
	            "snd_pcm_hw_params_set_access: %s\n",
	            snd_strerror(rc));
	    exit(1);
	  }


	  /* Signed 16-bit little-endian format */
	  rc = snd_pcm_hw_params_set_format(handle, params,
	                              SND_PCM_FORMAT_S32_LE);
	  if (rc != 0) {
	    fprintf(stderr,
	            "snd_pcm_hw_params_set_format: %s\n",
	            snd_strerror(rc));
	    exit(1);
	  }


	  /* Two channels (stereo) */
	  rc = snd_pcm_hw_params_set_channels(handle, params, 2);

	  if (rc != 0) {
	    fprintf(stderr,
	            "snd_pcm_hw_params_set_channels: %s\n",
	            snd_strerror(rc));
	    exit(1);
	  }


	  /* 44000 bits per second (max) */
	  val = hertz;
	  rc = snd_pcm_hw_params_set_rate_near(handle, params,
	                                  &val, &dir);
	  if (rc != 0) {
	    fprintf(stderr,
	            "snd_pcm_hw_params_set_rate_near: %s\n",
	            snd_strerror(rc));
	    exit(1);
	  }


	  /* Set period size to frames. */
	  rc  = snd_pcm_hw_params_set_period_size_near(handle,
	                              params, &frames, &dir);

	  if (rc != 0) {
	    fprintf(stderr,
	            "snd_pcm_hw_params_set_period_size_near: %s\n",
	            snd_strerror(rc));
	    exit(1);
	  }

	  /* Write the parameters to the driver */
	  rc = snd_pcm_hw_params(handle, params);
	  if (rc < 0) {
	    fprintf(stderr,
	            "unable to set hw parameters: %s\n",
	            snd_strerror(rc));
	    exit(1);
	  }

	  /* Use a buffer large enough to hold one period */
	  snd_pcm_hw_params_get_period_size(params,
	                                      &frames, &dir);
	  size = frames * 8; /* 4 bytes/sample, 2 channels */
	  buffer = (char *) malloc(size);

	  /* We want to loop for 10 seconds */
	  snd_pcm_hw_params_get_period_time(params,
	                                         &val, &dir);

	  return handle;
}



