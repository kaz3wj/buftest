#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include "util_class.h"
#include "util_v4l2_enum_devices.h"

using namespace std;

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/

void utv4l2_EnumDevices::doEnum(UTV4L2_ENUMDEVICE_CB cb, void* userdata)
{
		_cameras = 0;

		for (int ii = 0; ii < _max_capture_devices; ii++) 
		{
			string device_name = "/dev/video" + to_string(ii);
	    int fd = open( device_name.c_str(), O_RDONLY);
			if(-1 == fd){
					break;
			}
			_cameras++;

	    struct v4l2_capability video_cap;
			if(-1 == ioctl(fd, VIDIOC_QUERYCAP, &video_cap)){
					cout << "  can't get capabilities";
					break;
			}
			else {
				if (cb) {
					cb(ii, video_cap, userdata );	//callback
				}
				cout << device_name << ":" << endl;
				cout << "  driver: '" << video_cap.driver << "'" << endl;
				cout << "  card  : '" << video_cap.card << "'" << endl;
			}
			close(fd);
		}
}
