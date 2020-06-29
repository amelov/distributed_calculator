
#include "uv_task.h"

#include <unistd.h>
#include <stdlib.h>

#include "../tools/mlist.h"
#include "../tools/mstring.h"

#include "calculate.h"
#include "json_tool.h"

#include "uv_server_proc.h"


typedef struct calc_thread_t {
	list_t* owner;
	uv_stream_t *client;
	char* expression_str;
	var_store_t var;
	NUM_t result;
	char* error_str;	// pointer to const string!
	uint8_t ready_flag;
} calc_thread_t;



static void dc_calc_on_calc_work_cb(uv_work_t* req)
{
	calc_thread_t* t_ctx = (calc_thread_t*)req->data;

	if (t_ctx) {

		if (!dc_calc_calculate(t_ctx->expression_str, &t_ctx->var, &t_ctx->result, &t_ctx->error_str)) {
			;
		} else {
			printf("calc: ERROR \"%s\" -> %s\r\n", t_ctx->expression_str, t_ctx->error_str);
		}

	}
}


static void dc_calc_on_after_calc_work_cb(uv_work_t* req, int status)
{
	calc_thread_t* t_ctx = (calc_thread_t*)req->data;

	if (t_ctx) {
		t_ctx->ready_flag = 1;
		//printf("on_after_calc_work_cb: \"%s\" -> %lld\r\n", t_ctx->expression_str, t_ctx->result);

		uint32_t ready_thread_count = 0;

		list_t* p_task = list_head(t_ctx->owner);
		while (p_task) {
			calc_thread_t* t_p = list_data(p_task);
			if (t_p->ready_flag) {
				ready_thread_count++;
			} else {
				break;
			}
			p_task = list_next(p_task);
		}
		if ( ready_thread_count == list_size(t_ctx->owner) ) {

			session_data_t sess;

			stack_create(&sess.expression, sizeof(char*), list_size(t_ctx->owner));
			stack_create(&sess.result, sizeof(NUM_t), list_size(t_ctx->owner));
			stack_create(&sess.error, sizeof(char*), list_size(t_ctx->owner));

			p_task = list_head(t_ctx->owner);
			while (p_task) {
				calc_thread_t* t_p = list_data(p_task);

				stack_push_back(&sess.expression, &t_p->expression_str);
				stack_push_back(&sess.result, &t_p->result);
				stack_push_back(&sess.error,  &t_p->error_str);

				p_task = list_next(p_task);
			}
			char *result_json = dc_calc_create_outgoing_json(&t_ctx->var, &sess);
			//printf("all task complete => %s!\r\n", result_json);

			dc_calc_send_data_to_client(t_ctx->client, result_json);
			free(result_json);
			stack_destroy(&sess.result);
			stack_destroy(&sess.expression);

			list_t* p_task = list_head(t_ctx->owner);
			while (p_task) {
				calc_thread_t* t_d = list_data(p_task);
				free(t_d->expression_str);
				dc_calc_var_destroy(&t_d->var);
				//t_d->error_str;	//! pointer to const string!
				p_task = list_next(p_task);
			}
			free(p_task);
		}

	}

	free(req);
}



//////////////////////////////////////////////////////////////////////////


uint32_t dc_calc_input_json_msg_handler(uv_stream_t *client, const char* json_str)
{
	uint32_t r_code = 0;

	mstack_t expression = {0};
	var_store_t variable = {0};

	if ( !dc_calc_parse_incoming_json(json_str, &expression, &variable) ) {

		list_t* calc_task_ctx = malloc(sizeof(*calc_task_ctx));

		list_create(calc_task_ctx);

		for (int exp_idx = 0; exp_idx<stack_size(&expression); ++exp_idx) { 
			char** expression_str = (char**)stack_element_at(&expression, exp_idx);

			calc_thread_t* ctc = (calc_thread_t*)calloc(1, sizeof(*ctc));
			ctc->owner = calc_task_ctx;
			ctc->client = client;
			ctc->expression_str = str_create_copy(*expression_str);
			ctc->error_str = NULL;

			dc_calc_var_init(&ctc->var, dc_calc_var_size(&variable));
			for (int i=0; i<dc_calc_var_size(&variable); ++i) {
				char* key; 
				NUM_t* val;
				if (!dc_calc_var_element_at(&variable, i, &key, &val)) {
					dc_calc_var_add(&ctc->var, key, *val);
				}
			}

			ctc->ready_flag = 0;
			list_add(calc_task_ctx, ctc, sizeof(*ctc));
		}

		list_t* p_list = list_head(calc_task_ctx);
		while (p_list) {
			uv_work_t* work_hnd = (uv_work_t*)malloc(sizeof(*work_hnd));
			if (work_hnd) {
				work_hnd->data = list_data(p_list);

				if (uv_queue_work(uv_default_loop(), work_hnd, dc_calc_on_calc_work_cb, dc_calc_on_after_calc_work_cb) == 0) {
					;
				} else {
					;
				}
				
			}

			p_list = list_next(p_list);
		}

		{
			void** data_ptr = NULL;
			while (NULL!=(data_ptr=stack_pop_back(&expression))) {
				free(*data_ptr);
			}
			dc_calc_var_destroy(&variable);
		}

	} else {
		printf("input_json_msg_handler - parsing error\r\n");
		r_code = 1;
	}

	return r_code;
}




