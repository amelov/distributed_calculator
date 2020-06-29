

#include "json_tool.h"
#include <jansson.h>
#include <string.h>
#include <assert.h>


char* dc_client_create_req_json(mstack_t* var_ctx, mstack_t* expressions_ctx)
{
	char* r_code = NULL;

	json_t* root = json_object();

	if ( root && stack_size(expressions_ctx) ) {

		size_t i = 0;

		if (stack_size(var_ctx)) {
			json_t *params_obj = json_object();
			for (i=0; i<stack_size(var_ctx); ++i) {
				dc_VAL_t* n;
				if (NULL !=(n = stack_element_at(var_ctx, i))) {
					//printf("%s = %d\r\n", key, *n);
					json_object_set_new(params_obj, n->name, json_integer(n->value));
				}
			}
			json_object_set_new(root, "params", params_obj);
		}


		if (stack_size(expressions_ctx)) {
			json_t *json_results = json_array();
			for (i=0; i<stack_size(expressions_ctx); ++i) {
				dc_VAL_t* n;
				if ( NULL != (n = (dc_VAL_t*)stack_element_at(expressions_ctx, i)) ) {
					json_array_append(json_results, json_string(n->name));
				}
			}
			json_object_set_new(root, "expressions", json_results);
		}
		r_code = json_dumps(root, 0);
		json_decref(root);
	}
	return r_code;
}


uint32_t dc_client_parse_result_json(char* in_str, mstack_t* var_ctx, mstack_t* exp_ctx)
{
	uint32_t r_code = 1;

	json_error_t error;
	json_t* root = json_loads(in_str, 0, &error);

	if (root) {

		json_t* results = json_object_get(root, "results");
		if (results && json_is_array(results)) {

			size_t idx;
			json_t* v;

			json_array_foreach(results, idx, v) {
				if (v && json_is_string(v)) {
					dc_VAL_t rv;
					const char*j_str = json_string_value(v);
					rv.name = malloc(strlen(j_str)+1);
					assert(rv.name);
					if (rv.name) {
						strcpy(rv.name, j_str);
						stack_push_back(exp_ctx, &rv);
						r_code = 0;
					} else {
						r_code = 2;
						break;
					}
				}
			}
		} else {
			printf("no \"results\" object!\r\n");
		}


		if (!r_code) {

			json_t* params = json_object_get(root, "params");
			if (params && json_is_object(params)) {

				const char *key = NULL;
				json_t* v;

				json_object_foreach(params, key, v) {
					if (v && json_is_number(v)) {
						//printf("%s -> %d\r\n", key, json_integer_value(v));
						dc_VAL_t rv;
						rv.name = (char*)malloc(strlen(key)+1);
						assert(rv.name);
						if (rv.name) {
							strcpy(rv.name, key);
							rv.value = json_integer_value(v);
							stack_push_back(var_ctx, &rv);
						}
					}
				}

			}

		}

		json_decref(root);
	}
	return r_code;
}
