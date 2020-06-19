
#pragma once

#include <stdint.h>


#define BALANCER_DEFAULT_PORT 6500
#define RECONNECT_TIMEOUT_ms 10*1000

#define MESSAGE_DELIMITER "\n"


void set_balancer_addr(const uint32_t ip, const uint16_t port);

struct sockaddr_in* get_balancer_addr();