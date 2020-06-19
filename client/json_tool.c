

#include "json_tool.h"
#include <jansson.h>
#include <string.h>


char* create_req_json(mstack_t* var_ctx, mstack_t* expressions_ctx)
{
	char* r_code = NULL;

	json_t *root = json_object();

	if ( root && stack_size(expressions_ctx) ) {

		size_t i = 0;

		if (stack_size(var_ctx)) {
			json_t *params_obj = json_object();
			for (i=0; i<stack_size(var_ctx); ++i) {
				VAL_t* n;
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
				VAL_t* n;
				if ( NULL != (n = (VAL_t*)stack_element_at(expressions_ctx, i)) ) {
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


uint32_t parse_result_json(char* in_str, mstack_t* var_ctx, mstack_t* exp_ctx)
{
	uint32_t r_code = 1;

	json_error_t error;
	json_t* root = json_loads(in_str, 0, &error);

	if (root) {

		json_t *expressions = json_object_get(root, "expressions");
		if (expressions && json_is_array(expressions)) {

			//char* p_str = NULL;
			size_t idx;
			json_t* v;

			json_array_foreach(expressions, idx, v) {
				if (v && json_is_string(v)) {
					VAL_t rv;
					const char*j_str = json_string_value(v);
					rv.name = malloc(strlen(j_str)+1);
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
			printf("no \"expressions\" object!\r\n");
		}


		if (!r_code) {

			json_t *params = json_object_get(root, "params");
			if (params && json_is_object(params)) {

				const char *key = NULL;
				json_t* v;

				json_object_foreach(params, key, v) {
					if (v && json_is_number(v)) {
						//printf("%s -> %d\r\n", key, json_integer_value(v));
						VAL_t rv;
						rv.name = (char*)malloc(strlen(key)+1);
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


/*

	mstack_t var_ctx;
	mstack_t exp_ctx;
	VAR_t v;
	VAR_t* p_v;

	stack_create(&var_ctx, sizeof(v), 16);
	v.name = (char*)malloc(10);
	strcpy(v.name, "a");
	v.value = 1;
	stack_push_back(&var_ctx, &v);

	stack_create(&exp_ctx, sizeof(v), 16);
	v.name = (char*)malloc(10);
	strcpy(v.name, "a+a");
	stack_push_back(&exp_ctx, &v);

	r_code = create_req_json(&var_ctx, &exp_ctx);
	printf("\"%s\"", r_code);

	free(r_code);


	while ( NULL != (p_v = stack_pop_back((&var_ctx))) ) {
		free(p_v->name);
	}
	stack_destroy(&var_ctx);

	while ( NULL != (p_v = stack_pop_back((&exp_ctx))) ) {
		free(p_v->name);
	}
	stack_destroy(&exp_ctx);

        */