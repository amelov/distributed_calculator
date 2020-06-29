
#include <stdint.h>


#include "common.h"


#ifdef MAKE_TEST
	#include "convert_to_rpn.h"
#endif

#include "uv_server_proc.h"
#include <uv.h>

//////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
#if MAKE_TEST
	dc_calc_convert_to_rpn_TEST();
	dc_calc_calculate_TEST();
#else 

	uint16_t server_port = DEFAULT_SERVER_PORT;
	if ( (argc>1) && atoi(argv[1]) ) {
		server_port = atoi(argv[1]);
	}

	uv_tcp_t server;
	if (!dc_calc_start_tcp_server(&server, server_port)) {
		return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
	}

#endif

	return 1;
}
