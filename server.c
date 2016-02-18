#include <stdio.h>
#include <stdlib.h>

#include "network.h"
#include "audio.h"
#include "setsched.h"

long long xrun_count;

NetLayer *nl;

AudioL *al;

AudioP alp;

int
main(int argc, char **argv)
{
	if(argc < 7)
	{
		printf("usage: cmd <PCMname> <PCMformat> <channels> <rate> "
	"<period_size> <port>\n");
		return 1;
	}

	int8_t *buf;
	ssize_t len;
	int err;

	int port;

	int index = 1;

	alp.name = argv[index++];
	alp.format = argv[index++];
	alp.channels = atoi(argv[index++]);
	alp.rate = atoi(argv[index++]);
	alp.period_size = atoi(argv[index++]);

	port = atoi(argv[index++]);

	alp.capture = 0;

	if((al = audiolayer_new(&alp)) == NULL)
		return 1;

	if((nl = oc_netlayer_new(SOCK_STREAM, port, NULL)) == NULL)
		goto _go_free0;

	if((buf = malloc(alp.psize_ib)) == NULL)
	{
		printf("Error while alloc buf!\n");
		goto _go_free1;
	}

	if(setsched_realtime() == -1)
	{
		printf("Error in `setsched_realtime()`\n");
		goto _go_free2;
	}

	while(1)
	{
		if((len = oc_netlayer_recv(nl->socket_fd, (void*) buf,
	alp.psize_ib, 0)) == -1)
		{
			if(errno == EAGAIN)
				continue;

			perror("Error in `main()`, recv() ");
			goto _go_free2;
		}

		if(len == 0)
			goto _go_free2;

		if((err = audiolayer_writei(al, (const void*) buf,
	len / alp.fsize_ib)) < 0)
		{
			if(err == -EAGAIN)
				continue;
			else
			if(err == -EPIPE)
			{
				xrun_count++;
				snd_pcm_prepare(al->handle);
				continue;
			}
			else
				printf("Error in `snd_pcm_writei()`: %s\n",
	snd_strerror(err));

			goto _go_free2;
		}

	}

_go_free2:
	free(buf);

_go_free1:
	oc_netlayer_free(nl);

_go_free0:
	audiolayer_free(al);

	printf("underrun = %lld\n", xrun_count);

	return 1;
}
