
#include "json_common.h"

#include <stdlib.h>



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
