
#pragma once

#include <stdint.h>

#include "stack.h"
#include "variable.h"


typedef struct session_data_t {
	var_store_t var;
	stack_t expression;
	stack_t result;
} session_data_t;



uint8_t parse_incoming_json(char* in_msg, session_data_t* s);

char* create_outgoing_json(session_data_t* s);