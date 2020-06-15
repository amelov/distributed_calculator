
#include "uv_balancer_client.h"

#include <stdlib.h>
#include "json_common.h"
#include "configuration.h"


client_descr_t balancer_client;



void on_balancer_close_complete(uv_handle_t *client)
{
	client_descr_t* c = (client_descr_t*)client->data;//find_client_by_stream((uv_stream_t*)client);
	if (c) {
		c->state = UNDEF_STATE;
		buf_destroy(&c->rx);
	}
}


void on_balancer_alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) 
{
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
}


void on_balancer_read_complete(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
	client_descr_t* c = (client_descr_t*)client->data;

    if (nread < 0) {

		fprintf(stderr, "bcli: Read error %s\n", uv_err_name(nread));
		uv_close((uv_handle_t*) client, on_balancer_close_complete);

    } else if (nread > 0) {

    	buf_add(&c->rx, buf->base, nread);
   		printf("bcli: rcv: %s\r\n", c->rx.data);

		char* input_msg = NULL;
		char* b_i = c->rx.data;

		while ( NULL != (input_msg = get_msg_from_stream(&b_i, MESSAGE_DELIMITER)) ) {

			printf("bcli: find msg -> [%s]\r\n", input_msg);
			//send_to_user(c, input_msg);
		}
            
		if (b_i!=c->rx.data) {
			buf_skip_first_byte(&c->rx, (b_i-c->rx.data));
			printf("bcli: new rcv: %s\r\n", c->rx.data);
		}   

    }

   	if (buf->base) {
		free(buf->base);
	}
}


void on_connect_to_balancer(uv_connect_t* req, int status)
{
	client_descr_t* c = (client_descr_t*)req->data;

	if (status<0) {
		c->state = UNDEF_STATE;
		//buf_destroy(c->rx);
        fprintf(stderr, "bcli connection error: %s\n", uv_strerror(status));
        return;
	}

	printf("bcli connect\n");
	
	c->state = READY_STATE;
	buf_create(&c->rx);

	uv_tcp_keepalive((uv_tcp_t*)req->handle, 1, 50);
	uv_read_start(req->handle, on_balancer_alloc_buffer, on_balancer_read_complete);
}


uint8_t start_uv_tcp_client(client_descr_t* c)
{
	printf("bcli start: %s:%d\n", inet_ntoa(c->addr.sin_addr), htons(c->addr.sin_port));

	uv_tcp_init(uv_default_loop(), &c->handle);
	if (0==uv_tcp_connect(&c->connect, &c->handle, (const struct sockaddr*)&c->addr, on_connect_to_balancer)) {
		c->state = CONNECTING_STATE;
   		return 0;
	}
	c->state = UNDEF_STATE;
	return 1;
}


void on_reconnect_timer_cb(uv_timer_t* handle)
{
	if (balancer_client.state == UNDEF_STATE) {
		start_uv_tcp_client(&balancer_client);
	}
}