
#include "uv_readline_proc.h"

#include <readline/readline.h>
#include <readline/history.h>


void on_readline_work_cb(uv_work_t* req)
{
	char* input = NULL;

	char shell_prompt[100] = "> ";
	//snprintf(shell_prompt, sizeof(shell_prompt), "%s:%s $ ", getenv("USER"), getcwd(NULL, 1024));

	while (1) {
		input = readline(shell_prompt);
	        // eof
        	if (!input)
	            break;
		// path autocompletion when tabulation hit
        	rl_bind_key('\t', rl_complete);
        
	        // adding the previous input into history
        	add_history(input);

	        /* do stuff */

        	free(input);
	}
}


void on_after_readline_work_cb(uv_work_t* req, int status)
{
	;
}
