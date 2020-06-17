
#pragma once

#include <uv.h>

#include "../tools/mbuf.h"


typedef void (*calc_proc_cb_t)(uv_stream_t* req_client, char* req_str, char* result_str);
typedef void (*calc_error_cb_t)(uv_stream_t* req_client, char* req_str);

typedef struct calc_client_descr_t {
		uv_connect_t connect;
		uv_tcp_t handle;

        struct sockaddr_in addr;

        buf_t rx;

        uv_stream_t* req_stream;	// requested client 
        char* req_json;

        calc_proc_cb_t on_result_fn;
        calc_error_cb_t on_err_fn;

        uint32_t dbg_id;
} calc_client_descr_t;



uint32_t send_req_to_calc(uv_stream_t* req_client, char* json_msg_str, struct sockaddr_in* calc_addr, 
							calc_proc_cb_t fn, calc_error_cb_t err_fn);