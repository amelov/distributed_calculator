

#include "configuration.h"
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include "common.h"


static struct sockaddr_in* _dc_client_cfg_balancer_addr_ptr = NULL;


void dc_client_cfg_create(const uint32_t ip, const uint16_t port)
{
	if (ip && port) {
		_dc_client_cfg_balancer_addr_ptr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
		assert(_dc_client_cfg_balancer_addr_ptr);

		_dc_client_cfg_balancer_addr_ptr->sin_family = AF_INET;
		_dc_client_cfg_balancer_addr_ptr->sin_addr.s_addr = ip;
		_dc_client_cfg_balancer_addr_ptr->sin_port = htons( port );
	}
}


struct sockaddr_in* dc_client_cfg_balancer_addr()
{
	if (!_dc_client_cfg_balancer_addr_ptr) {
		dc_client_cfg_create(inet_addr("127.0.0.1"), BALANCER_DEFAULT_PORT);
	}
	return _dc_client_cfg_balancer_addr_ptr;
}


