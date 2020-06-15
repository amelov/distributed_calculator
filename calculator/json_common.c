
#include "json_common.h"

#include <stdlib.h>
#include <string.h>

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
