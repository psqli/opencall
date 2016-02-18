#include <alsa/asoundlib.h>

#include <stdio.h>
#include <stdlib.h>

#include "audio.h"

/* CURIOUS ...
 * 
 * If we have [2 channels] with [16-bit sample] in a [48000 rate] ...
 *
 * our Frame is [2 channels] * [2 bytes (16-bit sample)] = [4 bytes]
 *
 * .-----------------------------.
 * | Our total = 192 kilobytes/s |
 * `-----------------------------´
 */

static int
set_hw_params(AudioL *al, AudioP *ap)
{
	int err;
	int dir;

	snd_pcm_hw_params_t *hw_params;

	snd_pcm_hw_params_alloca(&hw_params);

	snd_pcm_hw_params_any(al->handle, hw_params);

	snd_pcm_hw_params_set_access(al->handle, hw_params,
	SND_PCM_ACCESS_RW_INTERLEAVED);

	/* translate string into format value */
	ap->pcm_format = snd_pcm_format_value(ap->format);

	snd_pcm_hw_params_set_format(al->handle, hw_params, ap->pcm_format);

	snd_pcm_hw_params_set_channels(al->handle, hw_params, ap->channels);

	snd_pcm_hw_params_set_rate_near(al->handle, hw_params, &ap->rate, NULL);

	ap->psize_if = ap->period_size;

	snd_pcm_hw_params_set_period_size_near(al->handle, hw_params,
	&ap->psize_if, NULL);

	if((err = snd_pcm_hw_params(al->handle, hw_params)) < 0)
		return -1;

	if((ap->ssize_ib = snd_pcm_format_size(ap->pcm_format, 1)) < 0)
		return -1;

/* calc frame size (in bytes) */
	ap->fsize_ib = ap->ssize_ib * ap->channels;
/* calc period size (in bytes) */
	ap->psize_ib = ap->psize_if * ap->fsize_ib;

	return 0;
}

AudioL*
audiolayer_new(AudioP *ap)
{
	if(ap == NULL)
		return NULL;

	AudioL *al;

	int err;

	if((al = malloc(sizeof(AudioL))) == NULL)
	{
		puts("Error while allocating!");
		return NULL;
	}

	if((err = snd_pcm_open(&al->handle, ap->name,
	(ap->capture ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK),
	0)) < 0)
	{
		printf("PCM open error: %s\n", snd_strerror(err));
		goto _go_free;
	}

	if(set_hw_params(al, ap) == -1)
	{
		printf("PCM set hw params error!\n");
		goto _go_close;
	}

	printf("PCM:\n"
	"  name = %s\n"
	"  format = %s\n"
	"  channels = %d\n"
	"  rate = %d\n"
	"  period size (bytes) = %d\n",
	ap->name, ap->format, ap->channels, ap->rate, ap->psize_ib);

	return al;

	_go_close:
	snd_pcm_close(al->handle);

	_go_free:
	free(al);

	return NULL;
}

int
audiolayer_free(AudioL *al)
{
	if(al == NULL)
		return -1;

	snd_pcm_close(al->handle);

	free(al);

	return 0;
}

inline snd_pcm_sframes_t
audiolayer_readi(AudioL *al, void *buffer, snd_pcm_uframes_t size)
{
	return snd_pcm_readi(al->handle, buffer, size);
}

inline snd_pcm_uframes_t
audiolayer_writei(AudioL *al, const void *buffer, snd_pcm_uframes_t size)
{
	return snd_pcm_writei(al->handle, buffer, size);
}

inline ssize_t
audiolayer_frames_to_bytes(AudioL *al, snd_pcm_sframes_t frames)
{
	return snd_pcm_frames_to_bytes(al->handle, frames);
}

#if 0
void
audiolayer_virtual_device()
{
	snd_pcm_ioplug_t ioplug;
	snd_pcm_ioplug_callback_t ioplug_params;

	snd_pcm_ioplug_create(&ioplug);
}
#endif
