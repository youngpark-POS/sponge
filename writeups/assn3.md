Assignment 3 Writeup
=============

My name: [youngjae Park]

My POVIS ID: [youngpark]

My student ID (numeric): [20190980]

This assignment took me about [20] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPSender:
[
    TCPSender: a class that send TCP segment

    TCPSender::fill_window
    make segments and send then to receiver as much as possible.
    it calculates the length of segment size based on receiver's window, max segment size, and so on.
    And read bytes from instream, push them into segment and manipulate its header.
    SYN/FIN flag, seqno should be written by TCPSender.
    as long as making segment is done, adjust sender's seqno and save its copy
    to deal with outstanding segment.
    finally, send segment and turn the timer ON.
    this procedure runs until 
    1. no more windows are available / 2. instream is empty / 3. all data has been sent.

    TCPSender::ack_received
    first, revise receiver's window size.
    and if received ACK is already acked or outstanding, ignore it.
    delete the copies of acked segments.
    after that, adjust next bytes to be acked.
    reset retransmission # and timer, turn OFF the timer if there are no outstanding segments.

    TCPSender::tick
    let the timer elapse for given amount of time.
    it timer is expired, peek the oldest outstanding segment and retransmit it.
    if receiver's window size is nonzero, doubles the RTO.
    and reset timer and turn the timer ON.

    TCPSender::consecutive_retransmission
    returns the number of consecutive retransmission.
    it refers the timer's retx_num.

    TCPSender::send_empty_segment
    send empty segment. only the "seqno" field is filled.

    TCPSender::bytes_in_flight
    returns the number of outstanding bytes.
    calculated by (next byte to be sent) - (next byte to be acked)
]

Implementation Challenges:
[
    SYN flag can be calculated with simple condition: next abs seqno == 0.
    but, it was quite difficult to find a conditions that set FIN flag.
    the followings are conditions twhich set FIN flag that I found.
    first, EOf of instream should be set. 
    if not, instream is not yet finished.
    second, sequence payload's length should not exceed MAX_PAYLOAD_SIZE.
    if not, FIN flag cannot be sent though instream set EOF.
    and last, overall header size should not exceed remaining window size.
    if not, FIN flag cannot be sent too.

    by solving this problem, I concretely understood about differences of 
    sequence space length and payload length, and also seqno and abs-seqno.
]

Remaining Bugs:
[
    As the document said, there is no extreme edge case where this program doesn't work well
    if all the test was passed.
]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
