
#pragma once

#include <stdint.h>

/*
	return string from (*b_i)[0] to delimiter,
	setup *b_i to next symbol
 */
char* get_msg_from_stream(char** b_i, char* msg_delimiter);
