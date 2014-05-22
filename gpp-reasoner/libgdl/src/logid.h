#pragma once

/*
Stream friendly log out position

Use LOGID or LOGIDXL inside your std::cout << LOGID << "Something went wrong" stream..
*/


#include <sstream>

namespace logid {

/// Does a format like
/// /home/nos/coding/cg_beleg/TetraViewer2/bindebug/build_dir/fine_drawer.cpp:47 virtual void fine_drawer::render() 
static inline std::string format_long (const char * filename, int line, const char * function){
	std::ostringstream os;
	os << filename << ":" << line << " " << function << " ";
	return os.str ();
}

/// Throws away filename/line number
/// Looks like virtual void fine_drawer::render()
static inline std::string format_short (const char * filename, int line, const char * function){
	std::ostringstream os;
	os << filename << "(" << line << "): ";
	return os.str ();
}

}

// Windows version
#ifdef _MSC_VER
/// Caller for format_long
#define LOGIDXL logid::format_long (__FILE__, __LINE__, __FUNCTION__)
/// Caller for format_short
#define LOGID logid::format_short  (__FILE__, __LINE__, __FUNCTION__)


#else
// GCC Version

/// Caller for format_long
#define LOGIDXL logid::format_long (__FILE__, __LINE__, __PRETTY_FUNCTION__)
/// Caller for format_short
#define LOGID logid::format_short  (__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif
