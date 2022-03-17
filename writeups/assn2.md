Assignment 2 Writeup
=============

My name: [Yeongjae Park]

My POVIS ID: [youngpark]

My student ID (numeric): [20190980]

This assignment took me about [10] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPReceiver and wrap/unwrap routines:
[
    wrap():
    since absoulte seqno = "n"th bit from isn,
    add two uint64 numbers(isn, n) and modulo to 1<<32(let us call 1<<32 PERIOD). 

    unwrap():
    Among all candidates which can become seqno, choose one that
    the distance between checkpoint and it is less then PERIOD(let us call it TEMP_SEQNO).
    (i.e. when n = 100,isn = 99,checkpoint = PERIOD * 2, choose seqno = PERIOD * 2 + 1.)
    and calculate the offset = seqno_candidate - checkpoint (signed operation)
    if offset is too large(more than PERIOD/2), choose TEMP_SEQNO - PERIOD.
    but, choose TEMP_SEQNO when TEMP-SEQNO < 0.
    if offset is too small(less than -PERIOD/2), choose TEMP_SEQNO + PERIOD.
    else, choose TEMP_SEQNO. but if TEMP_SEQNO < 0, choose TEMP_SEQNO + PERIOD.

    TCPReceiver::window_size():
    first unread byte equals to stream_out().byte_read().
    first unacceptable byte equals to stream_out().byte_written().
    therefore, window_size = _capacity - (stream_out().byte_written() - stream_out().byte_read())!

    TCPReceiver::segment_received():
    when seq is served, check syn_get flag and set isn.
    when fin is served, check fin_get flag.
    push payload to reassembler with index and fin flag.
    (seq and flag should not be pushed.)
    update abs_seqno = first unacceptable byte (+ syn flag + fin flag if eof).
    update seqno by wrapping abs_seqno.

    TCPReceiver::ackno():
    if seq is not served, return nullopt.
    else, return current ackno ("seq" variable in my code).

]

Implementation Challenges:
[
    wrap(): 
    Just warmup :)
    unwarp():
    When should I convert unsigned to signed and vice versa?
    How can I adjust seqno so that it can be nonnegative number?
    TCPReceiver::window_size():
    How can I know first unread byte and first unacceptable byte?
    TCPReceiver::segment_received():
    How can I push payload with appropriate index and eof signal?
    How can I calculate sequence number after pushing?
    TCPReceiver::ackno():
    Should I implement seqno and ackno separately?
]

Remaining Bugs:
[
    Perhaps none...?
]

- Optional: I had unexpected difficulty with: [converting abs_seqno to seqno]

- Optional: I think you could make this assignment better by: [None]

- Optional: I was surprised by: [None]

- Optional: I'm not sure about: [None]
