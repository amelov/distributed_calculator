
#include <stdint.h>

#include "configuration.h"

#include "uv_proc.h"
#include <uv.h>

#include "calc_client.h"
//////////////////////////////////////////////////////////////////////////



int main()
{
	if (!load_config(BALANCER_CONFIG_FILE_NAME)) {

		size_t idx = 0;
		client_descr_t* p = NULL;
		while ( NULL != (p=get_calc_host(idx++)) ) {
			//client_descr_t* a = (client_descr_t*)stack_element_at(ch, i);
			//a.sin_addr.s_addr = inet_addr( json_string_value(ip_v) );
			//a.sin_port = json_number_value(port_v);
			printf("%s:%d state:%d\n", inet_ntoa(p->addr.sin_addr), ntohs(p->addr.sin_port), p->state);
		}



		idx = 0;
		while ( NULL != (p=get_calc_host(idx++)) ) {
			start_uv_tcp_client(p);
		}

		uv_timer_t reconnect_timer;
		uv_timer_init(uv_default_loop(), &reconnect_timer);
		uv_timer_start(&reconnect_timer, on_reconnect_timer_cb, RECONNECT_TIMEOUT_ms, RECONNECT_TIMEOUT_ms);


		if (!start_uv_tcp_server(/*SERVER_PORT*/7001)) {



			return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
		}
	} else {
		printf("error read %s file!\n", BALANCER_CONFIG_FILE_NAME);
	}
	return 1;
}
