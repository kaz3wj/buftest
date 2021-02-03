#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <signal.h>
#include <poll.h>
#include <linux/videodev2.h>
#include <cuda_runtime.h>
#include "core_test_extn.h"
#include "util_class.h"
#include "util_v4l2.h"
#include "util_v4l2_state.h"
#include "util_context.h"

using namespace std;

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
utV4l2_State* utV4l2_State::_pseudoThis = NULL;

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
void utV4l2_State::do_change_state(utV4l2_Camera *camera, utV4l2_State *s)
{
	camera->change_state(s);
}

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
utV4l2_State *utV4l2_State::instance()
{
	if (!_pseudoThis) {
		_pseudoThis = new utV4l2_State();
	}
	return _pseudoThis;
}

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
utV4l2Stat_Closed* utV4l2Stat_Closed::_pseudoThis = NULL;

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
bool utV4l2Stat_Closed::do_open(utV4l2_Camera *camera)
{ 
	// cout << "uutV4l2Stat_Closed::do_open: " << camera->dev_name() << std::endl;
	int ret = -1;
	std::string cap;
	bool bDebugOut = (0==camera->_dev_no);
	bool bDryrun  = camera->_ctx_param->isDryrun();

	std::string name_to_open = camera->dev_name();

	camera->_fd = open(name_to_open.c_str(), O_RDWR);
	if (camera->_fd<0) {
		cout << UTCOL_RED << "FAILED: Open(): " << name_to_open << UTCOL_WHITE << endl;
		return false;
	}

	if (bDebugOut) {
		utV4l2_Camera::list_formats(camera->_fd);
	}

	// init format
	struct v4l2_format fmt, fmt1;
	fmt1.type = camera->_type;

	if (bDryrun) {
		cout << UTCOL_BLUE << "VIDIOC_G_FMT" << endl;
	}
	else {
		ret = utV4l2_Camera::xioctl(camera->_fd, VIDIOC_G_FMT, &fmt1);
		if (ret< 0) {
			utV4l2_Camera::error_text("VIDIOC_G_FMT");
			return false;
		}
	}

	if (bDebugOut) {
		cout << UTCOL_GRAY << "G_FMT: width: " << to_string(fmt1.fmt.pix.width) << UTCOL_WHITE << endl;
		cout << UTCOL_GRAY << "G_FMT: height: " << to_string(fmt1.fmt.pix.height) <<  UTCOL_WHITE << endl;
		cout << UTCOL_GRAY << "G_FMT: pixelformat: " << utV4l2_Camera::fourcc_text(fmt1.fmt.pix.pixelformat) <<  UTCOL_WHITE << endl;
	}

	fmt.type = camera->_type;

	// fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	fmt.fmt.pix.width       = camera->_width;
	fmt.fmt.pix.height      = camera->_height;
	// fmt.fmt.pix.width = fmt1.fmt.pix.width;
	// fmt.fmt.pix.height = fmt1.fmt.pix.height;

	// fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV16;
	// fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;

	// if (fmt.fmt.pix.pixelformat != fmt1.fmt.pix.pixelformat) {
	// 	cout << "inquired: " << fourcc_text( fmt1.fmt.pix.pixelformat) << endl;
	// 	cout << "set to  : " << fourcc_text(fmt.fmt.pix.pixelformat) << endl;
		fmt.fmt.pix.pixelformat = fmt1.fmt.pix.pixelformat;
	// }

	if (bDebugOut) {
		cout << "S_FMT: pix.width: " << to_string(fmt.fmt.pix.width) << endl;
		cout << "S_FMT: pix.height: " << to_string(fmt.fmt.pix.height) << endl;
		cout << "S_FMT: pix.pixelformat: " << utV4l2_Camera::fourcc_text(fmt.fmt.pix.pixelformat) << endl;
	}
	// fmt.fmt.pix.field       = V4L2_FIELD_NONE;
	fmt.fmt.pix.field = fmt1.fmt.pix.field;

	if (bDryrun) {
		cout << UTCOL_BLUE << "VIDIOC_S_FMT" << endl;
	}
	else {
		ret = utV4l2_Camera::xioctl(camera->_fd, VIDIOC_S_FMT, &fmt);
		if(ret< 0 /*|| fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_NV16*/ ||
			fmt.fmt.pix.width<=0 || fmt.fmt.pix.height<=0) {
			utV4l2_Camera::error_text("VIDIOC_S_FMT");
			return false;
		}
		if (bDebugOut) {
			cout << UTCOL_CYAN << "VIDIOC_S_FMT" << UTCOL_WHITE << endl;
		}
	}

	// init buffer
	struct v4l2_requestbuffers req;
	req.count               = camera->_page_count;
	req.type                = camera->_type;
	req.memory              = V4L2_MEMORY_USERPTR;


	if (bDryrun) {
		cout << UTCOL_BLUE << "VIDIOC_REQBUFS" << endl;
	}
	else {
		if(utV4l2_Camera::xioctl(camera->_fd, VIDIOC_REQBUFS, &req) < 0) {
			utV4l2_Camera::error_text("Failed to request v4l2 buffers: VIDIOC_REQBUFS");
			return false;
		}
		if (req.count != camera->_page_count) {
			cout << UTCOL_RED 
				<< "V4l2 buffer number (" << to_string(req.count)
				<<") is not as desired (" << to_string(camera->_page_count) << ")"
				<< UTCOL_WHITE << endl;
			return false;
		}

		if (bDebugOut) {
			cout << UTCOL_CYAN << "VIDIOC_REQBUFS" << UTCOL_WHITE << endl;
		}
	}

	// malloc preparation (check cuda free mem)
	if (bDebugOut) {

		void *dataTmp;

		// cudaMemRangeGetAttribute(dataTmp,
		// 												 size_t dataSize,
		// 												 enum cudaMemRangeAttribute attribute,
		// 												 const void *devPtr,
		// 												 size_t count);

		size_t free_mem, total_mem;
		cudaMemGetInfo(&free_mem, &total_mem);
		cout << UTCOL_GREEN 
			<< "[CUDA] Avail mem: " << to_string(free_mem/(1024*1024)) << "MB (" << to_string(free_mem) << " bytes)"
			<< UTCOL_WHITE << endl;

		cout << UTCOL_GREEN
			 << "[CUDA] Total mem: " << to_string(total_mem/(1024*1024)) << "MB (" << to_string(total_mem) << " bytes)" 
			 << UTCOL_WHITE << endl;
	}

	size_t alloc_size = fmt.fmt.pix.width * fmt.fmt.pix.height * sizeof(uint16_t);

	uint32_t dur_cudaMalloc = 0;
	uint32_t dur_qbuf = 0;
	{
		utTimer t("VIDIOC_QBUF", true);
		for (size_t ii=0; ii<camera->_page_count; ii++)	{
				uint8_t* p_data = NULL;
				cap = "[/dev/video" + to_string(camera->_dev_no) + "." + to_string(ii) + "] cudaMalloc(): " + to_string(alloc_size) + " bytes";
				{
					utTimer t( cap, true);
					// ref. https://developer.nvidia.com/blog/unified-memory-cuda-beginners/

					cudaError_t cuErr = cudaMallocManaged((void **)&p_data, alloc_size );
					if (cuErr == cudaSuccess) {
							cout << UTCOL_GREEN 
									<< "[CUDA] cudaMallocManaged(): adrs=" <<int_to_hex(reinterpret_cast<unsigned long>(p_data)) << ", 64byte Boundary: " 
									<< (0==(reinterpret_cast<unsigned long>(p_data)%64) ? "YES" : "NO" ) 
									<< UTCOL_WHITE << endl;

							cuErr = cudaDeviceSynchronize();
							if (cuErr != cudaSuccess) {
								cout << UTCOL_RED 
									<<"Failure: cudaDeviceSynchronize" 
									<< UTCOL_WHITE << endl;
							}

							camera->_buf.push_back( p_data );
							{
								struct v4l2_buffer buf;
								CLEAR(buf);

								if (bDryrun) {
									cout << UTCOL_BLUE << "VIDIOC_QUERYBUF" << endl;
								}
								else {

									buf.index		= ii;
									buf.type		= camera->_type; //V4L2_BUF_TYPE_VIDEO_CAPTURE
									buf.memory  = V4L2_MEMORY_USERPTR;
									ret = utV4l2_Camera::xioctl(camera->_fd, VIDIOC_QUERYBUF, &buf);
									if(ret<0) {
										utV4l2_Camera::error_text("Failed to query buff: VIDIOC_QUERYBUF");
										return false;
									}

									if (bDebugOut) {
										cout << UTCOL_CYAN << "VIDIOC_QUERYBUF: buffer len=" << to_string(buf.length) << " bytes" << UTCOL_WHITE << endl;
									}
								}

								// buffers[i].start = memalign(getpagesize (), alloc_size );
								// buf.m.userptr = (unsigned long) buffers[i].start;
								// printf("new pointer for buffer %d = %p\n", i, buffers[i].start);
								//---<>

								if (bDryrun) {
									cout << UTCOL_BLUE << "VIDIOC_QUERYBUF" << endl;
								}
								else {
									buf.m.userptr       = reinterpret_cast<unsigned long>( camera->_buf[ii]);
									buf.length          = alloc_size;
									{
										ret = utV4l2_Camera::xioctl(camera->_fd, VIDIOC_QBUF, &buf);
										if(ret<0) {
											utV4l2_Camera::error_text("Failed to enqueue buffers: VIDIOC_QBUF");
											return false;
										}
										if (bDebugOut) {
											cout << UTCOL_CYAN << "VIDIOC_QBUF" << UTCOL_WHITE << endl;
										}
										dur_qbuf += t.elapsed();
									}
								}
							}
					}
					else {
							cout << UTCOL_RED << "cudaMallocManaged()" << UTCOL_WHITE << endl;
							// return false;
					}
					dur_cudaMalloc += t.elapsed();
				}
		}
	}
	return true;
}

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
utV4l2Stat_Closed *utV4l2Stat_Closed::instance(){
	if (!_pseudoThis) {
		_pseudoThis = new utV4l2Stat_Closed();
	}
	return _pseudoThis;
}


/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
utV4l2Stat_Opened* utV4l2Stat_Opened::_pseudoThis = NULL;

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
bool utV4l2Stat_Opened::do_close(utV4l2_Camera *camera)
{ 
	// cout << "utV4l2Stat_Opened::do_close: " << camera->dev_name() << std::endl;

	uint32_t dur_cudaFree = 0;
	{
		utTimer t("cudaFree", false);
		for (uint8_t*& p_data : camera->_buf) {
			cudaFree(p_data);
		}
	}

	if (-1 != camera->_fd) {
		close(camera->_fd);
		camera->_fd = -1;
	}
	return true;
}



/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
bool utV4l2Stat_Opened::do_start(utV4l2_Camera *camera)
{
	utTimer("do_start");

	bool bDryrun  = camera->_ctx_param->isDryrun();

	int ret;
	// cout << "uutV4l2Stat_Opened::do_start(): " << camera->dev_name() << std::endl;

	if (bDryrun) {
		cout << UTCOL_BLUE << "VIDIOC_STREAMON" << UTCOL_WHITE << endl;
	}
	else {
		ret = utV4l2_Camera::xioctl(camera->_fd, VIDIOC_STREAMON, &camera->_type);
		if(ret<0) {
			utV4l2_Camera::error_text("Failed to start streaming: VIDIOC_STREAMON");
			return false;
		}
		cout << UTCOL_CYAN << "VIDIOC_STREAMON" << UTCOL_WHITE << endl;
	}

	// wait a little...
	usleep(200);

	bool result = true;
	bool bOutput = (0==camera->_dev_no);
	struct pollfd fds[1];
	fds[0].fd = camera->_fd;
	fds[0].events = POLLIN;
	int32_t poll_timeout = camera->_ctx_param->timeout();
	
	uint32_t dur_dq = 0;
	uint32_t dur_eq = 0;
	uint32_t dur_frame = 0;
	uint32_t dur_poll = 0;

	int32_t frame = 0;
	int32_t proc_step = 10;

	int32_t max_loop = camera->_ctx_param->grab_count();

	utTimer tFrame("CAPTURE", false);
	/* Wait for camera event with timeout */
	while(!utContext::g_quit)
	{
		if (frame>max_loop && (-1!=max_loop)) {
			break;
		}

		if (poll(fds, sizeof(fds)/sizeof(fds[0]), poll_timeout)==0) {
			if (errno = ESRCH) {
				if (bOutput) {
					cout << UTCOL_YELLOW << "poll timeout: " << to_string(poll_timeout) <<"ms" << UTCOL_WHITE << endl;
				}
			}
			continue;
		}

		if (fds[0].revents & POLLIN) {
				++frame;
				dur_poll = tFrame.elapsed();
				{
					struct v4l2_buffer buf;
					CLEAR(buf);
					buf.type = camera->_type;
					buf.memory = V4L2_MEMORY_USERPTR;

					/* Dequeue a camera buff */
					{
						utTimer t("VIDIOC_DQBUF", false);
						if (bDryrun) {
						}
						else {
							ret = utV4l2_Camera::xioctl(camera->_fd, VIDIOC_DQBUF, &buf);
							if (ret < 0) {
								utV4l2_Camera::error_text("Failed to dequeue camera buff: VIDIOC_DQBUF");
								result = false;
								break;
							}
						}
						dur_dq += t.elapsed();
					}






					/* Enqueue camera buffer back to driver */
					{
						utTimer t("VIDIOC_QBUF", false);
						if (bDryrun) {
						}
						else {
							ret = utV4l2_Camera::xioctl(camera->_fd, VIDIOC_QBUF, &buf);
							if (ret < 0){
								utV4l2_Camera::error_text("Failed to queue camera buffers: VIDIOC_QBUF");
								result = false;
								break;
							}
						}
						dur_eq += t.elapsed();
					}
			}	

			dur_frame = tFrame.elapsed();
			if (frame%proc_step==0) 
			{
				int32_t avg_dur = dur_frame / frame;
				cout << ((0 == camera->_dev_no) ? UTCOL_CYAN : UTCOL_BLUE)
						 << "Frame Duration(avg.): " << to_string(avg_dur) << " msec, " 
						 << to_string(1000/avg_dur) << " fps"
						 << UTCOL_WHITE << endl;
			}
		}
	}

	if (bDryrun) {
		cout << UTCOL_BLUE << "VIDIOC_STREAMOFF" << UTCOL_WHITE << endl;
	}
	else {
		ret = utV4l2_Camera::xioctl(camera->_fd, VIDIOC_STREAMOFF, &(camera->_type));
		if (ret<0) {
			utV4l2_Camera::error_text("Failed to stop streaming: VIDIOC_STREAMOFF");
			return false;
		}
		cout << UTCOL_CYAN << "VIDIOC_STREAMOFF" << UTCOL_WHITE << endl;
	}
	return result;
}



/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/

utV4l2Stat_Opened *utV4l2Stat_Opened::instance(){
	if (!_pseudoThis) {
		_pseudoThis = new utV4l2Stat_Opened();
	}
	return _pseudoThis;
}


/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
utV4l2Stat_Running* utV4l2Stat_Running::_pseudoThis = NULL;


/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
bool utV4l2Stat_Running::do_stop(utV4l2_Camera *camera)
{ 
	// cout << "utV4l2Stat_Running::do_stop(): " << camera->dev_name() << std::endl;
	utContext::g_quit = true;
	return true;
}

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/

utV4l2Stat_Running *utV4l2Stat_Running::instance(){
	if (!_pseudoThis) {
		_pseudoThis = new utV4l2Stat_Running();
	}
	return _pseudoThis;
}
