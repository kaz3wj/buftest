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
#include "core_test_extn.h"

#include "util_v4l2.h"
#include "util_class.h"
#include "util_context.h"
#include "v4l2_proc.h"
#include "util_v4l2_state.h"
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
    utContext* ctx,
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
    const bool bUseCUI = context->useCUI();
    const bool b_use_osd = context->useOSD();
    const bool b_use_utImageEventHandler = context->useImageEventHandler();
    const int32_t frame_to_grab = context->grab_count();

    cout << "grab_count: " << to_string(frame_to_grab) << endl;

    int exitCode = 0;

	// Before using any pylon methods, 
	// the pylon runtime must be initialized. 
	PylonInitialize();

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

        if (context->useThread()) {

            auto thread_func = [](CInstantCamera &camera,
                                  const std::string &caption,
                                  const size_t &index,
                                  utContext *context
                                ) 
                            {

                cout << UTCOL_CYAN << caption << endl;

                int32_t grab_count = context->grab_count();


                // cout << "Start Grabbing" << endl;
                camera.StartGrabbing();

                if (!camera.IsGrabbing()) {
                    cout << UTCOL_RED << "FAILED: Grabbing" << endl;
                }

                CGrabResultPtr ptrGrabResult;
                CPylonImage image;
                CImageFormatConverter fc;
                fc.OutputPixelFormat = PixelType_BGR8packed;

                GenApi::CIntegerPtr width( camera.GetNodeMap().GetNode("Width"));
                GenApi::CIntegerPtr height( camera.GetNodeMap().GetNode("Height"));

                // if (context->preview_width() != width->GetValue()) {
                //     cout << UTCOL_MAGENTA << "Not mathced: ctx.width=" << to_string(context->preview_width()) << ", Gen.Width()=" << to_string(width->GetValue()) << endl;
                // }

                cv::Mat cv_img( width->GetValue(), height->GetValue(), CV_8UC3);
#ifdef _MY_USE_CV_CUDA
                cv::cuda::GpuMat cv_cuda_img( width->GetValue(), height->GetValue(), CV_8UC3);
#endif
                uint32_t frame_count = 0;
                utTimer proc_timer;
                intptr_t cameraContextValue = -1;

                // cout << UTCOL_YELLOW << "BEFORE LOOP" << endl;

                for( int32_t i=0; (i<grab_count || -1==grab_count) && camera.IsGrabbing() && !utContext::g_quit; ++i)
                {
                    camera.RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_ThrowException);

                    if (ptrGrabResult->GrabSucceeded())   {
                        // cout << UTCOL_BLUE << "Grab: " << to_string(ptrGrabResult->GetHeight()) << "x"<< to_string(ptrGrabResult->GetWidth()) << endl;

                        const uint8_t *pImageBuffer = (uint8_t *) ptrGrabResult->GetBuffer();

                        cameraContextValue = ptrGrabResult->GetCameraContext();
                        uint32_t dur_convert = 0;
                        {
                            utTimer t("fc.Convert()", false);
                            fc.Convert(image, ptrGrabResult);
                            dur_convert = t.elapsed();
                        }

                        cv_img = cv::Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uint8_t *) image.GetBuffer());
                        cv::Mat cv_dst;
                        uint32_t dur_resize = 0;
                        {
                            utTimer t("resize", false);
                            resize(cv_img, cv_dst, cv::Size(context->preview_width(), context->preview_height()));
                            dur_resize = t.elapsed();
                        }

                        uint32_t dur_togray = 0;
                        // if ( 0 == cameraContextValue)
                        // {
                        //     utTimer t("RGBtoGray", false);
                        //     cvtColor(cv_dst, cv_dst, cv::COLOR_BGR2GRAY );
                        //     dur_togray = t.elapsed();
                        // }

                        frame_count++;

                        if (context->useOSD())
                        {


                            int32_t frameToComplete = grab_count - i;
                            int32_t frame_rate = (int32_t) (1000.0 / ((float)(proc_timer.elapsed() / (float)frame_count)));
                            disp_osd( context, cv_dst, CV_RGB(0x09,0x5E,0x20), ptrGrabResult, frameToComplete, dur_convert, dur_resize, dur_togray, frame_rate );
                        }
                        imshow(caption, cv_dst);
                        waitKey(1);
                    }
                    else {
                        cout << UTCOL_RED << "Fail: Grab" << endl;
                    }
                }
                camera.StopGrabbing();
                cout << "Terminated: " << caption<< std::endl;
            };

            std::vector<std::thread> cam_control_threads;

            // Create and attach all Pylon Devices.
            for ( size_t i = 0; i < cameras.GetSize(); ++i) {
                cameras[i].Attach( tlFactory.CreateDevice( devices[i]));

                // if (b_use_utImageEventHandler) {
                //     utImageEventHandler* pHandler = new utImageEventHandler;
                //     pHandler->set_index(i);
                //     cameras[i].RegisterImageEventHandler(pHandler, RegistrationMode_Append, Cleanup_Delete );
                // }

                caption_for_im[i] = "MultiThread [" + std::to_string(i+1) + "] ";
                caption_for_im[i] += cameras[i].GetDeviceInfo().GetModelName() + " S/N:" + cameras[i].GetDeviceInfo().GetSerialNumber();
                caption_for_im[i] += " " + to_string(context->preview_width()) + "x" + to_string(context->preview_height());

                namedWindow(caption_for_im[i], WINDOW_NORMAL);
                resizeWindow(caption_for_im[i], context->preview_width(), context->preview_height());

                cam_control_threads.push_back(std::thread( 
                                                thread_func, 
                                                std::ref(cameras[i]), 
                                                std::cref(caption_for_im[i]), 
                                                std::cref(i),
                                                std::cref(context)
                                            ));
            }
            // Join
            for (std::thread &th : cam_control_threads){
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
                resizeWindow(caption_for_im[i] , context->preview_width(), context->preview_height());
            }

            cameras.StartGrabbing();
            // This smart pointer will receive the grab result data.
            CGrabResultPtr ptrGrabResult;

            CPylonImage image;
            CImageFormatConverter fc;
            fc.OutputPixelFormat = PixelType_BGR8packed;

            GenApi::CIntegerPtr width( cameras[0].GetNodeMap().GetNode("Width"));
            GenApi::CIntegerPtr height( cameras[0].GetNodeMap().GetNode("Height"));

            Mat cv_img( width->GetValue(), height->GetValue(), CV_8UC3);

            // Grab frame_to_grab from the cameras.
            for( uint32_t i = 0; i < frame_to_grab && cameras.IsGrabbing() && !utContext::g_quit; ++i)
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
                    uint32_t dur_resize = 0;
                    {
                        utTimer t("resize", false);
                        resize(cv_img, cv_dst, cv::Size(context->preview_width(),context->preview_height()));
                        dur_resize = t.elapsed();
                    }

                    uint32_t dur_togray = 0;
                    // if (0 == cameraContextValue) {
                    //     utTimer t("RGBtoGray", false);
                    //     cvtColor(cv_dst, cv_dst, cv::COLOR_BGR2GRAY );
                    //     dur_togray = t.elapsed();
                    // }
                    frame_counts[cameraContextValue]++;

                    if (b_use_osd) {
                        uint32_t frameToComplete = frame_to_grab - i;
                        uint32_t frame_rate = (uint32_t) (1000.0 / ((float)(proc_timer[cameraContextValue].elapsed() / (float)frame_counts[cameraContextValue])));
                        disp_osd( context, cv_dst, CV_RGB(0x09,0x5E,0xFF), ptrGrabResult, frameToComplete, dur_convert, dur_resize, dur_togray, frame_rate );
                    }
                    imshow(caption_for_im[cameraContextValue], cv_dst);
                    waitKey(1);
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

 	// Releases all pylon resources. 
	PylonTerminate();

    return exitCode;
}


/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/

static void disp_osd( 
    utContext* ctx,
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


    cv::Mat overlay = cv_dst(cv::Rect(sx, offs_y, 200, 100));
    cv_dst.copyTo(overlay);

    cv::rectangle( overlay, Rect(sx, offs_y, 200, 100), color_osd_background, -1 /*means FILL*/);

    uint64_t timestamp = ptrGrabResult->GetTimeStamp();
    s = "time: " + to_string(timestamp);
    putText(overlay, s, Point(sx,offs_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255,255,255), 1.0);
    offs_y += y_step;

    s = "frames to stop: " + to_string(frameToComplete);
    putText(overlay, s, Point(sx,offs_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255,255,255), 1.0);
    offs_y += y_step;

    s = "size: " + to_string(ptrGrabResult->GetWidth()) + "x" + to_string( ptrGrabResult->GetHeight());
    putText(overlay, s, Point(sx,offs_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255,255,255), 1.0);
    offs_y += y_step;

    s = "convert: " + to_string(dur_convert) + "ms";
    putText(overlay, s, Point(sx,offs_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255,255,255), 1.0);
    offs_y += y_step;

    s = "resize: " + to_string(dur_resize) + "ms";
    putText(overlay, s, Point(sx,offs_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255,255,255), 1.0);
    offs_y += y_step;

    s = "to_gray: " + to_string(dur_togray) + "ms";
    putText(overlay, s, Point(sx,offs_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255,255,255), 1.0);
    offs_y += y_step;

    uint32_t dur_total = dur_convert+dur_resize+dur_togray;

    s = "TOTAL: " + to_string(dur_total) + "ms, " + to_string(frame_rate) + "fps";
    putText(overlay, s, Point(sx,offs_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255,255,255), 1.0);
    offs_y += y_step;

    double alpha = 0.7;
    cv::addWeighted(overlay, alpha, cv_dst, 1.0 - alpha , 0.0, cv_dst);

    int32_t ind_x = (abs(frameToComplete)* (ctx->preview_width()/10)) % ctx->preview_width();
    cv::line(cv_dst, Point(ind_x, 0), Point(ind_x, ctx->preview_height()), CV_RGB(0x0,0xFF,0x0), 1, 4 );    
}
