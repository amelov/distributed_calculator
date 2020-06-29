

#include "uv_server_proc.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "common.h"
#include "json_tool.h"
#include "uv_task.h"

#include "../tools/json_common.h"
#include "../tools/mlist.h"
#include "../tools/mbuf.h"

static uv_tcp_t server;


//////////////////////////////////////////////////////////////////////////

// uv_tcp_t.data = 
typedef struct socket_ctx_t {
	buf_t rx;
	uint32_t id;
} socket_ctx_t;


static socket_ctx_t* dc_calc_init_client_ctx()
{
	static uint32_t _client_id = 0;

	socket_ctx_t* c_ctx = (socket_ctx_t*)calloc(1, sizeof(socket_ctx_t));
	buf_create(&c_ctx->rx);
	c_ctx->id = _client_id++;
	return c_ctx;
}


static void dc_calc_destroy_client_ctx(socket_ctx_t* c)
{
	if (c) {
		buf_destroy(&c->rx);
		free(c);
	}
}

//////////////////////////////////////////////////////////////////////////

static void dc_calc_on_alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) 
{
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
}


static void dc_calc_on_close_complete(uv_handle_t *client)
{
	socket_ctx_t* c = (socket_ctx_t*)client->data;
	printf("cli[%d]:  dc_calc_on_close_complete\r\n", c->id);
	
	dc_calc_destroy_client_ctx((socket_ctx_t*)client->data);
	free(client);
}


static void dc_calc_on_write_complete(uv_write_t *req, int status) 
{
	if (status) {
		fprintf(stderr, "Write error %s\n", uv_strerror(status));
	}
	free(req);
}


static void dc_calc_on_read_complete(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
	socket_ctx_t* c = (socket_ctx_t*)client->data;

	if (nread < 0) {
		
		if (nread != UV_EOF) {
			fprintf(stderr, "cli[%d]: Read error %s\n", c->id, uv_err_name(nread));
			uv_close((uv_handle_t*) client, dc_calc_on_close_complete);
		}

	} else if (nread > 0) {

		if (c) {

			buf_add(&c->rx, buf->base, nread);

			char* input_msg = NULL;
			char* b_i = c->rx.data;

			while ( NULL != (input_msg = get_msg_from_stream(&b_i, MESSAGE_DELIMITER)) ) {

				printf("\r\ncli[%d]: find msg -> [%s]\r\n", c->id, input_msg);

				dc_calc_input_json_msg_handler(client, input_msg);
				free(input_msg);
			}

			if (b_i!=c->rx.data) {
				buf_skip_first_byte(&c->rx, (b_i-c->rx.data));
			}			
		
		}
	}

	if (buf->base) {
		free(buf->base);
	}
}


static void dc_calc_on_new_connection(uv_stream_t *server, int status) 
{
	if (status < 0) {
		fprintf(stderr, "New connection error %s\n", uv_strerror(status));
		return;
	}
                              
	uv_tcp_t* client = malloc(sizeof(uv_tcp_t)); 
	assert(client);
	client->data = dc_calc_init_client_ctx();

	uv_tcp_init(uv_default_loop(), client);

	if (uv_accept(server, (uv_stream_t*)client) == 0) {
		uv_tcp_keepalive(client, 1, 50);
		uv_read_start((uv_stream_t*)client, dc_calc_on_alloc_buffer, dc_calc_on_read_complete);
	} else {
		uv_close((uv_handle_t*)client, dc_calc_on_close_complete);
	}
}

////////////////////////////////////////////////////////////////////

uint8_t dc_calc_start_tcp_server(const uint16_t server_port)
{
	struct sockaddr_in addr;

	uv_tcp_init(uv_default_loop(), &server);
	uv_ip4_addr("0.0.0.0", server_port, &addr);

	uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
	int r = uv_listen((uv_stream_t*)&server, MAX_CLIENT_COUNT, dc_calc_on_new_connection);
	if (r) {
		fprintf(stderr, "Listen error %s\n", uv_strerror(r));
		return 1;
	}
	printf("server start [:%u]\r\n", server_port);
	return 0;
}



void dc_calc_send_data_to_client(uv_stream_t *client, char* data)
{
	if ( uv_is_active((uv_handle_t*)client) && data && strlen(data) ) {
		uv_write_t *req = (uv_write_t *)malloc(sizeof(uv_write_t));
		req->data = client;

		socket_ctx_t* s_ctx = client->data;

		printf("\r\ncli[%d]: result: \"%s\"\r\n", s_ctx->id, data);

		uv_buf_t wrbuf[2];
		wrbuf[0] = uv_buf_init(data, strlen(data));
		wrbuf[1] = uv_buf_init(MESSAGE_DELIMITER, strlen(MESSAGE_DELIMITER));
		uv_write(req, client, wrbuf, 2, dc_calc_on_write_complete);
	}
}