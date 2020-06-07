	
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

#include "json_proc.h"


int main()
{
#if MAKE_TEST
   	convert_to_rpn_TEST();
 	calculate_TEST();
#else 
	
	char* json_text = "{\"params\": {\"x\": 3, \"y\": 5, \"z\": -1}, \"expressions\": [\"(x + y) * 5\", \"3 * x - z + 2 * y\"]}";
		
	session_data_t sess;
	sess.var = zero_stack;
	sess.expression = zero_stack;
	sess.result = zero_stack;
	
	if (!parse_incoming_json(json_text, &sess)) {

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

		{
			char *out_json = create_outgoing_json(&sess);

			if (out_json) {
                                printf("%s\r\n", out_json);
				free(out_json);
			}
		}



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
#endif

	return 0;
}
