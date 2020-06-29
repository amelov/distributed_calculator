
#pragma once

#include <uv.h>


uint8_t dc_calc_start_tcp_server(uv_tcp_t* server, const uint16_t server_port);


void dc_calc_send_data_to_client(uv_stream_t *client, char* data);