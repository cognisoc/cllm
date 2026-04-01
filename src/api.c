/*
 * api.c - Simple API handlers for our LLM server
 */

#include "network.h"
#include "c_model_interface.h"
#include "string.h"
#include "kernel.h"

// Handle health check endpoint
int handle_health_check(http_response_t* response) {
    serial_write("handle_health_check: Starting\r\n");
    response->status_code = 200;
    strncpy(response->headers, "Content-Type: application/json\r\n", sizeof(response->headers));
    strncpy(response->body, "{\"status\": \"ok\"}", sizeof(response->body));
    response->body_length = strlen(response->body);
    serial_write("handle_health_check: Completed\r\n");
    return 0;
}

// Handle model metadata endpoint
int handle_model_metadata(http_response_t* response) {
    response->status_code = 200;
    strncpy(response->headers, "Content-Type: application/json\r\n", sizeof(response->headers));

    // Build a small JSON with metadata from the embedded model
    model_metadata_t meta = c_get_model_metadata();
    char json[256];
    // architecture is a fixed-size char array in meta.architecture; ensure null-terminated copy
    char arch[33];
    size_t i = 0;
    for (; i < sizeof(arch) - 1 && i < sizeof(meta.architecture) && meta.architecture[i] != '\0'; ++i) {
        arch[i] = meta.architecture[i];
    }
    arch[i] = '\0';
    
    int n = snprintf(
        json,
        sizeof(json),
        "{\"magic\":\"%c%c%c%c\",\"version\":%u,\"tensor_count\":%u,\"kv_count\":%u,\"architecture\":\"%s\",\"context_length\":%u}",
        meta.magic[0], meta.magic[1], meta.magic[2], meta.magic[3],
        meta.version, meta.tensor_count, meta.kv_count, arch, meta.context_length
    );
    if (n < 0) {
        response->status_code = 500;
        strncpy(response->body, "{\"error\":\"failed to format metadata\"}", sizeof(response->body));
        response->body_length = strlen(response->body);
        return -1;
    }
    if ((size_t)n >= sizeof(response->body)) {
        // Truncate if needed
        n = (int)sizeof(response->body) - 1;
    }
    strncpy(response->body, json, (size_t)n);
    response->body[n] = '\0';
    response->body_length = (size_t)n;
    return 0;
}

// Handle model loading endpoint
int handle_load_model(http_response_t* response) {
    response->status_code = 200;
    strncpy(response->headers, "Content-Type: application/json\r\n", sizeof(response->headers));
    strncpy(response->body, "{\"status\": \"loading\", \"message\": \"Model loading not yet implemented\"}", sizeof(response->body));
    response->body_length = strlen(response->body);
    return 0;
}

// Handle text completion endpoint
int handle_completion(http_response_t* response) {
    response->status_code = 200;
    strncpy(response->headers, "Content-Type: application/json\r\n", sizeof(response->headers));
    strncpy(response->body, "{\"status\": \"generating\", \"message\": \"Text completion not yet implemented\"}", sizeof(response->body));
    response->body_length = strlen(response->body);
    return 0;
}

// Route HTTP requests to appropriate handlers
int route_request(http_request_t* request, http_response_t* response) {
    serial_write("route_request: Starting\r\n");
    
    // Default headers
    strncpy(response->headers, "Content-Type: application/json\r\n", sizeof(response->headers));
    
    // Route based on path
    if (strncmp(request->path, "/health", 8) == 0) {
        serial_write("route_request: /health\r\n");
        int rc = handle_health_check(response);
        response->body_length = strlen(response->body);
        return rc;
    } else if (strncmp(request->path, "/model/metadata", 15) == 0) {
        serial_write("route_request: /model/metadata\r\n");
        int rc = handle_model_metadata(response);
        response->body_length = strlen(response->body);
        return rc;
    } else if (strncmp(request->path, "/model/load", 12) == 0) {
        serial_write("route_request: /model/load\r\n");
        int rc = handle_load_model(response);
        response->body_length = strlen(response->body);
        return rc;
    } else if (strncmp(request->path, "/completion", 11) == 0) {
        serial_write("route_request: /completion\r\n");
        int rc = handle_completion(response);
        response->body_length = strlen(response->body);
        return rc;
    }
    
    // Not found
    response->status_code = 404;
    strncpy(response->body, "{\"error\":\"not found\"}", sizeof(response->body));
    response->body_length = strlen(response->body);
    serial_write("route_request: 404\r\n");
    return 0;
}
