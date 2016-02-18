#include <netinet/in.h>

typedef struct oc_netlayer_t
{
	int socket_type;
	int socket_fd;
	struct sockaddr_in socket_addr;
} NetLayer;

NetLayer*
oc_netlayer_new(int, uint16_t, const char*);

int
oc_netlayer_free(NetLayer*);

inline ssize_t
oc_netlayer_recv(int, void*, size_t, int);