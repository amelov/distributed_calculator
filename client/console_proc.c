
#include "console_proc.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>


#include "../tools/mstack.h"
#include "../tools/mstring.h"

#include "json_tool.h"
#include "communication_proc.h"

enum {
	NO_ERROR = 0,
	PARSE_ERROR = 1,
	NO_EXPRESSION_ERROR = 2,
	INVALID_NUM_ERROR,
};


mstack_t input_var_ctx;
mstack_t input_exp_ctx;


typedef int (*command_function_ptr_t)(char*);


typedef struct Command_t {
	char *name;			/* User printable name of the function. */
	command_function_ptr_t func;		/* Function to call to do the job. */
} Command_t;

static int set_cmd(char*);
static int add_cmd(char*);
static int calculate_cmd(char*);

Command_t commands[] = {
  {"set", set_cmd},
  {"add", add_cmd},
  {"calculate", calculate_cmd},
  {NULL, (command_function_ptr_t)NULL }
};



void on_calc_result_cb(char* result_str);



static void create_val_ctx(mstack_t* p1, mstack_t* p2)
{
	stack_create(p1, sizeof(VAL_t), 16);
	stack_create(p2, sizeof(VAL_t), 16);
}


static void destroy_val_ctx(mstack_t* p1, mstack_t* p2)
{
	for (int id=0; id<2; id++) {
		mstack_t* p = id? p2 : p1;
		for (size_t i=0; i<stack_size(p); i++) {
			VAL_t* v = stack_element_at(p, i);
			free(v->name);
		}
		stack_destroy(p);
	}
}


// parse string: "aaa = xxxx  
static int set_cmd(char* param)
{	
	char* name_str = "";
	char* value_str = "";

	char* separator_p = strstr(param, "=");

	if (separator_p) {
		*separator_p = ' ';

		name_str = param;
		char* p = param;

		while ( *p && (!whitespace(*p)) ) {
			p++;
		}
		*p = 0;
		p++;
		
		while ( *p && whitespace(*p) ) {
			p++;
		}

		value_str = p;
		while ( *p && (!whitespace(*p)) ) {
			p++;
		}
		*p = 0;
	}

	if ( (*name_str) && (*value_str) ) {
		VAL_t a;
		a.name = str_create_copy(name_str);


		// Check number value
		char* p = value_str;
		while (*p) {
			if (!isdigit(*p)) {
				return INVALID_NUM_ERROR;
			}
			p++;
		}

		a.value = atoll(value_str);
		stack_push_back(&input_var_ctx, &a);
		return NO_ERROR;
	}
	return PARSE_ERROR;
}


static int add_cmd(char* param)
{
	VAL_t a;
	a.name = str_create_copy(param);
	a.value = 0;
	stack_push_back(&input_exp_ctx, &a);
	return NO_ERROR;
}


int calculate_cmd(char* param)
{
	if (stack_size(&input_exp_ctx)) {
		char* out_json = create_req_json(&input_var_ctx, &input_exp_ctx);
		if (out_json) {
			send_to_calc(out_json, &on_calc_result_cb);
		}
		destroy_val_ctx(&input_var_ctx, &input_exp_ctx);
		create_val_ctx(&input_var_ctx, &input_exp_ctx);
		return NO_ERROR;
	}
	return NO_EXPRESSION_ERROR;
}


void on_calc_result_cb(char* result_str)
{
	//printf("RESULT: %s", result_str);
	mstack_t var_ctx;
	mstack_t exp_ctx;

	create_val_ctx(&var_ctx, &exp_ctx);

	if ( !parse_result_json(result_str, &var_ctx, &exp_ctx) ) {

		printf("\r\n");

		size_t i = 0;
		VAL_t* v;
		while ( NULL != (v=stack_element_at(&var_ctx, i)) ) {
			if (i)	{
				printf("; ");
			}
			printf("%s = %ld", v->name, v->value);
			i++;
		}

		if (i) {
			printf("\r\n");
		}

		i = 0;
		while ( NULL != (v=stack_element_at(&exp_ctx, i)) ) {
			printf("%s\r\n", v->name);
			i++;
		}

	} else {
		printf("Error parse result JSON!\r\n");
	}

	destroy_val_ctx(&var_ctx, &exp_ctx);
}


void user_enter_line(char* in_str)
{
	if (strstr(in_str, ";")) {
		char *cmd_str = NULL;

		const size_t len = strstr(in_str, ";") - in_str;

		int i = 0;
		while (in_str[i] && whitespace(in_str[i])) {
			i++;
		}

		cmd_str = in_str + i;

		while ( in_str[i] && (!whitespace(in_str[i])) && (in_str[i]!=';') ) {
			i++;
		}

		if (in_str[i]) {
			in_str[i++] = 0;
		}

		uint8_t find_cmd_flag = 0;
		for (int cmd_idx=0; commands[cmd_idx].name; ++cmd_idx) {
			if (strcmp(cmd_str, commands[cmd_idx].name) == 0) { 


				find_cmd_flag = 1;
				while ( (i<len) && whitespace(in_str[i]) ) {
					i++;
				}
				char* param = in_str + i;

				if (param) {
					while ( (i<len) && (in_str[i]!=';') ) {
						i++;
					}
					in_str[i++] = 0;
				}

				if (commands[cmd_idx].func) {
					switch ((*(commands[cmd_idx].func))(param)) {
					case NO_ERROR:	
						break;
					case PARSE_ERROR:
						printf("command parse error!\r\n");
						break;
					case NO_EXPRESSION_ERROR:
						printf("no expression!\r\n");
						break;
					case INVALID_NUM_ERROR:
						printf("invalid num!\r\n");
						break;						
					}
				}
				break;
			}
		}

		if (!find_cmd_flag) {
			printf("\"%s\" - undefined command!\r\n", cmd_str);
		}
	} else {
		printf("Parse error. ';' expected\r\n");
	}

}


////////////////////////////////////////////////////////////////////


uint32_t console_handler()
{
	char c = 0;

	create_val_ctx(&input_var_ctx, &input_exp_ctx);

	mstack_t console_input_stack;
	stack_create(&console_input_stack, sizeof(c), 128);
	printf(">");

	while ( (c=getchar()) ) {

		if (c=='\n') {
			c = 0;
			stack_push_back(&console_input_stack, &c);
			char* input_str = (char*)calloc(1, stack_size(&console_input_stack));
			strcpy(input_str, (char*)stack_raw_data(&console_input_stack));

			user_enter_line(input_str);

			free(input_str);
			stack_reinit(&console_input_stack);
			printf(">");
		} else {
			stack_push_back(&console_input_stack, &c);
		}
	}
	return 0;
}