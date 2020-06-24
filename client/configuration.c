

#include "configuration.h"

#include <sys/types.h>
#include <netinet/in.h>

#include "common.h"



static struct sockaddr_in balancer_addr = {0};


void cfg_create(const uint32_t ip, const uint16_t port)
{
	if (ip && port) {
		balancer_addr.sin_family = AF_INET;
		balancer_addr.sin_addr.s_addr = ip;
		balancer_addr.sin_port = htons( port );
	}
}




struct sockaddr_in* cgf_balancer_addr()
{
	if (!balancer_addr.sin_addr.s_addr) {
		balancer_addr.sin_family = AF_INET;
		balancer_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		balancer_addr.sin_port = htons( BALANCER_DEFAULT_PORT );
	}

	return &balancer_addr;
}


