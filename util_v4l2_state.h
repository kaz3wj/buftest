#ifndef __UTIL_V4L2_STATE__H__INCLUDED__
#define __UTIL_V4L2_STATE__H__INCLUDED__

class utV4l2_Camera;

//////////////////////////////////////////////

class utV4l2_State 
{
protected:
	utV4l2_State(){}

public:
	virtual ~utV4l2_State(){}
	
public:
	// operation
	virtual bool do_open(utV4l2_Camera *camera){ return false;}
	virtual bool do_close(utV4l2_Camera *camera){ return false;}
	virtual bool do_start(utV4l2_Camera *camera){ return false;}
	virtual bool do_stop(utV4l2_Camera *camera){ return false;}

	// instance
	static utV4l2_State *instance();

protected:
	void do_change_state(utV4l2_Camera *camera, utV4l2_State *s);

private:
	static utV4l2_State *_pseudoThis;
};

//////////////////////////////////////////////

// template <class T>
// class utSingleton
// {
// public:

// 	static T *instance() {
// 		if (!_pseudoThis) {
// 			_pseudoThis = new T();
// 		}
// 		return _pseudoThis;
// 	}

// 	static void free_instance() {
// 		T *p = T::instance();
// 		delete p;
// 	}

// protected:
// 	static T *_pseudoThis = NULL;
// };

//////////////////////////////////////////////

#if 0


class utV4l2Stat_Closed : public utSingleton<utV4l2_State>
{
public:
	virtual ~utV4l2Stat_Closed();
	virtual bool do_open(utV4l2_Camera *camera);
};

// class utSingleton<utV4l2Stat_Closed_base> utV4l2Stat_Closed;


#else

class utV4l2Stat_Closed : public utV4l2_State
{
public:
	virtual ~utV4l2Stat_Closed();

	virtual bool do_open(utV4l2_Camera *camera);
	static utV4l2Stat_Closed *instance();

private:
	static utV4l2Stat_Closed *_pseudoThis;
};
#endif
//////////////////////////////////////////////

class utV4l2Stat_Opened : public utV4l2_State
{
public:
	virtual ~utV4l2Stat_Opened();
	
	virtual bool do_close(utV4l2_Camera *camera);
	virtual bool do_start(utV4l2_Camera *camera);

	static utV4l2Stat_Opened *instance();

private:
	static utV4l2Stat_Opened *_pseudoThis;
};

//////////////////////////////////////////////

class utV4l2Stat_Running : public utV4l2_State
{
public:
	virtual ~utV4l2Stat_Running();
	
	virtual bool do_stop(utV4l2_Camera *camera);
	static utV4l2Stat_Running *instance();

private:
	static utV4l2Stat_Running *_pseudoThis;
};


///////////////////////////////

template<class T>
void release_state_instance(){
	T *p = T::instance();
	delete p;
};

#endif //!__UTIL_V4L2_STATE__H__INCLUDED__