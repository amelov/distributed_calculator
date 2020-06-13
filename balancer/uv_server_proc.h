
#pragma once

#include <uv.h>
#include "uv_calc_client.h"


uint8_t start_uv_tcp_server(const uint16_t server_port);

void send_to_user(client_descr_t* c, char* out_msg);