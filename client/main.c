
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <uv.h>

#include "uv_readline_proc.h"
#include "uv_balancer_client.h"
#include "configuration.h"

//////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{

	client_descr_t balancer_client;
	balancer_client.addr.sin_family = AF_INET;
	balancer_client.addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	balancer_client.addr.sin_port = htons( BALANCER_PORT );
	balancer_client.state = UNDEF_STATE;
	balancer_client.connect.data = &balancer_client;
	balancer_client.handle.data = &balancer_client;
	start_uv_tcp_client(&balancer_client);

	uv_timer_t reconnect_timer;
	uv_timer_init(uv_default_loop(), &reconnect_timer);
	uv_timer_start(&reconnect_timer, on_reconnect_timer_cb, RECONNECT_TIMEOUT_ms, RECONNECT_TIMEOUT_ms);
	
	uv_work_t readline_hnd;
	if (uv_queue_work(uv_default_loop(), &readline_hnd, on_readline_work_cb, on_after_readline_work_cb) == 0) {
		;
	}

	return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
