/*
 * c_model_interface.c - Minimal C implementation of model interface (no host deps)
 */

#include "c_model_interface.h"
#include "kernel.h"
#include "string.h"
#include <stddef.h>
#include <stdint.h>

// Placeholder embedded GGUF-like bytes
static const uint8_t embedded_model_bytes[] = {
    'G','G','U','F',
    0x03,0x00,0x00,0x00, // version
    0x00,0x00,0x00,0x00, // tensor_count
    0x00,0x00,0x00,0x00, // kv_count
    0x00,0x01,0x02,0x03, 0x04,0x05,0x06,0x07,
    0x08,0x09,0x0A,0x0B,
};

#define EMBEDDED_MODEL_CHECKSUM 0x12345678u

static uint32_t crc32_calc(const uint8_t* data, size_t size) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < size; i++) {
        crc ^= (uint32_t)data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1u) crc = (crc >> 1) ^ 0xEDB88320u; else crc >>= 1;
        }
    }
    return crc ^ 0xFFFFFFFFu;
}

const uint8_t* c_get_embedded_model(void) {
    return embedded_model_bytes;
}

size_t c_get_embedded_model_size(void) {
    return sizeof(embedded_model_bytes);
}

int c_validate_embedded_model(void) {
    const uint8_t* d = c_get_embedded_model();
    size_t n = c_get_embedded_model_size();
    if (n < 4) return 0;
    const uint8_t magic[4] = { 'G','G','U','F' };
    return memcmp(d, magic, 4) == 0 ? 1 : 0;
}

embedded_model_info_t c_load_embedded_model(void) {
    embedded_model_info_t info;
    info.data = c_get_embedded_model();
    info.size = c_get_embedded_model_size();
    info.is_valid = c_validate_embedded_model();
    return info;
}

int c_validate_gguf_format(const uint8_t* data, size_t size) {
    if (!data || size < 4) return 0;
    const uint8_t magic[4] = { 'G','G','U','F' };
    return memcmp(data, magic, 4) == 0 ? 1 : 0;
}

int c_validate_header(const uint8_t* data, size_t size) {
    if (!data || size < 16) return 0;
    if (!c_validate_gguf_format(data, size)) return 0;
    return 1;
}

model_metadata_t c_get_model_metadata(void) {
    model_metadata_t m;
    const uint8_t* d = c_get_embedded_model();
    m.magic[0] = d[0]; m.magic[1] = d[1]; m.magic[2] = d[2]; m.magic[3] = d[3];
    m.version = 3;
    m.tensor_count = 0;
    m.kv_count = 0;
    strncpy(m.architecture, "llama", sizeof(m.architecture));
    m.architecture[sizeof(m.architecture)-1] = '\0';
    m.context_length = 2048;
    return m;
}

uint32_t c_calculate_model_checksum(const uint8_t* data, size_t size) {
    if (!data || size == 0) return 0;
    return crc32_calc(data, size);
}

int c_verify_model_integrity(const uint8_t* data, size_t size) {
    if (!data || size == 0) return 0;
    uint32_t c = crc32_calc(data, size);
    return (c == EMBEDDED_MODEL_CHECKSUM) ? 1 : 0;
}

