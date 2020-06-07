
#include "stack.h"

#include <memory.h>
#include <malloc.h>
#include <stdio.h>


stack_t zero_stack = {0, 0, 0, NULL};



uint8_t stack_create(stack_t* s, const uint32_t element_size, const uint32_t element_count)
{
	if ( s && element_size && element_count) {

		s->_offset = 0;
		s->_max_element_count = element_count;
		s->_element_size = element_size;
		s->_data_ptr = malloc(element_size * element_count);
		
		if (s->_data_ptr) {
			return stack_reinit(s);
		}
		return 2;
	}
	return 1;
}


uint8_t stack_reinit(stack_t* s)
{
	s->_offset = 0;
	return 0;
}


size_t stack_size(stack_t* s)
{
	return s->_offset;
}


uint8_t stack_push_back(stack_t* s, const void* new_element)
{
	if (s->_offset >= s->_max_element_count) {
		s->_max_element_count *= 2;
		s->_data_ptr = realloc(s->_data_ptr, s->_max_element_count * s->_element_size );
		if (!s->_data_ptr) {
			return 1;
		}
	}
	memcpy((uint8_t*)s->_data_ptr + s->_element_size * s->_offset, new_element, s->_element_size);
	s->_offset++;
	return 0;
}


void* stack_pop_back(stack_t* s)
{
	if (s->_offset) {
		s->_offset--;
		return (uint8_t*)s->_data_ptr + s->_element_size * s->_offset;
	}
	return NULL;
}


void* stack_top(stack_t* s)
{
	if (s->_offset) {
		return (uint8_t*)s->_data_ptr + s->_element_size * (s->_offset-1);
	}
	return NULL;
}


void* stack_element_at(stack_t* s, const uint32_t idx)
{
	if (idx < s->_offset ) {
		return (uint8_t*)s->_data_ptr + idx * s->_element_size;
	}
	return NULL;
}


void stack_destroy(stack_t* s)
{
	free(s->_data_ptr);
	s->_data_ptr = NULL;

	s->_element_size = s->_max_element_count = s->_offset = 0;
}