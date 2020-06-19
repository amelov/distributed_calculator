

#include "configuration.h"
#include "common.h"

#include <uv.h>

static struct sockaddr_in balancer_addr = {0};


void set_balancer_addr(const uint32_t ip, const uint16_t port)
{
	if (ip && port) {
		balancer_addr.sin_family = AF_INET;
		balancer_addr.sin_addr.s_addr = ip;
		balancer_addr.sin_port = htons( port );
	}
}


struct sockaddr_in* get_balancer_addr()
{
	if (!balancer_addr.sin_addr.s_addr) {
		balancer_addr.sin_family = AF_INET;
		balancer_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		balancer_addr.sin_port = htons( BALANCER_DEFAULT_PORT );
	}

	return &balancer_addr;
}


