
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>


#include <uv.h>

//////////////////////////////////////////////////////////////////////////

int main()
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
//	if (!start_uv_tcp_server(SERVER_PORT)) {
//		return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
//	}
	return 1;
}
