
#pragma once

#include <uv.h>


// uv_work_t.data = 
typedef struct task_ctx_t {
	//uv_work_t handle;
	uv_stream_t *req_client;

	char* in_str;	// input JSON string
	char* out_str;	// result JSON string
	uint32_t dbg_id;
} task_ctx_t;



void on_calc_work_cb(uv_work_t* req);


void on_after_calc_work_cb(uv_work_t* req, int status);