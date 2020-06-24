
#pragma once

#include <stdint.h>


typedef void (*calc_result_cb_t)(char* result_str);

uint32_t send_to_calc(char* req, calc_result_cb_t p_fn);