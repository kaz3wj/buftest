#ifndef __UTIL_CLASS_H__INCLUDED__
#define __UTIL_CLASS_H__INCLUDED__

#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <string.h>

#define CLEAR(x) memset (&(x), 0, sizeof (x))
#define _COUNTOF(a)   (sizeof(a)/sizeof((a)[0]))

/*****************************************************************************
 * Usage: print module usage
 *****************************************************************************
 * Print a short inline help. Message interface is initialized at this stage.
 *****************************************************************************/
#define COL(x)  "\033[" #x ";1m"
#define UTCOL_RED     COL(31)
#define UTCOL_GREEN   COL(32)
#define UTCOL_YELLOW  COL(33)
#define UTCOL_BLUE    COL(34)
#define UTCOL_MAGENTA COL(35)
#define UTCOL_CYAN    COL(36)
#define UTCOL_WHITE   COL(0)
#define UTCOL_GRAY    "\033[0m"
#define LINE_START      8
#define PADDING_SPACES 25


/*****************************************************************************

 *****************************************************************************/
class utTimer
{
// ctor, dtor
public:
	utTimer(const std::string& name = "_no_name_", 
          bool bOutputOnTerminate = false);
	virtual ~utTimer();

//Attributes
public:
		uint32_t elapsed();
public:
    uint32_t startTime;
	std::string identifier;
	bool bOutput;
};



/*****************************************************************************

 *****************************************************************************/

/*****************************************************************************

 *****************************************************************************/

template <typename T>
std::string int_to_hex(T i)
{
  std::stringstream stream;
  stream << "0x" 
         << std::setfill ('0') << std::setw(sizeof(T)*2) 
         << std::hex << i;
  return stream.str();
}

#endif //! __UTIL_CLASS_H__INCLUDED__
