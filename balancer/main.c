
#include <stdint.h>
#include <uv.h>

#include "configuration.h"

#include "uv_server_proc.h"
#include "uv_calc_client.h"
//////////////////////////////////////////////////////////////////////////
#include <string.h>
#include "json_common.h"

int main(int argc, char* argv[])
{
	if (!load_config(BALANCER_CONFIG_FILE_NAME)) {

		size_t idx = 0;
		client_descr_t* p = NULL;
		while ( NULL != (p=get_calc_host(idx++)) ) {
			p->connect.data = p;
			p->handle.data = p;

			start_uv_tcp_client(p);
		}

		uv_timer_t reconnect_timer;
		uv_timer_init(uv_default_loop(), &reconnect_timer);
		uv_timer_start(&reconnect_timer, on_reconnect_timer_cb, RECONNECT_TIMEOUT_ms, RECONNECT_TIMEOUT_ms);

		uint16_t server_port = get_server_port();
		if ( (argc>1) && atoi(argv[1]) ) {
			server_port = atoi(argv[1]);
		}

		if (!start_uv_tcp_server(server_port)) {
			return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
		}

	} else {
		printf("error read %s file!\n", BALANCER_CONFIG_FILE_NAME);
	}
	return 1;
}
