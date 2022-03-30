#ifndef SPONGE_LIBSPONGE_TCP_SENDER_HH
#define SPONGE_LIBSPONGE_TCP_SENDER_HH

#include "byte_stream.hh"
#include "tcp_config.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"

#include <functional>
#include <queue>
#include <vector>

//! \brief The "sender" part of a TCP implementation.

class retxTimer {
  private:
    const size_t initial_time;
    size_t current_time;
    size_t retx_num;
    bool ticking;

  public:
    retxTimer(size_t init_time) : initial_time(init_time), current_time(init_time), retx_num(0), ticking(false){};
    size_t get_current_time() const { return current_time; }
    void time_elapsed(size_t time) { current_time = (current_time > time ? current_time - time : 0); }
    void reset_time() { current_time = initial_time << retx_num; }
    bool is_time_expired() const { return current_time == 0; }
    bool is_ticking() const { return ticking; }
    void set_ticking(bool tking) { ticking = tking; }
    void increase_retx_num() { retx_num++; }
    void reset_retx_num() { retx_num = 0; }
    size_t get_retx_num() const { return retx_num; }
};

//! Accepts a ByteStream, divides it up into segments and sends the
//! segments, keeps track of which segments are still in-flight,
//! maintains the Retransmission Timer, and retransmits in-flight
//! segments if the retransmission timer expires.
class TCPSender {
  private:
    //! our initial sequence number, the number for our SYN.
    const WrappingInt32 _isn;
    WrappingInt32 next_send;
    WrappingInt32 next_ack;

    //! outbound queue of segments that the TCPSender wants sent
    std::queue<TCPSegment> _segments_out{};

    //! retransmission timer for the connection
    const unsigned int _initial_retransmission_timeout;

    uint16_t receiver_window_size;

    //! outgoing stream of bytes that have not yet been sent
    ByteStream _stream;

    //! the (absolute) sequence number for the next byte to be sent
    uint64_t _next_seqno{0};
    retxTimer timer;

    bool syn_flag() const { return _next_seqno == 0UL; }
    bool eof_flag() const { return stream_in().eof(); }
    bool fin_flag() const { return _next_seqno == stream_in().bytes_written() + 2; }

    struct SegmentInfo {
        WrappingInt32 seqno;
        size_t size;
        std::string payload;
        bool syn, fin;

        SegmentInfo(WrappingInt32 start_seq, size_t seg_size, std::string seg_payload, bool seg_syn, bool seg_fin)
            : seqno(start_seq), size(seg_size), payload(seg_payload), syn(seg_syn), fin(seg_fin) {}

        bool operator==(const SegmentInfo &other) {
            return seqno == other.seqno && size == other.size && payload == other.payload && syn == other.syn &&
                   fin == other.fin;
        }
    };

    std::queue<SegmentInfo> _seginfo_buffer{};

  public:
    //! Initialize a TCPSender
    TCPSender(const size_t capacity = TCPConfig::DEFAULT_CAPACITY,
              const uint16_t retx_timeout = TCPConfig::TIMEOUT_DFLT,
              const std::optional<WrappingInt32> fixed_isn = {});

    //! \name "Input" interface for the writer
    //!@{
    ByteStream &stream_in() { return _stream; }
    const ByteStream &stream_in() const { return _stream; }
    //!@}

    //! \name Methods that can cause the TCPSender to send a segment
    //!@{

    //! \brief A new acknowledgment was received
    void ack_received(const WrappingInt32 ackno, const uint16_t window_size);

    //! \brief Generate an empty-payload segment (useful for creating empty ACK segments)
    void send_empty_segment();

    //! \brief create and send segments to fill as much of the window as possible
    void fill_window();

    //! \brief Notifies the TCPSender of the passage of time
    void tick(const size_t ms_since_last_tick);
    //!@}

    //! \name Accessors
    //!@{

    //! \brief How many sequence numbers are occupied by segments sent but not yet acknowledged?
    //! \note count is in "sequence space," i.e. SYN and FIN each count for one byte
    //! (see TCPSegment::length_in_sequence_space())
    size_t bytes_in_flight() const;

    //! \brief Number of consecutive retransmissions that have occurred in a row
    unsigned int consecutive_retransmissions() const;

    //! \brief TCPSegments that the TCPSender has enqueued for transmission.
    //! \note These must be dequeued and sent by the TCPConnection,
    //! which will need to fill in the fields that are set by the TCPReceiver
    //! (ackno and window size) before sending.
    std::queue<TCPSegment> &segments_out() { return _segments_out; }
    //!@}

    //! \name What is the next sequence number? (used for testing)
    //!@{

    //! \brief absolute seqno for the next byte to be sent
    uint64_t next_seqno_absolute() const { return _next_seqno; }

    //! \brief relative seqno for the next byte to be sent
    WrappingInt32 next_seqno() const { return wrap(_next_seqno, _isn); }
    //!@}
};

#endif  // SPONGE_LIBSPONGE_TCP_SENDER_HH
