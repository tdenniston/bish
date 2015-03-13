#ifndef __BISH_UTIL_H__
#define __BISH_UTIL_H__

#include <limits.h>
#include <string>
#include <sstream>
#include <vector>

// Convert the given string to the template type.
template <typename T>
inline T convert_string(const std::string &s) {
    T t;
    std::istringstream(s) >> t;
    return t;
}

// Convert int to string
inline std::string convert_string(int i) {
  return dynamic_cast<std::ostringstream &>((std::ostringstream() << i )).str();
}

// Return the absolute path from the given path.
std::string abspath(const std::string &path);

#endif
