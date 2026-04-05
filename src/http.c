/*
 * http.c - Simple HTTP parser implementation
 */

#include "network.h"
#include "string.h"

#include <stdbool.h>

// Helper: case-insensitive string prefix comparison
static bool str_starts_with_ci(const char* str, const char* prefix) {
    while (*prefix) {
        char c1 = *str;
        char c2 = *prefix;
        // Convert to lowercase
        if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
        if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
        if (c1 != c2) return false;
        str++;
        prefix++;
    }
    return true;
}

// Parse a single header and extract known values
static void parse_header(const char* header, size_t len, http_request_t* request) {
    // Check for Content-Length
    if (str_starts_with_ci(header, "Content-Length:")) {
        const char* value = header + 15;
        while (*value == ' ') value++;
        request->content_length = 0;
        while (*value >= '0' && *value <= '9') {
            request->content_length = request->content_length * 10 + (*value - '0');
            value++;
        }
    }
    // Check for Content-Type
    else if (str_starts_with_ci(header, "Content-Type:")) {
        const char* value = header + 13;
        while (*value == ' ') value++;
        size_t i = 0;
        while (value[i] && value[i] != '\r' && value[i] != '\n' &&
               value[i] != ';' && i < sizeof(request->content_type) - 1) {
            request->content_type[i] = value[i];
            i++;
        }
        request->content_type[i] = '\0';
    }
}

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
    request->content_type[0] = '\0';
    request->content_length = 0;

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

    // Parse headers and find body
    size_t pos = line_end + 2; // Skip first \r\n
    size_t headers_start = pos;
    size_t body_start = 0;

    // Find end of headers (empty line: \r\n\r\n)
    while (pos + 1 < length) {
        if (raw_request[pos] == '\r' && raw_request[pos + 1] == '\n') {
            // Check if this is the end of headers (double CRLF)
            if (pos + 3 < length && raw_request[pos + 2] == '\r' && raw_request[pos + 3] == '\n') {
                body_start = pos + 4;
                break;
            }
            // Parse this header line
            size_t header_start = headers_start;
            while (header_start < pos) {
                size_t header_end = header_start;
                while (header_end < pos && !(raw_request[header_end] == '\r' && raw_request[header_end + 1] == '\n')) {
                    header_end++;
                }
                if (header_end > header_start) {
                    parse_header(raw_request + header_start, header_end - header_start, request);
                }
                header_start = header_end + 2; // Skip \r\n
            }
            headers_start = pos + 2;
        }
        pos++;
    }

    // If no double CRLF found, treat entire remainder as headers
    if (body_start == 0) {
        body_start = length;
    }

    // Copy headers
    size_t headers_length = body_start > headers_start + 4 ? body_start - headers_start - 4 : 0;
    if (headers_length > 0) {
        if (headers_length >= sizeof(request->headers)) {
            headers_length = sizeof(request->headers) - 1;
        }
        strncpy(request->headers, raw_request + line_end + 2, headers_length);
        request->headers[headers_length] = '\0';
    }

    // Parse any remaining headers
    const char* h = request->headers;
    while (*h) {
        const char* line_start = h;
        while (*h && !(*h == '\r' || *h == '\n')) h++;
        if (h > line_start) {
            parse_header(line_start, h - line_start, request);
        }
        while (*h == '\r' || *h == '\n') h++;
    }

    // Copy body if present
    if (body_start < length) {
        size_t body_len = length - body_start;

        // Use Content-Length if available, otherwise use remaining data
        if (request->content_length > 0 && request->content_length < body_len) {
            body_len = request->content_length;
        }

        if (body_len >= sizeof(request->body)) {
            body_len = sizeof(request->body) - 1;
        }

        strncpy(request->body, raw_request + body_start, body_len);
        request->body[body_len] = '\0';
        request->body_length = body_len;
    }

    return 0;
}

// Generate HTTP response
int http_generate_response(const http_response_t* response, char* buffer, size_t* length) {
    // Format status line
    char status_line[64];
    int status_len;

    switch (response->status_code) {
        case 200:
            status_len = snprintf(status_line, sizeof(status_line), "HTTP/1.1 200 OK\r\n");
            break;
        case 400:
            status_len = snprintf(status_line, sizeof(status_line), "HTTP/1.1 400 Bad Request\r\n");
            break;
        case 404:
            status_len = snprintf(status_line, sizeof(status_line), "HTTP/1.1 404 Not Found\r\n");
            break;
        case 405:
            status_len = snprintf(status_line, sizeof(status_line), "HTTP/1.1 405 Method Not Allowed\r\n");
            break;
        case 422:
            status_len = snprintf(status_line, sizeof(status_line), "HTTP/1.1 422 Unprocessable Entity\r\n");
            break;
        case 500:
            status_len = snprintf(status_line, sizeof(status_line), "HTTP/1.1 500 Internal Server Error\r\n");
            break;
        case 503:
            status_len = snprintf(status_line, sizeof(status_line), "HTTP/1.1 503 Service Unavailable\r\n");
            break;
        default:
            status_len = snprintf(status_line, sizeof(status_line), "HTTP/1.1 200 OK\r\n");
            break;
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