
#pragma once

#include <stdint.h>

#include "../tools/mstack.h"
#include "variable.h"


typedef struct session_data_t {
	mstack_t expression;
	mstack_t result;
	mstack_t error;
} session_data_t;


char *str_create_copy(const char* s);

uint8_t parse_incoming_json(const char* in_msg, mstack_t* expression, var_store_t* variable);

char* create_outgoing_json(var_store_t* var, session_data_t* s);