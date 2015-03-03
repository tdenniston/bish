#ifndef __BISH_UTIL_H__
#define __BISH_UTIL_H__

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

template <typename T>
void vec_erase(std::vector<T> &v, const T &t) {
    for (std::vector<T>::iterator I = v.begin(), E = v.end(); I != E; ) {
        if (*I == t) {
            I = nodes.erase(I);
        } else {
            ++I;
        }
    }
}

#endif
