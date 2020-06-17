
#include "uv_calc_client.h"

#include <string.h>

#include "configuration.h"
#include "uv_server_proc.h"
#include "../tools/json_common.h"



void on_calc_close_complete(uv_handle_t *client)
{
	calc_client_descr_t* c = (calc_client_descr_t*)client->data;//find_client_by_stream((uv_stream_t*)client);
	if (c) {
		printf("calc[%d]: close_complete\n", c->dbg_id);
		buf_destroy(&c->rx);
		free(c);
	}
}


void on_calc_alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) 
{
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
}


void on_calc_read_complete(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
	calc_client_descr_t* c = (calc_client_descr_t*)client->data;

    if (nread < 0) {

		fprintf(stderr, "calc[%d]: Read error %s\n", c->dbg_id, uv_err_name(nread));
		uv_close((uv_handle_t*) client, on_calc_close_complete);

    } else if (nread > 0) {

    	buf_add(&c->rx, buf->base, nread);
   		printf("calc[%d]: rcv: %s\r\n", c->dbg_id, c->rx.data);

		char* input_msg = NULL;
		char* b_i = c->rx.data;

		while ( NULL != (input_msg = get_msg_from_stream(&b_i, MESSAGE_DELIMITER)) ) {

			printf("calc[%d]: find msg -> [%s]\r\n", c->dbg_id, input_msg);
			//send_to_user(c->req_stream, input_msg);
			if (c->on_result_fn) {
				(*c->on_result_fn)(c->req_stream, c->req_json, input_msg);
			}
		}
            
		if (b_i!=c->rx.data) {
			buf_skip_first_byte(&c->rx, (b_i-c->rx.data));
			//printf("calc[%d]: new rcv: %s\r\n", c->dbg_id, c->rx.data);
		}   

    }

   	if (buf->base) {
		free(buf->base);
	}
}


void on_client_write_complete(uv_write_t *req, int status) 
{
	calc_client_descr_t* c = (calc_client_descr_t*)req->data;
    if (status) {
        fprintf(stderr, "calc[%d]: Write error %s\n", c->dbg_id, uv_strerror(status));
        if (c->on_err_fn) {
        	(*c->on_err_fn)(c->req_stream, c->req_json);
        }
        uv_close((uv_handle_t*)&c->handle, on_calc_close_complete);
    }
    free(req);
}


void on_connect_to_calc(uv_connect_t* req, int status)
{
	calc_client_descr_t* c = (calc_client_descr_t*)req->data;
	if (status<0) {
		uv_close((uv_handle_t*)&c->handle, on_calc_close_complete);
        fprintf(stderr, "calc[%d]: connection error : %s\n", c->dbg_id, uv_strerror(status));
        //calc_connection_error(c->req_stream, c->req_json);
        if (c->on_err_fn) {
        	(*c->on_err_fn)(c->req_stream, c->req_json);
        }
        return;
	}
	
	printf("calc[%d]: connect complete\r\n", c->dbg_id);
	//c->state = READY_STATE;
	//buf_create(&c->rx);
	uv_tcp_keepalive((uv_tcp_t*)req->handle, 1, 50);
	uv_read_start(req->handle, on_calc_alloc_buffer, on_calc_read_complete);


	{
		uv_write_t *req = (uv_write_t *)malloc(sizeof(uv_write_t));
		req->data = c;
		uv_buf_t wrbuf = uv_buf_init(c->req_json, strlen(c->req_json));
		uv_write(req, (uv_stream_t*)&c->handle, &wrbuf, 1, on_client_write_complete);
	}
}


/*
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
*/

/*
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
*/



uint32_t send_req_to_calc(uv_stream_t* req_client, char* json_msg_str, struct sockaddr_in* calc_addr, calc_proc_cb_t fn, calc_error_cb_t err_fn)
{
	static uint32_t dbg_id = 0;

	calc_client_descr_t* c = malloc(sizeof(*c));
	{
		memcpy(&c->addr, calc_addr, sizeof(c->addr));
		c->connect.data = c;
		c->handle.data = c;
		c->req_stream = req_client;
		c->req_json = json_msg_str;
		c->on_result_fn = fn;
		c->on_err_fn = err_fn;
		buf_create(&c->rx);
		c->dbg_id = dbg_id++;
	}

	printf("calc[%d]: start connection: %s:%d\r\n", c->dbg_id, inet_ntoa(c->addr.sin_addr), ntohs(c->addr.sin_port));

	uv_tcp_init(uv_default_loop(), &c->handle);
	if (0==uv_tcp_connect(&c->connect, &c->handle, (const struct sockaddr*)&c->addr, on_connect_to_calc)) {
   		return 0;
	}

	buf_destroy(&c->rx);
	free(c);
	return 1;
}
