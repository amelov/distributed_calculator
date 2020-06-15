
#pragma once

#include <stdint.h>

//uint8_t is_valid_json(const char* s);


/*
	return string from (*b_i)[0] to delimiter,
	setup *b_i to next symbol
 */
char* get_msg_from_stream(char** b_i, char* msg_delimiter);
