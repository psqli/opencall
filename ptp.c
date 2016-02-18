#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include "network.h"
#include "audio.h"
#include "setsched.h"

#define gotowithstatus(status, point) \
do { \
	ret = status; \
	goto point; \
} while(0);

#if 1
long long overrun_count;
long long underrun_count;
#endif

NetLayer *nl;
AudioL *alc;
AudioL *alp;
AudioP alparams;

pthread_t recv_thread;
pthread_t send_thread;

void*
recv_routine(void *arg)
{
	int8_t *buf;
	ssize_t len;
	int err;

	if((buf = malloc(alparams.psize_ib)) == NULL)
	{
		printf("Error while alloc buf!\n");
		return NULL;
	}

	while(1)
	{
		if((len = oc_netlayer_recv(nl->socket_fd, (void*) buf,
	alparams.psize_ib, 0)) == -1)
		{
			if(errno == EAGAIN)
				continue;

			perror("Error in `oc_netlayer_recv()`");
			goto _go_free0;
		}

		if(len == 0)
			goto _go_free0;

		if((err = audiolayer_writei(alp, (const void*) buf,
	len / alparams.fsize_ib)) < 0)
		{
			if(err == -EAGAIN)
				continue;
			else
			if(err == -EPIPE)
			{
				underrun_count++;
				snd_pcm_prepare(alp->handle);
				continue;
			}
			else
				printf("Error in `audiolayer_writei()`: %s\n",
	snd_strerror(err));

			goto _go_free0;
		}

	}

_go_free0:
	free(buf);

	return NULL;
}

void*
send_routine(void *arg)
{
	int8_t *buf;
	ssize_t len;
	int err;

	if((buf = malloc(alparams.psize_ib)) == NULL)
	{
		printf("Error while alloc buf!\n");
		return NULL;
	}

	while(1)
	{
		if((err = audiolayer_readi(alc, (void*) buf,
	alparams.psize_if)) < 0)
		{
			if(err == -EAGAIN)
				continue;
			else
			if(err == -EPIPE)
			{
				overrun_count++;
				snd_pcm_prepare(alc->handle);
				continue;
			}
			else
				printf("Error in `audiolayer_readi()`: %s\n",
	snd_strerror(err));
			goto _go_free0;
		}

		if((len = oc_netlayer_send(nl->socket_fd, (void*) buf,
	err * alparams.fsize_ib, 0)) == -1)
		{
			if(errno == EAGAIN)
				continue;

			perror("Error in `oc_netlayer_send()`");
			goto _go_free0;
		}

	}

_go_free0:
	free(buf);

	return NULL;
}

int
main(int argc, char **argv)
{
	if(argc < 8)
	{
		printf("usage: cmd <PCMname> <PCMformat> <channels> <rate> "
	"<period_size> <port>\n <(for client mode) address>\n");
		return 1;
	}

	int port;
	char *addr;

	void *recv_thread_ret;
	void *send_thread_ret;

	int ret = 0;

	int index = 1;

	alparams.name = argv[index++];
	alparams.format = argv[index++];
	alparams.channels = atoi(argv[index++]);
	alparams.rate = atoi(argv[index++]);
	alparams.period_size = atoi(argv[index++]);

	port = atoi(argv[index++]);
	addr = argv[index++];

	if(strcmp("0", addr) == 0)
	{
		/* operating in server mode */
		addr = NULL;
	}

	alparams.capture = 1;
	if((alc = audiolayer_new(&alparams)) == NULL)
		return 1;

	alparams.capture = 0;
	if((alp = audiolayer_new(&alparams)) == NULL)
		gotowithstatus(1, _go_free0);

	if((nl = oc_netlayer_new(SOCK_STREAM, port, addr)) == NULL)
		gotowithstatus(1, _go_free1);

	if(setsched_realtime() == -1)
		gotowithstatus(1, _go_free2);

	pthread_create(&recv_thread, NULL, recv_routine, NULL);
	pthread_create(&send_thread, NULL, send_routine, NULL);

	pthread_join(recv_thread, &recv_thread_ret);
	pthread_join(send_thread, &send_thread_ret);

_go_free2:
	oc_netlayer_free(nl);

_go_free1:
	audiolayer_free(alp);

_go_free0:
	audiolayer_free(alc);

	printf("overrun = %lld\n"
	"underrun = %lld\n", overrun_count, underrun_count);

	return ret;
}
