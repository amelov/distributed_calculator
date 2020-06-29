
#pragma once

#include <uv.h>

#include "../tools/mbuf.h"


typedef void (*dc_balancer_calc_proc_cb_t)(uv_stream_t* req_client, char* req_str, char* result_str);
typedef void (*dc_balancer_calc_error_cb_t)(uv_stream_t* req_client, char* req_str);

typedef struct calc_client_descr_t {
		uv_connect_t connect;
		uv_tcp_t handle;

        struct sockaddr_in addr;

        buf_t rx;

        uv_stream_t* req_stream;	// requested client 
        char* req_json;

        dc_balancer_calc_proc_cb_t on_result_fn;
        dc_balancer_calc_error_cb_t on_err_fn;

        uint32_t dbg_id;
} calc_client_descr_t;



uint32_t dc_balancer_send_job_to_calc(uv_stream_t* req_client, char* json_msg_str, struct sockaddr_in* calc_addr, 
							dc_balancer_calc_proc_cb_t fn, 
                                                        dc_balancer_calc_error_cb_t err_fn);