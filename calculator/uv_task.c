
#include "uv_task.h"

#include <unistd.h>

#include "calculate.h"
#include "json_tool.h"

#include "uv_server_proc.h"


char* input_json_msg_handler(const char* json_str)
{
	char* r_code = NULL;

	session_data_t sess = {0};

	if (!parse_incoming_json(json_str, &sess)) {

		int expression_idx;

		stack_create(&sess.result, sizeof(NUM_t), stack_size(&sess.expression));

		for (expression_idx = 0; expression_idx<stack_size(&sess.expression); ++expression_idx) {
			char** expression_str = (char**)stack_element_at(&sess.expression, expression_idx);

			NUM_t res;
			char* err_str = NULL;
			if (!calculate(*expression_str, &sess.var, &res, &err_str)) {
				//printf("calc[%d]: \"%s\" -> %lld\r\n", expression_idx, *expression_str, res);
				stack_push_back(&sess.result, &res);
			} else {
				printf("calc[%d]: ERROR \"%s\" -> %s\r\n", expression_idx, *expression_str, err_str);
				break;
			}
		}
		r_code = create_outgoing_json(&sess);
	} else {
		printf("input_json_msg_handler - parsing error\r\n");
	}

	{// Release session
		void** data_ptr = NULL;
		var_destroy(&sess.var);
		while (NULL!=(data_ptr=stack_pop_back(&sess.expression))) {
			free(*data_ptr);
		}
		stack_destroy(&sess.expression);
		stack_destroy(&sess.result);
	}

	return r_code;
}

//////////////////////////////////////////////////////////////////////////
/*
void on_calc_work_close_cb(uv_handle_t *req)
{
	printf("#2");
	work_ctx_t* w_ctx = (work_ctx_t*)req->data;
	if (w_ctx) {
		printf("[%u]+ calc_close_cb\r\n", w_ctx->dbg_id);

		free(w_ctx->in_str);
		free(w_ctx->out_str);
		free(w_ctx);
	}
}
*/

void on_calc_work_cb(uv_work_t* req)
{
	task_ctx_t* t_ctx = (task_ctx_t*)req->data;
	t_ctx->out_str = input_json_msg_handler(t_ctx->in_str);
	//printf("[%u]+ delay(2) result: %s\r\n", w_ctx->dbg_id, w_ctx->out_str);	
	//sleep(2);
	//printf("[%u]- calc_work_cb\r\n", w_ctx->dbg_id);
}


void on_after_calc_work_cb(uv_work_t* req, int status)
{
	task_ctx_t* w_ctx = req->data;
	
	if (w_ctx) {
		printf("[%u] on_after_calc_proc(1)\r\n", w_ctx->dbg_id);

		if (status != UV_ECANCELED) {
			send_data_to_client(w_ctx->req_client, req, w_ctx->out_str);
		}

		printf("#2");
		if (w_ctx) {
			printf("[%u]+ calc_close_cb\r\n", w_ctx->dbg_id);

			free(w_ctx->in_str);
			free(w_ctx->out_str);
			free(w_ctx);
		}
	}

	free(req);
	//uv_close((uv_handle_t*)req, on_calc_work_close_cb);
}