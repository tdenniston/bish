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

// Return the absolute path to the standard library.
std::string get_stdlib_path();
// Return the absolute path from the given path.
std::string abspath(const std::string &path);
// Return the basename (last component) of a path.
std::string basename(const std::string &path);
// Return the directory name of a path.
std::string dirname(const std::string &path);
// Return the string with any suffix beginning with "marker" removed.
// E.g. remove_suffix("test.bish", ".") returns "test"
std::string remove_suffix(const std::string &s, const std::string &marker);
// Strip leading and trailing whitespace.
std::string strip(const std::string &s);
// Return the name of a module from a pathname.
// E.g. module_name_from_path("/a/b/test.bish") returns "test"
std::string module_name_from_path(const std::string &path);
#endif
