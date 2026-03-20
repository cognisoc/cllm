#ifndef ERROR_H
#define ERROR_H

#include <stddef.h>

// Error codes
typedef enum {
    ERR_SUCCESS = 0,
    ERR_OUT_OF_MEMORY,
    ERR_INVALID_PARAMETER,
    ERR_NOT_IMPLEMENTED,
    ERR_IO_ERROR,
    ERR_NETWORK_ERROR,
    ERR_MODEL_LOAD_ERROR,
    ERR_INFERENCE_ERROR,
    ERR_CUDA_ERROR,
    ERR_UNKNOWN_ERROR
} error_code_t;

// Error information structure
typedef struct {
    error_code_t code;
    const char* message;
    const char* file;
    int line;
} error_info_t;

// Log levels
typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} log_level_t;

// Function declarations
void log_message(log_level_t level, const char* file, int line, const char* format, ...);
void log_error(error_code_t code, const char* file, int line, const char* format, ...);
void log_panic(const char* file, int line, const char* format, ...);

// Macro definitions for easier use
#define LOG_DEBUG(fmt, ...) log_message(LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) log_message(LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) log_message(LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) log_message(LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) log_message(LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_ERR(code, fmt, ...) log_error(code, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define PANIC(fmt, ...) log_panic(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

// Error handling macros
#define RETURN_ON_ERROR(expr) do { \
    error_code_t err = (expr); \
    if (err != ERR_SUCCESS) { \
        return err; \
    } \
} while(0)

#define RETURN_ERR_IF(condition, code, msg) do { \
    if (condition) { \
        LOG_ERR(code, msg); \
        return code; \
    } \
} while(0)

#endif // ERROR_H