
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

#include "common.h"

#include "configuration.h"
#include "console_proc.h"


int main(int argc, char* argv[])
{
	cfg_create(inet_addr("127.0.0.1"), BALANCER_DEFAULT_PORT);
	switch (argc) {
	case 2:
		cfg_create(inet_addr("127.0.0.1"), atoi(argv[1]));
		break;
	case 3:
		cfg_create(inet_addr(argv[1]), atoi(argv[2]));
		break;
	}

	return console_handler();
}
