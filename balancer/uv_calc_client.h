
#pragma once

#include "configuration.h"


uint8_t start_uv_tcp_client(client_descr_t* c);


void on_reconnect_timer_cb(uv_timer_t* handle);