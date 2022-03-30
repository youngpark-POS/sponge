#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define CLIP(a, b) (((a) > (b)) ? ((a) - (b)) : 0)

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , next_send(_isn)
    , next_ack(_isn)
    , _initial_retransmission_timeout{retx_timeout}
    , receiver_window_size(1)
    , _stream(capacity)
    , timer(retx_timeout) {}

uint64_t TCPSender::bytes_in_flight() const {
    return unwrap(next_send, _isn, _next_seqno) - unwrap(next_ack, _isn, _next_seqno);
}

void TCPSender::fill_window() {
    while (true) {
        size_t remained_window = MAX(1, receiver_window_size) - bytes_in_flight();

        // window filled
        if (remained_window == 0U)
            break;

        // instream empty
        if (stream_in().buffer_empty() && !syn_flag() && !eof_flag())
            break;

        // sent all data
        if (eof_flag() && fin_flag())
            break;

        size_t seg_size = MIN(remained_window, TCPConfig::MAX_PAYLOAD_SIZE);
        string seg_body = stream_in().read(seg_size - (syn_flag() ? 1 : 0));
        TCPSegment seg;

        // build new segment
        seg.payload() = move(seg_body);
        seg.header().seqno = next_send;
        seg.header().syn = syn_flag();
        seg.header().fin = eof_flag() && (seg.payload().size() <= TCPConfig::MAX_PAYLOAD_SIZE) &&
                           (seg.length_in_sequence_space() < remained_window);

        // adjust seqno
        _next_seqno += seg.length_in_sequence_space();
        next_send = wrap(_next_seqno, _isn);

        // make copy of segment to store outgoing segment
        // and send a segment
        if (seg.length_in_sequence_space() != 0)
            timer.set_ticking(true);
        _seginfo_buffer.push(SegmentInfo(seg.header().seqno,
                                         seg.length_in_sequence_space(),
                                         string(seg.payload().str()),
                                         seg.header().syn,
                                         seg.header().fin));
        _segments_out.push(seg);
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    // adjust window size
    receiver_window_size = window_size;

    // ignore invalid or passed ackno
    if (unwrap(ackno, _isn, _next_seqno) <= unwrap(next_ack, _isn, _next_seqno))
        return;
    if (unwrap(ackno, _isn, _next_seqno) > unwrap(next_send, _isn, _next_seqno))
        return;

    // delete already acked outgoing segments
    while (!_seginfo_buffer.empty()) {
        SegmentInfo &first_unacked = _seginfo_buffer.front();
        if (unwrap(first_unacked.seqno, _isn, _next_seqno) + first_unacked.size - 1 < unwrap(ackno, _isn, _next_seqno))
            _seginfo_buffer.pop();
        else
            break;
    }

    // adjust ackno
    next_ack = ackno;

    // timer manipulation
    timer.reset_retx_num();
    timer.reset_time();

    if (bytes_in_flight() == 0UL)
        timer.set_ticking(false);
    else
        timer.set_ticking(true);
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    // time goes on
    if (timer.is_ticking())
        timer.time_elapsed(ms_since_last_tick);

    if (timer.is_time_expired()) {
        // build a segment and send
        SegmentInfo &seginfo = _seginfo_buffer.front();
        TCPSegment seg;
        seg.payload() = move(string(seginfo.payload));
        seg.header().seqno = seginfo.seqno;
        seg.header().syn = seginfo.syn;
        seg.header().fin = seginfo.fin;
        _segments_out.push(move(seg));

        // timer manipulation
        if (receiver_window_size > 0)
            timer.increase_retx_num();

        timer.reset_time();
        timer.set_ticking(true);
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return timer.get_retx_num(); }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = next_send;
    _segments_out.push(move(seg));
}
