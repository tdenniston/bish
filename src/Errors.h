#ifndef __BISH_ERRORS_H__
#define __BISH_ERRORS_H__

#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>

namespace Bish {

class ErrorReport {
public:
    ErrorReport(const char *f, int l, bool abort=false) {
        file = f;
        lineno = l;
        abort_condition = abort;
    }

    ~ErrorReport() {
        if (abort_condition) {
            std::cerr << "Bish error: " << msg.str() << "\n";
            abort();
        }
    }
    
    template<typename T>
    ErrorReport &operator<<(T x) {
        msg << x;
        return *this;
    }

private:
    const char *file;
    int lineno;
    std::ostringstream msg;
    bool abort_condition;
};

#define bish_abort()      Bish::ErrorReport(__FILE__, __LINE__, true)
#define bish_assert(cond) Bish::ErrorReport(__FILE__, __LINE__, !(cond))

};

#endif
