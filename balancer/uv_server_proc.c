
#include "uv_server_proc.h"


#include <unistd.h>
#include <string.h>

#include "configuration.h"


#include "json_common.h"
#include "uv_calc_client.h"
#include "mbuf.h"


static uv_tcp_t server;

//////////////////////////////////////////////////////////////////////////

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


client_descr_t* select_calc_client()
{
    static size_t base_calculator_id = 0;

    client_descr_t* r_code = NULL;

    for (int i=0; i<get_calc_host_count(); i++) {
        client_descr_t* p = get_calc_host((base_calculator_id + i) % get_calc_host_count());
        if (p->state == 0) {
            base_calculator_id++;
            base_calculator_id %= get_calc_host_count();
            r_code = p;
            break;
        }
    }
    return r_code;
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
    		printf("SRV: r: %s\r\n", c->rx.data);

            char* input_msg = NULL;
            char* b_i = c->rx.data;

            while ( NULL != (input_msg = get_msg_from_stream(&b_i, MESSAGE_DELIMITER)) ) {

                printf("SRV: find msg -> [%s]\r\n", input_msg);

                client_descr_t* p = select_calc_client();
                if (p) {
                    printf("SRV: active calculator: %d\r\n", p->dbg_id);
                    send_to_calc(p, client, input_msg);       // TODO: save req context!
                } else {
                    printf("SRV: no active calculator!\r\n");
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
	int r = uv_listen((uv_stream_t*)&server, MAX_CLIENT_COUNT, on_new_connection);
	if (r) {
		fprintf(stderr, "Listen error %s\n", uv_strerror(r));
		return 1;
	}
	printf("server start [%u]\r\n", server_port);
	return 0;
}


// TODO: send to save req context!
void send_to_user(client_descr_t* c, char* result_msg)
{
    printf("send_to_user -> %s\r\n", result_msg);

    uv_write_t *req = (uv_write_t *)malloc(sizeof(uv_write_t));
    uv_buf_t wrbuf = uv_buf_init(result_msg, strlen(result_msg));
    uv_write(req, c->req_stream, &wrbuf, 1, on_write_complete);
}