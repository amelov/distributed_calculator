
#include "stack.h"

#include <memory.h>
#include <malloc.h>
#include <stdio.h>



uint8_t stack_init(stack_t* s, const uint32_t element_size, const uint32_t element_count)
{
	if ( s && element_size && element_count) {
		s->_max_element_count = element_count;
		s->_element_size = element_size;
		s->_data_ptr = malloc(element_size * element_count);
		if (s->_data_ptr) {
			return 0;
		}
		return 2;
	}
	return 1;
}


uint8_t stack_reinit(stack_t* s)
{
	s->_offset = 0;
}


uint32_t stack_size(stack_t* s)
{
	return s->_offset;
}


void stack_push_back(stack_t* s, const void* new_element)
{
	if (s->_offset >= s->_max_element_count) {
		s->_max_element_count *= 2;
		s->_data_ptr = realloc(s->_data_ptr, s->_max_element_count * s->_element_size );
		if (!s->_data_ptr) {
			return;
		}
	}
	memcpy((uint8_t*)s->_data_ptr + s->_element_size * s->_offset, new_element, s->_element_size);
	s->_offset++;
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
