
#pragma once

#include <stdint.h>
#include "../tools/mstack.h"
#include "../tools/dc_common.h"


char* dc_client_create_req_json(mstack_t* var_ctx, mstack_t* expressions_ctx);

uint32_t dc_client_parse_result_json(char* in_str, mstack_t* var_ctx, mstack_t* exp_ctx);