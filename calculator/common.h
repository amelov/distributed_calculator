
#pragma once


#include "mstack.h"

//#define MAKE_TEST

#define SERVER_PORT 7000

#define INIT_QUEUE_COUNT	16
#define INIT_STACK_COUNT	32

#define INIT_VARIABLE_COUNT	32

#define MAX_NUM_LENGTH (10+1)

#define MAX_NUM_T_TO_STR_LEN 20		/* strlen( itoa(NUM_t) )*/

typedef long long	NUM_t;

// Parse item:
enum item_type_t {
	UNDEF_T,
	NUMBER_T,
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
	size_t len;	
} item_t;



// Context for calculation

typedef struct {

	// for RPN converters, result store in <queue>
	mstack_t stack;
	mstack_t queue;

	char* err_str;
} ctx_t;
