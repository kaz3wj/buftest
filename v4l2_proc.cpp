#include <iostream>
#include <thread>
#include <stdexcept>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/cuda.hpp"
#include "opencv2/core/core.hpp"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "util_class.h"
#include "util_v4l2.h"
#include "util_v4l2_state.h"
#include "util_v4l2_enum_devices.h"
#include "v4l2_proc.h"
#include "core_test_extn.h"

// Namespace for OpenCV
using namespace cv;

// Namespace for using cout.
using namespace std;



/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
int do_v4l2_proc(utContext* context)
{
    const int32_t ww = context->preview_width();
    const int32_t hh = context->preview_height();
    const bool bUseCUI = context->useCUI();
    const int32_t buffer_count = context->page_count();
    const int32_t timeout = context->timeout();
    const int32_t frame_to_grab = context->grab_count();
    const bool b_use_thread = context->useThread();

    int exitCode = 0;
    vector<utV4l2_Camera*> cameras;

    // vector<utV4l2_State> states;
    // states.push_back(utV4l2Stat_Closed::instance());
    // states.push_back(utV4l2Stat_Opened::instance());
    // states.push_back(utV4l2Stat_Running::instance());

    try
    {
        utv4l2_EnumDevices devices;
        devices.doEnum();

        if (b_use_thread) {
            auto thread_func = [](utV4l2_Camera& camera, const bool& bCUI) 
                                {
                                    std::string name = camera.dev_name();
                                    camera.open();
                                    camera.start();
                                    camera.close();
                                };

            size_t camera_count = min(devices.size(), c_maxCamerasToUse);
            for (size_t ii=0; ii<camera_count; ii++){
                utV4l2_Camera* cam = new utV4l2_Camera(ii, context ); 
                cameras.push_back(cam);
            }

            std::thread cam_control_threads[camera_count];
            for (size_t ii=0; ii<camera_count; ii++) {
                cam_control_threads[ii]= std::thread( thread_func, std::ref(*cameras[ii]),  std::cref(bUseCUI));
            }
            for (size_t ii=0; ii<camera_count; ii++) {
                cam_control_threads[ii].join();
            }
        }
        else {
            // Register to list
            uint32_t dur_create = 0;
            {
                utTimer t("create", true);
                for (size_t ii=0; ii< min( devices.size(), c_maxCamerasToUse); ii++) {
                    // cout << "Add [" << to_string(ii) << "] camera" << endl;
                    cameras.push_back( new utV4l2_Camera(ii, context) );
                }
                dur_create = t.elapsed();
            }

            // initialize
            uint32_t dur_init = 0;
            {
                utTimer t("init", true);
                for (utV4l2_Camera* cam : cameras) {
                    cout << "init CAMERA" << endl;
                    if (!cam->open()) {
                        throw std::runtime_error( "Initialize/Open camera failure.");
                    }
                }
                dur_init = t.elapsed();
            }
        }
    }
    catch (std::runtime_error &e)
    {
        // Error handling
        cerr << "An exception occurred." << endl
        << e.what() << endl;
        exitCode = 1;
    }


    // cleanup
    uint32_t dur_cleanup = 0;
    {
        utTimer t("cleanup", false);
        for (utV4l2_Camera*& cam : cameras) {
            cout << "[" << cam->dev_name() << "] cleanup CAMERA: " << endl;
            delete cam;
        }

        // state class
 #if 1
        release_instance<utV4l2Stat_Closed>();

#else
        utV4l2Stat_Closed * s_closed = utV4l2Stat_Closed::instance();
        if (s_closed) {
            delete s_closed;
        }
        utV4l2Stat_Opened * s_opened = utV4l2Stat_Opened::instance();
        if (s_opened) {
            delete s_opened;
        }
        utV4l2Stat_Running * s_running = utV4l2Stat_Running::instance();
        if (s_running) {
            delete s_running;
        }
#endif



        dur_cleanup = t.elapsed();
    }
    return exitCode;
}