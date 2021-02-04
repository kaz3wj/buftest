#ifndef __UTIL_V4L2_STATE__H__INCLUDED__
#define __UTIL_V4L2_STATE__H__INCLUDED__

class utV4l2_Camera;

//////////////////////////////////////////////

class utV4l2_State 
{
public:
	utV4l2_State(){}
	virtual ~utV4l2_State() {}

	virtual bool do_open(utV4l2_Camera *camera){ return false;}
	virtual bool do_close(utV4l2_Camera *camera){ return false;}
	virtual bool do_start(utV4l2_Camera *camera){ return false;}
	virtual bool do_stop(utV4l2_Camera *camera){ return false;}
	static utV4l2_State *instance();

protected:
	void do_change_state(utV4l2_Camera *camera, utV4l2_State *s);
private:
	static utV4l2_State *_pseudoThis;
};


//////////////////////////////////////////////

class utV4l2Stat_Closed : public utV4l2_State
{
public:
	virtual bool do_open(utV4l2_Camera *camera);
	static utV4l2Stat_Closed *instance();

private:
	static utV4l2Stat_Closed *_pseudoThis;
};

//////////////////////////////////////////////

class utV4l2Stat_Opened : public utV4l2_State
{
public:
	// virtual bool do_open(utV4l2_Camera *camera);
	virtual bool do_close(utV4l2_Camera *camera);
	virtual bool do_start(utV4l2_Camera *camera);
	// virtual bool do_stop(utV4l2_Camera *camera);

	static utV4l2Stat_Opened *instance();

private:
	static utV4l2Stat_Opened *_pseudoThis;
};

//////////////////////////////////////////////

class utV4l2Stat_Running : public utV4l2_State
{
public:
	// virtual bool do_open(utV4l2_Camera *camera);
	// virtual bool do_close(utV4l2_Camera *camera);
	// virtual bool do_start(utV4l2_Camera *camera);
	virtual bool do_stop(utV4l2_Camera *camera);
	static utV4l2Stat_Running *instance();

private:
	static utV4l2Stat_Running *_pseudoThis;
};


template<class T>
void release_instance()
{
	T *p = T::instance();
	delete p;
}

#endif //!__UTIL_V4L2_STATE__H__INCLUDED__