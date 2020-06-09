

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "uv_proc.h"
#include "json_tool.h"


extern char* input_json_msg_handler(const char* json_str);


static uv_tcp_t server;


//////////////////////////////////////////////////////////////////////////

typedef struct client_ctx_t {
    uv_tcp_t handle;
    size_t   data_len;
    char* data;
} client_ctx_t;



client_ctx_t* init_client_ctx()
{
    client_ctx_t* r_code = malloc(sizeof(client_ctx_t));
    r_code->data_len = 0;
    r_code->data = NULL;
    return r_code;
}


void destroy_client_ctx(client_ctx_t* c)
{
    if (c) {
        c->data_len = 0;
        free(c->data);
        c->data = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) 
{
    buf->base = (char*)malloc(suggested_size);
    buf->len = suggested_size;
}


void on_write_complete(uv_write_t *req, int status) 
{
    if (status) {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    free(req);
}


void on_close_complete(uv_handle_t *client)
{
    printf("on_close_complete\r\n");
}


void on_read_complete(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    if (nread < 0) {

        destroy_client_ctx((client_ctx_t*)client);

        if (nread != UV_EOF) {
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
            uv_close((uv_handle_t*) client, NULL);
        }

    } else if (nread > 0) {

        client_ctx_t* c = (client_ctx_t*)client;

		{
			c->data_len += nread;
			c->data = (char*)realloc(c->data, c->data_len+1);
			memcpy(c->data+c->data_len-nread, buf->base, nread);
            c->data[c->data_len] = 0;

            printf("rcv: %s\r\n", c->data);

            if (is_valid_json(c->data)) {

                char* result_str = input_json_msg_handler(c->data);

                if (result_str) {
                    uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));
                    uv_buf_t wrbuf = uv_buf_init(result_str, strlen(result_str));
                    uv_write(req, client, &wrbuf, 1, on_write_complete);
                    free(result_str);
                }

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
                              
    client_ctx_t *client_ctx = init_client_ctx();

    if (client_ctx) {
        uv_tcp_init(uv_default_loop(), &client_ctx->handle);

        if (uv_accept(server, (uv_stream_t*)&client_ctx->handle) == 0) {
    		uv_tcp_keepalive(&client_ctx->handle, 1, 50);
            uv_read_start((uv_stream_t*)&client_ctx->handle, alloc_buffer, on_read_complete);
        } else {
            uv_close((uv_handle_t*) &client_ctx->handle, NULL);
        }
    }
}


uint8_t start_uv_tcp_server(const uint16_t server_port)
{
	struct sockaddr_in addr;

	uv_tcp_init(uv_default_loop(), &server);
	uv_ip4_addr("0.0.0.0", server_port, &addr);

	uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
	int r = uv_listen((uv_stream_t*)&server, 128, on_new_connection);
	if (r) {
		fprintf(stderr, "Listen error %s\n", uv_strerror(r));
		return 1;
	}
	printf("server start [%u]\r\n", server_port);
	return 0;
}
