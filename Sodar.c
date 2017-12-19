
/* Use the newer ALSA API */

#define ALSA_PCM_NEW_HW_PARAMS_API

#include <math.h>
#include <alsa/asoundlib.h>

#define JITTER_RANGE 10  // range to check for correlation in.  -10 to +10 samples.
#define IGNORE_SECONDS 0.3  // Microphone take time to warm up.

typedef struct
{
	int count;
	double sx2;
	double sy2;
	double correlation;
	double position;
} correlation_results;


static void calcCorrelation(char *, int,correlation_results  *);
static void testEndian();


int main()
{
  long loops;
  int rc;
  int size;
  unsigned int val;
  int dir;
  correlation_results corr_results;
  double position = 0;
  double varianceSum = 0;
  char output[100];
  int len;
  char *buffer;
  int ignore_count;
  int k;
  int countSum = 0;

  snd_pcm_uframes_t frames;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;


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

  /* Set period size to 1024 frames. */
  frames = 1024;
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

  /* We want to loop for 10 seconds */
  snd_pcm_hw_params_get_period_time(params,
                                         &val, &dir);
  loops = 100000 / val;
  ignore_count = IGNORE_SECONDS/val;

  k = 0;

  while (loops > 0)
  {
    loops--;k++;

    rc = snd_pcm_readi(handle, buffer, frames);
    if (rc == -EPIPE)
    {
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

    if (k > ignore_count)
    {
    	calcCorrelation(buffer, rc, &corr_results);
    	position = position + corr_results.position * corr_results.sy2 * corr_results.correlation;
    	varianceSum = varianceSum + corr_results.sy2 * corr_results.correlation;
    	countSum = countSum + corr_results.count;
    }
  }

  len = sprintf(output, "%f, %f\n",position/varianceSum, log(varianceSum/countSum));
  write(1, output, len);


  snd_pcm_drain(handle);
  snd_pcm_close(handle);
  free(buffer);

  return 0;
}

int32_t getBufferVal(char *src, int offset, int channel, int channels)
{
	return ((int32_t *)src)[(offset * channels) + channel];
}

void calcCorrelation(char *src, int size, correlation_results  *corr_results)
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
	double maxCorr = -2;
	int offset = 0;

	int count = size - 2 * JITTER_RANGE;

	corr_results->count = count;

	if (size - 3 * JITTER_RANGE < 0)
	{
		return;
	}

	for (i = 0;i < 2*JITTER_RANGE;i++)
	{
		sumx = 0;
		sumy=0;
		sumxy = 0;
		sumx2 = 0;
		sumy2 = 0;

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
		if (corr > maxCorr)
		{
			maxCorr = corr;
			offset = i - JITTER_RANGE;
			corr_results->correlation = maxCorr;
			corr_results->position = offset;
			corr_results->sx2 = sumx2/count - (sumx/count) * (sumx/count);
			corr_results->sy2 = sumy2/count - (sumy/count) * (sumy/count);
		}
	}

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
