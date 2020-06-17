
#include <stdint.h>
#include <uv.h>

#include "configuration.h"

#include "uv_server_proc.h"
#include "uv_calc_client.h"
//////////////////////////////////////////////////////////////////////////
#include <string.h>
#include "../tools/json_common.h"

int main(int argc, char* argv[])
{
	if (!load_config(BALANCER_CONFIG_FILE_NAME)) {

		for (size_t i=0; i<get_calc_host_count(); ++i) {
			calc_ctx_t a;
			if (!get_calc_host_addr(i, &a)) {
				printf("calc[%ld]: %s:%d\r\n", i, inet_ntoa(a.addr.sin_addr), ntohs(a.addr.sin_port));
			}
		}

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
