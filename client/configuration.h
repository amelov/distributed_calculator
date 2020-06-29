
#pragma once

#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>



void dc_client_cfg_create(const uint32_t ip, const uint16_t port);

struct sockaddr_in* dc_client_cfg_balancer_addr();