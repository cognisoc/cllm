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

// Log message function
void log_message(log_level_t level, const char* file, int line, const char* format, ...) {
    // Format the log message
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
    if (level >= 0 && level <= LOG_FATAL) {
        const char* level_str = log_level_strings[level];
        size_t level_len = strlen(level_str);
        if (offset + level_len + 3 < sizeof(buffer) - 1) {
            buffer[offset++] = ' ';
            buffer[offset++] = '[';
            strncpy(buffer + offset, level_str, level_len);
            offset += level_len;
            buffer[offset++] = ']';
        }
    }
    
    // Add file and line information for error levels
    if (level >= LOG_ERROR) {
        size_t file_len = strlen(file);
        // Extract filename from path
        const char* filename = file;
        for (int i = file_len - 1; i >= 0; i--) {
            if (file[i] == '/') {
                filename = file + i + 1;
                break;
            }
        }
        
        size_t fn_len = strlen(filename);
        if (offset + fn_len + 16 < sizeof(buffer) - 1) {
            buffer[offset++] = ' ';
            buffer[offset++] = '[';
            strncpy(buffer + offset, filename, fn_len);
            offset += fn_len;
            
            // Add line number
            char line_str[16];
            int line_num = line;
            int pos = 0;
            
            // Convert line number to string (reverse order)
            if (line_num == 0) {
                line_str[pos++] = '0';
            } else {
                while (line_num > 0 && pos < 15) {
                    line_str[pos++] = '0' + (line_num % 10);
                    line_num /= 10;
                }
            }
            
            // Add colon
            line_str[pos++] = ':';
            
            // Copy in correct order
            for (int j = pos - 1; j >= 0 && offset < sizeof(buffer) - 1; j--) {
                buffer[offset++] = line_str[j];
            }
            buffer[offset++] = ']';
        }
    }
    
    // Add colon separator
    if (offset < sizeof(buffer) - 3) {
        buffer[offset++] = ':';
        buffer[offset++] = ' ';
    }
    
    // Add formatted message
    if (offset < sizeof(buffer) - 1) {
        va_list args;
        va_start(args, format);
        size_t remaining = sizeof(buffer) - 1 - offset;
        int written = vsnprintf(buffer + offset, remaining, format, args);
        va_end(args);
        offset += written;
    }
    
    // Ensure null termination
    if (offset >= sizeof(buffer) - 1) {
        offset = sizeof(buffer) - 2;
    }
    buffer[offset] = '\n';
    buffer[offset + 1] = '\0';
    
    // Send to serial output
    serial_write(buffer);
    
    // Also send to terminal for INFO and higher levels
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
    // Format the error message
    char buffer[512];
    size_t offset = 0;
    
    // Add error code
    if (code >= 0 && code <= ERR_UNKNOWN_ERROR) {
        const char* code_str = error_code_strings[code];
        size_t code_len = strlen(code_str);
        if (offset + code_len + 4 < sizeof(buffer) - 1) {
            buffer[offset++] = '[';
            strncpy(buffer + offset, code_str, code_len);
            offset += code_len;
            buffer[offset++] = ']';
            buffer[offset++] = ' ';
        }
    }
    
    // Add formatted message
    if (offset < sizeof(buffer) - 1) {
        va_list args;
        va_start(args, format);
        size_t remaining = sizeof(buffer) - 1 - offset;
        int written = vsnprintf(buffer + offset, remaining, format, args);
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
    // Format the panic message
    char buffer[512];
    
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Log fatal error and halt
    log_message(LOG_FATAL, file, line, "%s", buffer);
}