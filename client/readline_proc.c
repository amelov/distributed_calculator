
#include "readline_proc.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define READLINE_LIBRARY
#include <readline/readline.h>
#include <readline/history.h>

#include "../tools/mstack.h"
#include "../tools/mstring.h"

#include "json_tool.h"
#include "communication_proc.h"

enum {
	NO_ERROR = 0,
	PARSE_ERROR = 1,
	NO_EXPRESSION_ERROR = 2,
	INVALID_NUM_ERROR = 3,
};


static mstack_t input_var_ctx;
static mstack_t input_exp_ctx;


typedef int (*command_function_ptr_t)(char*);


typedef struct Command_t {
	char *name;			/* User printable name of the function. */
	command_function_ptr_t func;		/* Function to call to do the job. */
} Command_t;

static int dc_client_console_set_cmd(char*);
static int dc_client_console_add_cmd(char*);
static int dc_client_console_calculate_cmd(char*);

Command_t commands[] = {
	{"set", dc_client_console_set_cmd},
	{"add", dc_client_console_add_cmd},
	{"calculate", dc_client_console_calculate_cmd},
	{NULL, (command_function_ptr_t)NULL}
};



static void dc_client_console_ctx_create(mstack_t* p1, mstack_t* p2)
{
	stack_create(p1, sizeof(dc_VAL_t), 16);
	stack_create(p2, sizeof(dc_VAL_t), 16);
}


static void dc_client_console_ctx_destroy(mstack_t* p1, mstack_t* p2)
{
	for (int id=0; id<2; id++) {
		mstack_t* p = id? p2 : p1;
		for (size_t i=0; i<stack_size(p); i++) {
			dc_VAL_t* v = stack_element_at(p, i);
			free(v->name);
		}
		stack_destroy(p);
	}
}


static void dc_client_net_rx_calc_result_cb(char* result_str)
{
	mstack_t var_ctx;
	mstack_t exp_ctx;

	dc_client_console_ctx_create(&var_ctx, &exp_ctx);

	if ( !dc_client_json_result_parse(result_str, &var_ctx, &exp_ctx) ) {

		printf("\r\n");

		size_t i = 0;
		dc_VAL_t* v;
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

	dc_client_console_ctx_destroy(&var_ctx, &exp_ctx);
}


// parse string: "aaa = xxxx
static int dc_client_console_set_cmd(char* param)
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
		dc_VAL_t a;
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


static int dc_client_console_add_cmd(char* param)
{
	dc_VAL_t a;
	a.name = str_create_copy(param);
	a.value = 0;
	stack_push_back(&input_exp_ctx, &a);
	return NO_ERROR;
}


static int dc_client_console_calculate_cmd(char* param)
{
	if (stack_size(&input_exp_ctx)) {
		char* out_json = dc_client_json_create(&input_var_ctx, &input_exp_ctx);
		if (out_json) {
			dc_client_net_send_calculation_job(out_json, &dc_client_net_rx_calc_result_cb);
		}
		dc_client_console_ctx_destroy(&input_var_ctx, &input_exp_ctx);
		dc_client_console_ctx_create(&input_var_ctx, &input_exp_ctx);
		return NO_ERROR;
	}
	return NO_EXPRESSION_ERROR;
}


static char* dc_client_console_command_generator(const char* text, int state)
{
	static int list_index, len;
	char *name;

	if (!state) {
		list_index = 0;
		len = strlen(text);
	}

	while (NULL != (name = commands[list_index].name)) {
		list_index++;
		if (strncmp (name, text, len) == 0) {
			return str_create_copy(name);
		}
	}

	return (char *)NULL;
}


static char** dc_client_console_command_completion(const char *text, int start, int end)
{
	char **matches = (char **)NULL;
	if (start == 0) {
		matches = rl_completion_matches(text, dc_client_console_command_generator);
	}
	return (matches);
}


static void dc_client_console_make_operation(char* in_str)
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

int dc_client_start_readline()
{
	printf("\n");
	
	rl_attempted_completion_function = dc_client_console_command_completion;
	rl_bind_key('\t', rl_complete);

	dc_client_console_ctx_create(&input_var_ctx, &input_exp_ctx);

	while (1) {
		char* input = readline("> ");
		if (!*input) {
			return 0;
		}

		add_history(input);
		dc_client_console_make_operation(input);
		free(input);
	}

	dc_client_console_ctx_destroy(&input_var_ctx, &input_exp_ctx);
	return 0;
}
