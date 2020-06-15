
#pragma once

#include <stdint.h>

#include "mstack.h"
#include "variable.h"


typedef struct session_data_t {
	var_store_t var;
	mstack_t expression;
	mstack_t result;
} session_data_t;


uint8_t parse_incoming_json(const char* in_msg, session_data_t* s);

char* create_outgoing_json(session_data_t* s);