
#include "uv_server_proc.h"


#include <unistd.h>
#include <string.h>

#include "configuration.h"

#include "uv_calc_client.h"

#include "../tools/json_common.h"
#include "../tools/mbuf.h"


static uv_tcp_t server;

//////////////////////////////////////////////////////////////////////////

typedef struct socket_ctx_t {
    buf_t rx;
    uint32_t error_count;
} socket_ctx_t;



socket_ctx_t* init_client_ctx()
{
    socket_ctx_t* r_code = (socket_ctx_t*)calloc(1, sizeof(socket_ctx_t));
    buf_create(&r_code->rx);
    r_code->error_count = 0;
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
    destroy_client_ctx((socket_ctx_t*)client->data);
    free(client);
}


uint32_t find_free_calc_client(calc_ctx_t* c)
{
    uint32_t r_code = 1;   
    static size_t base_calculator_id = 0;

    for (int i=0; i<get_calc_host_count(); i++) {
        if ( !get_calc_host_addr((base_calculator_id + i) % get_calc_host_count(), c) ) {
            r_code = 0;
            break;
        }
    }
    base_calculator_id++;
    return r_code;
}


static void on_calc_proc(uv_stream_t* req_client, char* req_str, char* result_str)
{
    if (uv_is_active((uv_handle_t*)req_client)) {

        socket_ctx_t* c = (socket_ctx_t*)req_client->data;
        if (c) {
            c->error_count = 0;
        }

        printf("send_to_user -> %s\r\n", result_str);
        uv_write_t *req = (uv_write_t *)malloc(sizeof(uv_write_t));
        uv_buf_t wrbuf = uv_buf_init(result_str, strlen(result_str));
        uv_write(req, req_client, &wrbuf, 1, on_write_complete);
    }
    free(req_str);
    free(result_str);
}


static void on_calc_error(uv_stream_t* req_client, char* req_str)
{
    socket_ctx_t* c = (socket_ctx_t*)req_client->data;

    c->error_count++;
    printf("SRV: on_calc_error (%s) err_count: %d\r\n", req_str, c->error_count);


    calc_ctx_t c_addr;
    if (!find_free_calc_client(&c_addr)) {
        if (send_req_to_calc(req_client, req_str, &c_addr.addr, &on_calc_proc, &on_calc_error)) {
            printf("SRV: send_req_to_calc - error!\r\n");
            free(req_str);
        }
    }
}


void on_read_complete(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    if (nread < 0) {
        //if (nread != UV_EOF) 
        {
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
            uv_close((uv_handle_t*) client, on_close_complete);
        }

    } else if (nread > 0) {

        socket_ctx_t* c = (socket_ctx_t*)client->data;

    	if (c) {

            buf_add(&c->rx, buf->base, nread);
    		printf("SRV: r: %s\r\n", c->rx.data);

            char* input_msg = NULL;
            char* b_i = c->rx.data;

            while ( NULL != (input_msg = get_msg_from_stream(&b_i, MESSAGE_DELIMITER)) ) {

                printf("SRV: find msg -> [%s]\r\n", input_msg);

                calc_ctx_t c_addr;
                if (!find_free_calc_client(&c_addr)) {
                    if (send_req_to_calc(client, input_msg, &c_addr.addr, &on_calc_proc, &on_calc_error)) {
                        printf("SRV: send_req_to_calc - error!\r\n");
                        free(input_msg);
                    }
                }
              
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


void on_new_connection(uv_stream_t *server, int status) 
{
    if (status < 0) {
        fprintf(stderr, "SRV: New connection error %s\n", uv_strerror(status));
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
	int r = uv_listen((uv_stream_t*)&server, MAX_CLIENT_COUNT, on_new_connection);
	if (r) {
		fprintf(stderr, "Listen error %s\n", uv_strerror(r));
		return 1;
	}
	printf("server start [%u]\r\n", server_port);
	return 0;
}
