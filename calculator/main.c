	
#include <memory.h>
#include <malloc.h>

#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>


#include "common.h"
#include "calculate.h"


#ifdef MAKE_TEST
	#include "convert_to_rpn.h"
#endif

#include "json_tool.h"
#include "uv_proc.h"


//#define UV_EXTERN
#include <uv.h>

//////////////////////////////////////////////////////////////////////////

char* input_json_msg_handler(const char* json_str)
{
	char* r_code = NULL;

	session_data_t sess;
	sess.var = zero_stack;
	sess.expression = zero_stack;
	sess.result = zero_stack;

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

int main()
{
#if MAKE_TEST
   	convert_to_rpn_TEST();
 	calculate_TEST();
#else 

/*
	struct sockaddr_in addr;


	uv_loop_t *loop = uv_default_loop();



	char* in_json_str = "{\"params\": {\"x\": 3, \"y\": 5, \"z\": -1}, \"expressions\": [\"(x + y) * 5\", \"3 * x - z + 2 * y\"]}";

	char* out_json_str = input_json_msg_handler(in_json_str);

	if (out_json_str) {
		printf("-> %s", out_json_str);
		free(out_json_str);
	}
 */

	uint16_t server_port = 7000;

	uv_loop_t *loop = uv_default_loop();

	if (!start_uv_tcp_server(loop, server_port)) {
		return uv_run(loop, UV_RUN_DEFAULT);
	}


#endif

	return 0;
}
