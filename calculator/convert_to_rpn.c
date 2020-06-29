/* 

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


#include "common.h"

#ifdef MAKE_TEST
	#include "context.h"
	#include <string.h>
#endif


//#define DEBUG_PRINTF
static void PRINT(char* s, ctx_t* ctx);




//////////////////////////////////////////////////////////////////////////

static uint8_t ismathoperation(const char c)
{
	return (c=='+') || (c=='-') || (c=='*') || (c=='/') || (c=='^') || (c=='(') || (c==')');
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

		i->t = NUMBER_T;		
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

		i->right_associative = 0;

		switch (*s) {
		case '+':
		case '-':
			i->t = OPERATION_T;
			i->precedence = 2;
			break;

		case '*':
		case '/':
			i->t = OPERATION_T;
			i->precedence = 3;
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



//////////////////////////////////////////////////////////////////////////

#define out_push(x) stack_push_back(&ctx->queue, x)
#define out_sz()    stack_size(&ctx->queue)


#define stack_push(x)	stack_push_back(&ctx->stack, x)
#define stack_pop()		stack_pop_back(&ctx->stack)
#define stack_peek()		((item_t*)mstack_top(&ctx->stack))
#define stack_sz()		stack_size(&ctx->stack)

//////////////////////////////////////////////////////////////////////////



uint8_t dc_calc_convert_to_rpn(ctx_t* ctx, char* s)
{
	item_t temp_item;

	PRINT(s, ctx);

	while (*s) {

		if (parse_item(&s, &temp_item)) {

			switch (temp_item.t) {
			case NUMBER_T:
			case VARIABLE_T:
				if ( out_push(&temp_item) ) {
					ctx->err_str = "not enough memory";
				}
				PRINT(s, ctx);
				break;

			case OPERATION_T:
				{
					while ( (stack_sz()>0) && (stack_peek()->t == OPERATION_T) ) {

						int c = temp_item.precedence - stack_peek()->precedence;

						if ( ((c <0) || (!temp_item.right_associative)) && (c<=0)  ) {
							out_push( stack_pop() );
						} else {
							break;
						}
						PRINT(s, ctx);
					}
					stack_push(&temp_item);
					PRINT(s, ctx);
				}
				break;

			case OPEN_BRACE_T:
				stack_push(&temp_item);
				PRINT(s, ctx);
				break;

			case CLOSE_BRACE_T:
				{
					while (stack_sz()) {
						if ( stack_peek()->t != OPEN_BRACE_T) {
							out_push( stack_pop() );
						} else {
							stack_pop();
							break;
						}
					}
					PRINT(s, ctx);
				}
				// TODO: if (top != "(") throw new ArgumentException("No matching left parenthesis.");
				break;
			case UNDEF_T:
				ctx->err_str = "Parse error!";
				return 1;
				break;
			}
		}
	}

	while (stack_sz()>0) {
		if ( stack_peek()->t != OPERATION_T ) { 
			ctx->err_str = "No matching right parenthesis."; 
			break;
		}
		out_push( stack_pop() );
	}

	PRINT(s, ctx);

	return (ctx->err_str != NULL);
}



//////////////////////////////////////////////////////////////////////////


void PRINT(char* s, ctx_t* ctx)
{
#ifdef DEBUG_PRINTF
	item_t* p;
	uint32_t i, j;

	printf("\r\ntext : %s:", s);
	printf("\r\nstack: ");

	for (i=0; i<stack_sz(); i++) {
		//if (i) printf(" ");
		printf("%c ", ((item_t*)stack_element_at(&ctx->stack, i))->p[0] );
	}

	printf("\r\nqueue: ");
	for (i=0; i<stack_size(&ctx->queue); i++) {
		if (i) printf(" ");
		p = stack_element_at(&ctx->queue, i);
		for (j=0; j<p->len; j++) {
			printf("%c", p->p[j]);
		}
	}
	printf("\r\n");
#endif
}

#ifdef MAKE_TEST

void get_result_string(ctx_t* c, mstack_t* out_stack)
{
	size_t i, j;
	char s_eol = 0;
	char s_separator = ' ';

	stack_create(out_stack, sizeof(uint8_t), stack_size(&c->queue));
	for (i=0; i<stack_size(&c->queue); ++i) {

		item_t* p = (item_t*)stack_element_at(&c->queue, i);

		if (i) {
			stack_push_back(out_stack, &s_separator);
		}
		
		for (j=0; j<p->len; ++j) {
			stack_push_back(out_stack, &p->p[j]);
		}
		
	}
	stack_push_back(out_stack, &s_eol);
}


uint8_t dc_calc_convert_to_rpn_TEST()
{
	uint32_t r_code = 0;
	ctx_t ctx;
	mstack_t res_stack;

	struct {
		char* in_str;
		char* out_str;
	} test_case[] = { 

			{"1+2*3", "1 2 3 * +"},
			{"(1+2)*(3-4)", "1 2 + 3 4 - *"},
			{"1+(2*(3-4))", "1 2 3 4 - * +"},
			{"3 * x - z + 2 * y", "3 x * z - 2 y * +"},
			{"3 + 4 * 2 / ( 1 - 5 ) ^ 2 ^ 3", "3 4 2 * 1 5 - 2 3 ^ ^ / +"}, 
			
			{0, 0}
	};
	
	uint32_t tid = 0;
	while (test_case[tid].in_str) {
		init_ctx(&ctx);
		if (!convert_to_rpn(&ctx, test_case[tid].in_str)) {

			get_result_string(&ctx, &res_stack);

			if ( stack_size(&res_stack) ) {

				if (0==strcmp(test_case[tid].out_str, (char*)stack_element_at(&res_stack, 0))) {
					//printf("convert_to_rpn_TEST(#%d): ok\r\n", tid);
					r_code++;
				} else {
					printf("convert_to_rpn_TEST(#%d): ERROR wait(%s) != result(%s)\r\n", tid, test_case[tid].out_str, (char*)stack_element_at(&res_stack, 0));
				}
			}

			stack_destroy(&res_stack);
		} else {
			printf("convert error: %s\r\n", ctx.err_str);
		}
		deinit_ctx(&ctx);
		++tid;
	}	

	printf("convert_to_rpn_TEST() -> %s\r\n", (!r_code)||(r_code!=tid)? "ERROR" : "ok");
	return (r_code != tid);
}

#endif