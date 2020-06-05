
#pragma once



#define INIT_ELEMENT_COUNT  	256

#define INIT_STACK_COUNT	16


// Parse item:

enum item_type_t {
	UNDEF_T,
	NUM_T,
	OPERATION_T,
	VARIABLE_T,
	OPEN_BRACE_T,
	CLOSE_BRACE_T,
};


typedef struct {
	uint8_t/*item_type_t*/ t;
	int8_t precedence;
	uint8_t right_associative;

	char* p;
	uint32_t len;	
} item_t;



// Context for calculation

typedef struct {
	item_t*  item;
	uint32_t item_count;

	uint32_t max_stack_sz;
	item_t*  stack;
	uint32_t stack_count;

	uint32_t max_queue_sz;
	item_t*  queue;
	uint32_t queue_count;

	char* err_str;
} ctx_t;
