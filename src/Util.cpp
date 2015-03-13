#include <cstdlib>
#include "Util.h"

std::string abspath(const std::string &path) {
    const char *s = path.c_str();
    char abs[PATH_MAX];
    char *p = realpath(s, abs);
    if (p) {
        return std::string(p);
    } else {
        return "";
    }
}
