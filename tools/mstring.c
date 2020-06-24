
#include "mstring.h"

#include <string.h>
#include <stdlib.h>


char *str_create_copy(char* s)
{
	char *r_code = malloc(strlen(s) + 1);
	strcpy(r_code, s);
	return r_code;
}


char* stripwhite(char* in_str)
{
	char* r_code = in_str;
	for (; whitespace(*r_code); r_code++) {
		;
	}

	if (*r_code != 0) {
		char* t = r_code + strlen(r_code) - 1;
		while ( (t > r_code) && whitespace(*t)) {
			t--;
		}
		*++t = '\0';
	}

	return r_code;
}