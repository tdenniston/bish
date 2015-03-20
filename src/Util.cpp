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

std::string basename(const std::string &path) {
    std::size_t idx = path.rfind("/");
    if (idx != std::string::npos) {
        return path.substr(idx + 1, std::string::npos);
    } else {
        return path;
    }
}

std::string remove_suffix(const std::string &s, const std::string &marker) {
    std::size_t idx = s.rfind(marker);
    if (idx != std::string::npos) {
        return s.substr(0, idx);
    }
    return s;
}
