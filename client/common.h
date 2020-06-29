
#pragma once

#include <stdint.h>


#define BALANCER_DEFAULT_PORT 6500

#define MESSAGE_DELIMITER "\n"


typedef uint64_t dc_NUM_t ;

typedef struct dc_VAL_t {
	char* name;
	dc_NUM_t value;
} dc_VAL_t;