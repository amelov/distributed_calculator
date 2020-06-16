
#pragma once

#include <stdint.h>
#include <stdio.h>


typedef struct node {
	//struct node* prev;
	struct node* next;

	size_t sz;
	uint8_t data[1];
} list_t;



void list_create(list_t* p_list);

void list_add(list_t* p_list, void* data, const size_t data_len);
void list_del(list_t* p_list, void* data, const size_t data_len);

list_t* list_head(list_t* p_list);
list_t* list_next(list_t* p_list);

size_t list_size(list_t* p_list);


void list_destroy(list_t* p_list);