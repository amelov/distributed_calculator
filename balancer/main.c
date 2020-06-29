
#include <stdint.h>
#include <string.h>
#include <uv.h>

#include "configuration.h"

#include "uv_server_proc.h"
#include "uv_calc_client.h"


#include "../tools/json_common.h"


int main(int argc, char* argv[])
{
	if (!dc_balancer_load_config(BALANCER_CONFIG_FILE_NAME)) {

		for (size_t i=0; i<dc_balancer_get_calc_host_count(); ++i) {
			calc_ctx_t a;
			if (!dc_balancer_get_calc_host_addr(i, &a)) {
				printf("calc[%ld]: %s:%d\r\n", i, inet_ntoa(a.addr.sin_addr), ntohs(a.addr.sin_port));
			}
		}

		uint16_t server_port = dc_balancer_get_server_port();
		if ( (argc>1) && atoi(argv[1]) ) {
			server_port = atoi(argv[1]);
		}

		if (!dc_balancer_start_tcp_server(server_port)) {
			return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
		}

	} else {
		printf("error read %s file!\n", BALANCER_CONFIG_FILE_NAME);
	}
	return 1;
}
