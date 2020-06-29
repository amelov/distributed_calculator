
#include "variable.h"
#include <stdlib.h>
#include <string.h>



typedef struct var_t {
	char* _key;
	NUM_t _value;
} var_t;


//////////////////////////////////////////////////////////////////////////

static int dc_calc_var_compare(const void* a, const void* b)
{
	var_t* av = (var_t*)a;
	var_t* bv = (var_t*)b;

	return strcmp(av->_key, bv->_key);
}

//////////////////////////////////////////////////////////////////////////


void dc_calc_var_init(var_store_t* vs, const size_t var_count)
{
	stack_create(vs, sizeof(var_t), (var_count? var_count : INIT_VARIABLE_COUNT));
}


void dc_calc_var_add(var_store_t* vs, const char* id, NUM_t v)
{
	var_t temp_v;
	temp_v._key = malloc(strlen(id)+1);
	strcpy(temp_v._key, id);
	temp_v._value = v;
	stack_push_back(vs, &temp_v);
}


void dc_calc_var_add_complete(var_store_t* vs)
{
	if (vs->_offset) {
		qsort(vs->_data_ptr, vs->_offset, vs->_element_size, dc_calc_var_compare);
	}
}


// -> return 1 if find success complete
uint8_t dc_calc_var_find(var_store_t* vs, char* id, NUM_t* v)
{
	if ( vs && id && vs->_offset && v ) {
		
		var_t* f = NULL;

		var_t find_key;
		find_key._key = id;

		f = bsearch(&find_key, vs->_data_ptr, vs->_offset, vs->_element_size, dc_calc_var_compare);

		if (f) {
			*v = f->_value;
			return 1;
		}
	}
	return 0;
}


size_t dc_calc_var_size(var_store_t* vs)
{
	return vs->_element_size;
}


uint8_t dc_calc_var_element_at(var_store_t* vs, const size_t idx, char** key, NUM_t** n)
{
	var_t* v = stack_element_at(vs, idx);

	if (v) {
		*key = v->_key;
		*n   = &v->_value;
		return 0;
	}
	return 1;
}


void dc_calc_var_destroy(var_store_t* vs)
{
	var_t* v;
	while (NULL!=(v = stack_pop_back(vs))) {
		free(v->_key);
	}
	stack_destroy(vs);
}