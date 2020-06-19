
#pragma once

#include <uv.h>
#include <stdint.h>
#include "../tools/mbuf.h"


typedef void(*on_calc_result_cb_t)(uv_tcp_t* client, char* result_str);

typedef enum CLIENT_STATE_t {
	UNDEF_STATE = -1,
	READY_STATE = 0,
	CONNECTING_STATE = 1,
} CLIENT_STATE_t;


typedef struct client_descr_t {
	uv_connect_t connect;
	uv_tcp_t handle;
	struct sockaddr_in addr;
	CLIENT_STATE_t state;
	uint32_t err_count;

	char* req_json;
	on_calc_result_cb_t result_cb_fn;

    buf_t rx;
} client_descr_t;


uint32_t send_to_calc(char* out_json, on_calc_result_cb_t result_fn);

void close_calc_connection();


//void on_reconnect_timer_cb(uv_timer_t* handle);
