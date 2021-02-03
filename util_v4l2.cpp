/*
 *  V4L2 video capture example
 *
 *  This program can be used and distributed without restrictions.
 *
 *      This program is provided with the V4L2 API
 * see https://linuxtv.org/docs.php for more information
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <cuda_runtime.h>

#include "util_class.h"
#include "util_context.h"
#include "util_v4l2.h"
#include "util_v4l2_state.h"

using namespace std;




/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
utV4l2_Camera::utV4l2_Camera( int32_t dev_video_no, utContext* context)
    : _ctx_param(context),
			_width(context->preview_width()), 
			_height(context->preview_height()), 
			_dev_no(dev_video_no),
			_page_count(context->page_count()),
			// _poll_timeout(context->timeout()), 
			// _loop_count(context->grab_count()),
			_type(V4L2_BUF_TYPE_VIDEO_CAPTURE)

// utV4l2_Camera::utV4l2_Camera( int32_t dev_video_no, int32_t w, int32_t h, 
// 															int32_t page_count, int32_t timeout, int32_t loop_count)
//     : _width(w), _height(h), _dev_no(dev_video_no),_page_count(page_count),
// 			_poll_timeout(timeout), 
// 			_loop_count(loop_count),
// 			_type(V4L2_BUF_TYPE_VIDEO_CAPTURE)

{
	cout << "utV4l2_Camera" << endl;
	_pushed	= 0;
	_popped = 0;
	_bExit = false;
	_debug_push_pop = false;
	_state = utV4l2Stat_Closed::instance();
}


/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
std::string utV4l2_Camera::error_text(const char* prefix)
{
	cout 
		<< UTCOL_RED 
		<< prefix << " - error " << to_string(errno) << ", " << strerror(errno)
		<< UTCOL_WHITE << endl;
}


/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
int utV4l2_Camera::xioctl(int fh, int request, void *arg)
{
    int ret = 0;
    while(true) {
			ret = ioctl(fh,request,arg);
			if (ret<0) {
					if (EINTR == errno) {
							continue;
					}
					return -errno;
			}
			break;
    }
    return ret;
}


/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
std::string utV4l2_Camera::dev_name()
{
	return "/dev/video" + to_string(_dev_no);
}


/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
std::string utV4l2_Camera::fourcc_text(uint32_t code)
{
	std::string s = "'";
	for (int ii=0; ii<4; ii++) {
		uint8_t c = (code>>8*ii) & 0xff;
		s += c;
	}
	s += "'";
	return s;
}


/** 
 * @brief enum supported formats
 * @param 
 * @return 
 * @note 
*/

void utV4l2_Camera::list_formats(int fh)
{
    struct v4l2_fmtdesc fmt;
		CLEAR(fmt);
    fmt.index = 0;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		printf("Supported formats:\n");

		int i=0;
    while(0 == xioctl(fh, VIDIOC_ENUM_FMT, &fmt)) {
				
        printf("%i: %c%c%c%c (%s)%s%s\n", fmt.index,
                           fmt.pixelformat >> 0, fmt.pixelformat >> 8,
                           fmt.pixelformat >> 16, fmt.pixelformat >> 24, 
													 fmt.description,
													 (fmt.flags&V4L2_FMT_FLAG_COMPRESSED == fmt.flags)? ", compressed":"",
													 (fmt.flags&V4L2_FMT_FLAG_EMULATED == fmt.flags)? ", emulated":""
													 );
        CLEAR(fmt);
        fmt.index = ++i;
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    }
}


/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
bool utV4l2_Camera::open(void)
{
	cout << "[" << this->dev_name() << "] "  << "OPEN" << endl;
	bool result = _state->do_open(this);
	if (result) {
		change_state(utV4l2Stat_Opened::instance());
	}
	return result;
}


/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/

bool utV4l2_Camera::close(void)
{
	cout << "[" << this->dev_name() << "] "  << "CLOSE" << endl;
	bool result = _state->do_close(this);
	if (result) {
		change_state(utV4l2Stat_Closed::instance());
	}
	return result;
}


/**
 * @brief dtor
 * @param 
 * @return 
 * @note 
*/
utV4l2_Camera::~utV4l2_Camera()
{
	// cout << "~V4l2_Camera" << endl;
}


/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
bool utV4l2_Camera::start(void)
{
	cout << "[" << this->dev_name() << "] " << "START" << endl;

	bool result = _state->do_start(this);
	if (result) {
		change_state(utV4l2Stat_Running::instance());
	}
	return result;
}


/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
bool utV4l2_Camera::stop(void)
{
	cout << "[" << this->dev_name() << "] "<< "STOP" << endl;

	bool result = _state->do_stop(this);
	if (result) {
		change_state(utV4l2Stat_Opened::instance());
	}
	return result;
}
