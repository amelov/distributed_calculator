
#pragma once

#include <stdint.h>


void set_balancer_addr(const uint32_t ip, const uint16_t port);

struct sockaddr_in* get_balancer_addr();