Assignment 4 Writeup
=============

My name: [youngjae Park]

My POVIS ID: [youngpark]

My student ID (numeric): [20190980]

This assignment took me about [15] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Your benchmark results (without reordering, with reordering): [0.07, 0.11]

Program Structure and Design of the TCPConnection:
[
    TCPConnection::remaining_outbound_capacity()
    returns the remaining capacity of outbound stream. simply refer sender's capacity.

    TCPConnection::bytes_in_flight()
    returns the number of bytes in flight. we already implemented this at TCPSender.

    TCPConnection::unassembled_bytes()
    returns the number of unassembled bytes. Thank you TCPReceiver for giving this method!

    TCPConnection::active()
    returns if this TCPConnection is active.

    TCPConnection::connect()
    fill sender's window and send segments. the first segment contains SYN flag.

    TCPConnection::end_input_stream()
    let the sender's input stream end, fill sender's window with remaining bytes and send.

    TCPConnection::segment_received()
    if received segment contains RST flag, set error and deactive.
    reset lingering time.
    deliver segment to receiver. if ACK flag is set, deliver segment to sender.
    and if segment's ackno is sender's next seqno, send ongoing bytes.
    if segment occupies any seqno, fill window and send segments(or empty segment if window is empty)
    if inbound stream has ended and outbound stream hasn't ended yet, no need to linger.
    if keep-alive segments comes, just send empty segment to respond.

    TCPConnection::write()
    write bytes into inbound stream, store written length. this value will be returned.
    fill window, send segments, and return written bytes.

    TCPConnection::tick()
    increases lingering time and let sender know the lapse of time.
    if sender made a segment to be retransmitted, check retx attempts.
    if # of retx exceeds its maximum, send RST and deactive. otherwise, send segment to be retxed.
    if prereq 1 through 3 are satisfied, check lingering and lingering time.
    if TCPConnection is not lingering or the lingering time is done, close connection cleanly.

    TCPConnection::_send_segments()
    send all segments made by sender.
    before sending, fill ackno and window size.

    TCPConnection::~TCPConnection()
    if TCPConnection destructed with active()=true, 
    send RST and deactive.

]

Implementation Challenges:
[
    The most difficult part was determining when to send segment or not.
    TCPConnection should send segment when needed, and should not send when not needed.
    any other behaviours are error.

    assignment document and TCP state diagram helped me a lot.
]

Remaining Bugs:
[
    some automated TCP test has failed, all of them are named as "t_*_128K_8K_lL".
    perhaps it causes the overflow of buffer or connection timeout.
    when connecting with other TCPConnection client, "Succeccfully connectedto x.x.x.x:x" message,
    but segments are sent correctly.
    also, benchmark score didn't exceed 0.10GB/s without ordering.
    since my receiver and sender stores outgoing and in-flight bytes, 
    it might lowered the performance in out-of-order case.
]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
