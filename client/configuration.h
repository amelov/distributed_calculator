
#pragma once

#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>



void cfg_create(const uint32_t ip, const uint16_t port);

struct sockaddr_in* cfg_balancer_addr();