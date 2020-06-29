

#include <string.h>

#include "json_tool.h"
#include <jansson.h>
#include "../tools/mstring.h"




uint8_t dc_calc_parse_incoming_json(const char* in_msg, mstack_t* out_exp, var_store_t* variable)
{
	uint8_t r_code = 1;

	json_error_t error;
	json_t *root;

	json_t *v = NULL;
	
	root = json_loads(in_msg, 0, &error);
	
	if (root) {
		
		json_t *j_exp = json_object_get(root, "expressions");
		if (j_exp && json_is_array(j_exp)) {

			char* p_str = NULL;
			size_t idx;

			stack_create(out_exp, sizeof(p_str), json_array_size(j_exp));
			//stack_create(&s->result, sizeof(NUM_t), json_array_size(j_exp));

			json_array_foreach(j_exp, idx, v) {
				if (v && json_is_string(v)) {
					const char*j_str = json_string_value(v);
					p_str =  str_create_copy(j_str);
					if (p_str) {
						stack_push_back(out_exp, &p_str);
						//printf("%s\r\n", j_str);
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

				dc_calc_var_init(variable, json_object_size(params));
				
				json_object_foreach(params, key, v) {
					if (v && json_is_number(v)) {
						//printf("%s -> %d\r\n", key, json_integer_value(v));
						dc_calc_var_add(variable, key, json_integer_value(v));
					}
				}

				dc_calc_var_add_complete(variable);
			}

		}

		json_decref(root);
	}

	return r_code;
}


char* dc_calc_create_outgoing_json(var_store_t* var, session_data_t* sess)
{
	char* r_code = NULL;

	json_t *root = json_object();

	if (root) {	
		size_t i = 0;

		if (var && dc_calc_var_size(var)) {
			json_t *params_obj = json_object();
			for (i=0; i<dc_calc_var_size(var); ++i) {
				char* key;
				NUM_t* n;

				if (!dc_calc_var_element_at(var, i, &key, &n)) {
					//printf("%s = %d\r\n", key, *n);
					json_object_set_new(params_obj, key, json_integer(*n));
				}
			}
			json_object_set_new(root, "params", params_obj);
		}


		if (stack_size(&sess->result)) {
			json_t *json_results = json_array();
			for (i=0; i<stack_size(&sess->result); ++i) {
				char* err_str = *((char**)stack_element_at(&sess->error, i));
				char* exp_str = *((char**)stack_element_at(&sess->expression, i));
				NUM_t* res = (NUM_t*)stack_element_at(&sess->result, i);

				if ( err_str ) {

					json_array_append(json_results, json_string(err_str));
				} else if (exp_str && res) {
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
