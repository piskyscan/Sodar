
/* Use the newer ALSA API */

#define ALSA_PCM_NEW_HW_PARAMS_API

#include <math.h>
#include <alsa/asoundlib.h>

#define JITTER_RANGE 10  // range to check for correlation in.  -10 to +10 samples.

static void calcCorrelation(char *, int );
static void testEndian();

int main() {
  long loops;
  int rc;
  int size;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  unsigned int val;
  int dir;
  snd_pcm_uframes_t frames;
  char *buffer;

  testEndian();

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
  snd_pcm_hw_params_set_access(handle, params,
                      SND_PCM_ACCESS_RW_INTERLEAVED);

  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(handle, params,
                              SND_PCM_FORMAT_S32_LE);

  /* Two channels (stereo) */
  snd_pcm_hw_params_set_channels(handle, params, 2);

  /* 48000 bits per second (max) */
  val = 48000;
  snd_pcm_hw_params_set_rate_near(handle, params,
                                  &val, &dir);

  /* Set period size to 32 frames. */
  frames = 2048;
  snd_pcm_hw_params_set_period_size_near(handle,
                              params, &frames, &dir);

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

  /* We want to loop for 1 seconds */
  snd_pcm_hw_params_get_period_time(params,
                                         &val, &dir);
  loops = 1000000 / val;

  while (loops > 0) {
    loops--;
    rc = snd_pcm_readi(handle, buffer, frames);
    if (rc == -EPIPE) {
      /* EPIPE means overrun */
      fprintf(stderr, "overrun occurred\n");
      snd_pcm_prepare(handle);
    } else if (rc < 0) {
      fprintf(stderr,
              "error from read: %s\n",
              snd_strerror(rc));
    } else if (rc != (int)frames) {
      fprintf(stderr, "short read, read %d frames\n", rc);
    }

    calcCorrelation(buffer, rc);

    // rc = write(1, buffer, size);
//    if (rc != size)
//      fprintf(stderr,
//              "short write: wrote %d bytes\n", rc);
  }

  snd_pcm_drain(handle);
  snd_pcm_close(handle);
  free(buffer);

  return 0;
}

int32_t getBufferVal(char *src, int offset, int channel, int channels)
{
	return ((int32_t *)src)[(offset * channels) + channel];
}

void calcCorrelation(char *src, int size)
{
	int i;
	int j;
	double sumx;
	double sumy;
	double sumx2;
	double sumy2;
	double sumxy;
	double x,y;
	double corr;
	char output[100];
	int len;

	int count = size - 2 * JITTER_RANGE;

	if (size - 3 * JITTER_RANGE < 0)
	{
		return;
	}

	for (i = 0;i < 2*JITTER_RANGE;i++)
	{
		sumx = 0;
		sumy=0;
		sumxy = 0;

		for (j = 0;j < count;j++)
		{
			x = getBufferVal(src,j+JITTER_RANGE,0,2);
			y = getBufferVal(src,j+i,1,2);

			sumx += x;
			sumy +=y;
			sumx2 += x*x;
			sumy2 +=y*y;
			sumxy += x*y;
		}

		corr = (count*sumxy - sumx * sumy)/(sqrt((count * sumx2 - sumx*sumx)*(count * sumy2 - sumy*sumy)));
		len = sprintf(output, "%d, %f\n",i - JITTER_RANGE,corr);
		write(1, output, len);
	}

	len = sprintf(output, "----------------\n");
	write(1, output, len);

}


char test[] = {0x00,0xC4,0xFF,0xFF,0x00,0x88,0xFF,0xFF};

void testEndian()
{
	int32_t v1 = getBufferVal(test,0,0,2);
	int32_t v2 = getBufferVal(test,0,1,2);

	if (v1 != -15360 || v2 != -30720)
	{
		fprintf(stderr,
				"Endian test failes\n");
	}
}
