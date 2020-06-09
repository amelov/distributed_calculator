

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "uv_proc.h"
#include "calculate.h"
#include "json_tool.h"



static uv_tcp_t server;

char* input_json_msg_handler(const char* json_str);
//////////////////////////////////////////////////////////////////////////

typedef struct receive_ctx_t {
    size_t   buf_len;
    char* buf;
} receive_ctx_t;



receive_ctx_t* init_client_ctx()
{
    receive_ctx_t* r_code = (receive_ctx_t*)calloc(1, sizeof(receive_ctx_t));
    r_code->buf_len = 0;
    r_code->buf = NULL;
    return r_code;
}


void destroy_client_ctx(receive_ctx_t* c)
{
    if (c) {
        c->buf_len = 0;
        free(c->buf);
        c->buf = NULL;
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
    destroy_client_ctx((receive_ctx_t*)client->data);
    free(client);
}


void on_read_complete(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    if (nread < 0) {
        if (nread != UV_EOF) {
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
            uv_close((uv_handle_t*) client, on_close_complete);
        }

    } else if (nread > 0) {

        receive_ctx_t* c = (receive_ctx_t*)client->data;

		if (c) {
			c->buf_len += nread;
			c->buf = (char*)realloc(c->buf, c->buf_len+1);
			memcpy(c->buf+c->buf_len-nread, buf->base, nread);
            c->buf[c->buf_len] = 0;

            printf("rcv: %s\r\n", c->buf);

            if (is_valid_json(c->buf)) {

                char* result_str = input_json_msg_handler(c->buf);
                if (result_str) {
                    uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));
                    uv_buf_t wrbuf = uv_buf_init(result_str, strlen(result_str));
                    uv_write(req, client, &wrbuf, 1, on_write_complete);
                    printf("snd: %s\r\n", result_str);
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
                              
    uv_tcp_t* client = malloc(sizeof(uv_tcp_t)); 
    client->data = init_client_ctx();

    uv_tcp_init(uv_default_loop(), client);

    if (uv_accept(server, (uv_stream_t*)client) == 0) {
    	uv_tcp_keepalive(client, 1, 50);
        uv_read_start((uv_stream_t*)client, alloc_buffer, on_read_complete);
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
	int r = uv_listen((uv_stream_t*)&server, 128, on_new_connection);
	if (r) {
		fprintf(stderr, "Listen error %s\n", uv_strerror(r));
		return 1;
	}
	printf("server start [%u]\r\n", server_port);
	return 0;
}


//////////////////////////////////////////////////////////////////////////

char* input_json_msg_handler(const char* json_str)
{
	char* r_code = NULL;

	session_data_t sess = {0};

	if (!parse_incoming_json(json_str, &sess)) {

		int expression_idx;

		stack_create(&sess.result, sizeof(NUM_t), stack_size(&sess.expression));

		for (expression_idx = 0; expression_idx<stack_size(&sess.expression); ++expression_idx) {
			char** expression_str = (char**)stack_element_at(&sess.expression, expression_idx);

			NUM_t res;
			char* err_str = NULL;
			if (!calculate(*expression_str, &sess.var, &res, &err_str)) {
				//printf("calc[%d]: \"%s\" -> %lld\r\n", expression_idx, *expression_str, res);
				stack_push_back(&sess.result, &res);
			} else {
				printf("calc[%d]: ERROR \"%s\" -> %s\r\n", expression_idx, *expression_str, err_str);
				break;
			}
		}
		r_code = create_outgoing_json(&sess);
	}

	{// Release session
		void** data_ptr = NULL;
		var_destroy(&sess.var);
		while (NULL!=(data_ptr=stack_pop_back(&sess.expression))) {
			free(*data_ptr);
		}
		stack_destroy(&sess.expression);
		stack_destroy(&sess.result);
	}

	return r_code;
}





