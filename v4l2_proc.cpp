#include <iostream>
#include <thread>
// Include files to use the pylon API.
#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>ifndef
#endif

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/cuda.hpp"
#include "opencv2/core/core.hpp"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "util_class.h"
#include "util_v4l2.h"
#include "v4l2_proc.h"
#include "core_test_extref.h"

// Namespace for using pylon objects.
using namespace Pylon;

// Namespace for OpenCV
using namespace cv;

// Namespace for using cout.
using namespace std;

// Number of images to be grabbed.
const uint32_t c_countOfFrameToGrab = 10000;

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
    std::vector<utV4l2_Camera*> cameras;
    // std::vector<std::string> caption_for_im;

    try
    {
        CTlFactory& tlFactory = CTlFactory::GetInstance();
        DeviceInfoList_t devices;
        if ( tlFactory.EnumerateDevices(devices) == 0 ){
            throw RUNTIME_EXCEPTION( "No camera present.");
        }

        if (b_use_thread) {

            auto thread_func = [](utV4l2_Camera& camera, const bool& bCUI) 
                                {
                                    std::string name = camera.dev_name();
                                        camera.open();
                                        camera.start();
                                        // camera->stop();
                                        camera.close();
                                };

            size_t camera_count = min(devices.size(), c_maxCamerasToUse);
            for (size_t ii=0; ii<camera_count; ii++){
                utV4l2_Camera* cam = new utV4l2_Camera(ii, ww, hh, buffer_count, timeout, frame_to_grab);
                cameras.push_back(cam);
            }

            std::thread cam_control_threads[camera_count];
            for (size_t ii=0; ii<camera_count; ii++) {
                // caption_for_im.push_back( cameras[ii]->dev_name() );
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
                    cameras.push_back(new utV4l2_Camera(ii, ww, hh, buffer_count) );
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
                        throw RUNTIME_EXCEPTION( "Initialize/Open camera failure.");
                    }
                }
                dur_init = t.elapsed();
            }
        }
    }
    catch (const GenericException &e)
    {
        // Error handling
        cerr << "An exception occurred." << endl
        << e.GetDescription() << endl;
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
        dur_cleanup = t.elapsed();
    }
    return exitCode;
}