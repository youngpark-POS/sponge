#include "tcp_receiver.hh"

#include <optional>

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    if (seg.header().syn) {
        isn = seg.header().seqno;
        seq = isn;
        syn_get = true;
    }
    if (seg.header().fin) {
        fin_get = true;
    }

    if (seg.header().syn)
        _reassembler.push_substring(
            string(seg.payload().str()), unwrap(seg.header().seqno, isn, abs_seq), seg.header().fin);
    else
        _reassembler.push_substring(
            string(seg.payload().str()), unwrap(seg.header().seqno, isn, abs_seq) - 1, seg.header().fin);

    abs_seq = stream_out().bytes_written() + static_cast<int>(syn_get) +
              static_cast<int>(fin_get && stream_out().input_ended());
    seq = wrap(abs_seq, isn);
}

optional<WrappingInt32> TCPReceiver::ackno() const { return syn_get ? optional<WrappingInt32>(seq) : nullopt; }

size_t TCPReceiver::window_size() const {
    return _capacity - (_reassembler.stream_out().bytes_written() - _reassembler.stream_out().bytes_read());
}
