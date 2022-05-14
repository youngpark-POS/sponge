#include "router.hh"

#include <iostream>

using namespace std;

// Dummy implementation of an IP router

// Given an incoming Internet datagram, the router decides
// (1) which interface to send it out on, and
// (2) what next hop address to send it to.

// For Lab 6, please replace with a real implementation that passes the
// automated checks run by `make check_lab6`.

// You will need to add private members to the class declaration in `router.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

//! \param[in] route_prefix The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
//! \param[in] prefix_length For this route to be applicable, how many high-order (most-significant) bits of the route_prefix will need to match the corresponding bits of the datagram's destination address?
//! \param[in] next_hop The IP address of the next hop. Will be empty if the network is directly attached to the router (in which case, the next hop address should be the datagram's final destination).
//! \param[in] interface_num The index of the interface to send the datagram out on.
void Router::add_route(const uint32_t route_prefix,
                       const uint8_t prefix_length,
                       const optional<Address> next_hop,
                       const size_t interface_num) {
    cerr << "DEBUG: adding route " << Address::from_ipv4_numeric(route_prefix).ip() << "/" << int(prefix_length)
         << " => " << (next_hop.has_value() ? next_hop->ip() : "(direct)") << " on interface " << interface_num << "\n";

    routing_table.push_back(RoutingInfo{route_prefix, prefix_length, next_hop, interface_num});
}

bool Router::is_matched(uint32_t target, uint32_t ip, uint8_t prefix) {
    if (prefix == 0)
        return true;
    else
        return (target >> (32 - prefix)) == (ip >> (32 - prefix));
}

//! \param[in] dgram The datagram to be routed
void Router::route_one_datagram(InternetDatagram &dgram) {
    if (dgram.header().ttl <= 1)
        return;  // TTL expired

    uint32_t target_ip = dgram.header().dst;
    RoutingInfo longest_match;
    int32_t longest_len = -1;
    for (auto iter = routing_table.begin(); iter != routing_table.end(); iter++) {
        if (is_matched(target_ip, iter->route_prefix, iter->prefix_len)) {
            int32_t new_len = static_cast<int32_t>(iter->prefix_len);
            if (new_len > longest_len) {
                longest_match = *iter;
                longest_len = new_len;
            }
        }
    }

    if (longest_len == -1)
        return;  // no matching route
    size_t ifnum = longest_match.interface_num;
    Address nexthop =
        longest_match.next_hop.has_value() ? longest_match.next_hop.value() : Address::from_ipv4_numeric(target_ip);

    dgram.header().ttl -= 1;
    interface(ifnum).send_datagram(dgram, nexthop);
}

void Router::route() {
    // Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
    for (auto &interface : _interfaces) {
        auto &queue = interface.datagrams_out();
        while (not queue.empty()) {
            route_one_datagram(queue.front());
            queue.pop();
        }
    }
}
