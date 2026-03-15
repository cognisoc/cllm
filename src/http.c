/*
 * http.c - Simple HTTP parser implementation
 */

#include "network.h"
#include "string.h"

#include <stdbool.h>

// Parse HTTP request line
static int parse_request_line(const char* line, http_request_t* request) {
    // Find first space (separates method from path)
    size_t method_end = 0;
    while (line[method_end] != ' ' && line[method_end] != '\0') {
        method_end++;
    }
    
    if (line[method_end] == '\0') {
        return -1; // Invalid request line
    }
    
    // Copy method
    if (method_end >= sizeof(request->method)) {
        return -1; // Method too long
    }
    strncpy(request->method, line, method_end);
    request->method[method_end] = '\0';
    
    // Find second space (separates path from version)
    size_t path_start = method_end + 1;
    size_t path_end = path_start;
    while (line[path_end] != ' ' && line[path_end] != '\0') {
        path_end++;
    }
    
    if (line[path_end] == '\0') {
        return -1; // Invalid request line
    }
    
    // Copy path
    size_t path_len = path_end - path_start;
    if (path_len >= sizeof(request->path)) {
        return -1; // Path too long
    }
    strncpy(request->path, line + path_start, path_len);
    request->path[path_len] = '\0';
    
    // Copy version
    size_t version_start = path_end + 1;
    size_t version_len = strlen(line + version_start);
    if (version_len >= sizeof(request->version)) {
        return -1; // Version too long
    }
    strncpy(request->version, line + version_start, version_len);
    request->version[version_len] = '\0';
    
    return 0;
}

// Parse HTTP request
int http_parse_request(const char* raw_request, size_t length, http_request_t* request) {
    // Initialize request structure
    request->method[0] = '\0';
    request->path[0] = '\0';
    request->version[0] = '\0';
    request->headers[0] = '\0';
    request->body[0] = '\0';
    request->body_length = 0;
    
    // Find end of request line (first \r\n)
    size_t line_end = 0;
    while (line_end + 1 < length) {
        if (raw_request[line_end] == '\r' && raw_request[line_end + 1] == '\n') {
            break;
        }
        line_end++;
    }
    
    if (line_end + 1 >= length) {
        return -1; // No request line found
    }
    
    // Parse request line
    char request_line[256];
    if (line_end >= sizeof(request_line)) {
        return -1; // Request line too long
    }
    strncpy(request_line, raw_request, line_end);
    request_line[line_end] = '\0';
    
    if (parse_request_line(request_line, request) != 0) {
        return -1; // Failed to parse request line
    }
    
    // For now, we'll just copy the rest as headers
    // In a full implementation, we would parse headers properly
    size_t headers_start = line_end + 2; // Skip \r\n
    size_t headers_length = length - headers_start;
    
    if (headers_length > 0) {
        if (headers_length >= sizeof(request->headers)) {
            headers_length = sizeof(request->headers) - 1;
        }
        strncpy(request->headers, raw_request + headers_start, headers_length);
        request->headers[headers_length] = '\0';
    }
    
    return 0;
}

// Generate HTTP response
int http_generate_response(const http_response_t* response, char* buffer, size_t* length) {
    // Format status line
    char status_line[64];
    int status_len;
    
    if (response->status_code == 200) {
        status_len = snprintf(status_line, sizeof(status_line), "HTTP/1.1 200 OK\r\n");
    } else if (response->status_code == 404) {
        status_len = snprintf(status_line, sizeof(status_line), "HTTP/1.1 404 Not Found\r\n");
    } else if (response->status_code == 500) {
        status_len = snprintf(status_line, sizeof(status_line), "HTTP/1.1 500 Internal Server Error\r\n");
    } else {
        status_len = snprintf(status_line, sizeof(status_line), "HTTP/1.1 200 OK\r\n");
    }
    
    // Copy status line to buffer
    if (status_len < 0 || status_len >= sizeof(status_line)) {
        return -1;
    }
    
    size_t offset = 0;
    strncpy(buffer + offset, status_line, status_len);
    offset += status_len;
    
    // Add headers
    size_t headers_len = strlen(response->headers);
    if (headers_len > 0) {
        if (offset + headers_len >= *length) {
            return -1; // Buffer too small
        }
        strncpy(buffer + offset, response->headers, headers_len);
        offset += headers_len;
    }
    
    // Add Content-Length header
    char content_length_header[64];
    int content_length_len = snprintf(content_length_header, sizeof(content_length_header), 
                                      "Content-Length: %zu\r\n", response->body_length);
    if (content_length_len < 0 || content_length_len >= sizeof(content_length_header)) {
        return -1;
    }
    
    if (offset + content_length_len >= *length) {
        return -1; // Buffer too small
    }
    strncpy(buffer + offset, content_length_header, content_length_len);
    offset += content_length_len;
    
    // Add extra CRLF to end headers
    if (offset + 2 >= *length) {
        return -1; // Buffer too small
    }
    buffer[offset] = '\r';
    buffer[offset + 1] = '\n';
    offset += 2;
    
    // Add body
    if (response->body_length > 0) {
        if (offset + response->body_length >= *length) {
            return -1; // Buffer too small
        }
        strncpy(buffer + offset, response->body, response->body_length);
        offset += response->body_length;
    }
    
    *length = offset;
    return 0;
}