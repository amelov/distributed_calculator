/*
		
 */

#pragma once

#include "common.h"
#include "variable.h"


uint8_t dc_calc_calculate(char* input_str, var_store_t* var_store, NUM_t* res, char** err_str);


#ifdef MAKE_TEST
	uint8_t dc_calc_calculate_TEST();
#endif