#include <cassert>
#include <cstdlib>
#include "Config.h"
#include "Util.h"

// Return the path to the standard library. This allows setting the
// path via BISH_STDLIB to override the default.
std::string get_stdlib_path() {
    char *stdlib = std::getenv("BISH_STDLIB");
    if (stdlib) {
        std::string abs = abspath(stdlib);
        assert(!abs.empty() && "Unable to resolve path specified in BISH_STDLIB.");
        return abs;
    } else {
        return STDLIB_PATH;
    }
}

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

std::string dirname(const std::string &path) {
    assert(!path.empty());
    std::size_t idx = path.size() - 1;
    // Ignore trailing slashes.
    while (idx > 0 && path[idx--] == '/') ;
    idx = path.rfind("/", idx);
    if (idx == 0) return "/";
    if (idx != std::string::npos) {
        return path.substr(0, idx);
    } else {
        return ".";
    }
}

std::string remove_suffix(const std::string &s, const std::string &marker) {
    std::size_t idx = s.rfind(marker);
    if (idx != std::string::npos) {
        return s.substr(0, idx);
    }
    return s;
}

std::string strip(const std::string &s) {
    const char *whitespace = " \t\n";
    std::string result = s;
    result.erase(0, result.find_first_not_of(whitespace));
    result.erase(result.find_last_not_of(whitespace) + 1);
    return result;
}

std::string module_name_from_path(const std::string &path) {
    return remove_suffix(basename(path), ".bish");
}
