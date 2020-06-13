
#include "json_common.h"

#include <stdlib.h>
#include <string.h>


uint8_t is_valid_json(const char* s)
{
	size_t open_brace = 0;
	size_t close_brace = 0;

	while (*s) {
		if ('{' ==*s) {
			open_brace++;
		} else if ('}' ==*s) {
			close_brace++;
		}
		s++;
	}
	return (open_brace) && (open_brace==close_brace);
}


/*
	return string from (*b_i)[0] to delimiter,
	setup *b_i to next symbol
 */
char* get_msg_from_stream(char** b_i, char* msg_delimiter)
{
	char* r_code = NULL;
	char* f_i = strstr(*b_i, msg_delimiter);

	if (f_i) {
		r_code = (char*)calloc(1, f_i-(*b_i)+1+1);
		//*f_i = 0;
		memcpy(r_code, *b_i, (f_i-(*b_i)+1));
		*b_i = f_i+1;
	}

	return r_code;
}
