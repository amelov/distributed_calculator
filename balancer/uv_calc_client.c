
#include "calc_client.h"




void on_calc_close_complete(uv_handle_t *client)
{
 	printf("calc_cli close_complete\n");
	//destroy_client_ctx((receive_ctx_t*)client->data);
	//free(client);
}


void on_calc_alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) 
{
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
}


void on_calc_read_complete(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    if (nread < 0) {
        if (nread != UV_EOF) {
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
            uv_close((uv_handle_t*) client, on_calc_close_complete);
            // state = UINT32_MAX;	
        } else {

			client_descr_t* p = find_client_by_stream(client);
			if (p) {
				p->state = UINT32_MAX;
			}

        }

    } else if (nread > 0) {

    }

   	if (buf->base) {
		free(buf->base);
	}
}



void on_connect_to_calc(uv_connect_t* req, int status)
{
	client_descr_t* c = (client_descr_t*)req;

	if (status<0) {
		//TODO: find client !!!!
		c->state = UINT32_MAX;
        fprintf(stderr, "calc_cli:%d connection error: %s\n", c->dbg_id, uv_strerror(status));
        return;
	}

	printf("calc_cli:%d connect\n", c->dbg_id);
	c->state = 1;

	uv_tcp_keepalive((uv_tcp_t*)req->handle, 1, 50);
	uv_read_start(req->handle, on_calc_alloc_buffer, on_calc_read_complete);
}


uint8_t start_uv_tcp_client(client_descr_t* c)
{
	printf("calc_cli:%d start: %s:%d\n", c->dbg_id, inet_ntoa(c->addr.sin_addr), htons(c->addr.sin_port));


	uv_tcp_init(uv_default_loop(), &c->handle);
	if (0==uv_tcp_connect(&c->connect, &c->handle, (const struct sockaddr*)&c->addr, on_connect_to_calc)) {
   		c->state = 1;
		return 0;
	}
	c->state = UINT32_MAX;
	return 1;
}



void on_reconnect_timer_cb(uv_timer_t* handle)
{
	uint32_t idx = 0;
	client_descr_t* p = NULL;
	while ( NULL != (p=get_calc_host(idx++)) ) {
		if (p->state == UINT32_MAX) {
			start_uv_tcp_client(p);
		}
	}
}