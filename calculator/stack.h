

#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef struct stack_t {
	size_t _element_size;
	size_t _max_element_count;
	size_t _offset;	// == element count
	void* _data_ptr;
} stack_t;

extern stack_t zero_stack;

//////////////////////////////////////////////////////////////////////////

uint8_t stack_create(stack_t* s, const uint32_t element_size, const uint32_t element_count);

uint8_t stack_reinit(stack_t* s);

size_t stack_size(stack_t* s);

uint8_t  stack_push_back(stack_t* s, const void* new_element);
void*    stack_pop_back(stack_t* s);
void*    stack_top(stack_t* s);

void*    stack_element_at(stack_t* s, const uint32_t idx);


void stack_destroy(stack_t* s);