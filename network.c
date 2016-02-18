#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "network.h"

#define DEFAULT_FAMILY AF_INET

static int
oc_netlayer_internal_wait_request(NetLayer *nl)
{
	const char expected_buffer[] = "helloworld!\0";
	size_t expected_len;

	char *buffer;

	expected_len = strlen(expected_buffer) + 1;

	buffer = alloca(expected_len);

	if(oc_netlayer_recv(nl->socket_fd, buffer, expected_len, 0) !=
	expected_len)
		return -1;

	if(strcmp(buffer, expected_buffer) != 0)
		return -1;

	return 0;
}

static int
oc_netlayer_internal_send_request(NetLayer *nl)
{
	char buffer[] = "helloworld!\0";
	size_t len;

	len = strlen(buffer) + 1;

	if(oc_netlayer_send(nl->socket_fd, buffer, len, 0) != len)
		return -1;

	return 0;
}

/**
 * @p socket_type example: SOCK_STREAM, SOCK_DGRAM
 * @p port port number
 * @p addr if NULL: socket_addr = 0; else: socket_addr = addr; (almost it)
 */
NetLayer*
oc_netlayer_new(int socket_type, uint16_t port, const char *addr)
{
	NetLayer *nl;

	int tmpsfd;

	if((nl = malloc(sizeof(NetLayer))) == NULL)
	{
		/* there isn't `errno` in malloc() */
		puts("Error in malloc() `oc_netlayer_new()`!"); 
		return NULL;
	}

	nl->socket_type = socket_type;

	if((nl->socket_fd = socket(DEFAULT_FAMILY, nl->socket_type, 0)) == -1)
	{
		perror("Error while open socket socket() ");
		goto _go_free;
	}

	nl->socket_addr.sin_family = DEFAULT_FAMILY;
	nl->socket_addr.sin_port = (in_port_t) htons(port);

	if(nl->socket_type == SOCK_STREAM)
		setsockopt(nl->socket_fd, IPPROTO_TCP, TCP_NODELAY, NULL, 0);

	if(addr == NULL)
	{
		nl->socket_addr.sin_addr.s_addr = INADDR_ANY;
		
		if(bind(nl->socket_fd, (struct sockaddr*) &nl->socket_addr,
	sizeof(struct sockaddr_in)) == -1)
		{
			perror("Error while bind socket bind() ");
			goto _go_close;
		}

		if(nl->socket_type == SOCK_STREAM)
		{
			if(listen(nl->socket_fd, 1) == -1)
			{
				perror("Error in listen()");
				goto _go_close;
			}

			if((tmpsfd = accept(nl->socket_fd, NULL, NULL)) == -1)
			{
				perror("Error in accept()");
				goto _go_close;
			}
			close(nl->socket_fd);
			nl->socket_fd = tmpsfd;
		}

		if(oc_netlayer_internal_wait_request(nl) == -1)
			goto _go_close;
	}
	else
	{
		if((nl->socket_addr.sin_addr.s_addr = inet_addr(addr)) == -1)
		{
			/* there isn't errno in inet_addr() */
			puts("Put a valid address, like 192.168.0.1");
			goto _go_close;
		}

		if(connect(nl->socket_fd, (struct sockaddr*) &nl->socket_addr,
	sizeof(struct sockaddr_in)) == -1)
		{
			perror("Error while set address of socket connect() ");
			goto _go_close;
		}

		if(oc_netlayer_internal_send_request(nl) == -1)
			goto _go_close;
	}

	return nl;

// exit with error
_go_close:
	close(nl->socket_fd);

_go_free:
	free(nl);

	return NULL;
}

int
oc_netlayer_free(NetLayer *nl)
{
	if(nl == NULL)
		return -1;

	if(close(nl->socket_fd) == -1)
		perror("in close()");

	free(nl);

	return 0;
}

/*
 * The `recv()` function masked
 */
inline ssize_t
oc_netlayer_recv(int sockfd, void *buf, size_t len, int flags)
{
	return recv(sockfd, buf, len, flags);
}

/*
 * The `send()` function masked
 */
inline ssize_t
oc_netlayer_send(int sockfd, const void *buf, size_t len, int flags)
{
	return send(sockfd, buf, len, flags);
}
