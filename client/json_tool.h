
#pragma once

#include <stdint.h>
#include "../tools/mstack.h"
#include "common.h"


char* dc_client_json_create(mstack_t* var_ctx, mstack_t* expressions_ctx);

uint32_t dc_client_json_result_parse(char* in_str, mstack_t* var_ctx, mstack_t* exp_ctx);