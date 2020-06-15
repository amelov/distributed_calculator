
#include "uv_calc_client.h"

#include <string.h>

#include "configuration.h"
#include "uv_server_proc.h"
#include "json_common.h"



void on_calc_close_complete(uv_handle_t *client)
{
	client_descr_t* c = (client_descr_t*)client->data;//find_client_by_stream((uv_stream_t*)client);
	if (c) {
		printf("cli:%d close_complete\n", c->dbg_id);
		c->state = UNDEF_STATE;
		buf_destroy(&c->rx);
	}
}


void on_calc_alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) 
{
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
}


void on_calc_read_complete(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
	client_descr_t* c = (client_descr_t*)client->data;

    if (nread < 0) {

		fprintf(stderr, "cli:%d Read error %s\n", c->dbg_id, uv_err_name(nread));
		uv_close((uv_handle_t*) client, on_calc_close_complete);

    } else if (nread > 0) {

    	buf_add(&c->rx, buf->base, nread);
   		printf("cli%d: rcv: %s\r\n", c->dbg_id, c->rx.data);

		char* input_msg = NULL;
		char* b_i = c->rx.data;

		while ( NULL != (input_msg = get_msg_from_stream(&b_i, MESSAGE_DELIMITER)) ) {

			printf("cli%d: find msg -> [%s]\r\n", c->dbg_id, input_msg);
			send_to_user(c, input_msg);
		}
            
		if (b_i!=c->rx.data) {
			buf_skip_first_byte(&c->rx, (b_i-c->rx.data));
			printf("cli%d: new rcv: %s\r\n", c->dbg_id, c->rx.data);
		}   

    }

   	if (buf->base) {
		free(buf->base);
	}
}


void on_connect_to_calc(uv_connect_t* req, int status)
{
	client_descr_t* c = (client_descr_t*)req->data;
	if (status<0) {
		c->state = UNDEF_STATE;
		//buf_destroy(c->rx);
        fprintf(stderr, "calc_cli:%d connection error : %s\n", c->dbg_id, uv_strerror(status));
        return;
	}
	
	c->state = READY_STATE;
	buf_create(&c->rx);

	uv_tcp_keepalive((uv_tcp_t*)req->handle, 1, 50);
	uv_read_start(req->handle, on_calc_alloc_buffer, on_calc_read_complete);
}


uint8_t start_uv_tcp_client(client_descr_t* c)
{
	printf("calc_cli_%d start: %s:%d\n", c->dbg_id, inet_ntoa(c->addr.sin_addr), htons(c->addr.sin_port));

	uv_tcp_init(uv_default_loop(), &c->handle);
	if (0==uv_tcp_connect(&c->connect, &c->handle, (const struct sockaddr*)&c->addr, on_connect_to_calc)) {
		c->state = CONNECTING_STATE;
   		return 0;
	}
	c->state = UNDEF_STATE;
	return 1;
}


void on_reconnect_timer_cb(uv_timer_t* handle)
{
	uint32_t idx = 0;
	client_descr_t* p = NULL;
	while ( NULL != (p=get_calc_host(idx++)) ) {
		if (p->state == UNDEF_STATE) {
			start_uv_tcp_client(p);
		}
	}
}


void on_client_write_complete(uv_write_t *req, int status) 
{
    if (status) {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    free(req);
}


void send_to_calc(client_descr_t* c, uv_stream_t* req_stream, char* json_msg_for_calc)
{
	c->req_stream = req_stream;
	uv_write_t *req = (uv_write_t *)malloc(sizeof(uv_write_t));
	uv_buf_t wrbuf = uv_buf_init(json_msg_for_calc, strlen(json_msg_for_calc));
	uv_write(req, (uv_stream_t*)&c->handle, &wrbuf, 1, on_client_write_complete);
}
