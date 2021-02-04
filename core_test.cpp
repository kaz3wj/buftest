// Grab_MultipleCameras.cpp
/*
    Note: Before getting started, Basler recommends reading the "Programmer's Guide" topic
    in the pylon C++ API documentation delivered with pylon.
    If you are upgrading to a higher major version of pylon, Basler also
    strongly recommends reading the "Migrating from Previous Versions" topic in the pylon C++ API documentation.

    This sample illustrates how to grab and process images from multiple cameras
    using the CInstantCameraArray class. The CInstantCameraArray class represents
    an array of instant camera objects. It provides almost the same interface
    as the instant camera for grabbing.
    The main purpose of the CInstantCameraArray is to simplify waiting for images and
    camera events of multiple cameras in one thread. This is done by providing a single
    RetrieveResult method for all cameras in the array.
    Alternatively, the grabbing can be started using the internal grab loop threads
    of all cameras in the CInstantCameraArray. The grabbed images can then be processed by one or more
    image event handlers. Please note that this is not shown in this example.
*/

//#define   _MY_DEBUG_ARGS
//#define   _MY_USE_CV_CUDA

#include <thread>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <poll.h>

#include "util_v4l2.h"
#include "util_class.h"
#include "util_context.h"
#include "core_test.h"

// Namespace for using cout.
using namespace std;


// Limits the amount of cameras used for grabbing.
// It is important to manage the available bandwidth when grabbing with multiple cameras.
// This applies, for instance, if two GigE cameras are connected to the same network adapter via a switch.
// To manage the bandwidth, the GevSCPD interpacket delay parameter and the GevSCFTD transmission delay
// parameter can be set for each GigE camera device.
// The "Controlling Packet Transmission Timing with the Interpacket and Frame Transmission Delays on Basler GigE Vision Cameras"
// Application Notes (AW000649xx000)
// provide more information about this topic.
// The bandwidth used by a FireWire camera device can be limited by adjusting the packet size.
size_t c_maxCamerasToUse = 2;

// bool g_quit = false;

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/

static void
signal_handle(int signum)
{
    std::cout << UTCOL_MAGENTA
          << "Quit due to exit command from user!"
          << UTCOL_WHITE << endl;

    utContext::g_quit = true;
}

utContext *theCtx = NULL;

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
int main(int argc, char* argv[])
{
    // Register a shuwdown handler to ensure
    //  a clean shutdown if user types <ctrl+c>
    struct sigaction sig_action;
    CLEAR(sig_action);
    sig_action.sa_handler = signal_handle;
    sigemptyset(&sig_action.sa_mask);
    sig_action.sa_flags = 0;
    sigaction(SIGINT, &sig_action, NULL);

    int exitCode = -1;
    theCtx = utContext::get_instance();
    if (theCtx) {
        if (theCtx->parse_args(argc, argv)){
            // The exit code of the sample application.
            exitCode = theCtx->proc();
        }
        delete theCtx;
    }
    cout << "Completed successfully." << endl;
    return exitCode;
}
