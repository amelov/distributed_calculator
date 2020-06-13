
#include "uv_task.h"

#include <unistd.h>

#include "calculate.h"
#include "json_tool.h"

#include "uv_proc.h"


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

void on_calc_work_close_cb(uv_handle_t *req)
{
	work_ctx_t* w_ctx = (work_ctx_t*)req->data;
	if (w_ctx) {

		printf("[%u]+ calc_close_cb(%s)\n", w_ctx->dbg_id, w_ctx->in_str);

		free(w_ctx->in_str);
		free(w_ctx->out_str);
		free(w_ctx);
	}
}


void on_calc_work_cb(uv_work_t* req)
{
	work_ctx_t* w_ctx = (work_ctx_t*)req->data;


	printf("[%u]+ calc_work_cb(%s)\n", w_ctx->dbg_id, w_ctx->in_str);

	w_ctx->out_str = input_json_msg_handler(w_ctx->in_str);

	printf("[%u]+ delay(10) result: %s", w_ctx->dbg_id, w_ctx->out_str);	
	sleep(10);

	printf("[%u]- calc_work_cb\n", w_ctx->dbg_id);
}


void on_after_calc_work_cb(uv_work_t* req, int status)
{
	work_ctx_t* w_ctx = (work_ctx_t*)req->data;

	fprintf(stderr, "[%u]= on_after_calc_proc() (%s) -> (%s)\n", w_ctx->dbg_id, w_ctx->in_str, w_ctx->out_str);

	if (status != UV_ECANCELED) {
		send_data_to_client(w_ctx->req_client, req, w_ctx->out_str);
	}

	uv_close((uv_handle_t*)req, on_calc_work_close_cb);
}
