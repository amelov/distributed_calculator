
#include <stdint.h>


#include "common.h"


#ifdef MAKE_TEST
	#include "convert_to_rpn.h"
#endif

#include "uv_proc.h"
#include <uv.h>

//////////////////////////////////////////////////////////////////////////

int main()
{
#if MAKE_TEST
   	convert_to_rpn_TEST();
 	calculate_TEST();
#else 

/*
	struct sockaddr_in addr;


	uv_loop_t *loop = uv_default_loop();



	char* in_json_str = "{\"params\": {\"x\": 3, \"y\": 5, \"z\": -1}, \"expressions\": [\"(x + y) * 5\", \"3 * x - z + 2 * y\"]}";

	char* out_json_str = input_json_msg_handler(in_json_str);

	if (out_json_str) {
		printf("-> %s", out_json_str);
		free(out_json_str);
	}
 */

	if (!start_uv_tcp_server(SERVER_PORT)) {
		return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
	}


#endif

	return 0;
}
