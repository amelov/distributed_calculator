
#pragma once

#include <stdint.h>


typedef void (*dc_client_calculation_result_cb_t)(char* result_str);

uint32_t dc_client_send_calculation_job(char* req, dc_client_calculation_result_cb_t p_fn);