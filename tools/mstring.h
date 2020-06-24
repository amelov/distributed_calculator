
#pragma once

#ifndef whitespace
	#define whitespace(c) (((c) == ' ') || ((c) == '\t'))
#endif



char *str_create_copy(char* s);

char* stripwhite(char* in_str);