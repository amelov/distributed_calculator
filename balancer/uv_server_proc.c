
#include "uv_server_proc.h"


#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "configuration.h"

#include "uv_calc_client.h"

#include "../tools/json_common.h"
#include "../tools/mbuf.h"


//////////////////////////////////////////////////////////////////////////

typedef struct socket_ctx_t {
	buf_t rx;
	uint32_t error_count;
	uint32_t id;
} socket_ctx_t;



static socket_ctx_t* dc_balancer_sp_ctx_init()
{
	static uint32_t _user_client_id = 0;

	socket_ctx_t* s_c = (socket_ctx_t*)calloc(1, sizeof(socket_ctx_t));
	buf_create(&s_c->rx);
	s_c->error_count = 0;
	s_c->id = _user_client_id++;
	return s_c;
}


static void dc_balancer_sp_ctx_destroy(socket_ctx_t* c)
{
	if (c) {
		buf_destroy(&c->rx);
		free(c);
	}
}

//////////////////////////////////////////////////////////////////////////

static void dc_balancer_sp_alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) 
{
	buf->base = (char*)malloc(suggested_size);
	assert(buf->base);
	buf->len = suggested_size;
}


static void dc_balancer_sp_on_write_complete(uv_write_t *req, int status) 
{
	if (status) {
		fprintf(stderr, "Write error %s\n", uv_strerror(status));
	}
	free(req);
}


static void dc_balancer_sp_on_close_complete(uv_handle_t *client)
{
	socket_ctx_t* s_c = client->data;
	printf("SRV[%d]: on_close_complete\r\n", s_c->id);
	dc_balancer_sp_ctx_destroy((socket_ctx_t*)client->data);
	free(client);
}


static uint32_t dc_balancer_sp_find_free_calc_client(calc_ctx_t* c)
{
	uint32_t r_code = 1;
	static size_t base_calculator_id = 0;

	for (int i=0; i<dc_balancer_cfg_get_calc_host_count(); i++) {
		if ( !dc_balancer_cfg_get_calc_host_addr((base_calculator_id + i) % dc_balancer_cfg_get_calc_host_count(), c) ) {
			r_code = 0;
			break;
		}
	}
	base_calculator_id++;
	return r_code;
}


static void dc_balancer_sp_on_calc_proc(uv_stream_t* req_client, char* req_str, char* result_str)
{
	if (uv_is_active((uv_handle_t*)req_client)) {

		socket_ctx_t* c = (socket_ctx_t*)req_client->data;
		if (c) {
			c->error_count = 0;
		}

		printf("SRV[%d]: send_to_user -> %s\r\n", c->id, result_str);
		uv_write_t *req = (uv_write_t *)malloc(sizeof(uv_write_t));
	assert(req);
		uv_buf_t wrbuf = uv_buf_init(result_str, strlen(result_str));
		uv_write(req, req_client, &wrbuf, 1, dc_balancer_sp_on_write_complete);
	}
	free(req_str);
	free(result_str);
}


static void dc_balancer_sp_on_calc_error(uv_stream_t* req_client, char* req_str)
{
	socket_ctx_t* c = (socket_ctx_t*)req_client->data;

	c->error_count++;
	printf("SRV[%d]: on_calc_error (%s) err_count: %d\r\n", c->id, req_str, c->error_count);

	calc_ctx_t c_addr;
	if (!dc_balancer_sp_find_free_calc_client(&c_addr)) {
		if (dc_balancer_cc_send_job(req_client, req_str, &c_addr.addr, &dc_balancer_sp_on_calc_proc, &dc_balancer_sp_on_calc_error)) {
			printf("SRV[%d]: send_req_to_calc - error!\r\n", c->id);
			free(req_str);
		}
	}
}


static void dc_balancer_sp_on_read_complete(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
	socket_ctx_t* c = (socket_ctx_t*)client->data;

	if (nread < 0) {
		//if (nread != UV_EOF) 
		{
			fprintf(stderr, "SRV[%d]: Read error %s\n", c->id, uv_err_name(nread));
			uv_close((uv_handle_t*) client, dc_balancer_sp_on_close_complete);
		}

	} else if (nread > 0) {

		if (c) {

			buf_add(&c->rx, buf->base, nread);
			printf("SRV[%d]: r: %s\r\n", c->id, c->rx.data);

			char* input_msg = NULL;
			char* b_i = c->rx.data;

			while ( NULL != (input_msg = get_msg_from_stream(&b_i, MESSAGE_DELIMITER)) ) {

				printf("SRV[%d]: find msg -> [%s]\r\n", c->id, input_msg);

				calc_ctx_t c_addr;
				if (!dc_balancer_sp_find_free_calc_client(&c_addr)) {
					if (dc_balancer_cc_send_job(client, input_msg, &c_addr.addr, &dc_balancer_sp_on_calc_proc, &dc_balancer_sp_on_calc_error)) {
						printf("SRV[%d]: send_req_to_calc - error!\r\n", c->id);
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


static void dc_balancer_sp_on_new_connection(uv_stream_t *server, int status) 
{
	if (status < 0) {
		fprintf(stderr, "SRV: New connection error %s\n", uv_strerror(status));
		return;
	}

	uv_tcp_t* client = malloc(sizeof(uv_tcp_t)); 
	assert(client);
	client->data = dc_balancer_sp_ctx_init();

	uv_tcp_init(uv_default_loop(), client);

	if (uv_accept(server, (uv_stream_t*)client) == 0) {
		uv_tcp_keepalive(client, 1, 50);
		uv_read_start((uv_stream_t*)client, dc_balancer_sp_alloc_buffer, dc_balancer_sp_on_read_complete);
	} else {
		uv_close((uv_handle_t*)client, dc_balancer_sp_on_close_complete);
	}
}

////////////////////////////////////////////////////////////////////

uint8_t dc_balancer_sp_start(uv_tcp_t* p_server, const uint16_t server_port)
{
	struct sockaddr_in addr;

	uv_tcp_init(uv_default_loop(), p_server);
	uv_ip4_addr("0.0.0.0", server_port, &addr);

	uv_tcp_bind(p_server, (const struct sockaddr*)&addr, 0);
	int r = uv_listen((uv_stream_t*)p_server, MAX_CLIENT_COUNT, dc_balancer_sp_on_new_connection);
	if (r) {
		fprintf(stderr, "Listen error %s\n", uv_strerror(r));
		return 1;
	}
	printf("server start [%u]\r\n", server_port);
	return 0;
}
