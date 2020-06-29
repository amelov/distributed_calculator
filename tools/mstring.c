
#include "mstring.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

char *str_create_copy(const char* s)
{
	char *res_str = malloc(strlen(s) + 1);
	assert(res_str);
	strcpy(res_str, s);
	return res_str;
}


char* str_strip_white(char* in_str)
{
	char* res_str = in_str;
	for (; whitespace(*res_str); res_str++) {
		;
	}

	if (*res_str != 0) {
		char* t = res_str + strlen(res_str) - 1;
		while ( (t > res_str) && whitespace(*t)) {
			t--;
		}
		*++t = '\0';
	}

	return res_str;
}