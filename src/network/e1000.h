#ifndef E1000_H
#define E1000_H

#include <stdint.h>
#include "pci.h"

// e1000 PCI IDs
#define E1000_VENDOR_ID 0x8086
#define E1000_DEVICE_ID 0x100E

// e1000 Registers
#define E1000_CTRL     0x00000  // Device Control
#define E1000_STATUS   0x00008  // Device Status
#define E1000_EEPROM   0x00014  // EEPROM Read
#define E1000_CTRL_EXT 0x00018  // Extended Device Control
#define E1000_IMASK    0x000D0  // Interrupt Mask Set/Read
#define E1000_RCTRL    0x00100  // Receive Control
#define E1000_RXDESCLO 0x02800  // Receive Descriptor Base Low
#define E1000_RXDESCHI 0x02804  // Receive Descriptor Base High
#define E1000_RXDESCLEN 0x02808 // Receive Descriptor Length
#define E1000_RXDESCHEAD 0x02810 // Receive Descriptor Head
#define E1000_RXDESCTAIL 0x02818 // Receive Descriptor Tail
#define E1000_TCTRL    0x00400  // Transmit Control
#define E1000_TXDESCLO 0x03800  // Transmit Descriptor Base Low
#define E1000_TXDESCHI 0x03804  // Transmit Descriptor Base High
#define E1000_TXDESCLEN 0x03808 // Transmit Descriptor Length
#define E1000_TXDESCHEAD 0x03810 // Transmit Descriptor Head
#define E1000_TXDESCTAIL 0x03818 // Transmit Descriptor Tail

// CTRL Register Bits
#define E1000_CTRL_LRST    0x00000008  // Link Reset
#define E1000_CTRL_ASDE    0x00000020  // Auto-Speed Detection Enable
#define E1000_CTRL_SLU     0x00000040  // Set Link Up
#define E1000_CTRL_ILOS    0x00000080  // Invert Loss-of-Signal (LOS)
#define E1000_CTRL_RST     0x04000000  // Device Reset

// RCTL Register Bits
#define E1000_RCTL_EN      0x00000002  // Receiver Enable
#define E1000_RCTL_SBP     0x00000004  // Store Bad Packets
#define E1000_RCTL_UPE     0x00000008  // Unicast Promiscuous Enabled
#define E1000_RCTL_MPE     0x00000010  // Multicast Promiscuous Enabled
#define E1000_RCTL_LPE     0x00000020  // Long Packet Reception Enable
#define E1000_RCTL_LBM_NO  0x00000000  // No Loopback
#define E1000_RCTL_LBM_MAC 0x00000040  // MAC Loopback
#define E1000_RCTL_RDMTS_HALF 0x00000000  // Rx Desc Min Threshold Size
#define E1000_RCTL_RDMTS_QUAT 0x00000100  // Rx Desc Min Threshold Size
#define E1000_RCTL_RDMTS_EIGTH 0x00000200  // Rx Desc Min Threshold Size
#define E1000_RCTL_MO_36   0x00000000  // Multicast Offset
#define E1000_RCTL_MO_35   0x00000400  // Multicast Offset
#define E1000_RCTL_MO_34   0x00000800  // Multicast Offset
#define E1000_RCTL_MO_32   0x00000C00  // Multicast Offset
#define E1000_RCTL_BAM     0x00008000  // Broadcast Accept Mode
#define E1000_RCTL_VFE     0x00040000  // VLAN Filter Enable
#define E1000_RCTL_CFIEN   0x00080000  // Canonical Form Indicator Enable
#define E1000_RCTL_CFI     0x00100000  // Canonical Form Indicator Value
#define E1000_RCTL_DPF     0x00400000  // Discard Pause Frames
#define E1000_RCTL_PMCF    0x00800000  // Pass MAC Control Frames
#define E1000_RCTL_BSEX    0x02000000  // Buffer Size Extension
#define E1000_RCTL_SECRC   0x04000000  // Strip Ethernet CRC

// Receive Buffer Size
#define E1000_RCTL_BSIZE_2048  0x00000000  // 2048 bytes
#define E1000_RCTL_BSIZE_1024  0x00010000  // 1024 bytes
#define E1000_RCTL_BSIZE_512   0x00020000  // 512 bytes
#define E1000_RCTL_BSIZE_256   0x00030000  // 256 bytes

// TCTL Register Bits
#define E1000_TCTL_EN      0x00000002  // Transmit Enable
#define E1000_TCTL_PSP     0x00000008  // Pad Short Packets
#define E1000_TCTL_CT      0x00000FF0  // Collision Threshold
#define E1000_TCTL_COLD    0x003FF000  // Collision Distance
#define E1000_TCTL_SWXOFF  0x00400000  // Software XOFF Transmission
#define E1000_TCTL_PBE     0x00800000  // Packet Burst Enable
#define E1000_TCTL_RTLC    0x01000000  // Re-transmit on Late Collision
#define E1000_TCTL_NRTU    0x02000000  // No Re-transmit on Underrun

// Transmit Buffer Size
#define E1000_TCTL_COLD_FULL_DUPLEX 0x00040000  // Full Duplex Collision Distance

// Receive Descriptor Status Bits
#define E1000_RXD_STAT_DD    0x01  // Descriptor Done
#define E1000_RXD_STAT_EOP   0x02  // End of Packet

// Transmit Descriptor Command Bits
#define E1000_TXD_CMD_EOP    0x01  // End of Packet
#define E1000_TXD_CMD_IFCS   0x02  // Insert FCS
#define E1000_TXD_CMD_IC     0x04  // Insert Checksum
#define E1000_TXD_CMD_RS     0x08  // Report Status
#define E1000_TXD_CMD_RPS    0x10  // Report Packet Sent

// Transmit Descriptor Status Bits
#define E1000_TXD_STAT_DD    0x01  // Descriptor Done
#define E1000_TXD_STAT_EC    0x02  // Excess Collisions
#define E1000_TXD_STAT_LC    0x04  // Late Collision

// Receive Descriptor
typedef struct {
    uint64_t buffer_addr;  // Address of the descriptor's data buffer
    uint16_t length;       // Length of data DMAed into data buffer
    uint16_t checksum;     // Packet checksum
    uint8_t status;        // Descriptor status
    uint8_t errors;        // Descriptor Errors
    uint16_t special;      // Special field
} __attribute__((packed)) e1000_rx_desc_t;

// Transmit Descriptor
typedef struct {
    uint64_t buffer_addr;  // Address of the descriptor's data buffer
    uint16_t length;       // Length of data buffer
    uint8_t cso;           // Checksum offset
    uint8_t command;       // Descriptor control bits
    uint8_t status;        // Descriptor status
    uint8_t css;           // Checksum start
    uint16_t special;      // Special field
} __attribute__((packed)) e1000_tx_desc_t;

// Function declarations
int e1000_init(void);
int e1000_send_packet(const void* data, uint16_t length);
int e1000_receive_packet(void* buffer, uint16_t* length);

#endif // E1000_H