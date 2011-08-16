/*
 * libtins is a net packet wrapper library for crafting and
 * interpreting sniffed packets.
 *
 * Copyright (C) 2011 Nasel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <string>
#include <cstring>
#include <cassert>
#include <netinet/in.h>
#include "arp.h"
#include "ethernetII.h"
#include "rawpdu.h"
#include "utils.h"


using namespace std;

Tins::PDU* Tins::ARP::make_arp_request(const std::string& iface,
                                       const std::string& target,
                                       const std::string& sender,
                                       const uint8_t* hw_snd) {
    uint32_t target_ip = Tins::Utils::resolve_ip(target);
    uint32_t sender_ip = Tins::Utils::resolve_ip(sender);
    return make_arp_request(iface, target_ip, sender_ip, hw_snd);
}

Tins::PDU* Tins::ARP::make_arp_request(const std::string& iface,
                                       uint32_t target,
                                       uint32_t sender,
                                       const uint8_t* hw_snd) {

    /* Create ARP packet and set its attributes */
    ARP* arp = new ARP();
    arp->_arp.ar_tip = target;
    arp->_arp.ar_sip = sender;
    if (hw_snd) {
        memcpy(arp->_arp.ar_sha, hw_snd, 6);
    }
    arp->_arp.ar_op = Tins::ARP::REQUEST;

    /* Create the EthernetII PDU with the ARP PDU as its inner PDU */
    EthernetII* eth = new EthernetII(iface, Tins::EthernetII::BROADCAST, hw_snd, arp);
    return eth;
}

Tins::PDU* Tins::ARP::make_arp_reply(const std::string& iface,
                                     const std::string& target,
                                     const std::string& sender,
                                     const uint8_t* hw_tgt,
                                     const uint8_t* hw_snd) {

    uint32_t target_ip = Tins::Utils::resolve_ip(target);
    uint32_t sender_ip = Tins::Utils::resolve_ip(sender);
    return make_arp_reply(iface, target_ip, sender_ip, hw_tgt, hw_snd);
}

Tins::PDU* Tins::ARP::make_arp_reply(const std::string& iface,
                                     uint32_t target,
                                     uint32_t sender,
                                     const uint8_t* hw_tgt,
                                     const uint8_t* hw_snd) {

    /* Create ARP packet and set its attributes */
    ARP* arp = new ARP();
    arp->_arp.ar_tip = target;
    arp->_arp.ar_sip = sender;
    memcpy(arp->_arp.ar_sha, hw_snd, 6);
    memcpy(arp->_arp.ar_tha, hw_tgt, 6);
    arp->_arp.ar_op = Tins::ARP::REPLY;

    /* Create the EthernetII PDU with the ARP PDU as its inner PDU */
    EthernetII* eth = new EthernetII(iface, hw_tgt, hw_snd, arp);
    return eth;
}

Tins::ARP::ARP() : PDU(0x0608) {
    std::memset(&_arp, 0, sizeof(arphdr));
    _arp.ar_hrd = 0x0100;
    _arp.ar_pro = 0x0008;
    _arp.ar_hln = 6;
    _arp.ar_pln = 4;
}

Tins::ARP::ARP(arphdr *arp_ptr) : PDU(0x0608) {
    memcpy(&_arp, arp_ptr, sizeof(arphdr));
}

void Tins::ARP::sender_hw_addr(uint8_t* new_snd_hw_addr) {
    memcpy(this->_arp.ar_sha, new_snd_hw_addr, 6); //Should this use hardware address' length?
}

void Tins::ARP::sender_ip_addr(uint32_t new_snd_ip_addr) {
    this->_arp.ar_sip = new_snd_ip_addr;
}

void Tins::ARP::target_hw_addr(uint8_t* new_tgt_hw_addr) {
    memcpy(this->_arp.ar_tha, new_tgt_hw_addr, 6); //Should this use hardware address' length?
}

void Tins::ARP::target_ip_addr(uint32_t new_tgt_ip_addr) {
    this->_arp.ar_tip = new_tgt_ip_addr;
}

void Tins::ARP::hw_addr_format(uint16_t new_hw_addr_fmt) {
    this->_arp.ar_hrd = new_hw_addr_fmt;
}

void Tins::ARP::prot_addr_format(uint16_t new_prot_addr_fmt) {
    this->_arp.ar_pro = new_prot_addr_fmt;
}

void Tins::ARP::hw_addr_length(uint8_t new_hw_addr_len) {
    this->_arp.ar_hln = new_hw_addr_len;
}

void Tins::ARP::prot_addr_length(uint8_t new_prot_addr_len) {
    this->_arp.ar_pln = new_prot_addr_len;
}

void Tins::ARP::opcode(Flags new_opcode) {
    this->_arp.ar_op = new_opcode;
}

void Tins::ARP::set_arp_request(const std::string& ip_tgt, const std::string& ip_snd, const uint8_t* hw_snd) {
    this->_arp.ar_tip = Utils::resolve_ip(ip_tgt);
    this->_arp.ar_sip = Utils::resolve_ip(ip_snd);
    if (hw_snd) {
        memcpy(this->_arp.ar_sha, hw_snd, 6);
    }
    this->_arp.ar_op = REQUEST;
}

void Tins::ARP::set_arp_reply(const std::string& ip_tgt,
                              const std::string& ip_snd,
                              const uint8_t* hw_tgt,
                              const uint8_t* hw_snd) {

    this->_arp.ar_tip = Utils::resolve_ip(ip_tgt);
    this->_arp.ar_sip = Utils::resolve_ip(ip_snd);
    memcpy(this->_arp.ar_tha, hw_tgt, 6);
    memcpy(this->_arp.ar_sha, hw_snd, 6);
    this->_arp.ar_op = REPLY;

}

uint32_t Tins::ARP::header_size() const {
    return sizeof(arphdr);
}

void Tins::ARP::write_serialization(uint8_t *buffer, uint32_t total_sz, const PDU *) {
    assert(total_sz >= sizeof(arphdr));
    memcpy(buffer, &_arp, sizeof(arphdr));
}

bool Tins::ARP::matches_response(uint8_t *ptr, uint32_t total_sz) {
    if(total_sz < sizeof(arphdr))
        return false;
    arphdr *arp_ptr = (arphdr*)ptr;
    return arp_ptr->ar_sip == _arp.ar_tip && arp_ptr->ar_tip == _arp.ar_sip;
}

Tins::PDU *Tins::ARP::clone_packet(uint8_t *ptr, uint32_t total_sz) {
    if(total_sz < sizeof(arphdr))
        return 0;
    arphdr *arp_ptr = (arphdr*)ptr;
    PDU *child = 0, *cloned;
    if(total_sz > sizeof(arphdr)) {
        if(inner_pdu()) {
            child = inner_pdu()->clone_packet(ptr + sizeof(arphdr), total_sz - sizeof(arphdr));
            if(!child)
                return 0;
        }
        else
            child = new RawPDU(ptr + sizeof(arphdr), total_sz - sizeof(arphdr));
    }
    cloned = new ARP(arp_ptr);
    cloned->inner_pdu(child);
    return cloned;
}