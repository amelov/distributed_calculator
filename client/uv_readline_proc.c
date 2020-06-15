
#include "uv_readline_proc.h"

#include <string.h>
#include <stdlib.h>


//#define READLINE_LIBRARY
#include <readline/readline.h>
#include <readline/history.h>



typedef int (*function_ptr_t)(char*, char*);


typedef struct Command_t {
	char *name;			/* User printable name of the function. */
	function_ptr_t func;		/* Function to call to do the job. */
	//char *doc;			/* Documentation for this function.  */
} Command_t;


//int set_cmd(char* param1, char* param2);
//int add_cmd(char* param1, char* param2);
//int calculate_cmd(char* param1, char* param2);




int set_cmd(char* param1, char* param2)
{	
	return 1;
}


int add_cmd(char* param1, char* param2)
{
	return 1;
}


int calculate_cmd(char* param1, char* param2)
{
	return 1;
}



Command_t commands[] = {
  {"set", set_cmd},
  {"add", add_cmd},
  {"calculate", calculate_cmd},
  {NULL, (function_ptr_t)NULL }
};


int is_whitespace(const char a)
{
	return (' ' == a);
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


void make_operation(char* in_str)
{
#if 0	
  	char *word = NULL;

	/* Isolate the command word. */
	int i = 0;
	while (in_str[i] && whitespace(in_str[i])) {
    	i++;
	}

  	word = in_str + i;

  	while (in_str[i] && !whitespace (in_str[i])) {
    	i++;
  	}

	if (in_str[i]) {
		in_str[i++] = '\0';
	}

	Command_t *command = find_command(word);

	if (!command) {
		fprintf (stderr, "%s: No such command for FileMan.\n", word);
		return (-1);
	}

	/* Get argument to command, if any. */
//	while (whitespace (line[i])) {
//    	i++;
//	}

//	word = line + i;
#endif
	/* Call the function. */
	//return  0;//((*(command->func))(word));
}


char *create_copy(char* s)
{
	char *r_code = malloc(strlen(s) + 1);
	strcpy(r_code, s);
	return r_code;
}


char* command_generator(int* text, int state)
{
	static int list_index, len;
	char *name;

	/* If this is a new word to complete, initialize now.  This includes
     saving the length of TEXT for efficiency, and initializing the index
     variable to 0. */
	if (!state) {
		list_index = 0;
		len = strlen (text);
	}

	/* Return the next name which partially matches from the command list. */
	while (NULL != (name = commands[list_index].name)) {
		list_index++;
		if (strncmp (name, text, len) == 0) {
			return create_copy(name);
		}
    }

	/* If no names matched, then return NULL. */
	return (char *)NULL;
}


char** command_completion(char *text, int start, int end)
{
	char **matches = (char **)NULL;

printf("#%s", text);

  /* If this word is at the start of the line, then it is a command
     to complete.  Otherwise it is the name of a file in the current
     directory. */
	if (start == 0) {
		matches = completion_matches(text, command_generator);
	}
	return (matches);
}







void on_readline_work_cb(uv_work_t* req)
{
	char* input = NULL;

	char* shell_prompt = "> ";

	//snprintf(shell_prompt, sizeof(shell_prompt), "%s:%s $ ", getenv("USER"), getcwd(NULL, 1024));
	rl_attempted_completion_function = (CPPFunction *)command_completion;

	while (1) {
		input = readline(shell_prompt);
		// eof
		if (!input) {
			break;
		}
		// path autocompletion when tabulation hit
		//rl_bind_key('\t', rl_complete);
        
		char* s = stripwhite(input);
		if (s) {
			// adding the previous input into history
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
