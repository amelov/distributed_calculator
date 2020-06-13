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

#include "uv_calc_client.h"



#define BALANCER_CONFIG_FILE_NAME "./balancer.json"

#define SERVER_PORT 		6999
#define MAX_CLIENT_COUNT	128
#define RECONNECT_TIMEOUT_ms 65*1000

#define MESSAGE_DELIMITER "\n"





uint8_t load_config(const char* fn);

uint16_t get_server_port();


client_descr_t* get_calc_host(const size_t idx);
size_t get_calc_host_count();

client_descr_t* find_client_by_stream(uv_stream_t* c);


