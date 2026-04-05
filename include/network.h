/*
 * network.h - Simple network interface for our unikernel
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>
#include <stdint.h>

// Network buffer size
#define NETWORK_BUFFER_SIZE 2048

// HTTP buffer sizes for API payloads
#define HTTP_REQUEST_BODY_SIZE  8192   // 8KB for larger prompts
#define HTTP_RESPONSE_BODY_SIZE 16384  // 16KB for responses

// HTTP request structure
typedef struct {
    char method[16];
    char path[256];                         // Increased for query params
    char version[16];
    char headers[1024];                     // Increased for more headers
    char body[HTTP_REQUEST_BODY_SIZE];
    size_t body_length;
    char content_type[64];                  // Parsed Content-Type header
    size_t content_length;                  // Parsed Content-Length value
} http_request_t;

// HTTP response structure
typedef struct {
    int status_code;
    char headers[1024];
    char body[HTTP_RESPONSE_BODY_SIZE];
    size_t body_length;
} http_response_t;

// Network interface functions
int network_init(void);
int network_receive_packet(char* buffer, size_t* length);
int network_send_packet(const char* buffer, size_t length);
int http_parse_request(const char* raw_request, size_t length, http_request_t* request);
int http_generate_response(const http_response_t* response, char* buffer, size_t* length);
int route_request(http_request_t* request, http_response_t* response);

// Simple networking loop (polls NIC, logs frames, optional echo)
void network_loop(void);

#endif
