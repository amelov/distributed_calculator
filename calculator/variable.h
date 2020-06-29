/*
	Variables storage:
		<key(name)> <value>
 */


#pragma once

#include "common.h"

typedef mstack_t var_store_t;

void dc_calc_var_init(var_store_t* vs, const size_t var_count);
void dc_calc_var_add(var_store_t* vs, const char* id, NUM_t v);
void dc_calc_var_add_complete(var_store_t* vs);

// -> return 1 if find success complete
uint8_t dc_calc_var_find(var_store_t* vs, char* id, NUM_t* v);


size_t  dc_calc_var_size(var_store_t* vs);
uint8_t dc_calc_var_element_at(var_store_t* vs, const size_t idx, char**key, NUM_t **n);

void dc_calc_var_destroy(var_store_t* vs);