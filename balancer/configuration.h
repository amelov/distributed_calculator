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



uint8_t  dc_balancer_load_config(const char* fn);

uint16_t dc_balancer_get_server_port();


uint32_t dc_balancer_get_calc_host_addr(const size_t idx, calc_ctx_t* addr);
size_t   dc_balancer_get_calc_host_count();


