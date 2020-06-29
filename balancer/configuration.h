/*
	Load JSON configuration from file.
	File format:

	{
		"calculator": [
			{"ip":"ip_dot_str", "port":port_uint_16},
			...
		]
	}
 */

#pragma once

#include <stdint.h>
#include <sys/types.h>

#include "../tools/mstack.h"
#include <uv.h>

#include "uv_calc_client.h"



#define BALANCER_CONFIG_FILE_NAME "./balancer.json"

#define SERVER_PORT 		6999
#define MAX_CLIENT_COUNT	128
#define RECONNECT_TIMEOUT_ms 30*1000

#define MESSAGE_DELIMITER "\n"


typedef struct calc_ctx_t {
	struct sockaddr_in addr;
	//enum ADDR_STATE_t state;
} calc_ctx_t;


typedef struct dc_balancer_cfg_ctx_t { 
	mstack_t calc_addr_stack;
	uint16_t server_port;
} dc_balancer_cfg_ctx_t;

void dc_balancer_cfg_create(dc_balancer_cfg_ctx_t* ctx);

uint8_t  dc_balancer_cfg_load(const char* fn);

uint16_t dc_balancer_cfg_get_server_port();


uint32_t dc_balancer_cfg_get_calc_host_addr(const size_t idx, calc_ctx_t* addr);
size_t   dc_balancer_cfg_get_calc_host_count();


