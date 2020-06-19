
#include "uv_balancer_client.h"

#include <stdlib.h>
#include <string.h>

#include "../tools/json_common.h"
#include "common.h"
#include "configuration.h"


void on_client_close_complete(uv_handle_t *client)
{
	client_descr_t* c = (client_descr_t*)client->data;//find_client_by_stream((uv_stream_t*)client);
	if (c) {
		c->state = UNDEF_STATE;
		buf_destroy(&c->rx);
		free(c->req_json);
		free(c);
	}
}


void on_client_alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) 
{
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
}


void on_client_read_complete(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
	client_descr_t* c = (client_descr_t*)client->data;

    if (nread < 0) {

		fprintf(stderr, "bcli: Read error %s\n", uv_err_name(nread));
		uv_close((uv_handle_t*)&c->handle, on_client_close_complete);

    } else if (nread > 0) {

    	buf_add(&c->rx, buf->base, nread);
   		//printf("bcli: rcv: %s\r\n", c->rx.data);

		char* input_msg = NULL;
		char* b_i = c->rx.data;

		while ( NULL != (input_msg = get_msg_from_stream(&b_i, MESSAGE_DELIMITER)) ) {
			//printf("bcli: find msg -> [%s]\r\n", input_msg);
	        if (c->result_cb_fn) {
	        	(*c->result_cb_fn)(&c->handle, input_msg);
    	    }
    	    free(input_msg);
		}
            
		if (b_i!=c->rx.data) {
			buf_skip_first_byte(&c->rx, (b_i-c->rx.data));
		}   

    }

   	if (buf->base) {
		free(buf->base);
	}
}


void on_client_write_complete(uv_write_t *req, int status) 
{
	//client_descr_t* c = (client_descr_t*)req->data;
    if (status) {
        fprintf(stderr, "bcli: Write error %s\n", uv_strerror(status));
    }
    free(req);
}


void on_connect_to_balancer(uv_connect_t* req, int status)
{
	client_descr_t* c = (client_descr_t*)req->data;

	if (status<0) {
		c->err_count++;
		c->state = UNDEF_STATE;
        fprintf(stderr, "bcli connection error: %s\n", uv_strerror(status));
        uv_close((uv_handle_t*)&c->handle, on_client_close_complete);
        return;
	}

	//printf("bcli connect\n");
	c->state = READY_STATE;

	//uv_tcp_keepalive((uv_tcp_t*)req->handle, 1, 50);
	uv_read_start(req->handle, on_client_alloc_buffer, on_client_read_complete);

	{
		uv_write_t *req = (uv_write_t *)malloc(sizeof(uv_write_t));
		req->data = c;
		uv_buf_t wrbuf[2];
		wrbuf[0] = uv_buf_init(c->req_json, strlen(c->req_json));
		wrbuf[1] = uv_buf_init(MESSAGE_DELIMITER, strlen(MESSAGE_DELIMITER));
		uv_write(req, (uv_stream_t*)&c->handle, wrbuf, 2, on_client_write_complete);
	}
}


uint32_t send_to_calc(char* out_json, on_calc_result_cb_t result_fn)
{
	client_descr_t* bc = (client_descr_t*)malloc( sizeof(*bc) );

	memcpy(&bc->addr, get_balancer_addr(), sizeof(bc->addr));
	bc->state = UNDEF_STATE;
	bc->err_count = 0;
	bc->connect.data = bc;
	bc->handle.data = bc;
	bc->req_json = out_json;
	bc->result_cb_fn = result_fn;
	buf_create(&bc->rx);
	//printf("bcli start: %s:%d\n", inet_ntoa(bc->addr.sin_addr), ntohs(bc->addr.sin_port));

	uv_tcp_init(uv_default_loop(), &bc->handle);
	if (0==uv_tcp_connect(&bc->connect, &bc->handle, (const struct sockaddr*)&bc->addr, on_connect_to_balancer)) {
		bc->state = CONNECTING_STATE;
   		return 0;
	}

	buf_destroy(&bc->rx);
	free(bc);
	
	return 1;	
}


void close_calc_connection(uv_stream_t *client)
{
	uv_close((uv_handle_t*)client, on_client_close_complete);
}