#ifndef __UTIL_V4L2_ENUM_DEVICES__H__INCLUDED__
#define __UTIL_V4L2_ENUM_DEVICES__H__INCLUDED__

#include <linux/videodev2.h>

//////////////////////////////////////////////

typedef void (*UTV4L2_ENUMDEVICE_CB)(int device_no, struct v4l2_capability &cap, void* userdata);

//////////////////////////////////////////////

class utv4l2_EnumDevices
{
		public:
        void doEnum(UTV4L2_ENUMDEVICE_CB cb = NULL, void* userdata=NULL );
        size_t size() { return _cameras; }

    public:
			const int32_t _max_capture_devices = 64;

		public:
			int32_t _cameras;
};

#endif //!__UTIL_V4L2_ENUM_DEVICES__H__INCLUDED__
