#ifndef PCI_H
#define PCI_H

#include <stdint.h>

// PCI Configuration Space Access
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

// PCI Configuration Space Registers
#define PCI_VENDOR_ID 0x00
#define PCI_DEVICE_ID 0x02
#define PCI_COMMAND 0x04
#define PCI_STATUS 0x06
#define PCI_CLASS_CODE 0x0B
#define PCI_SUBCLASS 0x0A
#define PCI_HEADER_TYPE 0x0E
#define PCI_BASE_ADDRESS_0 0x10
#define PCI_BASE_ADDRESS_1 0x14
#define PCI_BASE_ADDRESS_2 0x18
#define PCI_BASE_ADDRESS_3 0x1C
#define PCI_BASE_ADDRESS_4 0x20
#define PCI_BASE_ADDRESS_5 0x24
#define PCI_INTERRUPT_LINE 0x3C
#define PCI_INTERRUPT_PIN 0x3D

// PCI Device Structure
typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t header_type;
    uint32_t bar0;
    uint32_t bar1;
    uint32_t bar2;
    uint32_t bar3;
    uint32_t bar4;
    uint32_t bar5;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
} pci_device_t;

// Function declarations
uint32_t pci_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pci_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint8_t pci_read_byte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_write_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);
void pci_write_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t value);
void pci_write_byte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint8_t value);
void pci_enumerate_devices(void);
pci_device_t* pci_find_device(uint16_t vendor_id, uint16_t device_id);

#endif // PCI_H