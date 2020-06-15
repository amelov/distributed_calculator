
#pragma once

#include <uv.h>

#include "mbuf.h"


enum {
	UNDEF_STATE = -1,
	READY_STATE = 0,
	CONNECTING_STATE = 1,
};


typedef struct client_descr_t {
		uv_connect_t connect;
		/*uv_stream_t*/uv_tcp_t handle;

        struct sockaddr_in addr;
        uint32_t state;

        buf_t rx;

        uv_stream_t* req_stream;	// last request client 

        uint32_t dbg_id;
} client_descr_t;


uint8_t start_uv_tcp_client(client_descr_t* c);

void on_reconnect_timer_cb(uv_timer_t* handle);


void send_to_calc(client_descr_t* c, uv_stream_t* req_stream, char* json_msg_str);