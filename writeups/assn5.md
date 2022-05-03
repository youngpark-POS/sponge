Assignment 5 Writeup
=============

My name: [youngjae Park]

My POVIS ID: [youngpark]

My student ID (numeric): [20190980]

This assignment took me about [7] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Your benchmark results (without reordering, with reordering): [0.07, 0.11]

Program Structure and Design of the NetworkInterface:
[
    NetworkInterface has its own private property:
    - its own Ethernet & IP address
    - outgoing frame queue
    - IP to Ethernet mapping table
    - list of datagrams waiting for ARP reply
    - ... and some helper methods

    NetworkInterface::_find_mapping
    iterate all of mapping table, return Ethernet if mapping exists.
    otherwise, return nullopt.

    NetworkInterface::_send_arp
    send an ARP with given destination IP & Ethernet address and ARP opcode.
    only responsible for sending ARP.

    NetworkInterface::_is_arp_sent
    returns whether ARP with given IP address is sent within 5 secs.

    NetworkInterface::send_datagram
    with given IP address and datagram, find the mapping about given IP.
    if mapping exists, make a frame and send it.
    otherwise, check whether ARP with same IP has recently sent.
    if not sent, send ARP with given IP to make mapping and
    put a datagram into waiting list(=_datagram_list).

    NetworkInterface::recv_frame
    receive a frame.
    if its destination address is not met, drop it.
    if the frame is not ARP, just parse and return datagram.
    if the frame is ARP, parse and relieve sender's IP and ethernet address.
    make mapping with given information.
    if that frame is for ARP request, send ARP reply immediately.
    if some of the waiting datagrams are able to be sent with new mapping, 
    find and send'em.

    NetworkInterface::tick
    let the time elapse for all of the mappings and ARP cooldowns.
    if some of the mappings are expired, delete them.
    if some of the ARP has reached its retx cooldown(=5 sec),
    re-send the same ARP and adjust its cooldown.
]

Implementation Challenges:
[
    in this assignment, two lists are used to store mappings and datagrams.
    list is a simple and easy structure, but have relatively poor time complexity.
    I have tried to use std::map, but it was not sufficient to store three components:
    IP addr, ETH addr, and lifetime of the mapping.
]

Remaining Bugs:
[
    Errors about TCPConnection still burdens me.
    but, there are no bugs in assignment 5 only(NetworkInterface).
]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
