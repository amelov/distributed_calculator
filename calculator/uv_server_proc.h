
#pragma once

#include <uv.h>


uint8_t start_uv_tcp_server(const uint16_t server_port);


void send_data_to_client(uv_stream_t *client, char* data);