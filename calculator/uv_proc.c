

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "uv_proc.h"
#include "json_tool.h"


extern char* input_json_msg_handler(const char* json_str);


uv_loop_t * loop = NULL;
uv_tcp_t server;

static char* rcv_data = NULL;
static size_t rcv_data_len = 0;


static void free_receiver_ctx()
{
    rcv_data_len = 0;
    free(rcv_data);
    rcv_data = NULL;
}


void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char*)malloc(suggested_size);
    buf->len = suggested_size;
}



void on_write_complete(uv_write_t *req, int status) {
    if (status) {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    free(req);
}



void on_read_complete(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
    if (nread < 0) {
        if (nread != UV_EOF) {
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
		
            free_receiver_ctx();
            //printf("close: %u", client);	

            uv_close((uv_handle_t*) client, NULL);
        }
    } else if (nread > 0) {

		//printf("%u recv: %lu\r\n", client, nread);
	
		{
			rcv_data_len += nread;
			rcv_data = realloc(rcv_data, rcv_data_len+1);
			memcpy(rcv_data+rcv_data_len-nread, buf->base, nread);
            rcv_data[rcv_data_len] = 0;

            printf("rcv: %s\r\n", rcv_data);

            if (is_valid_json(rcv_data)) {

                char* result_str = input_json_msg_handler(rcv_data);

                if (result_str) {
                    uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));
                    uv_buf_t wrbuf = uv_buf_init(result_str, strlen(result_str));
                    uv_write(req, client, &wrbuf, 1, on_write_complete);
                    free(result_str);
                }

                free_receiver_ctx();
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
                              
    uv_tcp_t *client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);

    if (uv_accept(server, (uv_stream_t*) client) == 0) {
		uv_tcp_keepalive(client, 1, 10);
	
		//printf("new connect: %u\r\n", client);
        free_receiver_ctx();
        
        uv_read_start((uv_stream_t*)client, alloc_buffer, on_read_complete);
    } else {
        uv_close((uv_handle_t*) client, NULL);
    }
}






uint8_t start_uv_tcp_server(uv_loop_t *main_loop, const uint16_t server_port)
{
	struct sockaddr_in addr;
	loop = main_loop;

	uv_tcp_init(loop, &server);
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
