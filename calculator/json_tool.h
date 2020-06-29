
#pragma once

#include <stdint.h>

#include "../tools/mstack.h"
#include "variable.h"


typedef struct session_data_t {
	mstack_t expression;
	mstack_t result;
	mstack_t error;
} session_data_t;


uint8_t dc_calc_parse_incoming_json(const char* in_msg, mstack_t* expression, var_store_t* variable);

char* dc_calc_create_outgoing_json(var_store_t* var, session_data_t* s);