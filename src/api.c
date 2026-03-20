/*
 * api.c - Simple API handlers for our LLM server
 */

#include "network.h"
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
    
    // Simplify the test - just return immediately
    response->status_code = 200;
    serial_write("route_request: Setting status code\r\n");
    
    // Test with a simple loop instead of strncpy
    serial_write("route_request: Setting headers with loop\r\n");
    const char* headers = "Content-Type: application/json\r\n";
    for (int i = 0; i < 512 && headers[i] != '\0'; i++) {
        response->headers[i] = headers[i];
    }
    response->headers[strlen(headers)] = '\0'; // Ensure null termination
    serial_write("route_request: Headers set with loop\r\n");
    
    serial_write("route_request: Setting body with loop\r\n");
    const char* body = "{\"status\": \"ok\"}";
    for (int i = 0; i < 2048 && body[i] != '\0'; i++) {
        response->body[i] = body[i];
    }
    response->body[strlen(body)] = '\0'; // Ensure null termination
    serial_write("route_request: Body set with loop\r\n");
    
    response->body_length = strlen(response->body);
    serial_write("route_request: Completed\r\n");
    return 0;
}
