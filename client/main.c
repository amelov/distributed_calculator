
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define _GNU_SOURCE
#include <getopt.h>

#include "common.h"

#include "configuration.h"
#include "readline_proc.h"



void dc_client_print_help()
{
	printf("Usage: ./client [options] <ip>\r\n"
			"Valid options are:\r\n"
			"-p, --port                    server port\r\n"
			"-h, --help                    show this text and exit\r\n");
}



int main(int argc, char* argv[])
{

	uint32_t balancer_ip = 0;
	uint16_t balancer_port = BALANCER_DEFAULT_PORT;

	static struct option long_options[] = {
		{"help", 	no_argument,       NULL,	'h'},
		{"port",    required_argument, NULL,	'p'},
		{" ",    	required_argument, NULL,	' '},
		{0,         no_argument,       NULL,	0}
	};

	int option_index = -1;
	int opt_r_code;
	while ((opt_r_code = getopt_long(argc, argv, "p:h?", long_options, &option_index)) != -1) {
		switch (opt_r_code) {
		case 'p':
			{
				if (optarg!=NULL) {
					balancer_port = atoi(optarg);
				} else {
					dc_client_print_help();
					return 3;
				}
			}
			break;
		case 'h':
		case '?':
			dc_client_print_help();
			return 0;
		}
		option_index = -1;
	}

	if (optind < argc) {
		balancer_ip = inet_addr(*(argv+optind));
		if ( (!balancer_ip) || (balancer_ip == INADDR_NONE) ) {
			printf("Invalid ip addr!\r\n");
			return 2;
		}
	} else {
		dc_client_print_help();
		return 1;
	}

	if (!balancer_port) {
		printf("Invalid port!\r\n");
		return 2;
	}

	dc_client_cfg_create(balancer_ip, balancer_port);

	return dc_client_start_readline();
}
