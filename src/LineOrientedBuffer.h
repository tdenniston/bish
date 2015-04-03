#ifndef __BISH_LINE_ORIENTED_BUFFER_H__
#define __BISH_LINE_ORIENTED_BUFFER_H__

#include <iostream>
#include <string>
#include <vector>
#include "Util.h"

namespace Bish {

// A "line oriented" buffer is one that constructs its output line by
// line. The buffer is flushed to an output stream either explicitly,
// or implicitly when the buffer is destroyed. The main advantage of
// this over a regular output stream is the ability to add a line
// previous to the one currently being constructed.
class LineOrientedBuffer {
public:
    LineOrientedBuffer(std::ostream &os) : output_stream(os) {}
    ~LineOrientedBuffer() {
        flush();
    }
    // Flush all buffered output to the underlying stream. This clears
    // the buffers.
    void flush();
    // Buffer the given string.
    void add(const std::string &s);
    // Insert the given string as a line previous to the current line.
    void insert_line_prev(const std::string &s);
private:
    std::ostream &output_stream;
    std::vector<std::string> buffer;
    std::string current_line;
};

template <typename T>
LineOrientedBuffer &operator<<(LineOrientedBuffer &os, const T &data) {
    os.add(as_string(data));
    return os;
}

}

#endif
