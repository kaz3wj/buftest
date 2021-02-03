#ifndef __UTIL_V4L2_ENUM_DEVICES__H__INCLUDED__
#define __UTIL_V4L2_ENUM_DEVICES__H__INCLUDED__

//////////////////////////////////////////////

typedef void (*UTV4L2_ENUMDEVICE_CB)(struct video_capability &cap,
                 struct video_window &win,
                 struct video_picture &picture);

//////////////////////////////////////////////

class utv4l2_EnumDevices
{
    public:
			utv4l2_EnumDevices();

		public:
        void doEnum(UTV4L2_ENUMDEVICE_CB cb = NULL );
        size_t size() { return _cameras; }

    public:
			const int32_t _max_capture_devices = 64;
			
			int32_t _cameras;
};

#endif //!__UTIL_V4L2_ENUM_DEVICES__H__INCLUDED__
