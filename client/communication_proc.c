
#include "communication_proc.h"
#include <stdio.h>


uint32_t send_to_calc(char* req, calc_result_cb_t p_fn)
{
	printf("send_to_calc -> %s", req);
	return 1;
}