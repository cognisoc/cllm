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