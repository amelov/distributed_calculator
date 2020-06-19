
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <uv.h>

#include "common.h"

#include "uv_readline_proc.h"
#include "configuration.h"




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

	start_readline();

	return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
