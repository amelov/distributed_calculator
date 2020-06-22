

#include "uv_server_proc.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

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
} socket_ctx_t;


socket_ctx_t* init_client_ctx()
{
	socket_ctx_t* r_code = (socket_ctx_t*)calloc(1, sizeof(socket_ctx_t));
	buf_create(&r_code->rx);
	return r_code;
}


void destroy_client_ctx(socket_ctx_t* c)
{
	if (c) {
		buf_destroy(&c->rx);
		free(c);
	}
}

//////////////////////////////////////////////////////////////////////////

void on_alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) 
{
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
}


void on_close_complete(uv_handle_t *client)
{
	printf("on_close_complete\r\n");
	destroy_client_ctx((socket_ctx_t*)client->data);
	free(client);
}


void on_write_complete(uv_write_t *req, int status) 
{
	if (status) {
		fprintf(stderr, "Write error %s\n", uv_strerror(status));
	}
	free(req);

//	uv_stream_t* c = (uv_stream_t *)req->data;
//	printf("on_write_complete -> %d", status);
//	uv_read_stop(c);
//	if (c) {
//		uv_close((uv_handle_t*)c, on_close_complete);
//	}
}


void on_read_complete(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
	if (nread < 0) {
		
		if (nread != UV_EOF) {
			fprintf(stderr, "Read error %s\n", uv_err_name(nread));
			uv_close((uv_handle_t*) client, on_close_complete);
		}

	} else if (nread > 0) {

		socket_ctx_t* c = (socket_ctx_t*)client->data;

		if (c) {

			buf_add(&c->rx, buf->base, nread);
			//printf("rcv_buf[%ld]: [%s]\r\n", c->rx.data_sz, c->rx.data);

			char* input_msg = NULL;
			char* b_i = c->rx.data;

			while ( NULL != (input_msg = get_msg_from_stream(&b_i, MESSAGE_DELIMITER)) ) {

				printf("\r\nfind msg -> [%s]\r\n", input_msg);

				input_json_msg_handler(client, input_msg);
				free(input_msg);
			}

			if (b_i!=c->rx.data) {
				buf_skip_first_byte(&c->rx, (b_i-c->rx.data));
				//printf("new rcv_buf[%ld]: [%s]\r\n", c->rx.data_sz, c->rx.data);
			}			
		
		}
	}

	if (buf->base) {
		free(buf->base);
	}
}


void on_new_connection(uv_stream_t *server, int status) 
{
	if (status < 0) {
		fprintf(stderr, "New connection error %s\n", uv_strerror(status));
		return;
	}
                              
	uv_tcp_t* client = malloc(sizeof(uv_tcp_t)); 
	client->data = init_client_ctx();

	uv_tcp_init(uv_default_loop(), client);

	if (uv_accept(server, (uv_stream_t*)client) == 0) {
		uv_tcp_keepalive(client, 1, 50);
		uv_read_start((uv_stream_t*)client, on_alloc_buffer, on_read_complete);
	} else {
		uv_close((uv_handle_t*)client, on_close_complete);
	}
}


uint8_t start_uv_tcp_server(const uint16_t server_port)
{
	struct sockaddr_in addr;

	uv_tcp_init(uv_default_loop(), &server);
	uv_ip4_addr("0.0.0.0", server_port, &addr);

	uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
	int r = uv_listen((uv_stream_t*)&server, MAX_CLIENT_COUNT, on_new_connection);
	if (r) {
		fprintf(stderr, "Listen error %s\n", uv_strerror(r));
		return 1;
	}
	printf("server start [:%u]\r\n", server_port);
	return 0;
}



void send_data_to_client(uv_stream_t *client, char* data)
{
	if ( uv_is_active((uv_handle_t*)client) && data && strlen(data) ) {
		uv_write_t *req = (uv_write_t *)malloc(sizeof(uv_write_t));
		req->data = client;
		printf("\r\nsend_data_to_client():\"%s\"\r\n", data);

		uv_buf_t wrbuf[2];
		wrbuf[0] = uv_buf_init(data, strlen(data));
		wrbuf[1] = uv_buf_init(MESSAGE_DELIMITER, strlen(MESSAGE_DELIMITER));
		uv_write(req, client, wrbuf, 2, on_write_complete);
	}
}