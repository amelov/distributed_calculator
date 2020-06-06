/* 
Based on:	http://rosettacode.org/wiki/Parsing/Shunting-yard_algorithm#C

operator 	precedence 	associativity 	operation
^ 			4 			right 			exponentiation
* 			3 			left 			multiplication
/ 			3 			left 			division
+ 			2 			left 			addition
- 			2 			left 			subtraction 

*/

#include <memory.h>
#include <malloc.h>

#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////




#include "common.h"



//////////////////////////////////////////////////////////////////////////

static uint8_t ismathoperation(const char c)
{
	return (c=='+') || (c=='-') || (c=='*') || (c=='/') || (c=='^');
}


static int parse_item(char **p_s, item_t* i)
{
	char* s = *p_s;

	while (*s==' ') s++;

	i->t = UNDEF_T;
	i->len = 0;
	i->p = s;

	if (!*s) return 0;

	if (isdigit(*s)) {

		i->t = NUM_T;		
		while ( (*s) && isalnum(*s) ) {
			i->len++;
			s++;
		}
		*p_s = s;
		return 1;

	} else if (isalpha(*s)) {

		i->t = VARIABLE_T;
		while ( (*s) && (*s!=' ') && (!ismathoperation(*s)) ) {
			i->len++;
			s++;
		}
		*p_s = s;
		return 1;		

	} else {

		switch (*s) {
		case '+':
		case '-':
			i->t = OPERATION_T;
			i->precedence = 2;
			i->right_associative = 1;
			break;

		case '*':
		case '/':
			i->t = OPERATION_T;
			i->precedence = 3;
			i->right_associative = 0;
			break;


		case '^':
			i->t = OPERATION_T;
			i->precedence = 4;
			i->right_associative = 1;
			break;

		case '(':	i->t = OPEN_BRACE_T;	break;
		case ')':	i->t = CLOSE_BRACE_T;	break;
		}


		if (i->t != UNDEF_T) {
			i->len = 1;
			s++;
			*p_s = s;
			return 1;
		}

	}
	return 0;
}



//#define output_push(x)	 ctx->queue[ ctx->queue_sz++ ] = &x

void output_push(ctx_t* ctx, item_t* x) 
{
	if (ctx->queue_count>=ctx->max_queue_sz) {
		ctx->max_queue_sz *= 2;
		ctx->queue = (item_t*)realloc(ctx->queue, ctx->max_queue_sz * sizeof(*ctx->queue) );
		if (!ctx->queue) {
			ctx->err_str = "not enough memory";
			return;
			//return 2;	
		}
	}
	memcpy(&ctx->queue[ ctx->queue_count++ ], x, sizeof(*x));
}


#define stack_sz()		 ctx->stack_count

//#define stack_push(x)	 ctx->stack[ ctx->stack_count++ ] = x
void stack_push(ctx_t* ctx, item_t* x)
{
	if (ctx->stack_count>=ctx->max_stack_sz) {
		ctx->max_stack_sz *= 2;
		ctx->stack = (item_t*)realloc(ctx->stack, ctx->max_stack_sz * sizeof(*ctx->stack) );
		if (!ctx->stack) {
			ctx->err_str = "not enough memory";
			return;
			//return 2;	
		}
	}
	memcpy(&ctx->stack[ ctx->stack_count++ ], x, sizeof(*x));
}


#define stack_pop()		 ctx->stack[ --ctx->stack_count ]
#define stack_top()		(ctx->stack[ctx->stack_count-1])

//////////////////////////////////////////////////////////////////////////

void PRINT(ctx_t* ctx)
{
//	int i, j;
	printf("\r\nstack: ");

// 	for (i=0; i<ctx->stack_count; i++) {
// 		if (i) printf(",");
// 		printf("%c ", ctx->stack[i].p[0]);
// 	}
// 
// 	printf("\r\nqueue: ");
// 	for (i=0; i<ctx->queue_count; i++) {
// 		if (i) printf(" ");
// 		for (j=0; j<ctx->queue[i].len; j++) {
// 			printf("%c", ctx->queue[i].p[j]);
// 		}
// 	}
	printf("\r\n");
}


//////////////////////////////////////////////////////////////////////////

static uint8_t round_ctx(ctx_t* ctx)
{
	ctx->queue_count = ctx->stack_count = 0;
	ctx->err_str = NULL;
	return 0;
}


static uint8_t init_ctx(ctx_t* ctx)
{
	ctx->item_count = INIT_ELEMENT_COUNT;
	ctx->item = (item_t*)malloc(ctx->item_count * sizeof(*ctx->item) );
	if (!ctx->item) {
		return 1;
	}

	ctx->max_queue_sz = INIT_STACK_COUNT;
	ctx->queue = (item_t*)malloc(ctx->max_queue_sz * sizeof(*ctx->queue) );
	if (!ctx->queue) {
		return 2;
	}

	ctx->max_stack_sz = INIT_STACK_COUNT;
	ctx->stack = (item_t*)malloc(ctx->max_stack_sz * sizeof(*ctx->stack) );
	if (!ctx->stack) {
		return 3;
	}

	return round_ctx(ctx);
}


static uint8_t check_ctx(ctx_t* ctx, const uint32_t new_item_count)
{
	ctx->err_str = 0;
	if (new_item_count>=ctx->item_count) {
		ctx->item_count *= 2;
		ctx->item = (item_t*)realloc(ctx->item, ctx->item_count * sizeof(*ctx->item) );
		if (!ctx->item) {
			return 1;
		}
	}

// 	if ( ctx->queue_sz+1 >= ctx->max_queue_sz ) {
// 		ctx->max_queue_sz *= 2;
// 		ctx->queue = (item_t**)realloc(ctx->queue, ctx->max_queue_sz * sizeof(*ctx->queue) );
// 		if (!ctx->queue) {
// 			return 2;
// 		}
// 	}
// 
// 	if ( ctx->stack_sz+1 >= ctx->max_stack_sz ) {
// 		ctx->max_stack_sz *= 2;
// 		ctx->stack = (item_t**)realloc(ctx->stack, ctx->max_stack_sz * sizeof(*ctx->stack) );
// 		if (!ctx->stack) {
// 			return 3;
// 		}	
// 	}
	return 0;
}


static void deinit_ctx(ctx_t* ctx)
{
	free(ctx->item);
	free(ctx->queue);
	free(ctx->stack);
}


//////////////////////////////////////////////////////////////////////////

uint8_t convert_to_RPN(ctx_t* ctx, char* s)
{
	uint32_t i = 0;
	

	while (*s) {

		if (parse_item(&s, &ctx->item[i])) {

			switch (ctx->item[i].t) {
			case NUM_T:
			case VARIABLE_T:
				output_push(ctx, &(ctx->item[i]));
				PRINT(ctx);
				break;

			case OPERATION_T:
				{
					while ( (stack_sz()>0) /*&& operators.TryGetValue(stack.Peek(), out var op2)*/) {
						int c = (stack_top().precedence < ctx->item[i].precedence);
						if ( (c<0) || !ctx->item[i].right_associative && (c<=0) ) {
							output_push(ctx, &stack_pop() );
						} else {
							break;
						}
					}
					stack_push(ctx, &ctx->item[i]);
					PRINT(ctx);
				}
				break;

			case OPEN_BRACE_T:
				stack_push(ctx, &ctx->item[i]);
				PRINT(ctx);
				break;

			case CLOSE_BRACE_T:
				{
					while (stack_sz()>0) {
						if (stack_top().t != OPEN_BRACE_T) {
							output_push(ctx, &stack_pop() );
						} else {
							stack_pop();
							break;
						}
					}
					PRINT(ctx);
				}
				//if (top != "(") throw new ArgumentException("No matching left parenthesis.");
				break;
			}
		}

		if (check_ctx(ctx, ++i)) {
			ctx->err_str = "not enough memory";
			break;
		}
	}

	while (stack_sz()>0) {
		if (stack_top().t != OPERATION_T) { 
			ctx->err_str = "No matching right parenthesis."; 
			break;
		}
		output_push(ctx, &stack_pop() );
	}

	ctx->item[i].t = UNDEF_T;

	PRINT(ctx);

	return (ctx->err_str != NULL);
}

//////////////////////////////////////////////////////////////////////////


int main()
{
	item_t test[100];
	
	
	uint32_t i, j;

	ctx_t ctx;

	char* input_s = "12345678901234567890+ x123 * 2 / ( 1 - 5 ) ^ 2 ^ 3";

//	char input_s[1024];//[2*1024];	
// 	for (i=0; i<sizeof(input_s)-1; ++i) {
// 		input_s[i] = (i%2)? '+' : '1';
// 	}
// 	input_s[sizeof(input_s)-1] = 0;


	if (!init_ctx(&ctx)) {

		if (!round_ctx(&ctx)) {

			if (!convert_to_RPN(&ctx, input_s)) {

				printf("\r\nConvert OK:\r\n \"%s\"", input_s);

				printf("\r\nRESULT:");
				for (i=0; i<ctx.queue_count; i++) {
					if (i) printf(" ");
					for (j=0; j<ctx.queue[i].len; j++) {
						printf("%c", ctx.queue[i].p[j]);
					}
				}

			}
		}
	}
	
	//memcpy(test, ctx.item, min(sizeof(test)/sizeof(test[0]), ctx.item_count*sizeof(item_t)) );

	deinit_ctx(&ctx);

	return 0;
}