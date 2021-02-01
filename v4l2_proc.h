#ifndef __V4L2_PROC__H__INCLUDED__
#define __V4L2_PROC__H__INCLUDED__

#include "util_context.h"
#include "util_v4l2.h"

void thread_push_func(utV4l2_Camera& camera, bool useCUI);
void thread_pop_func(utV4l2_Camera& camera, bool useCUI);

int do_v4l2_proc(utContext* context);


#endif //!__V4L2_PROC__H__INCLUDED__
