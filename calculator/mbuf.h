
#pragma once

#include <stdio.h>

typedef struct buf_t {
        char*  data;
        size_t data_sz;
} buf_t;



void buf_create(buf_t* b);
buf_t* buf_make();

void buf_add(buf_t* b, const char* data, const size_t data_len);

void buf_skip_first_byte(buf_t* b, const size_t skip_count);

void buf_destroy(buf_t* b);