

#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef struct mstack_t {
	size_t _element_size;
	size_t _max_element_count;
	size_t _offset;	// == element count
	void* _data_ptr;
} mstack_t;


//////////////////////////////////////////////////////////////////////////

uint8_t stack_create(mstack_t* s, const uint32_t element_size, const uint32_t element_count);

uint8_t stack_reinit(mstack_t* s);

size_t stack_size(mstack_t* s);

uint8_t  stack_push_back(mstack_t* s, const void* new_element);
void*    stack_pop_back(mstack_t* s);
void*    mstack_top(mstack_t* s);

void*    stack_element_at(mstack_t* s, const uint32_t idx);


void stack_destroy(mstack_t* s);