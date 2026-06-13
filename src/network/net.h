#ifndef NET_H
#define NET_H

#include <stdint.h>
#include <stddef.h>

// Ethernet constants
#define ETHERTYPE_IP   0x0800
#define ETHERTYPE_ARP  0x0806
#define ETH_ADDR_LEN   6
#define ETH_HDR_LEN    14

// IP constants
#define IP_PROTO_ICMP  1
#define IP_PROTO_TCP   6
#define IP_HDR_LEN     20

// TCP constants
#define TCP_HDR_LEN    20
#define TCP_FLAG_FIN   0x01
#define TCP_FLAG_SYN   0x02
#define TCP_FLAG_RST   0x04
#define TCP_FLAG_PSH   0x08
#define TCP_FLAG_ACK   0x10
#define TCP_FLAG_URG   0x20

// Network configuration (static for now)
#define NET_IP_ADDR    0x0A000002  // 10.0.0.2
#define NET_GATEWAY    0x0A000002  // 10.0.0.2 (QEMU user-mode gateway)
#define NET_NETMASK    0xFF000000  // 255.0.0.0
#define NET_MAC_ADDR   {0x52, 0x54, 0x00, 0x12, 0x34, 0x56}

// Packet buffer
#define NET_MTU        1500
#define NET_BUF_SIZE   2048

// Header structs (packed)
typedef struct __attribute__((packed)) {
    uint8_t  dst[ETH_ADDR_LEN];
    uint8_t  src[ETH_ADDR_LEN];
    uint16_t ethertype;
} eth_header_t;

typedef struct __attribute__((packed)) {
    uint8_t  ihl_version;
    uint8_t  tos;
    uint16_t total_len;
    uint16_t id;
    uint16_t flags_frag;
    uint8_t  ttl;
    uint8_t  proto;
    uint16_t checksum;
    uint32_t src;
    uint32_t dst;
} ip_header_t;

typedef struct __attribute__((packed)) {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq;
    uint32_t ack;
    uint8_t  data_offset;
    uint8_t  flags;
    uint16_t window;
    uint16_t checksum;
    uint16_t urgent;
} tcp_header_t;

// ARP operations
void arp_init(void);
int  arp_resolve(uint32_t ip, uint8_t* mac_out);
void arp_receive(const uint8_t* frame, size_t len);

// IP/TCP transmit helpers
int net_send_eth(uint16_t ethertype, const uint8_t* dst_mac,
                 const uint8_t* payload, size_t payload_len);
int net_send_ip(uint8_t proto, uint32_t dst_ip,
                const uint8_t* payload, size_t payload_len);
int net_send_tcp(uint16_t src_port, uint16_t dst_port, uint32_t dst_ip,
                 uint32_t seq, uint32_t ack, uint8_t flags,
                 const uint8_t* payload, size_t payload_len);

// Packet dispatch
void ip_receive(const uint8_t* packet, size_t len);
void tcp_receive(const uint8_t* packet, size_t len);

// Stack init
void net_stack_init(void);
void tcp_init(void);
void tcp_periodic(void);

#endif // NET_H
