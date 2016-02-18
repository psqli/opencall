#include <alsa/asoundlib.h>

typedef struct audiolayer_t AudioL;
typedef struct audiolayerparams_t AudioP;

struct audiolayerparams_t
{
	char *name;
	char *format;
	int channels;
	int rate;
	int period_size; /* in frames */
	int capture; /* is capture device? (bool) */

/* internal defined: */
	snd_pcm_format_t pcm_format;
	size_t ssize_ib; /* sample size (in bytes) */
	size_t fsize_ib; /* frame size (in bytes) */
	snd_pcm_uframes_t psize_if; /* period size (in frames) */
	size_t psize_ib; /* period size (in bytes) */
};

struct audiolayer_t
{
	snd_pcm_t *handle;
};

AudioL*
audiolayer_new(AudioP*);

int
audiolayer_free(AudioL*);

inline snd_pcm_sframes_t
audiolayer_readi(AudioL*, void*, snd_pcm_uframes_t);

inline snd_pcm_uframes_t
audiolayer_writei(AudioL*, const void*, snd_pcm_uframes_t);

inline ssize_t
audiolayer_frames_to_bytes(AudioL*, snd_pcm_sframes_t);