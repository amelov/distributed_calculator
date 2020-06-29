

#include "common.h"
#include "calculate.h"
#include "convert_to_rpn.h"
#include "context.h"

#include <memory.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>


#ifdef MAKE_TEST
	#include <string.h>
#endif


//#define DEBUG_PRINTF
//static void dc_calc_PRINT(mstack_t* s);


static char* dc_calc_get_string(item_t* p)
{
	char* r_code = (char*)malloc(p->len+1);
	if (r_code) {
		memcpy(r_code, p->p, p->len);
		r_code[ p->len ] = 0;
	}
	return r_code;
}


uint8_t dc_calc_rpn_ctx_calculate(ctx_t* ctx, var_store_t* var_store, NUM_t* result)
{
	NUM_t a, b;
	item_t* temp_item;

	uint32_t i;

	uint8_t r_code = 0;

	mstack_t stack;
	stack_create(&stack, 16, sizeof(NUM_t));

	for (i=0; (r_code==0) && (i<stack_size(&ctx->queue)); ++i){
		temp_item = (item_t*)stack_element_at(&ctx->queue, i);

		if (temp_item->t == NUMBER_T) {
			
			char* s = dc_calc_get_string(temp_item);
			a = atoll(s);	// TODO: ?
			stack_push_back(&stack, &a);
			free(s);

		} else if (temp_item->t == VARIABLE_T) {

			NUM_t v;
			char* s = dc_calc_get_string(temp_item);
			if (dc_calc_var_find(var_store, s, &v)) {
				stack_push_back(&stack, &v);
			}
			free(s);

		} else {

			if (stack_size(&stack)>=2) {
				switch (*temp_item->p) {
				case '+':
					a = *((NUM_t*)stack_pop_back(&stack));
					b = *((NUM_t*)stack_pop_back(&stack));
					a+=b;
					stack_push_back(&stack, &a);
					break;
				case '-':
					a = *((NUM_t*)stack_pop_back(&stack));
					b = *((NUM_t*)stack_pop_back(&stack));
					b-=a;
					stack_push_back(&stack, &b);
					break;
				case '*':
					a = *((NUM_t*)stack_pop_back(&stack));
					b = *((NUM_t*)stack_pop_back(&stack));
					a*=b;
					stack_push_back(&stack, &a);
					break;
				case '/':
					a = *((NUM_t*)stack_pop_back(&stack));
					b = *((NUM_t*)stack_pop_back(&stack));
					if (a) {
						b/=a;
						stack_push_back(&stack, &b);
					} else {
						ctx->err_str = "Division by zero";
						r_code = 2;
					}
					break;
				case '^':
					a = *((NUM_t*)stack_pop_back(&stack));
					b = *((NUM_t*)stack_pop_back(&stack));
					a = (NUM_t)pow(b, a);
					stack_push_back(&stack, &a);
					break;
				}
			} else {
				ctx->err_str = "Unexpected eol";
				r_code = 1;
			}
		}

	}

	if (!r_code) {
		*result =  *((NUM_t*)stack_pop_back(&stack));
	}

	stack_destroy(&stack);
	return r_code;
}


//////////////////////////////////////////////////////////////////////////


uint8_t dc_calc_calculate(char* input_str, var_store_t* var_store, NUM_t* res, char** err_str)
{
	ctx_t ctx;
	
	uint8_t r_code = 0;

	if (!dc_calc_rpn_ctx_init(&ctx)) {
		
		if (!dc_calc_rpn_ctx_convert(&ctx, input_str)) {
		
#ifdef DEBUG_PRINTF
			uint32_t i, j;
			item_t* p_temp_item;

			printf("INPUT: %s\r\n", input_str);

			for (i=0; i<stack_size(&ctx.queue); i++) {
				if (i) printf(" ");
				p_temp_item = (item_t*)stack_element_at(&ctx.queue, i);
				for (j=0; p_temp_item && (j<p_temp_item->len); j++) {
					printf("%c", p_temp_item->p[j]);
				}
			}
			printf("\r\n");
#endif 
			r_code = dc_calc_rpn_ctx_calculate(&ctx, var_store, res);

		} else{
			r_code = 1;
		}
		dc_calc_rpn_ctx_deinit(&ctx);
	}

	if (r_code && err_str) {
		*err_str = ctx.err_str;
	}

	return r_code;
}

//////////////////////////////////////////////////////////////////////////

#ifdef MAKE_TEST


uint8_t dc_calc_calculate_TEST()
{

	uint8_t r_code = 0;
	uint32_t tid = 0;

	uint32_t i;
	uint32_t out_str_sz;
	char* out_str = NULL;

	struct {
		char* in_str;
		NUM_t res;
		char* err;
	} test_case[] = { 
		{"123", 123, ""},
		{"2 ^ 20", 1048576, ""},
		{"(3 + 4)  * 2^2", 28, ""},
		{"3 + 4 * 2 / ( 1 - 5 ) ^ 2 ^ 3", 3, ""},
		{"(((((((1+2+3**(4 + 5))))))", 0, "No matching right parenthesis."},
		{"(1^2)^3", 1, ""},
		{"2 + 2 *", 0, "Unexpected eol"},

		{"3*3+1+2*5", 20, ""},
		{"(3+5)*5", 40, ""},

		{NULL, 0, NULL}
	};


	while (test_case[tid].in_str) {

		NUM_t res = 0;
		char* err_str = "";

		if (calculate(test_case[tid].in_str, NULL, &res, &err_str)) {

			if (!strcmp(test_case[tid].err, err_str)) {
				r_code++;
			} else {
				printf("calculate_TEST(#%u) ERROR\r\n", tid);
				tid++;
				break;
			}
			
		} else {

			uint8_t err_flag = 0;
			
			if (test_case[tid].res != res) {
				printf("calculate_TEST(#%u) ERROR", tid);
				printf(" wait(%d) != result(%d)\r\n", (int32_t)test_case[tid].res, (int32_t)res);	//TODO: ?
				err_flag++;
			}
			if (strcmp(test_case[tid].err, err_str)) {
				printf("calculate_TEST(#%u) ERROR", tid);
				printf(" wait(%s) != result(%s)\r\n", test_case[tid].err, err_str);
				err_flag++;
			}

			if (!err_flag) {
				r_code++;
			}
		}
		tid++;
	}

	{ // Big string test ->

		NUM_t res;
		char* err_str = "";

		out_str_sz = 32*1024*1024;
		out_str = (char*)malloc(out_str_sz);
		for (i=0; i<out_str_sz-1; i++) {
			out_str[i] = (i%2)? '+' : '1';
		}
		out_str[i] = 0;


		if (!calculate(out_str, NULL, &res, &err_str)) {
			if (res == out_str_sz/2) {
				r_code++;
			} else {
				printf("calculate_TEST(#%u) ERROR_1\r\n", tid);
			}
		} else {
			printf("calculate_TEST(#%u) ERROR_2\r\n", tid);
		}

		free(out_str);
		tid++;
	}

	printf("calculate_TEST -> %s\r\n", (!r_code)||(r_code!=tid)? "ERROR" : "ok");
	return (r_code == tid);
}

#endif