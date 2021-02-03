/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/

#include <string>
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
#include "util_class.h"
#include "util_v4l2_enum_devices.h"

using namespace std;

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/

utv4l2_EnumDevices::utv4l2_EnumDevices()
{
	_cameras = 0;
}


/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/

void utv4l2_EnumDevices::doEnum(UTV4L2_ENUMDEVICE_CB cb)
{
    int fd;
    struct v4l2_capability video_cap;
    // struct video_window     video_win;
    // struct video_picture   video_pic;

		for (int ii = 0; ii < _max_capture_devices; ii++) 
		{
			string device_name = "/dev/video" + to_string(ii);

			if((fd = open( device_name.c_str(), O_RDONLY)) == -1){
					cout << UTCOL_RED << "cam_info: Can't open device" << endl;
					break;
			}
			_cameras++;

			if(ioctl(fd, VIDIOC_QUERYCAP, &video_cap) == -1){
					cout << "cam_info: Can't get capabilities";
			}
			else {
					cout << "driver: '" << video_cap.driver << "'" << endl;
					cout << "card  : '" << video_cap.card << "'" << endl;
			}

			// if(ioctl(fd, VIDIOCGWIN, &video_win) == -1)
			// 		perror("cam_info: Can't get window information");
			// else
			// 		printf("Current size:\t%d x %d\n", video_win.width, video_win.height);

			// if(ioctl(fd, VIDIOCGPICT, &video_pic) == -1)
			// 		perror("cam_info: Can't get picture information");
			// else
			// 		printf("Current depth:\t%d\n", video_pic.depth);
			// }
		}
}
