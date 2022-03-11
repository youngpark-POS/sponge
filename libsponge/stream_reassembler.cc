#include "stream_reassembler.hh"

#include <cassert>

#define EMPTY_BYTE 0x1
#define WRITTEN_BYTE 0x2
#define USED_BYTE 0x4

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity)
    , _capacity(capacity)
    , segments()
    , byte_status()
    , stream_starts_at(0)
    , stream_last_byte(0)
    , got_eof(false) {
    segments.resize(_capacity);
    byte_status.resize(_capacity, EMPTY_BYTE);
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    segments.erase(segments.begin(), segments.begin() + _output.bytes_read() - stream_starts_at);
    byte_status.erase(byte_status.begin(), byte_status.begin() + _output.bytes_read() - stream_starts_at);

    segments.resize(_capacity);
    byte_status.resize(_capacity, EMPTY_BYTE);

    stream_starts_at = _output.bytes_read();

    int segment_start_index = static_cast<int>(index) - stream_starts_at;
    for (int i = 0; i < static_cast<int>(data.length()); i++) {
        if (segment_start_index + i < 0)
            continue;
        if (segment_start_index + i > static_cast<int>(_capacity - 1))
            break;
        if (is_written_byte(segment_start_index + i))
            continue;
        if (is_empty_byte(segment_start_index + i)) {
            segments[segment_start_index + i] = data[i];
            byte_status[segment_start_index + i] = USED_BYTE;
        }
    }

    for (int i = 0; i < static_cast<int>(_capacity) && !is_empty_byte(i); i++) {
        if (is_written_byte(i))
            continue;
        _output.write(string(1, segments[i]));
        byte_status[i] = WRITTEN_BYTE;
    }

    if (eof) {
        got_eof = true;
        stream_last_byte = index + data.length();
    }
    if (got_eof && empty() && _output.bytes_written() == stream_last_byte)
        _output.end_input();
}

size_t StreamReassembler::unassembled_bytes() const {
    size_t unassembled = 0;
    for (size_t i = 0; i < _capacity; i++) {
        unassembled += static_cast<int>(is_used_byte(i));
    }
    return unassembled;
}

bool StreamReassembler::empty() const { return unassembled_bytes() == 0UL; }
