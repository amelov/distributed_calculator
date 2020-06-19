
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
	set_balancer_addr(inet_addr("127.0.0.1"), BALANCER_DEFAULT_PORT);
	switch (argc) {
	case 2:
		set_balancer_addr(inet_addr("127.0.0.1"), atoi(argv[1]));
		break;
	case 3:
		set_balancer_addr(inet_addr(argv[1]), atoi(argv[2]));
		break;
	}
	
//	char* a = "{\"expressions\":[\"1+2+3+4+5\"]}\r\n";
//	send_to_calc(a, NULL);

	uv_work_t readline_hnd;
	if (uv_queue_work(uv_default_loop(), &readline_hnd, on_readline_work_cb, on_after_readline_work_cb) == 0) {
		;
	}

	return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
