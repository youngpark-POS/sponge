Assignment 6 Writeup
=============

My name: [youngjae park]

My POVIS ID: [youngpark]

My student ID (numeric): [20190980]

This assignment took me about [5] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the Router:
[
    Router::RoutingInfo
    routing infomation class that consisted of route prefix/length, next hop, interface number

    Router::routing_table
    a list that contains routing infomation.
    modified by add_route() method.

    Router::is_matched()
    returns whether target address matches with routing table entry or not.

    Router::add_route()
    add given routing infomation into the routing table.

    Router::route_one_datagram()
    forward a given datagram.
    search all routing table and determine whether a route prefix matches with target address.
    if so, check the length of matching prefix and update longest match if needed.
    finally, forward datagram to the appropriate network interface if the matching was found.
    if not, drop the datagram.
]

Implementation Challenges:
[
    It was not a tough work to implement longest matching algorithm.
    But, designing longest matching algorithm was a little bit difficult.
]

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
