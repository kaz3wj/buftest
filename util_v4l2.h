#ifndef __UTIL_V4L2__H__INCLUDED__
#define __UTIL_V4L2__H__INCLUDED__

#include <linux/videodev2.h>
#include <string>
#include <vector>

class utV4l2_State;
class utV4l2Stat_Closed;
class utV4l2Stat_Opened;
class utV4l2Stat_Running;
class utContext;


//////////////////////////////////////////////

class utV4l2_Camera
{
    public:
        utV4l2_Camera(int32_t dev_video_no, utContext* context);
        virtual ~utV4l2_Camera();

    // state
    private:
        friend class utV4l2_State;
        friend class utV4l2Stat_Closed;
        friend class utV4l2Stat_Opened;
        friend class utV4l2Stat_Running;

        void change_state(utV4l2_State *s) { _state = s; }

    private:
        utV4l2_State* _state;

    // Operation
    public:
        bool open(void);
        bool close(void);
        bool start(void);
        bool stop(void);

    // Attributes
    public:
        std::string dev_name();
        inline int width() { return _width; }
        inline int height() { return _height; }
        inline bool isTermAcquired() { return _bExit;}

    private:
        static int xioctl(int fh, int request, void *arg);
        static void list_formats(int fh);
        static std::string error_text(const char* prefix);
        static std::string fourcc_text(uint32_t code);

    private:
        utContext* _ctx_param;

        enum v4l2_buf_type _type;

        int _fd;
        int32_t _width;
        int32_t _height;

        int32_t _dev_no;
        int32_t _page_count;
        //
        int32_t _pushed;
        int32_t _popped;
        // 
        bool _bExit;
        bool _debug_push_pop;
        //
        // int32_t _poll_timeout;
        // int32_t _loop_count;

        std::vector<uint8_t*> _buf;
};

#endif //!__UTIL_V4L2__H__INCLUDED__
