

#pragma once

#include <stdint.h>



typedef struct stack_t {
	uint32_t _element_size;
	uint32_t _max_element_count;
	uint32_t _offset;	
	void* _data_ptr;
} stack_t;



uint8_t stack_init(stack_t* s, const uint32_t element_size, const uint32_t element_count);
uint8_t stack_reinit(stack_t* s);

uint32_t stack_size(stack_t* s);

void  stack_push_back(stack_t* s, const void* new_element);
void* stack_pop_back(stack_t* s);
void* stack_top(stack_t* s);
