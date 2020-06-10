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
#include "mstack.h"
#include <uv.h>


#define BALANCER_CONFIG_FILE_NAME "./balancer.json"

#define SERVER_PORT 		6999
#define MAX_CLIENT_COUNT	128
#define RECONNECT_TIMEOUT_ms 5*1000


typedef struct client_descr_t {
		uv_connect_t connect;
		uv_tcp_t handle;

        struct sockaddr_in addr;
        uint32_t state;

        uint32_t dbg_id;
} client_descr_t;


uint8_t load_config(const char* fn);

client_descr_t* get_calc_host(const size_t idx);



client_descr_t* find_client_by_stream(uv_stream_t* c);