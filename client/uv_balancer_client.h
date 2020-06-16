
#pragma once

#include <uv.h>
#include <stdint.h>
#include "../tools/mbuf.h"


typedef enum CLIENT_STATE_t {
	UNDEF_STATE = -1,
	READY_STATE = 0,
	CONNECTING_STATE = 1,
} CLIENT_STATE_t;


typedef struct client_descr_t {
	uv_connect_t connect;
	/*uv_stream_t*/uv_tcp_t handle;

        struct sockaddr_in addr;
        CLIENT_STATE_t state;

        buf_t rx;
} client_descr_t;



extern client_descr_t balancer_client;

uint8_t start_uv_tcp_client(client_descr_t* c);


void on_reconnect_timer_cb(uv_timer_t* handle);
