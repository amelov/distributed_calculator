
#pragma once

#include <uv.h>


//void on_read_complete(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);

//void on_new_connection(uv_stream_t *server, int status);



uint8_t start_uv_tcp_server(uv_loop_t *loop, const uint16_t server_port);