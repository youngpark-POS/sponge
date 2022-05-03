#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>

// Dummy implementation of a network interface
// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

// For Lab 5, please replace with a real implementation that passes the
// automated checks run by `make check_lab5`.

// You will need to add private members to the class declaration in `network_interface.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address) {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();
    optional<EthernetAddress> eth_addr = _find_mapping(next_hop);
    if (!eth_addr.has_value()) {
        if (_is_arp_sent(next_hop))
            return;

        _send_arp(next_hop_ip, ETHERNET_BROADCAST, ARPMessage::OPCODE_REQUEST);
        // store datagram into queue
        _datagram_list.push_back(_DatagramElem(dgram, next_hop));
    }
    // send Frame
    else {
        EthernetFrame frame;
        frame.header().type = EthernetHeader::TYPE_IPv4;
        frame.header().src = this->_ethernet_address;
        frame.header().dst = eth_addr.value();
        frame.payload() = dgram.serialize();
        _frames_out.push(move(frame));
    }
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    // drop invalid frame
    EthernetAddress eth_dst = frame.header().dst;
    if (eth_dst != this->_ethernet_address && eth_dst != ETHERNET_BROADCAST)
        return std::nullopt;

    if (frame.header().type == EthernetHeader::TYPE_ARP) {
        // if received frame is ARP
        ARPMessage arp_msg;
        if (arp_msg.parse(frame.payload()) != ParseResult::NoError)
            return std::nullopt;
        if (arp_msg.target_ip_address != this->_ip_address.ipv4_numeric())
            return std::nullopt;

        uint32_t sender_ip = arp_msg.sender_ip_address;
        EthernetAddress sender_eth = arp_msg.sender_ethernet_address;

        _mapping_table.push_back(_Mapping(sender_ip, sender_eth));

        if (arp_msg.opcode == ARPMessage::OPCODE_REQUEST) {
            _send_arp(sender_ip, sender_eth, ARPMessage::OPCODE_REPLY);
        }

        for (auto iter = _datagram_list.begin(); iter != _datagram_list.end();) {
            if (_find_mapping(iter->next_hop)) {
                send_datagram(iter->datagram, iter->next_hop);
                iter = _datagram_list.erase(iter);
            } else
                iter++;
        }
        return std::nullopt;
    } else if (frame.header().type == EthernetHeader::TYPE_IPv4) {
        // if receive frame is IPv4
        InternetDatagram ipv4_dgram;
        if (ipv4_dgram.parse(frame.payload()) != ParseResult::NoError)
            return std::nullopt;
        return ipv4_dgram;
    }

    else
        return std::nullopt;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) {
    for (auto iter = _mapping_table.begin(); iter != _mapping_table.end();) {
        if (iter->life <= ms_since_last_tick) {
            iter = _mapping_table.erase(iter);
            continue;
        }
        iter->life -= ms_since_last_tick;
        iter++;
    }
    for (auto iter = _datagram_list.begin(); iter != _datagram_list.end(); iter++) {
        if (iter->arp_cooldown <= ms_since_last_tick) {
            _send_arp(iter->next_hop.ipv4_numeric(), ETHERNET_BROADCAST, ARPMessage::OPCODE_REQUEST);
            iter->arp_cooldown = 5000;
            continue;
        }
        iter->arp_cooldown -= ms_since_last_tick;
    }
}

// find the mapping between IPv4 addr and Eth addr
optional<EthernetAddress> NetworkInterface::_find_mapping(Address addr) {
    for (auto iter = _mapping_table.begin(); iter != _mapping_table.end(); iter++) {
        if (iter->ipv4_addr == addr.ipv4_numeric())
            return iter->ether_addr;
    }
    return std::nullopt;
}

optional<EthernetAddress> NetworkInterface::_find_mapping(uint32_t addr) {
    for (auto iter = _mapping_table.begin(); iter != _mapping_table.end(); iter++) {
        if (iter->ipv4_addr == addr)
            return iter->ether_addr;
    }
    return std::nullopt;
}

void NetworkInterface::_send_arp(const uint32_t dst_ip, const EthernetAddress dst_eth, uint16_t opcode) {
    ARPMessage arp;
    EthernetFrame frame;

    static constexpr EthernetAddress ETHERNET_ZERO = {0, 0, 0, 0, 0, 0};

    arp.opcode = opcode;
    arp.sender_ethernet_address = this->_ethernet_address;
    arp.sender_ip_address = this->_ip_address.ipv4_numeric();
    arp.target_ethernet_address = (opcode == ARPMessage::OPCODE_REQUEST) ? ETHERNET_ZERO : dst_eth;
    arp.target_ip_address = dst_ip;

    frame.header().src = this->_ethernet_address;
    frame.header().dst = (opcode == ARPMessage::OPCODE_REQUEST) ? ETHERNET_BROADCAST : dst_eth;
    frame.header().type = EthernetHeader::TYPE_ARP;
    frame.payload() = arp.serialize();
    _frames_out.push(move(frame));
}

bool NetworkInterface::_is_arp_sent(const Address dst_ip) {
    for (auto iter = _datagram_list.begin(); iter != _datagram_list.end(); iter++) {
        if (iter->next_hop == dst_ip)
            return true;
    }
    return false;
}