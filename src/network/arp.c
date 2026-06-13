#include "net.h"
#include "e1000.h"
#include "../../include/kernel.h"
#include "../../include/string.h"

#define ARP_TABLE_SIZE 4

typedef struct {
    uint32_t ip;
    uint8_t  mac[ETH_ADDR_LEN];
    int      valid;
} arp_entry_t;

static arp_entry_t arp_table[ARP_TABLE_SIZE];
static const uint8_t broadcast_mac[ETH_ADDR_LEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
static const uint8_t our_mac[ETH_ADDR_LEN] = NET_MAC_ADDR;

typedef struct __attribute__((packed)) {
    uint16_t htype;
    uint16_t ptype;
    uint8_t  hlen;
    uint8_t  plen;
    uint16_t oper;
    uint8_t  sha[ETH_ADDR_LEN];
    uint32_t spa;
    uint8_t  tha[ETH_ADDR_LEN];
    uint32_t tpa;
} arp_packet_t;

static inline uint16_t be16(uint16_t x) { return (x >> 8) | (x << 8); }
static inline uint32_t be32(uint32_t x) {
    return ((x >> 24) & 0xFF) | (((x >> 16) & 0xFF) << 8) |
           (((x >> 8) & 0xFF) << 16) | ((x & 0xFF) << 24);
}

void arp_init(void) {
    memset(arp_table, 0, sizeof(arp_table));
}

static arp_entry_t* arp_lookup(uint32_t ip) {
    for (int i = 0; i < ARP_TABLE_SIZE; i++) {
        if (arp_table[i].valid && arp_table[i].ip == ip) {
            return &arp_table[i];
        }
    }
    return NULL;
}

static void arp_store(uint32_t ip, const uint8_t* mac) {
    arp_entry_t* e = arp_lookup(ip);
    if (!e) {
        for (int i = 0; i < ARP_TABLE_SIZE; i++) {
            if (!arp_table[i].valid) {
                e = &arp_table[i];
                break;
            }
        }
    }
    if (!e) {
        // Evict first entry as a simple LRU.
        e = &arp_table[0];
    }
    e->ip = ip;
    memcpy(e->mac, mac, ETH_ADDR_LEN);
    e->valid = 1;
}

int arp_resolve(uint32_t ip, uint8_t* mac_out) {
    // On the same subnet or loopback to gateway.
    arp_entry_t* e = arp_lookup(ip);
    if (e) {
        memcpy(mac_out, e->mac, ETH_ADDR_LEN);
        return 0;
    }

    // Send an ARP request.
    arp_packet_t req;
    req.htype = be16(1);
    req.ptype = be16(ETHERTYPE_IP);
    req.hlen  = ETH_ADDR_LEN;
    req.plen  = 4;
    req.oper  = be16(1); // request
    memcpy(req.sha, our_mac, ETH_ADDR_LEN);
    req.spa   = be32(NET_IP_ADDR);
    memcpy(req.tha, broadcast_mac, ETH_ADDR_LEN);
    req.tpa   = be32(ip);

    net_send_eth(ETHERTYPE_ARP, broadcast_mac, (const uint8_t*)&req, sizeof(req));
    return -1;
}

void arp_receive(const uint8_t* frame, size_t len) {
    if (len < sizeof(arp_packet_t)) return;
    const arp_packet_t* arp = (const arp_packet_t*)frame;
    if (be16(arp->htype) != 1 || be16(arp->ptype) != ETHERTYPE_IP) return;

    uint32_t sender_ip = be32(arp->spa);
    arp_store(sender_ip, arp->sha);

    if (be16(arp->oper) == 1 && be32(arp->tpa) == NET_IP_ADDR) {
        // Reply to the request.
        arp_packet_t reply;
        reply.htype = be16(1);
        reply.ptype = be16(ETHERTYPE_IP);
        reply.hlen  = ETH_ADDR_LEN;
        reply.plen  = 4;
        reply.oper  = be16(2); // reply
        memcpy(reply.sha, our_mac, ETH_ADDR_LEN);
        reply.spa   = be32(NET_IP_ADDR);
        memcpy(reply.tha, arp->sha, ETH_ADDR_LEN);
        reply.tpa   = be32(sender_ip);
        net_send_eth(ETHERTYPE_ARP, arp->sha, (const uint8_t*)&reply, sizeof(reply));
    }
}
