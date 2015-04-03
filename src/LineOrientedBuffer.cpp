#include "LineOrientedBuffer.h"
#include "Util.h"
#include <cassert>

using namespace Bish;

void LineOrientedBuffer::add(const std::string &s) {
    for (unsigned i = 0, n = s.size(); i < n; i++) {
        char c = s[i];
        if (c == '\n') {
            buffer.push_back(current_line);
            current_line.clear();
        } else {
            current_line.push_back(c);
        }
    }
}

void LineOrientedBuffer::insert_line_prev(const std::string &s) {
    assert(s.find('\n') == std::string::npos);
    buffer.push_back(s);
}

void LineOrientedBuffer::flush() {
    for (std::vector<std::string>::iterator I = buffer.begin(), E = buffer.end(); I != E; ++I) {
        output_stream << *I << "\n";
    }
    output_stream << current_line;
    buffer.clear();
    current_line.clear();
}
