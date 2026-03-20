#include "e1000.h"
#include "../../include/kernel.h"
#include "../../include/string.h"

// Global variables
static pci_device_t* e1000_device = NULL;
static uint8_t* e1000_base = NULL;

// Descriptor rings
#define RX_DESC_COUNT 32
#define TX_DESC_COUNT 32
#define RX_BUFFER_SIZE 2048
#define TX_BUFFER_SIZE 2048

static e1000_rx_desc_t rx_desc_ring[RX_DESC_COUNT] __attribute__((aligned(16)));
static e1000_tx_desc_t tx_desc_ring[TX_DESC_COUNT] __attribute__((aligned(16)));
static uint8_t rx_buffers[RX_DESC_COUNT][RX_BUFFER_SIZE] __attribute__((aligned(16)));
static uint8_t tx_buffers[TX_DESC_COUNT][TX_BUFFER_SIZE] __attribute__((aligned(16)));

static uint16_t rx_tail = 0;
static uint16_t tx_tail = 0;

// Read from e1000 register
static uint32_t e1000_read_reg(uint32_t reg) {
    return *((volatile uint32_t*)(e1000_base + reg));
}

// Write to e1000 register
static void e1000_write_reg(uint32_t reg, uint32_t value) {
    *((volatile uint32_t*)(e1000_base + reg)) = value;
}

// Initialize e1000 device
int e1000_init(void) {
    serial_write("e1000_init: Looking for e1000 device\n");
    
    // Find e1000 device
    e1000_device = pci_find_device(E1000_VENDOR_ID, E1000_DEVICE_ID);
    if (!e1000_device) {
        serial_write("e1000_init: e1000 device not found\n");
        return -1; // Device not found
    }
    
    serial_write("e1000_init: Found e1000 device\n");
    
    // Enable PCI bus mastering
    uint16_t command = pci_read_word(e1000_device->bus, e1000_device->slot, e1000_device->func, PCI_COMMAND);
    command |= 0x04; // Enable bus mastering
    pci_write_word(e1000_device->bus, e1000_device->slot, e1000_device->func, PCI_COMMAND, command);
    serial_write("e1000_init: Enabled PCI bus mastering\n");
    
    // Get base address
    uint32_t bar0 = pci_read_dword(e1000_device->bus, e1000_device->slot, e1000_device->func, PCI_BASE_ADDRESS_0);
    e1000_base = (uint8_t*)(bar0 & 0xFFFFFFF0); // Mask out flag bits
    
    serial_write("e1000_init: Device base address\n");
    
    // Software reset
    serial_write("e1000_init: Performing software reset\n");
    e1000_write_reg(E1000_CTRL, E1000_CTRL_RST);
    
    // Wait for reset to complete
    serial_write("e1000_init: Waiting for reset to complete\n");
    for (int i = 0; i < 1000; i++) {
        if (!(e1000_read_reg(E1000_CTRL) & E1000_CTRL_RST)) {
            break;
        }
    }
    
    serial_write("e1000_init: Initializing descriptor rings\n");
    
    // Initialize receive descriptors
    for (int i = 0; i < RX_DESC_COUNT; i++) {
        rx_desc_ring[i].buffer_addr = (uint64_t)(uintptr_t)rx_buffers[i];
        rx_desc_ring[i].length = 0;
        rx_desc_ring[i].checksum = 0;
        rx_desc_ring[i].status = 0;
        rx_desc_ring[i].errors = 0;
        rx_desc_ring[i].special = 0;
    }
    
    // Initialize transmit descriptors
    for (int i = 0; i < TX_DESC_COUNT; i++) {
        tx_desc_ring[i].buffer_addr = (uint64_t)(uintptr_t)tx_buffers[i];
        tx_desc_ring[i].length = 0;
        tx_desc_ring[i].cso = 0;
        tx_desc_ring[i].command = 0;
        tx_desc_ring[i].status = 0;
        tx_desc_ring[i].css = 0;
        tx_desc_ring[i].special = 0;
    }
    
    // Set up receive descriptor ring
    e1000_write_reg(E1000_RXDESCLO, (uint32_t)(uintptr_t)rx_desc_ring);
    e1000_write_reg(E1000_RXDESCHI, (uint32_t)((uint64_t)(uintptr_t)rx_desc_ring >> 32));
    e1000_write_reg(E1000_RXDESCLEN, sizeof(rx_desc_ring));
    e1000_write_reg(E1000_RXDESCHEAD, 0);
    e1000_write_reg(E1000_RXDESCTAIL, RX_DESC_COUNT - 1);
    rx_tail = RX_DESC_COUNT - 1;
    
    // Set up transmit descriptor ring
    e1000_write_reg(E1000_TXDESCLO, (uint32_t)(uintptr_t)tx_desc_ring);
    e1000_write_reg(E1000_TXDESCHI, (uint32_t)((uint64_t)(uintptr_t)tx_desc_ring >> 32));
    e1000_write_reg(E1000_TXDESCLEN, sizeof(tx_desc_ring));
    e1000_write_reg(E1000_TXDESCHEAD, 0);
    e1000_write_reg(E1000_TXDESCTAIL, 0);
    
    // Enable receiver and transmitter
    serial_write("e1000_init: Enabling receiver and transmitter\n");
    e1000_write_reg(E1000_RCTRL, 
        E1000_RCTL_EN | E1000_RCTL_SBP | E1000_RCTL_UPE | 
        E1000_RCTL_MPE | E1000_RCTL_LPE | E1000_RCTL_LBM_NO | 
        E1000_RCTL_RDMTS_HALF | E1000_RCTL_MO_36 | E1000_RCTL_BAM | 
        E1000_RCTL_BSEX | E1000_RCTL_SECRC | E1000_RCTL_BSIZE_2048);
    
    e1000_write_reg(E1000_TCTRL, 
        E1000_TCTL_EN | E1000_TCTL_PSP | E1000_TCTL_CT | 
        E1000_TCTL_COLD_FULL_DUPLEX | E1000_TCTL_SWXOFF | 
        E1000_TCTL_PBE | E1000_TCTL_RTLC | E1000_TCTL_NRTU);
    
    // Set link up
    serial_write("e1000_init: Setting link up\n");
    uint32_t ctrl = e1000_read_reg(E1000_CTRL);
    ctrl |= E1000_CTRL_SLU;
    e1000_write_reg(E1000_CTRL, ctrl);
    
    serial_write("e1000_init: Initialization complete\n");
    return 0; // Success
}

// Send a packet
int e1000_send_packet(const void* data, uint16_t length) {
    if (!e1000_base) {
        return -1; // Device not initialized
    }
    
    // Get current head
    uint16_t tx_head = e1000_read_reg(E1000_TXDESCHEAD);
    
    // Check if there's space in the ring
    uint16_t next_tail = (tx_tail + 1) % TX_DESC_COUNT;
    if (next_tail == tx_head) {
        return -1; // Ring full
    }
    
    // Copy data to buffer
    memcpy(tx_buffers[tx_tail], data, length);
    
    // Set up descriptor
    tx_desc_ring[tx_tail].length = length;
    tx_desc_ring[tx_tail].cso = 0;
    tx_desc_ring[tx_tail].command = E1000_TXD_CMD_EOP | E1000_TXD_CMD_IFCS | E1000_TXD_CMD_RS;
    tx_desc_ring[tx_tail].status = 0;
    tx_desc_ring[tx_tail].css = 0;
    tx_desc_ring[tx_tail].special = 0;
    
    // Update tail
    tx_tail = next_tail;
    e1000_write_reg(E1000_TXDESCTAIL, tx_tail);
    
    // Wait for transmission to complete
    for (int i = 0; i < 1000; i++) {
        if (tx_desc_ring[(tx_tail + TX_DESC_COUNT - 1) % TX_DESC_COUNT].status & E1000_TXD_STAT_DD) {
            return 0; // Success
        }
    }
    
    return -1; // Timeout
}

// Receive a packet
int e1000_receive_packet(void* buffer, uint16_t* length) {
    if (!e1000_base) {
        return -1; // Device not initialized
    }
    
    // Check if there's a packet available
    if (!(rx_desc_ring[rx_tail].status & E1000_RXD_STAT_DD)) {
        return -1; // No packet available
    }
    
    // Copy data from buffer
    *length = rx_desc_ring[rx_tail].length;
    memcpy(buffer, rx_buffers[rx_tail], *length);
    
    // Clear status
    rx_desc_ring[rx_tail].status = 0;
    
    // Update tail
    rx_tail = (rx_tail + 1) % RX_DESC_COUNT;
    e1000_write_reg(E1000_RXDESCTAIL, rx_tail);
    
    return 0; // Success
}