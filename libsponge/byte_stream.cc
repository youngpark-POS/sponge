#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t _capacity)
    : max_capacity(_capacity), used_capacity(0), byte_read(0), byte_written(0), is_input_ended(false) {}

size_t ByteStream::write(const string &data) {
    size_t remaining_capacity = max_capacity - used_capacity;
    size_t writable_bytes = remaining_capacity < data.length() ? remaining_capacity : data.length();
    bytestring += data.substr(0, writable_bytes);
    used_capacity += writable_bytes;
    byte_written += writable_bytes;
    return writable_bytes;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t readable_bytes = used_capacity < len ? used_capacity : len;
    return bytestring.substr(0, readable_bytes);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t readable_bytes = used_capacity < len ? used_capacity : len;
    bytestring = bytestring.erase(0, readable_bytes);
    used_capacity -= readable_bytes;
    byte_read += readable_bytes;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string result = this->peek_output(len);
    this->pop_output(len);
    return result;
}

void ByteStream::end_input() { is_input_ended = true; }

bool ByteStream::input_ended() const { return is_input_ended; }

size_t ByteStream::buffer_size() const { return used_capacity; }

bool ByteStream::buffer_empty() const { return used_capacity == 0; }

bool ByteStream::eof() const { return this->buffer_empty() && this->input_ended(); }

size_t ByteStream::bytes_written() const { return byte_written; }

size_t ByteStream::bytes_read() const { return byte_read; }

size_t ByteStream::remaining_capacity() const { return max_capacity - used_capacity; }
