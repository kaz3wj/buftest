#include <string>
#include "util_class.h"

using std::chrono::system_clock;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::cout;

/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
utTimer::utTimer(const std::string& name, bool bOutputOnTerminate)
	: identifier(name), bOutput(bOutputOnTerminate)
{
	startTime = (uint32_t) std::chrono::duration_cast<std::chrono::milliseconds>(system_clock::now().time_since_epoch()).count();
	identifier = name;
}
/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
utTimer::~utTimer()
{
	if (bOutput) {
		std::cout << identifier.c_str() << ": " << elapsed() << "ms " << std::endl;
	}
}
/**
 * @brief 
 * @param 
 * @return 
 * @note 
*/
uint32_t utTimer::elapsed()
{
	uint32_t curTime = (uint32_t) std::chrono::duration_cast<std::chrono::milliseconds>(system_clock::now().time_since_epoch()).count();
	return curTime - startTime;
}
