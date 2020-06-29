
#include "mbuf.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>


void buf_create(buf_t* b)
{
	b->data_sz = 0;
	b->data = NULL;
}

buf_t* buf_make()
{
	buf_t* r_code = malloc(sizeof(*r_code));	
	assert(r_code);
	r_code->data_sz = 0;
	r_code->data = NULL;
	return r_code;
}


void buf_add(buf_t* b, const char* data, const size_t data_len)
{
	b->data_sz += data_len;
	b->data = (char*)realloc(b->data, b->data_sz+1);
	memcpy(b->data + b->data_sz - data_len, data, data_len);
	b->data[b->data_sz] = 0;
}


void buf_skip_first_byte(buf_t* b, const size_t skip_count)
{
	if (b && (b->data_sz>=skip_count)) {
		char* new_buf = NULL;
	    b->data_sz -= skip_count;
		if (b->data_sz) {
			new_buf = (char*)malloc(b->data_sz);
			assert(new_buf);
			memcpy(new_buf, &b->data[skip_count], b->data_sz);
		}
		free(b->data);
		b->data = new_buf;
	}
}


void buf_destroy(buf_t* b)
{
	b->data_sz = 0;
	free(b->data);
}

