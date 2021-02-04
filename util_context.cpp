#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <signal.h>
#include <string.h>
#include <iostream>
#include <vector>
// // Include files to use the pylon API.
// #include <pylon/PylonIncludes.h>
// #ifdef PYLON_WIN_BUILD
// #    include <pylon/PylonGUI.h>ifndef
// #endif
#include "v4l2_proc.h"
#include "pylon_proc.h"
#include "util_context.h"

using namespace std;
// using namespace Pylon;

bool utContext::g_quit = false;
const char* utContext::optstring = "hc:p:t:l:s:f:m:";

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
utContext::utContext()
{
	set_default();
}

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
void utContext::set_default(void)
{
	_ww = 640;
	_hh = 360;
	// _ww = 1280;
	// _hh = 720;
	_page_count = 4;
	_camera_count = 2;
	_timeout = 5000;
	_frame_to_grab = 300;
	_cam_pixfmt = V4L2_PIX_FMT_NV16;
	_buf_memory_type = V4L2_MEMORY_USERPTR;

	b_use_pylon = true;
	b_use_thread = true;
	b_use_osd = true;
	b_output_list = false;
	b_use_cui = false;
	// using utImageHandler
	b_use_utImageEventHandler = false;
	b_debug = false;
	b_verbose = false;
	b_dryrun = false;
}


/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
utContext *utContext::_pseudoThis = NULL;


/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
utContext * utContext::get_instance()
{
	if (!_pseudoThis) {
		_pseudoThis = new utContext;
	}
	return _pseudoThis;
}


/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
int utContext::proc(void)
{
	int exitCode = -1;

	if (b_use_pylon) {
		exitCode = do_pylon_proc(this);
	}
	else {
		exitCode = do_v4l2_proc(this);
	}
	return exitCode;
}


/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/

bool utContext::parse_args(int argc, char *argv[])
{
	int long_index = 0;
	
	opterr = 0;

	// The exit code of the sample application.
	int exitCode = 0;

	if (argc<2) {
			print_usage(argv[0] );
	}
	int option_index = 0;

	int c;
	while((c=getopt_long(argc,argv,optstring,longopts,&long_index)) != -1)
	{
			switch(c) {
			case 0:
					if (longopts[option_index].flag != 0) {
							break;
					}
					printf("option %s", longopts[option_index].name);
					if (optarg) {
							printf(" with arg %s", optarg);
					}
					printf("\n");
					break;

			case 'h':
				print_usage(argv[0]);
				return false;
				break;

			case 'c':
				_camera_count = stol(optarg);
				std::cout << "camera_count=" << to_string(_camera_count) << endl;
				break;

			case 'p':
					_page_count = stol(optarg);
					std::cout << "page=" << to_string(_page_count) << endl;
					break;

			case 't':
					_timeout = stol(optarg);
					std::cout << "poll_timeout=" << to_string(_timeout) << endl;
					break;

			case 'l':
					_frame_to_grab = stol(optarg);
					std::cout << "frame_to_grab=" << to_string(_frame_to_grab) << endl;
					break;

			case 's':
					if (sscanf(optarg, "%dx%d",&_ww, &_hh) != 2){
							print_usage(argv[0]);
							return false;
					}
					else {
						std::cout << "camera_size=" << to_string(_ww)<<"x"<<to_string(_hh) << endl;
					}
					break;

			case 'f':
					if (strcmp(optarg, "YUYV") == 0)
							_cam_pixfmt = V4L2_PIX_FMT_YUYV;
					else if (strcmp(optarg, "YVYU") == 0)
							_cam_pixfmt = V4L2_PIX_FMT_YVYU;
					else if (strcmp(optarg, "VYUY") == 0)
							_cam_pixfmt = V4L2_PIX_FMT_VYUY;
					else if (strcmp(optarg, "UYVY") == 0)
							_cam_pixfmt = V4L2_PIX_FMT_UYVY;
					else if (strcmp(optarg, "GREY") == 0)
							_cam_pixfmt = V4L2_PIX_FMT_GREY;
					else if (strcmp(optarg, "MJPEG") == 0)
							_cam_pixfmt = V4L2_PIX_FMT_MJPEG;
					else if (strcmp(optarg, "NV16") == 0)
							_cam_pixfmt = V4L2_PIX_FMT_NV16;
					else if (strcmp(optarg, "NV12") == 0)
							_cam_pixfmt = V4L2_PIX_FMT_NV12;
					else
					{
							print_usage(argv[0]);
							return false;
					}
					break;

			case 'm':
				if (strcmp(optarg,"USER")==0) {
					_buf_memory_type = V4L2_MEMORY_USERPTR;
				}
				else if (strcmp(optarg,"DMA")==0) {
					_buf_memory_type = V4L2_MEMORY_DMABUF;
				}
				else if (strcmp(optarg,"MMAP")==0) {
					_buf_memory_type = V4L2_MEMORY_MMAP;
				}
				break;

			default:
					if (longopts->has_arg == required_argument){
							std::cout << "optarg=" << optarg;
					}
					if (longopts->flag){
							std::cout << "*flag=" << to_string(*longopts->flag);
					}
					std::cout << endl;
					break;
			}
	}

	while(optind < argc) {
			printf ("non-option ARGV-elements: ");
			while (optind < argc)
					printf ("%s ", argv[optind++]);
			printf ("\n");
	}

  b_dryrun = (1==_flag_dryrun);
	b_use_pylon = (1==_flag_pylon);
	b_use_thread = (1==_flag_thread);
	b_use_osd = (1==_flag_osd);
	// b_output_list = (1==_flag_list);
	b_use_cui = (1 == _flag_cui);
	b_use_utImageEventHandler = (1 == _flag_imagehandler);
	b_debug = (1==_flag_debug);
	b_verbose = (1==_flag_verbose);

	if (b_debug) {
			cout << "[DEBUG]" << endl;
	}
	return true;
}

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
void utContext::print_usage(const char* name)
{
    cout << "Usage: " << name << "[mode] [threading] [options]" << endl;

    std::vector<std::string> usage_list = 
    {
        "--help         : this message",
        "",
        "mode:",
        "  --pylon      : use Pylon framework(default)",
        "  --v4l2       : use V4L2 framework",
				"  --dryrun     : dry-run mode",
        "",
        "user interface:",
        "  --cui        : use console",
        "  --gui        : use GUI(window)",
        "",
        "buffer:",
        "  --page       : buffer page count",
        "  --dmabuf     : V4L2_MEMORY_DMABUF",
        "  --userptr    : V4L2_MEMORY_USERPTR",
        "  --mmap       : V4L2_MEMORY_MMAP",
        "",
        "threading:",
        "  --multi      : enable multi thread mode(default)",
        "  --single     : enable single thread mode",
        "",
				"",
        "options:",
        "  --osd        : enable OSD on the window",
        "  --ihandler   : use ImageHandler",
        "  --max-count  : max camera device count",
        "  --list       : display device list", 

				"  -c [camera_count]",
				"  -p [pages]",
				"  -t [poll timeout] : poll-timeout in msec",
				"  -l [frame_to_grab] : frame_to_grab",
        "  -s [w]x[h]         : preview size in pixel",
				"  -f [pixel format]  : YUYV, YVYU, VYUY, UYVY,"
				"                       GREY, MJPEG, NV16, NV12"
				"  -m [buffer mode]   : USER, DMA, MMAP"
    };

    for (std::string s : usage_list) {
        cout << s << endl;
    }
    exit(0);
}

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
void utContext::print_args(int argc, char* argv[]) 
{
	for (int i=0; i<argc; i++) {
			cout << "arg[" << to_string(i) << "] " << argv[i] << endl;
	}
}
