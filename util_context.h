#ifndef __UTIL_CONTEXT_H__INCLUDED__
#define __UTIL_CONTEXT_H__INCLUDED__

#include <getopt.h>
#include <linux/videodev2.h>

class utContext
{
private:
	utContext();

public:
	static utContext *get_instance();

public:
	//opration
	bool parse_args(int argc, char *argv[]);
	int proc(void);

	static void print_args(int argc, char* argv[]);

	// attr
public:
	const int32_t preview_width() { return _ww; }
	const int32_t preview_height() { return _hh; }
	const bool useCUI() { return b_use_cui; }
	const int32_t page_count() {return _page_count;}
	const int32_t timeout() { return _timeout; }

	// Number of images to be grabbed.
	const int32_t grab_count() { return _frame_to_grab; }

	const bool useThread() { return b_use_thread; }
	const bool useImageEventHandler() { return b_use_utImageEventHandler; }
	const bool useOSD() { return b_use_osd; }

private:
	void set_default(void);
	void print_usage(const char *name);

public:
	static bool g_quit;

private:
	static utContext *_pseudoThis;
	static const char *optstring;

	// V4L2_MEMORY_DMABUF
	// V4L2_MEMORY_MMAP
	// V4L2_MEMORY_USERPTR

	v4l2_memory _buf_memory_type;

	int32_t _ww;
	int32_t _hh;
	int32_t _page_count;
	int32_t _camera_count;
	int32_t _timeout;
	int32_t _frame_to_grab;

	int32_t _cam_pixfmt;

	bool b_use_pylon;
	bool b_verbose;
	bool b_use_thread;
	bool b_use_osd;
	bool b_output_list;
	bool b_use_cui;
	// using utImageHandler
	bool b_use_utImageEventHandler;
	bool b_debug;
	bool b_dryrun;

	int _flag_verbose;
	int _flag_dryrun;
	int _flag_pylon;
	int _flag_gpu;
	int _flag_cui;
	int _flag_osd;
	int _flag_thread;
	int _flag_debug;
	int _flag_imagehandler;

	const struct option longopts[20] = {
			 {"dryrun", no_argument, &_flag_dryrun, 1},
			 {"verbose", no_argument, &_flag_verbose, 1},
			 {"debug", no_argument, &_flag_debug, 1},
			 {"pylon", no_argument, &_flag_pylon, 1},
			 {"v4l2", no_argument, &_flag_pylon, 0},
			 {"gpu", no_argument, &_flag_gpu, 1},
			 {"cpu", no_argument, &_flag_gpu, 0},
			 {"cui", no_argument, &_flag_cui, 1},
			 {"gui", no_argument, &_flag_cui, 0},
			 {"osd", no_argument, &_flag_osd, 1},
			 {"multi", no_argument, &_flag_thread, 1},
			 {"single", no_argument, &_flag_thread, 0},
			 {"image-handler", no_argument, &_flag_imagehandler, 1},
			 {"camera", required_argument, 0, 'c'},
			 {"page", required_argument, 0, 'p'},
			 {"bufmode", required_argument, 0, 'm'},
			 {"size", required_argument, 0, 's'},
			 {"timeout", required_argument, 0, 't'},
			 {"loop", required_argument, 0, 'l'},
			 { NULL, 0, NULL, 0}
			 };
};

#endif //__UTIL_CONTEXT_H__INCLUDED__
