

#include "json_tool.h"
#include <string.h>

#include <jansson.h>


uint8_t is_valid_json(const char* s)
{
	size_t open_brace = 0;
	size_t close_brace = 0;

	while (*s) {
		if ('{' ==*s) {
			open_brace++;
		} else if ('}' ==*s) {
			close_brace++;
		}
		s++;
	}
	return (open_brace) && (open_brace==close_brace);
}


uint8_t parse_incoming_json(const char* in_msg, session_data_t* s)
{
	uint8_t r_code = 1;

	json_error_t error;
	json_t *root;

	json_t *v = NULL;
	
	root = json_loads(in_msg, 0, &error);
	
	if (root) {
		
		json_t *expressions = json_object_get(root, "expressions");
		if (expressions && json_is_array(expressions)) {

			char* p_str = NULL;
			size_t idx;

			stack_create(&s->expression, sizeof(p_str), json_array_size(expressions));
			stack_create(&s->result, sizeof(NUM_t), json_array_size(expressions));

			json_array_foreach(expressions, idx, v) {
				if (v && json_is_string(v)) {
					const char*j_str = json_string_value(v);
					p_str = malloc(strlen(j_str)+1);
					if (p_str) {
						strcpy(p_str, j_str);
						stack_push_back(&s->expression, &p_str);
						//printf("%s\r\n", j_str);
						r_code = 0;
					} else {
						r_code = 2;
						break;
					}
				}
			}
		}


		if (!r_code) {

			json_t *params = json_object_get(root, "params");
			if (params && json_is_object(params)) {

				const char *key = NULL;

				var_init(&s->var, json_object_size(params));
				
				json_object_foreach(params, key, v) {
					if (v && json_is_number(v)) {
						//printf("%s -> %d\r\n", key, json_integer_value(v));
						var_add(&s->var, key, json_integer_value(v));
					}
				}

				var_add_complete(&s->var);
			}

		}

		json_decref(root);
	}

	return r_code;
}


char* create_outgoing_json(session_data_t* sess)
{
	char* r_code = NULL;

	json_t *root = json_object();

	if (root) {	

		size_t i = 0;

		{
			json_t *params_obj = json_object();
			for (i=0; i<var_size(&sess->var); ++i) {
				char* key;
				NUM_t* n;

				if (!var_element_at(&sess->var, i, &key, &n)) {
					//printf("%s = %d\r\n", key, *n);
					json_object_set_new(params_obj, key, json_integer(*n));
				}
			}
			json_object_set_new(root, "params", params_obj);
		}


		{
			json_t *json_results = json_array();
			for (i=0; i<stack_size(&sess->result); ++i) {
				char* exp_str = *((char**)stack_element_at(&sess->expression, i));
				NUM_t* res = (NUM_t*)stack_element_at(&sess->result, i);

				if (exp_str && res) {
					char* temp_buf = (char*)malloc( strlen(exp_str) + 3 + MAX_NUM_T_TO_STR_LEN + 1);
					sprintf(temp_buf, "%s = %lld", exp_str, *res);
					//printf(":%s\r\n", temp_buf);
					json_array_append(json_results, json_string(temp_buf));
					free(temp_buf);
				}
			}
			json_object_set_new(root, "results", json_results);
		}

		r_code = json_dumps(root, 0);
		json_decref(root);
	}
	return r_code;
}
