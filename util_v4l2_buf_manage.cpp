#include <linux/videodev2.h>
#include "util_class.h"
#include "util_v4l2_buf_manage.h"

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
utV4l2_buf_manage::utV4l2_buf_manage(int& fd)
	: _fd(fd)
{
	CLEAR(buf);


}

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
utV4l2_buf_manage::~utV4l2_buf_manage()
{

}


/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
bool utV4l2_buf_manage::alloc()
{
	return do_alloc();
}

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
void utV4l2_buf_manage::free()
{
	do_free();
}

//////////////////////////////////

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
utV4l2_buf_DMABUF::utV4l2_buf_DMABUF(int fd)
	: utV4l2_buf_manage(fd)
{

}

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
bool utV4l2_buf_DMABUF::do_alloc()
{

}

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
void utV4l2_buf_DMABUF::do_free()
{


}



//////////////////////////////////

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
utV4l2_buf_MMAP::utV4l2_buf_MMAP(int fd)
	: utV4l2_buf_manage(fd)
{

}

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
bool utV4l2_buf_MMAP::do_alloc()
{

}

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
void utV4l2_buf_MMAP::do_free()
{

	
}


//////////////////////////////////

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
utV4l2_buf_USERPTR::utV4l2_buf_USERPTR(int fd)
	: utV4l2_buf_manage(fd)
{

}

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
bool utV4l2_buf_USERPTR::do_alloc()
{

}

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
void utV4l2_buf_USERPTR::do_free()
{

	
}
