#include "error.h"
#include "kernel.h"
#include "string.h"
#include <stdarg.h>

// Log level strings
static const char* log_level_strings[] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
};

// Error code strings
static const char* error_code_strings[] = {
    "SUCCESS",
    "OUT_OF_MEMORY",
    "INVALID_PARAMETER",
    "NOT_IMPLEMENTED",
    "IO_ERROR",
    "NETWORK_ERROR",
    "MODEL_LOAD_ERROR",
    "INFERENCE_ERROR",
    "CUDA_ERROR",
    "UNKNOWN_ERROR"
};

// Convert log level to string
static const char* log_level_to_string(log_level_t level) {
    if (level >= 0 && level <= LOG_FATAL) {
        return log_level_strings[level];
    }
    return "UNKNOWN";
}

// Convert error code to string
static const char* error_code_to_string(error_code_t code) {
    if (code >= 0 && code <= ERR_UNKNOWN_ERROR) {
        return error_code_strings[code];
    }
    return "UNKNOWN_ERROR";
}

// Simple vsnprintf implementation
static int simple_vsnprintf(char* str, size_t size, const char* format, va_list args) {
    if (size == 0) return 0;
    
    size_t i = 0;
    size_t format_i = 0;
    
    while (format[format_i] != '\0' && i < size - 1) {
        if (format[format_i] == '%' && format[format_i + 1] != '\0') {
            format_i++;
            if (format[format_i] == 's') {
                // String argument
                char* arg = va_arg(args, char*);
                size_t arg_len = strlen(arg);
                size_t copy_len = (i + arg_len < size - 1) ? arg_len : (size - 1 - i);
                strncpy(str + i, arg, copy_len);
                i += copy_len;
            } else if (format[format_i] == 'd' || format[format_i] == 'i') {
                // Integer argument
                int arg = va_arg(args, int);
                // Convert integer to string
                char int_str[12]; // Enough for 32-bit integer
                int pos = 0;
                int temp = arg;
                
                // Handle negative numbers
                if (arg < 0) {
                    if (i < size - 1) {
                        str[i++] = '-';
                    }
                    temp = -temp;
                }
                
                // Convert to string (reverse order)
                if (temp == 0) {
                    int_str[pos++] = '0';
                } else {
                    while (temp > 0 && pos < 11) {
                        int_str[pos++] = '0' + (temp % 10);
                        temp /= 10;
                    }
                }
                
                // Copy in correct order
                for (int j = pos - 1; j >= 0 && i < size - 1; j--) {
                    str[i++] = int_str[j];
                }
            } else if (format[format_i] == 'u') {
                // Unsigned integer argument
                unsigned int arg = va_arg(args, unsigned int);
                // Convert unsigned integer to string
                char int_str[12]; // Enough for 32-bit unsigned integer
                int pos = 0;
                unsigned int temp = arg;
                
                // Convert to string (reverse order)
                if (temp == 0) {
                    int_str[pos++] = '0';
                } else {
                    while (temp > 0 && pos < 11) {
                        int_str[pos++] = '0' + (temp % 10);
                        temp /= 10;
                    }
                }
                
                // Copy in correct order
                for (int j = pos - 1; j >= 0 && i < size - 1; j--) {
                    str[i++] = int_str[j];
                }
            } else if (format[format_i] == 'z' && format[format_i + 1] == 'u') {
                // size_t argument
                format_i++; // Skip 'z'
                size_t arg = va_arg(args, size_t);
                // Convert size_t to string
                char int_str[24]; // Enough for 64-bit size_t
                int pos = 0;
                size_t temp = arg;
                
                // Convert to string (reverse order)
                if (temp == 0) {
                    int_str[pos++] = '0';
                } else {
                    while (temp > 0 && pos < 23) {
                        int_str[pos++] = '0' + (temp % 10);
                        temp /= 10;
                    }
                }
                
                // Copy in correct order
                for (int j = pos - 1; j >= 0 && i < size - 1; j--) {
                    str[i++] = int_str[j];
                }
            } else {
                // Unknown format specifier, just copy it
                if (i < size - 1) {
                    str[i++] = '%';
                }
                if (i < size - 1) {
                    str[i++] = format[format_i];
                }
            }
            format_i++;
        } else {
            // Regular character
            str[i++] = format[format_i++];
        }
    }
    
    str[i] = '\0';
    return i;
}

// Log message function
void log_message(log_level_t level, const char* file, int line, const char* format, ...) {
    // For now, we'll just send everything to serial output
    // In a real implementation, we might want to filter by log level
    
    char buffer[512];
    size_t offset = 0;
    
    // Add timestamp (placeholder)
    const char* timestamp = "[00:00:00]";
    size_t ts_len = strlen(timestamp);
    if (offset + ts_len < sizeof(buffer) - 1) {
        strncpy(buffer + offset, timestamp, ts_len);
        offset += ts_len;
    }
    
    // Add log level
    const char* level_str = log_level_to_string(level);
    size_t level_len = strlen(level_str);
    if (offset + level_len + 3 < sizeof(buffer) - 1) {
        buffer[offset++] = ' ';
        buffer[offset++] = '[';
        strncpy(buffer + offset, level_str, level_len);
        offset += level_len;
        buffer[offset++] = ']';
    }
    
    // Add formatted message
    if (offset < sizeof(buffer) - 1) {
        buffer[offset++] = ' ';
        buffer[offset++] = '-';
        buffer[offset++] = ' ';
    }
    
    if (offset < sizeof(buffer) - 1) {
        va_list args;
        va_start(args, format);
        size_t remaining = sizeof(buffer) - 1 - offset;
        int written = simple_vsnprintf(buffer + offset, remaining, format, args);
        va_end(args);
        offset += written;
    }
    
    // Ensure null termination
    if (offset >= sizeof(buffer) - 1) {
        offset = sizeof(buffer) - 2;
    }
    buffer[offset] = '\n';
    buffer[offset + 1] = '\0';
    
    serial_write(buffer);
    
    // Also output to terminal for INFO and higher levels
    if (level >= LOG_INFO) {
        terminal_writestring(buffer);
    }
    
    // For fatal errors, halt the system
    if (level == LOG_FATAL) {
        serial_write("SYSTEM HALTED due to fatal error\n");
        while (1) {
            __asm__("hlt");
        }
    }
}

// Log error function
void log_error(error_code_t code, const char* file, int line, const char* format, ...) {
    char buffer[512];
    size_t offset = 0;
    
    // Add error code
    const char* code_str = error_code_to_string(code);
    size_t code_len = strlen(code_str);
    if (offset + code_len + 4 < sizeof(buffer) - 1) {
        buffer[offset++] = '[';
        strncpy(buffer + offset, code_str, code_len);
        offset += code_len;
        buffer[offset++] = ']';
        buffer[offset++] = ' ';
    }
    
    // Add formatted message
    if (offset < sizeof(buffer) - 1) {
        va_list args;
        va_start(args, format);
        size_t remaining = sizeof(buffer) - 1 - offset;
        int written = simple_vsnprintf(buffer + offset, remaining, format, args);
        va_end(args);
        offset += written;
    }
    
    // Ensure null termination
    if (offset >= sizeof(buffer) - 1) {
        offset = sizeof(buffer) - 2;
    }
    buffer[offset] = '\n';
    buffer[offset + 1] = '\0';
    
    // Log the error
    log_message(LOG_ERROR, file, line, "%s", buffer);
}

// Log panic function
void log_panic(const char* file, int line, const char* format, ...) {
    char buffer[512];
    va_list args;
    va_start(args, format);
    simple_vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Log fatal error and halt
    log_message(LOG_FATAL, file, line, "%s", buffer);
}