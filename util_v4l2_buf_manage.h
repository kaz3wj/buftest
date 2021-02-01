#ifndef __UTIL_V4L2_BUF_MANAGE_H__INCLUDED__
#define __UTIL_V4L2_BUF_MANAGE_H__INCLUDED__

	// V4L2_MEMORY_DMABUF
	// V4L2_MEMORY_MMAP
	// V4L2_MEMORY_USERPTR

class utV4l2_buf_manage
{
	public:
		utV4l2_buf_manage(int fd);
		virtual ~utV4l2_buf_manage();

	public:
		bool alloc();
		void free();

	protected:
		virtual bool do_alloc() { return false; }
		virtual void do_free() {}

	protected:
		int _fd;
		struct v4l2_buffer buf;
};

class utV4l2_buf_DMABUF	: public utV4l2_buf_manage
{
	public:
		utV4l2_buf_DMABUF(int fd);
	protected:
		virtual bool do_alloc();
		virtual void do_free();
};

class utV4l2_buf_MMAP	: public utV4l2_buf_manage
{
	public:
		utV4l2_buf_MMAP(int fd);
	protected:
		virtual bool do_alloc();
		virtual void do_free();
};

class utV4l2_buf_USERPTR	: public utV4l2_buf_manage
{
	public:
		utV4l2_buf_USERPTR(int fd);
	protected:
		virtual bool do_alloc();
		virtual void do_free();
};

#endif //!__UTIL_V4L2_BUF_MANAGE_H__INCLUDED__
