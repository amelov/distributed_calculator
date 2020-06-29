/*
	Convert to Reverse Polish notation
	Based on:	http://rosettacode.org/wiki/Parsing/Shunting-yard_algorithm#C

 */

#pragma once

#include <stdint.h>
#include "common.h"

uint8_t dc_calc_rpn_ctx_convert(ctx_t* ctx, char* s);


#ifdef MAKE_TEST
	uint8_t dc_calc_convert_to_rpn_TEST();
#endif