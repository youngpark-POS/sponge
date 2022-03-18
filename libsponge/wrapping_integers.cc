#include "wrapping_integers.hh"

#define ABS(x) (((x) >= (0L)) ? (x) : (-(x)))

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    return WrappingInt32((n + uint64_t(isn.raw_value())) % (1UL << 32));
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    int64_t constexpr PERIOD = 1L << 32;
    int64_t cp_int64 = int64_t(checkpoint);
    int64_t temp_seqno;
    temp_seqno = ((n.raw_value()) - int64_t(isn.raw_value()) + PERIOD) % PERIOD;
    temp_seqno += ((cp_int64 >> 32) << 32);
    int64_t offset = temp_seqno - cp_int64;

    if (offset >= (PERIOD / 2))  // abs_seqno is too right
        return temp_seqno - PERIOD >= 0 ? uint64_t(temp_seqno - PERIOD) : uint64_t(temp_seqno);
    if (offset <= -(PERIOD / 2))  // abs_seqno is too left
        return uint64_t(temp_seqno + PERIOD);
    else  // appropriate distance
        return temp_seqno >= 0 ? uint64_t(temp_seqno) : uint64_t((PERIOD) + temp_seqno);
}
