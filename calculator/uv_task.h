
#pragma once

#include <uv.h>
#include <stdint.h>


uint32_t dc_calc_input_json_msg_handler(uv_stream_t *client, const char* json_str);
