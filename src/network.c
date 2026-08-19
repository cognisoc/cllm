#include "../include/network.h"
#include "network/net.h"
#include "network/pci.h"
#include "network/e1000.h"
#include "../include/string.h"
#include "../include/kernel.h"

// Initialize network stack
int network_init(void) {
    serial_write("network_init: Starting network initialization\n");

    // Initialize PCI subsystem
    serial_write("network_init: Enumerating PCI devices\n");
    pci_enumerate_devices();
    serial_write("network_init: PCI enumeration complete\n");

    // Initialize e1000 driver
    serial_write("network_init: Initializing e1000 driver\n");
    if (e1000_init() != 0) {
        serial_write("network_init: Failed to initialize e1000 driver\n");
        return -1;
    }

    // Initialize TCP/IP stack
    arp_init();
    tcp_init();

    serial_write("network_init: Network initialization complete\n");
    return 0;
}

// Receive a packet from the network
int network_receive_packet(char* buffer, size_t* length) {
    uint16_t packet_length;

    // Try to receive a packet
    if (e1000_receive_packet(buffer, &packet_length) != 0) {
        return -1; // No packet available
    }

    *length = packet_length;
    return 0;
}

// Send a packet to the network
int network_send_packet(const char* buffer, size_t length) {
    if (e1000_send_packet(buffer, (uint16_t)length) != 0) {
        return -1;
    }
    return 0;
}

// Minimal Ethernet header helpers
static inline uint16_t be16_to_cpu(uint16_t x) { return (uint16_t)((x >> 8) | (x << 8)); }

// Main network loop: poll NIC, dispatch packets, drive TCP timers
void network_loop(void) {
    serial_write("network_loop: Entering polling loop\n");
    static char buffer[NET_BUF_SIZE];
    size_t length = 0;
    uint32_t last_tcp_tick = 0;
    uint32_t packet_counter = 0;

    while (1) {
        int got_packet = (network_receive_packet(buffer, &length) == 0);

        if (got_packet && length >= ETH_HDR_LEN) {
            const eth_header_t* hdr = (const eth_header_t*)buffer;
            uint16_t et = be16_to_cpu(hdr->ethertype);
            const uint8_t* payload = (const uint8_t*)buffer + ETH_HDR_LEN;
            size_t payload_len = length - ETH_HDR_LEN;

            packet_counter++;
            if (packet_counter <= 10) {
                char dbg[64];
                snprintf(dbg, sizeof(dbg), "NET: rx packet %u ethertype=%04x len=%zu\n",
                         packet_counter, et, length);
                serial_write(dbg);
            }

            if (et == ETHERTYPE_ARP) {
                arp_receive(payload, payload_len);
            } else if (et == ETHERTYPE_IP) {
                ip_receive(payload, payload_len);
            }
        }

        // Drive TCP timers roughly every 100 ms (assuming ~1M iterations/sec).
        last_tcp_tick++;
        if (last_tcp_tick >= 100000) {
            tcp_periodic();
            last_tcp_tick = 0;
        }

        // Idle hint.  We currently rely on polling because interrupt delivery
        // (IDT/PIC/IOAPIC/e1000 ISR) is not yet wired up.  Once interrupts are
        // enabled, replace this with sti+hlt and wake on packet arrival.
        __asm__("pause");
    }
}
