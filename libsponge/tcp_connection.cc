#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_latest_receive; }

void TCPConnection::segment_received(const TCPSegment &seg) { 
    /*
    cerr << "DEBUG: segment received with " << (seg.header().ack ? 'A' : '-') << (seg.header().rst ? 'R' : '-');
    cerr <<  (seg.header().syn ? 'S' : '-') << (seg.header().fin ? 'F' : '-');
    cerr << ", seqno " << seg.header().seqno.raw_value() << ", ackno " << seg.header().ackno.raw_value();
    cerr << ", payload size " << seg.payload().size() << endl;
    */
    if(seg.header().rst) {
        // reset connection
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _is_active = false;
        return;
    }
    _time_since_latest_receive = 0UL;

    _receiver.segment_received(seg);
    if(seg.header().ack) {
        _sender.ack_received(seg.header().ackno, seg.header().win);
        if(seg.header().ackno == _sender.next_seqno()) {
            _sender.fill_window();
            _send_segments();
        }
    }

    if(seg.length_in_sequence_space() > 0) {
        _sender.fill_window();
        _sender.segments_out().empty() ? _send_empty_segment() : _send_segments();
    }

    if(_prereq_1() && !_sender.stream_in().eof()) {
        // cerr << "DEBUG: stop lingering" << endl;
        _linger_after_streams_finish = false;
    }

    // keep-alive control
    if (_receiver.ackno().has_value() && 
        seg.length_in_sequence_space() == 0 && 
        seg.header().seqno.raw_value() + 1 == _receiver.ackno().value().raw_value()) {
        // cerr << "DEBUG: keep-alive control" << endl;
        _send_empty_segment();
    }
}

bool TCPConnection::active() const { return _is_active; }

size_t TCPConnection::write(const string &data) {
    // cerr << "DEBUG: write data" << endl;
    size_t written_bytes = _sender.stream_in().write(data);
    _sender.fill_window();
    _send_segments();
    return written_bytes;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    // cerr << "DEBUG: " << ms_since_last_tick << " tick elapsed" << endl;
    _time_since_latest_receive += ms_since_last_tick;
    _sender.tick(ms_since_last_tick);

    // retransmit oldest segment
    if(!_sender.segments_out().empty()) {
        if(_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) 
            _send_rst_and_deactive();
        else {
            TCPSegment& seg = _sender.segments_out().front();
            seg.header().ack = true;
            seg.header().ackno = _receiver.ackno().value();
            seg.header().win = _receiver.window_size();
            _segments_out.push(move(seg));
            _sender.segments_out().pop();
        }
    }

    if(_prereq_1() && _prereq_2() && _prereq_3()) {
        if(!_linger_after_streams_finish) {
            // cerr << "DEBUG: clean connection done" << endl;
            _is_active = false;
        }
        else {
            if(_linger_after_streams_finish &&  _time_since_latest_receive >= 10 * _cfg.rt_timeout) {
                // cerr << "DEBUG: connection done by timeout" << endl;
                _is_active = false;
            }
        }
    }
}

void TCPConnection::end_input_stream() { 
    // set EOF and send remaining segments
    // cerr << "DEBUG: end input stream" << endl;
    _sender.stream_in().end_input(); 
    _sender.fill_window();
    _send_segments();
    }

void TCPConnection::connect() {
    // cerr << "DEBUG: connect" << endl;
    _sender.fill_window();
    _send_segments();
}

void TCPConnection::_send_rst_and_deactive() {
    // cerr << "DEBUG: send RST segment" << endl;
    _sender.send_empty_segment();
    TCPSegment& seg = _sender.segments_out().back();
    seg.header().rst = true;
    _segments_out.push(move(seg));
    _sender.segments_out().pop();


    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
    _is_active = false;
}

void TCPConnection::_send_segments() {
    // cerr << "DEBUG: _sent_segments() entered" << endl;
    while(!_sender.segments_out().empty()) {
        TCPSegment& seg = _sender.segments_out().front();

        seg.header().ack = _receiver.ackno().has_value();
        seg.header().ackno = _receiver.ackno().has_value() ?_receiver.ackno().value() : WrappingInt32(0);
        seg.header().win = _receiver.window_size();
        /*
        cerr << "DEBUG: segment sent with " << (seg.header().ack ? 'A' : '-') << (seg.header().rst ? 'R' : '-');
        cerr <<  (seg.header().syn ? 'S' : '-') << (seg.header().fin ? 'F' : '-');
        cerr << ", seqno " << seg.header().seqno.raw_value() << ", ackno " << seg.header().ackno.raw_value();
        cerr << ", payload size " << seg.payload().size() << endl;
        */

        _segments_out.push(move(seg));
        _sender.segments_out().pop();
    }
}

void TCPConnection::_send_empty_segment() {
    _sender.send_empty_segment();
    TCPSegment& seg = _sender.segments_out().back();
    seg.header().ack = true;
    seg.header().ackno = _receiver.ackno().value();
    seg.header().win = _receiver.window_size();

    /*
    cerr << "DEBUG: sent empty segment with " << (seg.header().ack ? 'A' : '-') << (seg.header().rst ? 'R' : '-');
    cerr <<  (seg.header().syn ? 'S' : '-') << (seg.header().fin ? 'F' : '-');
    cerr << ", seqno " << seg.header().seqno.raw_value() << ", ackno " << seg.header().ackno.raw_value();
    cerr << ", payload size " << seg.payload().size() << endl;
    */

    _segments_out.push(move(seg));
    _sender.segments_out().pop();

}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";
            _send_rst_and_deactive();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
