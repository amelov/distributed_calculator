
#pragma once

#include <uv.h>



void on_readline_work_cb(uv_work_t* req);
void on_after_readline_work_cb(uv_work_t* req, int status);