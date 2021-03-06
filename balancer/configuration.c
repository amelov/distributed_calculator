
#include "configuration.h"

#include <jansson.h>
#include <uv.h>
#include <string.h>


static dc_balancer_cfg_ctx_t* dc_balancer_cfg_ctx_ptr = NULL;


void dc_balancer_cfg_create(dc_balancer_cfg_ctx_t* ctx)
{
	if (ctx) {
		dc_balancer_cfg_ctx_ptr = ctx;
		dc_balancer_cfg_ctx_ptr->server_port = SERVER_PORT;
	}
}


uint8_t dc_balancer_cfg_load(const char* fn)
{
	if (dc_balancer_cfg_ctx_ptr) {
		json_error_t error;
		json_t* root = json_load_file(fn, 0, &error);

		stack_create(&dc_balancer_cfg_ctx_ptr->calc_addr_stack, sizeof(calc_ctx_t), 16);

		if (root) {

			json_t *server_v   = json_object_get(root, "server_port");
			if ( server_v && json_is_number(server_v) ) {
				dc_balancer_cfg_ctx_ptr->server_port = json_integer_value(server_v);
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
							calc_ctx_t a = {0};
							a.addr.sin_family = AF_INET;
							a.addr.sin_addr.s_addr = inet_addr( json_string_value(ip_v) );
							a.addr.sin_port = htons( json_integer_value(port_v) );
							//a.state = UNDEF_STATE;
							stack_push_back(&dc_balancer_cfg_ctx_ptr->calc_addr_stack, &a);
						}
					}
				}

				return !(stack_size(&dc_balancer_cfg_ctx_ptr->calc_addr_stack)>0);
			}
			json_decref(root);
		}
	}
	return 1;
}


uint32_t dc_balancer_cfg_get_calc_host_addr(const size_t idx, calc_ctx_t* c)
{
	if (c && (idx < stack_size(&dc_balancer_cfg_ctx_ptr->calc_addr_stack))) {
		//return (client_descr_t*)stack_element_at(&calc_host, idx);
		memcpy(c, stack_element_at(&dc_balancer_cfg_ctx_ptr->calc_addr_stack, idx), sizeof(*c));
		return 0;
	}
	return 1;
}


size_t dc_balancer_cfg_get_calc_host_count()
{
	return stack_size(&dc_balancer_cfg_ctx_ptr->calc_addr_stack);
}


uint16_t dc_balancer_cfg_get_server_port()
{
	return dc_balancer_cfg_ctx_ptr->server_port;
}