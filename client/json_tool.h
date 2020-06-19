
#pragma once

#include <stdint.h>
#include "../tools/mstack.h"


typedef uint64_t NUM_t ;

typedef struct VAL_t {
	char* name;
	NUM_t value;
} VAL_t;



char* create_req_json(mstack_t* var_ctx, mstack_t* expressions_ctx);

uint32_t parse_result_json(char* in_str, mstack_t* var_ctx, mstack_t* exp_ctx);