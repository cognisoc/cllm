#include "net.h"
#include "e1000.h"
#include "../../include/kernel.h"
#include "../../include/string.h"

static const uint8_t our_mac[ETH_ADDR_LEN] = NET_MAC_ADDR;

static inline uint16_t be16(uint16_t x) { return (x >> 8) | (x << 8); }
static inline uint32_t be32(uint32_t x) {
    return ((x >> 24) & 0xFF) | (((x >> 16) & 0xFF) << 8) |
           (((x >> 8) & 0xFF) << 16) | ((x & 0xFF) << 24);
}

static uint16_t ip_checksum(const void* data, size_t len) {
    const uint16_t* p = data;
    uint32_t sum = 0;
    while (len > 1) {
        sum += *p++;
        len -= 2;
    }
    if (len) {
        sum += *(const uint8_t*)p;
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return (uint16_t)~sum;
}

int net_send_eth(uint16_t ethertype, const uint8_t* dst_mac,
                 const uint8_t* payload, size_t payload_len) {
    if (payload_len > NET_BUF_SIZE - ETH_HDR_LEN) return -1;

    uint8_t frame[NET_BUF_SIZE];
    eth_header_t* eth = (eth_header_t*)frame;
    memcpy(eth->dst, dst_mac, ETH_ADDR_LEN);
    memcpy(eth->src, our_mac, ETH_ADDR_LEN);
    eth->ethertype = be16(ethertype);
    memcpy(frame + ETH_HDR_LEN, payload, payload_len);

    return e1000_send_packet(frame, (uint16_t)(ETH_HDR_LEN + payload_len));
}

int net_send_ip(uint8_t proto, uint32_t dst_ip,
                const uint8_t* payload, size_t payload_len) {
    if (payload_len > NET_BUF_SIZE - ETH_HDR_LEN - IP_HDR_LEN) return -1;

    uint8_t packet[NET_BUF_SIZE];
    ip_header_t* ip = (ip_header_t*)packet;
    ip->ihl_version = 0x45;
    ip->tos = 0;
    ip->total_len = be16((uint16_t)(IP_HDR_LEN + payload_len));
    static uint16_t ip_id = 1;
    ip->id = be16(ip_id++);
    ip->flags_frag = 0;
    ip->ttl = 64;
    ip->proto = proto;
    ip->checksum = 0;
    ip->src = be32(NET_IP_ADDR);
    ip->dst = be32(dst_ip);
    ip->checksum = ip_checksum(ip, IP_HDR_LEN);

    memcpy(packet + IP_HDR_LEN, payload, payload_len);

    uint8_t dst_mac[ETH_ADDR_LEN];
    if (arp_resolve(dst_ip, dst_mac) != 0) {
        return -1; // ARP resolution pending; caller should retry.
    }

    return net_send_eth(ETHERTYPE_IP, dst_mac, packet, IP_HDR_LEN + payload_len);
}

static uint16_t tcp_checksum(uint32_t src_ip, uint32_t dst_ip,
                             const uint8_t* tcp_seg, size_t seg_len) {
    typedef struct __attribute__((packed)) {
        uint32_t src;
        uint32_t dst;
        uint8_t  zero;
        uint8_t  proto;
        uint16_t len;
    } pseudo_header_t;
    pseudo_header_t ph;
    ph.src = be32(src_ip);
    ph.dst = be32(dst_ip);
    ph.zero = 0;
    ph.proto = IP_PROTO_TCP;
    ph.len = be16((uint16_t)seg_len);

    uint32_t sum = 0;
    const uint8_t* pph = (const uint8_t*)&ph;
    for (size_t i = 0; i + 1 < sizeof(ph); i += 2) {
        sum += (uint16_t)pph[i] | ((uint16_t)pph[i + 1] << 8);
    }
    if (sizeof(ph) & 1) {
        sum += pph[sizeof(ph) - 1];
    }
    const uint8_t* p = tcp_seg;
    size_t len = seg_len;
    while (len > 1) {
        sum += (uint16_t)p[0] | ((uint16_t)p[1] << 8);
        p += 2;
        len -= 2;
    }
    if (len) {
        sum += *p;
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return (uint16_t)~sum;
}

int net_send_tcp(uint16_t src_port, uint16_t dst_port, uint32_t dst_ip,
                 uint32_t seq, uint32_t ack, uint8_t flags,
                 const uint8_t* payload, size_t payload_len) {
    if (payload_len > NET_BUF_SIZE - ETH_HDR_LEN - IP_HDR_LEN - TCP_HDR_LEN) return -1;

    uint8_t seg[NET_BUF_SIZE];
    tcp_header_t* tcp = (tcp_header_t*)seg;
    tcp->src_port = be16(src_port);
    tcp->dst_port = be16(dst_port);
    tcp->seq = be32(seq);
    tcp->ack = be32(ack);
    tcp->data_offset = 0x50; // 20 bytes / 4 = 5, in upper nibble
    tcp->flags = flags;
    tcp->window = be16(4096);
    tcp->checksum = 0;
    tcp->urgent = 0;

    memcpy(seg + TCP_HDR_LEN, payload, payload_len);
    tcp->checksum = tcp_checksum(NET_IP_ADDR, dst_ip, seg, TCP_HDR_LEN + payload_len);

    return net_send_ip(IP_PROTO_TCP, dst_ip, seg, TCP_HDR_LEN + payload_len);
}

extern uint32_t tcp_current_remote_ip;

void ip_receive(const uint8_t* packet, size_t len) {
    if (len < IP_HDR_LEN) return;
    const ip_header_t* ip = (const ip_header_t*)packet;
    if ((ip->ihl_version & 0xF0) != 0x40) return;

    size_t ihl = (ip->ihl_version & 0x0F) * 4;
    if (ihl < IP_HDR_LEN || len < ihl) return;
    if (be32(ip->dst) != NET_IP_ADDR) return;

    uint8_t proto = ip->proto;
    const uint8_t* payload = packet + ihl;
    size_t payload_len = len - ihl;
    tcp_current_remote_ip = be32(ip->src);

    if (proto == IP_PROTO_ICMP) {
        if (payload_len >= 8 && payload[0] == 8) { // echo request
            uint8_t reply[NET_BUF_SIZE];
            ip_header_t* out = (ip_header_t*)reply;
            out->ihl_version = 0x45;
            out->tos = 0;
            out->total_len = be16((uint16_t)(IP_HDR_LEN + payload_len));
            static uint16_t id = 1;
            out->id = be16(id++);
            out->flags_frag = 0;
            out->ttl = 64;
            out->proto = IP_PROTO_ICMP;
            out->checksum = 0;
            out->src = be32(NET_IP_ADDR);
            out->dst = ip->src;
            out->checksum = ip_checksum(out, IP_HDR_LEN);

            memcpy(reply + IP_HDR_LEN, payload, payload_len);
            reply[IP_HDR_LEN] = 0; // echo reply type
            reply[IP_HDR_LEN + 2] = 0;
            reply[IP_HDR_LEN + 3] = 0;
            uint16_t csum = ip_checksum(reply + IP_HDR_LEN, payload_len);
            reply[IP_HDR_LEN + 2] = csum & 0xFF;
            reply[IP_HDR_LEN + 3] = csum >> 8;

            uint8_t dst_mac[ETH_ADDR_LEN];
            if (arp_resolve(be32(ip->src), dst_mac) == 0) {
                net_send_eth(ETHERTYPE_IP, dst_mac, reply, IP_HDR_LEN + payload_len);
            }
        }
    } else if (proto == IP_PROTO_TCP) {
        tcp_receive(payload, payload_len);
    }
}
