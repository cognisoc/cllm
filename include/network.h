/*
 * network.h - Simple network interface for our unikernel
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>
#include <stdint.h>

// Network buffer size
#define NETWORK_BUFFER_SIZE 2048

// HTTP request structure
typedef struct {
    char method[16];
    char path[128];
    char version[16];
    char headers[512];
    char body[1024];
    size_t body_length;
} http_request_t;

// HTTP response structure
typedef struct {
    int status_code;
    char headers[512];
    char body[2048];
    size_t body_length;
} http_response_t;

// Function declarations
int network_init(void);
int network_receive_packet(char* buffer, size_t* length);
int network_send_packet(const char* buffer, size_t length);
int http_parse_request(const char* raw_request, size_t length, http_request_t* request);
int http_generate_response(const http_response_t* response, char* buffer, size_t* length);
int route_request(const http_request_t* request, http_response_t* response);

#endif