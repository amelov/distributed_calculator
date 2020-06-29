
#include "context.h"
#include <stdio.h>


uint8_t dc_calc_init_ctx(ctx_t* ctx)
{
	if (stack_create(&ctx->queue, sizeof(item_t), INIT_STACK_COUNT)) {
		return 1;
	}

	if (stack_create(&ctx->stack, sizeof(item_t), INIT_STACK_COUNT)) {
		return 2;
	}

	return dc_calc_round_ctx(ctx);
}


uint8_t dc_calc_round_ctx(ctx_t* ctx)
{
	stack_reinit(&ctx->queue);
	stack_reinit(&ctx->stack);
	ctx->err_str = NULL;
	return 0;
}


void dc_calc_deinit_ctx(ctx_t* ctx)
{
	stack_destroy(&ctx->queue);
	stack_destroy(&ctx->stack);
}


