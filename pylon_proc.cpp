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

#include "core_test_defs.h"
#include "core_test_extref.h"

#include "util_v4l2.h"
#include "util_class.h"
#include "v4l2_proc.h"
#include "pylon_proc.h"

// Namespace for OpenCV
using namespace cv;

// Namespace for using pylon objects.
using namespace Pylon;

// Namespace for using cout.
using namespace std;



//Example of an image event handler.
class utImageEventHandler : public CImageEventHandler
{
public:
    virtual void OnImageGrabbed( CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult)
    {
#ifdef PYLON_WIN_BUILD
        // Display the image
        Pylon::DisplayImage(1, ptrGrabResult);
#endif
        cout << "utImageEventHandler::OnImageGrabbed called." << std::endl;
    }
public:
    void set_index(uint32_t index)  {
        _index = index;
    }

    uint32_t _index;
};


static void disp_osd( 
    cv::Mat cv_dst, 
    const Scalar& color_osd_background,
    CGrabResultPtr ptrGrabResult, 
    const int32_t& frameToComplete, 
    const uint32_t& dur_convert, 
    const uint32_t& dur_resize, 
    const uint32_t& dur_togray, 
    const int32_t& frame_rate );


/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
int do_pylon_proc( utContext *context )
{
    const int32_t ww = context->preview_width();
    const int32_t hh = context->preview_height();
    const bool bUseCUI = context->useCUI();
    const bool b_use_osd = context->useOSD();
    const bool b_use_utImageEventHandler = context->useImageEventHandler();
    const int32_t frame_to_grab = context->grab_count();

    int exitCode = 0;

    try
    {
        // Get the transport layer factory.
        CTlFactory& tlFactory = CTlFactory::GetInstance();

        // Get all attached devices and exit application if no device is found.
        DeviceInfoList_t devices;

        if ( tlFactory.EnumerateDevices(devices) == 0 )
        {
            throw RUNTIME_EXCEPTION( "No camera present.");
        }

        // Create an array of instant cameras for the found devices and avoid exceeding a maximum number of devices.
        CInstantCameraArray cameras( min( devices.size(), c_maxCamerasToUse));

        std::string caption_for_im[4];
        utTimer proc_timer[4];

        if (context->useThread() /*b_use_thread*/) {

            auto thread_func = [](CInstantCamera& camera, 
                                const std::string& caption,
                                const int32_t& grab_count, 
                                const int& w, 
                                const int& h, 
                                const size_t& index, 
                                const bool& bDispInfo, 
                                const bool& bCUI
                                ) 
            {
                namedWindow(caption , WINDOW_NORMAL);
                resizeWindow(caption , w, h);

                camera.StartGrabbing();

                CGrabResultPtr ptrGrabResult;
                CPylonImage image;
                CImageFormatConverter fc;
                fc.OutputPixelFormat = PixelType_BGR8packed;

                GenApi::CIntegerPtr width( camera.GetNodeMap().GetNode("Width"));
                GenApi::CIntegerPtr height( camera.GetNodeMap().GetNode("Height"));

                cv::Mat cv_img( width->GetValue(), height->GetValue(), CV_8UC3);
#ifdef _MY_USE_CV_CUDA
                cv::cuda::GpuMat cv_cuda_img( width->GetValue(), height->GetValue(), CV_8UC3);
#endif
                uint32_t frame_count = 0;
                utTimer proc_timer;
                intptr_t cameraContextValue = -1;
                bool bExitLoop = false;

                for( int32_t i=0; i < grab_count && camera.IsGrabbing() && !bExitLoop; ++i)
                {
                    camera.RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_ThrowException);

                    if (ptrGrabResult->GrabSucceeded()) 
                    {
                        cameraContextValue = ptrGrabResult->GetCameraContext();
                        uint32_t dur_convert = 0;
                        {
                            utTimer t("fc.Convert()", false);
                            fc.Convert(image, ptrGrabResult);
                            dur_convert = t.elapsed();
                        }

                        cv_img = cv::Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uint8_t *) image.GetBuffer());
#ifdef _MY_USE_CV_CUDA
                        cv_cuda_img = cv::cuda::GpuMat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uint8_t *) image.GetBuffer());
#endif

                        cv::Mat cv_dst;
#ifdef _MY_USE_CV_CUDA
                        cv::cuda::GpuMat cv_cuda_dst;
#endif
                        uint32_t dur_resize = 0;
                        {
                            utTimer t("resize", false);
                            resize(cv_img, cv_dst, cv::Size(w,h));
                            dur_resize = t.elapsed();
                        }

                        uint32_t dur_togray = 0;
                        if ( 0 == cameraContextValue) {
                            utTimer t("RGBtoGray", false);
                            cvtColor(cv_dst, cv_dst, cv::COLOR_BGR2GRAY );
                            dur_togray = t.elapsed();
                        }

                        frame_count++;

                        if (bDispInfo) {
                            int32_t frameToComplete = grab_count - i;
                            int32_t frame_rate = (int32_t) (1000.0 / ((float)(proc_timer.elapsed() / (float)frame_count)));
                            disp_osd( cv_dst, CV_RGB(0x09,0x5E,0x20), ptrGrabResult, frameToComplete, dur_convert, dur_resize, dur_togray, frame_rate );
                        }
                        imshow(caption, cv_dst);
                        bExitLoop = (waitKey(1) == CHAR_ESC);
                    }
                }
                camera.StopGrabbing();
                cout << "Terminated: " << caption<< std::endl;                                cout << "thread_func" << endl;
            };

            std::vector<std::thread> cam_control_threads;

            // Create and attach all Pylon Devices.
            for ( size_t i = 0; i < cameras.GetSize(); ++i) {
                cameras[i].Attach( tlFactory.CreateDevice( devices[i]));
                if (b_use_utImageEventHandler) {
                    utImageEventHandler* pHandler = new utImageEventHandler;
                    pHandler->set_index(i);
                    cameras[i].RegisterImageEventHandler(pHandler, RegistrationMode_Append, Cleanup_Delete );
                }
                caption_for_im[i] = "MultiThread [" + std::to_string(i+1) + "] ";
                caption_for_im[i] += cameras[i].GetDeviceInfo().GetModelName() + " S/N:" + cameras[i].GetDeviceInfo().GetSerialNumber();
                cam_control_threads.push_back(std::thread( 
                                                thread_func, 
                                                std::ref(cameras[i]), 
                                                std::cref(caption_for_im[i]), 
                                                std::cref(frame_to_grab),
                                                std::cref(ww), std::cref(hh), 
                                                std::cref(i),
                                                std::cref(b_use_osd),
                                                std::cref(bUseCUI) 
                                            ));
            }
            // Join 
            for ( std::thread& th : cam_control_threads ) {
                th.join();
            }
        }
        else {
            // // Create and attach all Pylon Devices.  
            std::vector<uint32_t> frame_counts;

            for ( size_t i = 0; i < cameras.GetSize(); ++i) {
                frame_counts.push_back(0);
                cameras[i].Attach( tlFactory.CreateDevice(devices[i]));
                caption_for_im[i] = "SingleThread [" + std::to_string(i+1) + "]";
                caption_for_im[i] += cameras[i].GetDeviceInfo().GetModelName() + " S/N:" + cameras[i].GetDeviceInfo().GetSerialNumber();
                namedWindow( caption_for_im[i] , WINDOW_NORMAL);
                resizeWindow(caption_for_im[i] , ww, hh);
            }
            cameras.StartGrabbing();
            // This smart pointer will receive the grab result data.
            CGrabResultPtr ptrGrabResult;

            CPylonImage image;
            CImageFormatConverter fc;
            fc.OutputPixelFormat = PixelType_BGR8packed;

            GenApi::CIntegerPtr width( cameras[0].GetNodeMap().GetNode("Wi, str::ref(b_use_cui)dth"));
            GenApi::CIntegerPtr height( cameras[0].GetNodeMap().GetNode("Height"));

            Mat cv_img( width->GetValue(), height->GetValue(), CV_8UC3);
#ifdef _MY_USE_CV_CUDA
            cv::cuda::GpuMat cv_cuda_img( width->GetValue(), height->GetValue(), CV_8UC3);
#endif
            // Grab frame_to_grab from the cameras.
            bool bExitLoop = false;
            uint64_t timestamp = 0;

            for( uint32_t i = 0; i < frame_to_grab && cameras.IsGrabbing() && !bExitLoop; ++i)
            {
                cameras.RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_ThrowException);

                if (ptrGrabResult->GrabSucceeded()) 
                {
                    const uint8_t *pImageBuffer = (uint8_t *) ptrGrabResult->GetBuffer();

                    // When the cameras in the array are created the camera context value
                    // is set to the index of the camera in the array.
                    // The camera context is a user settable value., 
                    // This value is attached to each grab result and can be used
                    // to determine the camera that produced the grab result.
                    intptr_t cameraContextValue = ptrGrabResult->GetCameraContext();
                    uint32_t dur_convert = 0;
                    {
                        utTimer t("fc.Convert()", false);
                        fc.Convert(image, ptrGrabResult);
                        dur_convert = t.elapsed();
                    }

                    cv_img = Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uint8_t *) image.GetBuffer());
                    cv::Mat cv_dst;
#ifdef _MY_USE_CV_CUDA
                    cv_cuda_img = cv::cuda::GpuMat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uint8_t *) image.GetBuffer());
                    cv::cuda::GpuMat cv_cuda_dst;
#endif
                    uint32_t dur_resize = 0;
                    {
                        utTimer t("resize", false);
                        resize(cv_img, cv_dst, cv::Size(ww,hh));
                        dur_resize = t.elapsed();
                    }

                    uint32_t dur_togray = 0;
                    if (0 == cameraContextValue) {
                        utTimer t("RGBtoGray", false);
                        cvtColor(cv_dst, cv_dst, cv::COLOR_BGR2GRAY );
                        dur_togray = t.elapsed();
                    }
                    frame_counts[cameraContextValue]++;

                    if (b_use_osd) {
                        uint32_t frameToComplete = frame_to_grab - i;
                        uint32_t frame_rate = (uint32_t) (1000.0 / ((float)(proc_timer[cameraContextValue].elapsed() / (float)frame_counts[cameraContextValue])));
                        disp_osd( cv_dst, CV_RGB(0x09,0x5E,0xFF), ptrGrabResult, frameToComplete, dur_convert, dur_resize, dur_togray, frame_rate );
                    }
                    imshow(caption_for_im[cameraContextValue], cv_dst);
                    bExitLoop = (waitKey(1) == CHAR_ESC);
                }
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

 
    return exitCode;
}


/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/

static void disp_osd( 
    cv::Mat cv_dst, 
    const Scalar& color_osd_background,
    CGrabResultPtr ptrGrabResult, 
    const int32_t& frameToComplete, 
    const uint32_t& dur_convert, 
    const uint32_t& dur_resize, 
    const uint32_t& dur_togray, 
    const int32_t& frame_rate )
{
    int32_t sx = 50;
    int32_t offs_y = 0;
    int32_t y_step = 15;
    std::string s;

    cv::rectangle( cv_dst, Rect(sx, offs_y, 200, 150), color_osd_background, -1 /*means FILL*/);

    uint64_t timestamp = ptrGrabResult->GetTimeStamp();
    s = "time: " + to_string(timestamp);
    putText(cv_dst, s, Point(sx,offs_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255,255,255), 1.0);
    offs_y += y_step;

    s = "frames to stop: " + to_string(frameToComplete);
    putText(cv_dst, s, Point(sx,offs_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255,255,255), 1.0);
    offs_y += y_step;

    s = "size: " + to_string(ptrGrabResult->GetWidth()) + "x" + to_string( ptrGrabResult->GetHeight());
    putText(cv_dst, s, Point(sx,offs_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255,255,255), 1.0);
    offs_y += y_step;

    s = "convert: " + to_string(dur_convert) + "ms";
    putText(cv_dst, s, Point(sx,offs_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255,255,255), 1.0);
    offs_y += y_step;

    s = "resize: " + to_string(dur_resize) + "ms";
    putText(cv_dst, s, Point(sx,offs_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255,255,255), 1.0);
    offs_y += y_step;

    s = "to_gray: " + to_string(dur_togray) + "ms";
    putText(cv_dst, s, Point(sx,offs_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255,255,255), 1.0);
    offs_y += y_step;

    uint32_t dur_total = dur_convert+dur_resize+dur_togray;

    s = "TOTAL: " + to_string(dur_total) + "ms, " + to_string(frame_rate) + "fps";
    putText(cv_dst, s, Point(sx,offs_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255,255,255), 1.0);
    offs_y += y_step;
}
