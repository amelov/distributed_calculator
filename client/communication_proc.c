
#include "communication_proc.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "configuration.h"
#include "common.h"
#include "../tools/mbuf.h"


static uint32_t dc_client_socket_send(int socket_id, char* data, size_t data_sz)
{
	while (data_sz) {
		ssize_t s_code = send(socket_id, data, data_sz, 0);
		if (s_code !=-1) {
			data += s_code;
			data_sz -= s_code;
		} else {
			break;
		}
	}
	return data_sz;
}


uint32_t dc_client_send_calculation_job(char* req, dc_client_calculation_result_cb_t p_fn)
{
	uint32_t r_code = UINT32_MAX;

	int s_id = socket(AF_INET, SOCK_STREAM, 0/*IPPRO_TCP*//*SOCK_NONBLOCK*/);
	
	if (s_id!=-1) {

		int r = connect(s_id, (struct sockaddr*)dc_client_cfg_balancer_addr(), sizeof(struct sockaddr_in));

		if (!r) {

			if ( (!dc_client_socket_send(s_id, req, strlen(req))) && 
				 (!dc_client_socket_send(s_id, MESSAGE_DELIMITER, strlen(req))) ) {

				buf_t rx_buf;
				buf_create(&rx_buf);

				char buf[127];
				
				while (UINT32_MAX == r_code) {

					ssize_t rcv_code = recv(s_id, buf, sizeof(buf), 0/*MSG_DONTWAIT | MSG_PEEK*/);

					if (rcv_code>0) {

						buf_add(&rx_buf, buf, rcv_code);

						char* temp_p = rx_buf.data;

						while (temp_p<&rx_buf.data[rx_buf.data_sz]) {
							if ( (*temp_p)==MESSAGE_DELIMITER[0]) {
								*temp_p = 0;
								if (p_fn) {
									(*p_fn)(rx_buf.data);
								}
								r_code = 0;
								break;
							}
							temp_p++;
						}

					} else {
						break;
					}
				}

				buf_destroy(&rx_buf);

			} else {
				printf("send err: %d\r\n", errno);
			}

		} else {
			printf("connect err: %d\r\n", errno);
		}

		close(s_id);

	} else {
		printf("socket err: %d\r\n", errno);
	}
	return r_code;
}