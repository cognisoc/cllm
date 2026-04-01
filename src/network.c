#include "../include/network.h"
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
        return -1; // Failed to initialize network device
    }
    
    serial_write("network_init: Network initialization complete\n");
    return 0; // Success
}

// Receive a packet from the network
int network_receive_packet(char* buffer, size_t* length) {
    uint16_t packet_length;
    
    // Try to receive a packet
    if (e1000_receive_packet(buffer, &packet_length) != 0) {
        return -1; // No packet available
    }
    
    *length = packet_length;
    return 0; // Success
}

// Send a packet to the network
int network_send_packet(const char* buffer, size_t length) {
    // Send the packet
    if (e1000_send_packet(buffer, (uint16_t)length) != 0) {
        return -1; // Failed to send packet
    }
    
    return 0; // Success
}

// Minimal Ethernet header definition
typedef struct __attribute__((packed)) {
    uint8_t dst[6];
    uint8_t src[6];
    uint16_t ethertype; // big endian
} eth_hdr_t;

static inline uint16_t be16_to_cpu(uint16_t x) { return (uint16_t)((x >> 8) | (x << 8)); }
static inline uint16_t cpu_to_be16(uint16_t x) { return (uint16_t)((x >> 8) | (x << 8)); }

// Simple networking loop: poll NIC, log frames, echo custom ethertype 0x88B5
void network_loop(void) {
    serial_write("network_loop: Entering polling loop\n");
    char buffer[NETWORK_BUFFER_SIZE];
    size_t length = 0;

    while (1) {
        if (network_receive_packet(buffer, &length) == 0 && length >= sizeof(eth_hdr_t)) {
            eth_hdr_t* hdr = (eth_hdr_t*)buffer;
            uint16_t et = be16_to_cpu(hdr->ethertype);

            // Log basic info
            serial_write("network_loop: Received frame, ethertype=0x");
            char info[64];
            snprintf(info, sizeof(info), "%04x, len=%zu\n", (unsigned)et, length);
            serial_write(info);

            // If custom demo EtherType, echo back the same payload
            if (et == 0x88B5) {
                // Swap MAC addresses
                uint8_t tmp[6];
                for (int i = 0; i < 6; i++) tmp[i] = hdr->dst[i];
                for (int i = 0; i < 6; i++) hdr->dst[i] = hdr->src[i];
                for (int i = 0; i < 6; i++) hdr->src[i] = tmp[i];

                // Send it back
                if (network_send_packet(buffer, length) == 0) {
                    serial_write("network_loop: Echoed custom frame\n");
                } else {
                    serial_write("network_loop: Failed to send echo\n");
                }
            }
        }

        // Idle a bit: simple pause to reduce busy spin
        __asm__("hlt");
    }
}
