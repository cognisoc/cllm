#include "pci.h"
#include "../../include/kernel.h"
#include "../../include/string.h"

// Global variables
static pci_device_t pci_devices[32];
static int pci_device_count = 0;

// Function to write to PCI configuration space
static void pci_config_write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t address = (1 << 31) | (bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC);
    __asm__ volatile("outl %0, %1" : : "a"(address), "Nd"(PCI_CONFIG_ADDRESS));
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(PCI_CONFIG_DATA));
}

// Function to read from PCI configuration space
static uint32_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (1 << 31) | (bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC);
    __asm__ volatile("outl %0, %1" : : "a"(address), "Nd"(PCI_CONFIG_ADDRESS));
    uint32_t tmp;
    __asm__ volatile("inl %1, %0" : "=a"(tmp) : "Nd"(PCI_CONFIG_DATA));
    return tmp;
}

// Read 32-bit value from PCI configuration space
uint32_t pci_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    return pci_config_read(bus, slot, func, offset);
}

// Read 16-bit value from PCI configuration space
uint16_t pci_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t value = pci_config_read(bus, slot, func, offset);
    return (uint16_t)((value >> ((offset & 2) * 8)) & 0xFFFF);
}

// Read 8-bit value from PCI configuration space
uint8_t pci_read_byte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t value = pci_config_read(bus, slot, func, offset);
    return (uint8_t)((value >> ((offset & 3) * 8)) & 0xFF);
}

// Write 32-bit value to PCI configuration space
void pci_write_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    pci_config_write(bus, slot, func, offset, value);
}

// Write 16-bit value to PCI configuration space
void pci_write_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t value) {
    uint32_t tmp = pci_config_read(bus, slot, func, offset);
    tmp &= ~(0xFFFF << ((offset & 2) * 8));
    tmp |= (value << ((offset & 2) * 8));
    pci_config_write(bus, slot, func, offset, tmp);
}

// Write 8-bit value to PCI configuration space
void pci_write_byte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint8_t value) {
    uint32_t tmp = pci_config_read(bus, slot, func, offset);
    tmp &= ~(0xFF << ((offset & 3) * 8));
    tmp |= (value << ((offset & 3) * 8));
    pci_config_write(bus, slot, func, offset, tmp);
}

// Enumerate all PCI devices
void pci_enumerate_devices(void) {
    pci_device_count = 0;
    serial_write("pci_enumerate_devices: Starting PCI enumeration\n");
    
    for (uint8_t bus = 0; bus < 255; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint16_t vendor_id = pci_read_word(bus, slot, func, PCI_VENDOR_ID);
                
                // If vendor ID is 0xFFFF, device doesn't exist
                if (vendor_id == 0xFFFF) {
                    if (func == 0) {
                        // No functions in this device, skip to next slot
                        break;
                    }
                    // Skip to next function
                    continue;
                }
                
                // Create new device entry
                if (pci_device_count < 32) {
                    pci_devices[pci_device_count].vendor_id = vendor_id;
                    pci_devices[pci_device_count].device_id = pci_read_word(bus, slot, func, PCI_DEVICE_ID);
                    pci_devices[pci_device_count].bus = bus;
                    pci_devices[pci_device_count].slot = slot;
                    pci_devices[pci_device_count].func = func;
                    pci_devices[pci_device_count].class_code = pci_read_byte(bus, slot, func, PCI_CLASS_CODE);
                    pci_devices[pci_device_count].subclass = pci_read_byte(bus, slot, func, PCI_SUBCLASS);
                    pci_devices[pci_device_count].header_type = pci_read_byte(bus, slot, func, PCI_HEADER_TYPE);
                    pci_devices[pci_device_count].bar0 = pci_read_dword(bus, slot, func, PCI_BASE_ADDRESS_0);
                    pci_devices[pci_device_count].bar1 = pci_read_dword(bus, slot, func, PCI_BASE_ADDRESS_1);
                    pci_devices[pci_device_count].bar2 = pci_read_dword(bus, slot, func, PCI_BASE_ADDRESS_2);
                    pci_devices[pci_device_count].bar3 = pci_read_dword(bus, slot, func, PCI_BASE_ADDRESS_3);
                    pci_devices[pci_device_count].bar4 = pci_read_dword(bus, slot, func, PCI_BASE_ADDRESS_4);
                    pci_devices[pci_device_count].bar5 = pci_read_dword(bus, slot, func, PCI_BASE_ADDRESS_5);
                    pci_devices[pci_device_count].interrupt_line = pci_read_byte(bus, slot, func, PCI_INTERRUPT_LINE);
                    pci_devices[pci_device_count].interrupt_pin = pci_read_byte(bus, slot, func, PCI_INTERRUPT_PIN);
                    
                    // Print device info for debugging
                    serial_write("Found PCI device\n");
                    
                    pci_device_count++;
                }
                
                // If this is function 0 and it's a single function device, skip to next slot
                if (func == 0 && (pci_read_byte(bus, slot, func, PCI_HEADER_TYPE) & 0x80) == 0) {
                    break;
                }
            }
        }
    }
    
    serial_write("pci_enumerate_devices: Found PCI devices\n");
    
    // Print all found devices
    for (int i = 0; i < pci_device_count; i++) {
        serial_write("Device found\n");
    }
}

// Find a specific PCI device by vendor and device ID
pci_device_t* pci_find_device(uint16_t vendor_id, uint16_t device_id) {
    for (int i = 0; i < pci_device_count; i++) {
        if (pci_devices[i].vendor_id == vendor_id && pci_devices[i].device_id == device_id) {
            return &pci_devices[i];
        }
    }
    return NULL;
}