

#include "mlist.h"
#include <string.h>
#include <memory.h>
#include <malloc.h>
#include <assert.h>


void list_create(list_t* p_list)
{
	memset(p_list, 0, sizeof(*p_list));
}


void list_add(list_t* p_list, void* data, const size_t data_len)
{
	list_t* p = p_list;

	while (p->next) {
		p = p->next;
	}

	p->next = (list_t*)malloc( sizeof(list_t) + data_len - 1);
	assert(p->next);	
	if (p->next) {

		p_list->sz++;	// header size

		//new_p->prev = p;
		p->next->next = NULL;
		p->next->sz = data_len;
		memcpy(p->next->data, data, data_len);
	}
}


void list_del(list_t* p_list, void* data, const size_t data_len)
{
 	if ( p_list && (p_list->sz) && data) {
 		list_t* p = p_list;
 		while (p->next) {
 			if ( (p->next->sz==data_len) && (!memcmp(p->next->data, data, data_len)) ) {

 				list_t* d = p->next;
 				p->next = p->next->next;
 
 				free(d);	// list_t + data

				p_list->sz--;	// header size

 				break;
 			}
 			p = p->next;
 		}
 	}
}


list_t* list_head(list_t* p_list)
{
	if (p_list) {
		return p_list->next;
	}
	return NULL;
}


list_t* list_next(list_t* p_list)
{
	if (p_list) {
		return p_list->next;
	}
	return NULL;
}


void* list_data(list_t* p_list)
{
	if (p_list) {
		return p_list->data;
	}
	return NULL;
}


size_t list_size(list_t* p_list)
{
	if (p_list) {
		return p_list->sz;
	}
	return 0;
}


void list_destroy(list_t* p_list)
{
	list_t* p = p_list;
	while (p->next) {

		list_t* d = p;
		p = p->next;

		free(d);
	}
}