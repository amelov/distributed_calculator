
#include "uv_readline_proc.h"

#include <string.h>
#include <stdlib.h>


#define READLINE_LIBRARY
#include <readline/readline.h>
#include <readline/history.h>

#include "../tools/mstack.h"
#include "json_tool.h"
#include "uv_balancer_client.h"



static void on_calc_result(uv_tcp_t* client, char* result_str);


enum {
	NO_ERROR = 0,
	PARSE_ERROR = 1,
	NO_EXPRESSION_ERROR = 2,
};


mstack_t input_var_ctx;
mstack_t input_exp_ctx;


typedef int (*command_function_ptr_t)(char*);


typedef struct Command_t {
	char *name;			/* User printable name of the function. */
	command_function_ptr_t func;		/* Function to call to do the job. */
} Command_t;

int set_cmd(char*);
int add_cmd(char*);
int calculate_cmd(char*);

Command_t commands[] = {
  {"set", set_cmd},
  {"add", add_cmd},
  {"calculate", calculate_cmd},
  {NULL, (command_function_ptr_t)NULL }
};



char *str_create_copy(char* s)
{
	char *r_code = malloc(strlen(s) + 1);
	strcpy(r_code, s);
	return r_code;
}


void create_val_ctx(mstack_t* p1, mstack_t* p2)
{
	stack_create(p1, sizeof(VAL_t), 16);
	stack_create(p2, sizeof(VAL_t), 16);
}


void destroy_val_ctx(mstack_t* p1, mstack_t* p2)
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
int set_cmd(char* param)
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
		a.value = atoll(value_str);
		//printf("parse (%s) -> (\"%s\", %ld)\r\n", a.name, value_str, a.value);
		stack_push_back(&input_var_ctx, &a);
		return NO_ERROR;
	}
	return PARSE_ERROR;
}


int add_cmd(char* param)
{
	VAL_t a;
	a.name = str_create_copy(param);
	a.value = 0;
	stack_push_back(&input_exp_ctx, &a);
	printf("expression: (%s)\r\n", a.name);
	return NO_ERROR;
}


int calculate_cmd(char* param)
{
	if (stack_size(&input_exp_ctx)) {
		char* out_json = create_req_json(&input_var_ctx, &input_exp_ctx);
		if (out_json) {
			printf("#calculate_cmd -> (%s)\r\n", out_json);
			send_to_calc(out_json, &on_calc_result);
		}
		destroy_val_ctx(&input_var_ctx, &input_exp_ctx);
		create_val_ctx(&input_var_ctx, &input_exp_ctx);
		return NO_ERROR;
	} else {
		printf("no expressions!\r\n");
	}
	return NO_EXPRESSION_ERROR;
}



char* stripwhite(char* in_str)
{
	char* r_code = in_str;
	for (; whitespace(*r_code); r_code++) {
    	;
	}
    
	if (*r_code != 0) {
		char* t = r_code + strlen(r_code) - 1;
		while ( (t > r_code) && whitespace(*t)) {
			t--;
		}
		*++t = '\0';
	}

	return r_code;
}


char* command_generator(const char* text, int state)
{
	static int list_index, len;
	char *name;

	/* If this is a new word to complete, initialize now.  This includes
     saving the length of TEXT for efficiency, and initializing the index
     variable to 0. */
	if (!state) {
		list_index = 0;
		len = strlen(text);
	}

	/* Return the next name which partially matches from the command list. */
	while (NULL != (name = commands[list_index].name)) {
		list_index++;
		if (strncmp (name, text, len) == 0) {
			return str_create_copy(name);
		}
    }

	return (char *)NULL;
}


static char** command_completion(const char *text, int start, int end)
{
	char **matches = (char **)NULL;
	if (start == 0) {
		matches = rl_completion_matches(text, command_generator);
	}
	return (matches);
}



void make_operation(char* in_str)
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


void on_readline_work_cb(uv_work_t* req)
{

	char* input = NULL;
	char* shell_prompt = "> ";
	rl_attempted_completion_function = command_completion;

	create_val_ctx(&input_var_ctx, &input_exp_ctx);

	while (1) {
		input = readline(shell_prompt);
		if (!input) {
			break;
		}
		char* s = stripwhite(input);
		if (s) {
			add_history(s);
			make_operation(s);
		}
		free(input);
	}
}


void on_after_readline_work_cb(uv_work_t* req, int status)
{
	;
}



static void on_calc_result(uv_tcp_t* client, char* result_str)
{
	printf("RESULT: %s", result_str);

	mstack_t var_ctx;
	mstack_t exp_ctx;

	create_val_ctx(&var_ctx, &exp_ctx);

	if ( !parse_result_json(result_str, &var_ctx, &exp_ctx) ) {
		size_t i = 0;
		VAL_t* v;
		while ( NULL != (v=stack_element_at(&var_ctx, i))) {
			if (i)	{
				printf("; ");
			}
			printf("%s = %ld", v->name, v->value);
		}

		if (i) {
			printf("\r\n");
		}

		i = 0;
		while ( NULL != (v=stack_element_at(&exp_ctx, i))) {
			printf("%s\r\n", v->name);
		}
	} else {
		printf("Error parse result JSON!\r\n");
	}

	destroy_val_ctx(&var_ctx, &exp_ctx);

	close_calc_connection(client);
}