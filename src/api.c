/*
 * api.c - Simple API handlers for our LLM server
 */

#include "network.h"
#include "string.h"

// Handle health check endpoint
int handle_health_check(http_response_t* response) {
    response->status_code = 200;
    strncpy(response->headers, "Content-Type: application/json\r\n", sizeof(response->headers));
    strncpy(response->body, "{\"status\": \"ok\"}", sizeof(response->body));
    response->body_length = strlen(response->body);
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
int route_request(const http_request_t* request, http_response_t* response) {
    if (strncmp(request->path, "/health", 7) == 0) {
        return handle_health_check(response);
    } else if (strncmp(request->path, "/models/load", 12) == 0) {
        return handle_load_model(response);
    } else if (strncmp(request->path, "/completion", 11) == 0) {
        return handle_completion(response);
    } else {
        // Not found
        response->status_code = 404;
        strncpy(response->headers, "Content-Type: application/json\r\n", sizeof(response->headers));
        strncpy(response->body, "{\"error\": \"Endpoint not found\"}", sizeof(response->body));
        response->body_length = strlen(response->body);
        return 0;
    }
}
