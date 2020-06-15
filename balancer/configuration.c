
#include "configuration.h"

#include <jansson.h>
#include <uv.h>

static mstack_t calc_host = {0};

static uint16_t server_port = SERVER_PORT;


uint8_t load_config(const char* fn)
{
	json_error_t error;
	json_t* root = json_load_file(fn, 0, &error);

	stack_create(&calc_host, sizeof(client_descr_t), 16);

	if (root) {

		json_t *server_v   = json_object_get(root, "server_port");
		if ( server_v && json_is_number(server_v) ) {
			server_port = json_integer_value(server_v);
		}


		json_t *calcs = json_object_get(root, "calculator");
		if (calcs && json_is_array(calcs)) {

			json_t *v = NULL;
			size_t idx;
			json_array_foreach(calcs, idx, v) {
				if (v && json_is_object(v)) {

					json_t *ip_v   = json_object_get(v, "ip");
					json_t *port_v = json_object_get(v, "port");

					if ( ip_v  && port_v && json_is_string(ip_v) && json_is_number(port_v) ) {
						static uint32_t dbg_idx = 0;
						client_descr_t* a = (client_descr_t*)malloc(sizeof(*a));
						a->dbg_id = dbg_idx++;
						a->addr.sin_family = AF_INET;
						a->addr.sin_addr.s_addr = inet_addr( json_string_value(ip_v) );
						a->addr.sin_port = htons( json_integer_value(port_v) );
						a->state = UNDEF_STATE;
						//a->connect.data = a;
						//a->handle.data = a;
						//buf_create(&a.rx);
						stack_push_back(&calc_host, a);
					}
				}
			}

			return !(stack_size(&calc_host)>0);
		}
		json_decref(root);
	}

	return 1;
}


client_descr_t* get_calc_host(const size_t idx)
{
	if (idx < stack_size(&calc_host)) {
		return (client_descr_t*)stack_element_at(&calc_host, idx);
	}
	return NULL;
}

size_t get_calc_host_count()
{
	return stack_size(&calc_host);
}

/*
client_descr_t* find_client_by_stream(uv_stream_t* c)
{
	uint32_t idx = 0;
	client_descr_t* p = NULL;
	while ( NULL != (p=get_calc_host(idx++)) ) {
		if (p->connect.handle == c) {
		//if (&p->handle == (uv_tcp_t*)c) {
			return p;
		}
	}
	return NULL;
}
*/

uint16_t get_server_port()
{
	return server_port;
}